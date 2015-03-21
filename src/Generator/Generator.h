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


// optimization
#include "llvm/Analysis/Passes.h"
//#include "llvm/ExecutionEngine/ExecutionEngine.h"
//#include "llvm/ExecutionEngine/MCJIT.h"
//#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"

namespace lython{

class Generator
{
    public:
        typedef std::unordered_map<std::string, llvm::Value*> NamedVariables;

        Generator():
            module(new llvm::Module("my cool jit", llvm::getGlobalContext())),
            builder(llvm::IRBuilder<>(llvm::getGlobalContext()))
        #if LLVM_JIT
            ,
            exec_engine(llvm::EngineBuilder(module).create())
            data_layout(new llvm::DataLayout()),
            fpm(llvm::FunctionPassManager(module))
        #endif
        {

        #if LLVM_JIT
            // Set up the optimizer pipeline.  Start with registering info about how the
            // target lays out data structures.
            fpm.add(data_layout);
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

        ~Generator()
        {
            delete module;
        }

        // basic llvm stuff
        llvm::Module*     module;
        llvm::IRBuilder<> builder;
        NamedVariables    variables;

     #if LLVM_JIT
        // optimizer stuff
        llvm::DataLayout*         data_layout;
        llvm::FunctionPassManager fpm;
        llvm::ExecutionEngine*    exec_engine; // Should it be freed ?

        // Just in Time Compiler
    #endif
};

}

#endif

#endif

