#include "Magic.h"

#if LLVM_MCJIT && LLVM_CODEGEN

#include "../colors.h"

Value *ErrorV(const char *str)
{
  printf(MRED "Error" MRESET " %s \n", str);

  return 0;
}

Value *ErrorF(const char *str)
{
  printf(MRED "Error" MRESET " %s \n", str);

  return 0;
}

//===----------------------------------------------------------------------===//
// Quick and dirty hack
//===----------------------------------------------------------------------===//

// FIXME: Obviously we can do better than this
std::string GenerateUniqueName(const char *root)
{
    static int i = 0;
    char s[16];
    sprintf(s, "%s%d", root, i++);
    std::string S = s;

    return S;
}

std::string MakeLegalFunctionName(std::string Name)
{
    std::string NewName;
    if (!Name.length())
        return GenerateUniqueName("anon_func_");

    // Start with what we have
    NewName = Name;

    // Look for a numberic first character
    if (NewName.find_first_of("0123456789") == 0)
    {
        NewName.insert(0, 1, 'n');
    }

    // Replace illegal characters with their ASCII equivalent
    std::string legal_elements =
      "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    size_t pos;

    while ((pos = NewName.find_first_not_of(legal_elements)) !=
         std::string::npos)
    {
        char old_c = NewName.at(pos);
        char new_str[16];
        sprintf(new_str, "%d", (int)old_c);
        NewName = NewName.replace(pos, 1, new_str);
    }

    return NewName;
}
//===----------------------------------------------------------------------===//
// MCJIT helper class
//===----------------------------------------------------------------------===//
#if LLVM_MCJIT_HELPER

MCJITHelper::~MCJITHelper() {
  if (OpenModule)
    delete OpenModule;
  EngineVector::iterator begin = Engines.begin();
  EngineVector::iterator end = Engines.end();
  EngineVector::iterator it;
  for (it = begin; it != end; ++it)
    delete *it;
}

Function *MCJITHelper::getFunction(const std::string FnName) {
  ModuleVector::iterator begin = Modules.begin();
  ModuleVector::iterator end = Modules.end();
  ModuleVector::iterator it;
  for (it = begin; it != end; ++it) {
    Function *F = (*it)->getFunction(FnName);
    if (F) {
      if (*it == OpenModule)
        return F;

      assert(OpenModule != NULL);

      // This function is in a module that has already been JITed.
      // We need to generate a new prototype for external linkage.
      Function *PF = OpenModule->getFunction(FnName);
      if (PF && !PF->empty()) {
        ErrorF("redefinition of function across modules");
        return 0;
      }

      // If we don't have a prototype yet, create one.
      if (!PF)
        PF = Function::Create(F->getFunctionType(), Function::ExternalLinkage,
                              FnName, OpenModule);
      return PF;
    }
  }
  return NULL;
}

Module *MCJITHelper::getModuleForNewFunction() {
  // If we have a Module that hasn't been JITed, use that.
  if (OpenModule)
    return OpenModule;

  // Otherwise create a new Module.
  std::string ModName = GenerateUniqueName("mcjit_module_");
  Module *M = new Module(ModName, Context);
  Modules.push_back(M);
  OpenModule = M;
  return M;
}

void *MCJITHelper::getPointerToFunction(Function *F) {
  // See if an existing instance of MCJIT has this function.
  EngineVector::iterator begin = Engines.begin();
  EngineVector::iterator end = Engines.end();
  EngineVector::iterator it;
  for (it = begin; it != end; ++it) {
    void *P = (*it)->getPointerToFunction(F);
    if (P)
      return P;
  }

  // If we didn't find the function, see if we can generate it.
  if (OpenModule) {
    std::string ErrStr;
    ExecutionEngine *NewEngine =
        EngineBuilder(std::unique_ptr<Module>(OpenModule).get())
            .setErrorStr(&ErrStr)
            .setMCJITMemoryManager(std::unique_ptr<HelpingMemoryManager>(
                new HelpingMemoryManager(this)).get())
            .create();
    if (!NewEngine) {
      fprintf(stderr, "Could not create ExecutionEngine: %s\n", ErrStr.c_str());
      exit(1);
    }

    // Create a function pass manager for this engine
    auto *FPM = new legacy::FunctionPassManager(OpenModule);

    // Set up the optimizer pipeline.  Start with registering info about how the
    // target lays out data structures.
    OpenModule->setDataLayout(NewEngine->getDataLayout());
    // Provide basic AliasAnalysis support for GVN.
    FPM->add(createBasicAliasAnalysisPass());
    // Promote allocas to registers.
    FPM->add(createPromoteMemoryToRegisterPass());
    // Do simple "peephole" optimizations and bit-twiddling optzns.
    FPM->add(createInstructionCombiningPass());
    // Reassociate expressions.
    FPM->add(createReassociatePass());
    // Eliminate Common SubExpressions.
    FPM->add(createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    FPM->add(createCFGSimplificationPass());
    FPM->doInitialization();

    // For each function in the module
    Module::iterator it;
    Module::iterator end = OpenModule->end();
    for (it = OpenModule->begin(); it != end; ++it) {
      // Run the FPM on this function
      FPM->run(*it);
    }

    // We don't need this anymore
    delete FPM;

    OpenModule = NULL;
    Engines.push_back(NewEngine);
    NewEngine->finalizeObject();
    return NewEngine->getPointerToFunction(F);
  }
  return NULL;
}

void *MCJITHelper::getSymbolAddress(const std::string &Name) {
  // Look for the symbol in each of our execution engines.
  EngineVector::iterator begin = Engines.begin();
  EngineVector::iterator end = Engines.end();
  EngineVector::iterator it;

  for (it = begin; it != end; ++it) {
    uint64_t FAddr = (*it)->getFunctionAddress(Name);
    if (FAddr) {
      return (void *)FAddr;
    }
  }
  return NULL;
}

void MCJITHelper::dump() {
  ModuleVector::iterator begin = Modules.begin();
  ModuleVector::iterator end = Modules.end();
  ModuleVector::iterator it;
  for (it = begin; it != end; ++it)
    (*it)->dump();
}
#endif
//===----------------------------------------------------------------------===//
// helper class
//===----------------------------------------------------------------------===//

#if LLVM_MM_HELPER
uint64_t HelpingMemoryManager::getSymbolAddress(const std::string &Name)
{
    uint64_t FnAddr = SectionMemoryManager::getSymbolAddress(Name);
    if (FnAddr)
        return FnAddr;

    uint64_t HelperFun = (uint64_t)MasterHelper->getSymbolAddress(Name);
    if (!HelperFun)
        report_fatal_error("Program used extern function '" + Name +
                           "' which could not be resolved!");

    return HelperFun;
}
#endif

#endif
