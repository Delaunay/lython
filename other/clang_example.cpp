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

  // Create a new main function declaration
  FunctionDecl* MainDecl = FunctionDecl::Create(
      *Context,
      TUDecl,
      SourceLocation(),
      SourceLocation(),
      DeclarationName(&Context->Idents.get("main")),
      Context->getFunctionType(
          Context->IntTy,
          llvm::ArrayRef<QualType>(),
          FunctionProtoType::ExtProtoInfo()),
      nullptr,
      SC_None);

  // Set the main function as the current declaration context
  Context->setTranslationUnitDecl(MainDecl);

  // Create a new compound statement for the main function body
  CompoundStmt* MainBody = new (*Context) CompoundStmt(
      *Context, llvm::None, SourceLocation(), SourceLocation());

  // Create a new string literal expression for the "Hello, world!" string
  StringLiteral* HelloStr = StringLiteral::Create(
      *Context, "Hello, world!", StringLiteral::Ascii, false,
      Context->getStringLiteralArrayType(*Context->CharTy, 14),
      SourceLocation());

  // Create a new output stream expression for the "std::cout" object
  DeclRefExpr* CoutRef = new (*Context) DeclRefExpr(
      *Context, Context->getTranslationUnitDecl()->lookup(
                     &Context->Idents.get("std")->getMemberNameInfo()
                         .getName()
                         .getAsString())
                     .front(),
      false, Context->getFunctionType(Context->VoidTy, llvm::None,
                                       FunctionProtoType::ExtProtoInfo()),
      VK_LValue, SourceLocation());

  // Create a new call expression for the "operator<<" function of std::cout
  CallExpr* CoutCall = new (*Context) CallExpr(
      *Context, new (*Context) CXXMemberCallExpr(
                     *Context, CoutRef, llvm::ArrayRef<Expr*>(),
                     Context->getFunctionType(Context->VoidTy,
                                              llvm::ArrayRef<QualType>(),
                                              FunctionProtoType::ExtProtoInfo()),
                     VK_LValue, SourceLocation(), /*FP=*/nullptr),
      llvm::ArrayRef<Expr*>(HelloStr), Context->IntTy, VK_RValue,
      SourceLocation());

  // Create a new return statement for the main function
  ReturnStmt* MainReturn = new (*Context) ReturnStmt(
      SourceLocation(), CoutCall, nullptr);

  // Add the expressions and statements to the main function body
  MainBody->addStmt(CoutCall);
  MainBody->addStmt(MainReturn);

  // Set the main function body
  MainDecl->setBody(MainBody);

  // Print the generated AST
  MainDecl->print(llvm::outs(), PrintingPolicy(*Context->getLangOpts()));

  return 0;
}
