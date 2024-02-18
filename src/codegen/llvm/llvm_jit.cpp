#if WITH_LLVM && WITH_LLVM_CODEGEN 

// Include
#include "codegen/llvm/llvm_jit.h"

// LLVM
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/Support/Error.h"

namespace lython {

LythonLLVMJIT::LythonLLVMJIT(Unique<orc::ExecutionSession> ES,
                             orc::JITTargetMachineBuilder  JTMB,
                             llvm::DataLayout              DL):
    exec_session(std::move(ES)),                    //
    datalayout(std::move(DL)),                      //
    mangle(*this->exec_session, this->datalayout),  //
    object_layer(*this->exec_session,
                 []() { return std::make_unique<llvm::SectionMemoryManager>(); }),
    compiler_layer(*this->exec_session,
                   object_layer,
                   std::make_unique<orc::ConcurrentIRCompiler>(JTMB)),  // std::move(JTMB)
    main_jd(this->exec_session->createBareJITDylib("<main>"))           //
{                                                                       //
    main_jd.addGenerator(                                               //
        llvm::cantFail(                                                 //
            orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(DL.getGlobalPrefix()))  //
    );
    if (JTMB.getTargetTriple().isOSBinFormatCOFF()) {
        object_layer.setOverrideObjectFlagsWithResponsibilityFlags(true);
        object_layer.setAutoClaimResponsibilityForObjectSymbols(true);
    }
}

LythonLLVMJIT::~LythonLLVMJIT() {
    if (auto Err = exec_session->endSession())
        exec_session->reportError(std::move(Err));
}

llvm::Expected<Unique<LythonLLVMJIT>> LythonLLVMJIT::create() {
    auto EPC = orc::SelfExecutorProcessControl::Create();
    if (!EPC) {
        return EPC.takeError();
    }
    auto ES = std::make_unique<orc::ExecutionSession>(std::move(*EPC));

    orc::JITTargetMachineBuilder JTMB(ES->getExecutorProcessControl().getTargetTriple());

    auto DL = JTMB.getDefaultDataLayoutForTarget();
    if (!DL) {
        return DL.takeError();
    }
    return std::make_unique<LythonLLVMJIT>(std::move(ES), std::move(JTMB), std::move(*DL));
}

const llvm::DataLayout& LythonLLVMJIT::get_datalayout() const { return datalayout; }

orc::JITDylib& LythonLLVMJIT::get_main_jit_dylib() { return main_jd; }

llvm::Error LythonLLVMJIT::add_module(orc::ThreadSafeModule TSM, orc::ResourceTrackerSP RT) {
    if (!RT)
        RT = main_jd.getDefaultResourceTracker();

    return compiler_layer.add(RT, std::move(TSM));
}

llvm::Expected<orc::ExecutorSymbolDef> LythonLLVMJIT::lookup(llvm::StringRef Name) {
    return exec_session->lookup({&main_jd}, mangle(Name.str()));
}

llvm::ExitOnError ExitOnErr;

LythonJITExec::LythonJITExec(Unique<llvm::Module> llmod) {
    llmodule = llmod.get();
    llmodule->setDataLayout(jit->get_datalayout());
    context = std::make_unique<llvm::LLVMContext>();

    auto resc = jit->get_main_jit_dylib().createResourceTracker();
    auto jit_module = orc::ThreadSafeModule(std::move(llmod), std::move(context));

    ExitOnErr(jit->add_module(std::move(jit_module), resc));
    // InitializeModuleAndPassManager();

    // Evaluate
    auto ExprSymbol = ExitOnErr(jit->lookup("__anon_expr"));

    double (*FP)() = (double (*)())(intptr_t)ExprSymbol.getAddress();
    fprintf(stderr, "Evaluated to %f\n", FP());

    ExitOnErr(resc->remove());
}

}  // namespace lython

#endif