#include <iostream>
#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>
#include <llvm/Support/Host.h>

using namespace clang;

int main() {
  // Create a new AST context
  ASTContext* Context = new ASTContext(/*LangOpts=*/LangOptions());

  // Create a new translation unit
  TranslationUnitDecl* TUDecl = Context->getTranslationUnitDecl();

  // ... create AST nodes ...

  // Create an AST consumer to print the AST nodes
  class PrintASTConsumer : public ASTConsumer {
  public:
    virtual bool HandleTopLevelDecl(DeclGroupRef D) {
      for (auto const &decl : D) {
        decl->print(llvm::outs(), PrintingPolicy(getLangOpts()));
      }
      return true;
    }
  };

  // Create a compiler instance
  CompilerInstance CI;
  CI.createDiagnostics();

  // Create a file manager
  FileManager FM(llvm::vfs::getRealFileSystem());
  CI.setFileManager(&FM);

  // Create a source manager
  SourceManager SM(CI.getDiagnostics(), FM);
  CI.setSourceManager(&SM);

  // Create a preprocessor
  PreprocessorOptions PPOpts;
  CI.createPreprocessor(clang::TU_Prefix, nullptr, PPOpts, false);

  // Create a code generation action with the AST consumer
  CodeGenAction* Action = new CodeGenAction(new PrintASTConsumer());

  // Parse and generate the AST
  const char* SourceStr = "int main() { std::cout << \"Hello, world!\"; }";
  std::unique_ptr<llvm::MemoryBuffer> InputBuffer =
      llvm::MemoryBuffer::getMemBuffer(SourceStr);
  CI.getPreprocessor().EnterMainSourceFile();
  Action->BeginSourceFile(CI, CI.getFrontendOpts().Inputs[0]);
  ParseAST(CI.getPreprocessor(), Action->getCompilerInstance().getASTConsumer(),
           *Context);
  Action->EndSourceFile();

  return 0;
}
