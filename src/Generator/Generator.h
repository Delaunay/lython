#ifndef LYTHON_GENERATOR_GEN_HEADER
#define LYTHON_GENERATOR_GEN_HEADER

#include "../config.h"

#if LLVM_CODEGEN

#include <unordered_map>

//#include "../Parser/Parser.h"
//#include "../AbstractSyntaxTree/Expression.h"

#include "llvm/IR/Verifier.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#if LLVM_JIT

// optimization
#include "llvm/Analysis/Passes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"

#include "llvm/IR/LegacyPassManager.h"

#endif


namespace lython{

class Operators;
class ObjectManager;

class Generator
{
    public:
        //Chapter 7 introduce mutable Variable
        typedef std::unordered_map<std::string, llvm::AllocaInst*> NamedVariables;

        // before chapter 7
        //typedef std::unordered_map<std::string, llvm::Value*> NamedVariables;

        Generator(Operators& ops, ObjectManager& gc):
            module(new llvm::Module("lython", llvm::getGlobalContext())),
            builder(llvm::IRBuilder<>(llvm::getGlobalContext())),
            operators(ops), gc(gc)
        #if LLVM_JIT
            ,
            exec_engine(llvm::EngineBuilder(module).create()),
            data_layout(new llvm::DataLayout(*exec_engine->getDataLayout())),
            fpm(llvm::legacy::FunctionPassManager(module))
        #endif
        {
        #if LLVM_JIT
            // Set up the optimizer pipeline.  Start with registering info about how the
            // target lays out data structures.
            fpm.add( (llvm::Pass*) (data_layout));
            // Provide basic AliasAnalysis support for GVN.
            fpm.add(llvm::createBasicAliasAnalysisPass());
            // Do simple "peephole" optimizations and bit-twiddling optzns.
            fpm.add(llvm::createInstructionCombiningPass());
            // Reassociate expressions.
            fpm.add(llvm::createReassociatePass());
            // Eliminate Common SubExpressions.
            fpm.add(llvm::createGVNPass());
            // Simplify the control flow graph (deleting unreachable blocks, etc).
            fpm.add(llvm::createCFGSimplificationPass());

            fpm.doInitialization();
        #endif
        }

        static llvm::AllocaInst* create_entry_block_alloca(llvm::Function* f,
                                                           const std::string& var)
        {
            llvm::IRBuilder<> TmpB(&f->getEntryBlock(),
                                    f->getEntryBlock().begin());

             return TmpB.CreateAlloca(
                         llvm::Type::getDoubleTy(
                             llvm::getGlobalContext()), 0,
                                      var.c_str());
        }

        ~Generator()
        {
            delete module;

        #if LLVM_JIT
            delete data_layout;
        #endif
        }

        //llvm::Value* operator[] (const std::string&)


        // Registred operators
        Operators&        operators;

        // Compiler 'Garbage collector'
        ObjectManager&    gc;

        // basic llvm stuff
        llvm::Module*     module;
        llvm::IRBuilder<> builder;
        NamedVariables    variables;

     #if LLVM_JIT
        // optimizer stuff
        llvm::DataLayout*         data_layout;
        llvm::legacy::FunctionPassManager fpm;
        llvm::ExecutionEngine*    exec_engine; // Should it be freed ?

        // Just in Time Compiler
    #endif
};
}

#endif
#endif

