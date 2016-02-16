#ifndef LYTHON_MAGIC_GEN_HEADER
#define LYTHON_MAGIC_GEN_HEADER

/*
 *  don't ask
 */

#define LLVM_MCJIT_HELPER 1
#define LLVM_MCJIT 1

#if LLVM_MCJIT && LLVM_CODEGEN

// optimization
#include "llvm/Analysis/Passes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"
#include <cctype>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

using namespace llvm;

//===----------------------------------------------------------------------===//
// Quick and dirty hack
//===----------------------------------------------------------------------===//

// FIXME: Obviously we can do better than this
std::string GenerateUniqueName(const char *root);
std::string MakeLegalFunctionName(std::string Name);

//===----------------------------------------------------------------------===//
// MCJIT helper class
//===----------------------------------------------------------------------===//

class MCJITHelper
{
    public:
        MCJITHelper(LLVMContext &C) : Context(C), OpenModule(NULL) {}
        ~MCJITHelper();

      Function *getFunction(const std::string FnName);
      Module *getModuleForNewFunction();
      void *getPointerToFunction(Function *F);
      void *getSymbolAddress(const std::string &Name);
      void dump();

    private:
      typedef std::vector<Module *> ModuleVector;
      typedef std::vector<ExecutionEngine *> EngineVector;

      LLVMContext &Context;
      Module *OpenModule;
      ModuleVector Modules;
      EngineVector Engines;
};

#define LLVM_MM_HELPER 0
#if LLVM_MM_HELPER
class HelpingMemoryManager : public SectionMemoryManager
{
    HelpingMemoryManager(const HelpingMemoryManager &) = delete;

    void operator=(const HelpingMemoryManager &) = delete;

public:

    HelpingMemoryManager(MCJITHelper *Helper):
        MasterHelper(Helper)
    {}

    virtual ~HelpingMemoryManager()
    {}

    /// This method returns the address of the specified symbol.
    /// Our implementation will attempt to find symbols in other
    /// modules associated with the MCJITHelper to cross link symbols
    /// from one generated module to another.
    ///
    virtual uint64_t getSymbolAddress(const std::string &Name);

private:
    MCJITHelper *MasterHelper;
};
#endif

////===----------------------------------------------------------------------===//
//// Code Generation
////===----------------------------------------------------------------------===//

//static MCJITHelper *JITHelper;

#endif

#endif
