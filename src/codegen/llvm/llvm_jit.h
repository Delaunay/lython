
#ifndef LYTHON_LLVM_JIT_HEADER
#define LYTHON_LLVM_JIT_HEADER
#if WITH_LLVM && WITH_LLVM_CODEGEN 

// Lython
#include "dtypes.h"

// LLVM
#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/ExecutorProcessControl.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/Error.h"

// STL
#include <memory>

// // Compat
namespace llvm {
namespace orc {
using ExecutorSymbolDef = JITEvaluatedSymbol;
}
}  // namespace llvm

namespace lython {

namespace orc = llvm::orc;

class LythonLLVMJIT {
    private:
    Unique<orc::ExecutionSession> exec_session;

    llvm::DataLayout              datalayout;
    orc::MangleAndInterner        mangle;
    orc::RTDyldObjectLinkingLayer object_layer;
    orc::IRCompileLayer           compiler_layer;
    orc::JITDylib&                main_jd;

    public:
    LythonLLVMJIT(Unique<orc::ExecutionSession> exec_session,
                  orc::JITTargetMachineBuilder  target,
                  llvm::DataLayout              datalayout);

    ~LythonLLVMJIT();

    static llvm::Expected<Unique<LythonLLVMJIT>> create();

    const llvm::DataLayout& get_datalayout() const;

    orc::JITDylib& get_main_jit_dylib();

    llvm::Error add_module(orc::ThreadSafeModule TSM, orc::ResourceTrackerSP RT = nullptr);

    llvm::Expected<orc::ExecutorSymbolDef> lookup(llvm::StringRef Name);
};

class LythonJITExec {
    public:
    LythonJITExec(Unique<llvm::Module> llmod);

    //
    Unique<llvm::LLVMContext> context;
    llvm::Module*             llmodule;
    Unique<LythonLLVMJIT>     jit;
};

}  // namespace lython

#endif
#endif