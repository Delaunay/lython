
// Include
#include "codegen/clang/clang_gen.h"

#if WITH_CLANG_CODEGEN

// Kiwi
#include "utilities/printing.h"
#include "builtin/operators.h"
#include "utilities/guard.h"
#include "utilities/strings.h"


// Clang
#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/Builtins.h>

#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>

#include <llvm/Support/Host.h>



namespace lython {

using StmtRet = ClangGen::StmtRet;
using ExprRet = ClangGen::ExprRet;
using ModRet  = ClangGen::ModRet;
using PatRet  = ClangGen::PatRet;

using namespace clang;

struct Stuff {
    IntrusiveRefCntPtr<DiagnosticIDs>         Diags;
    IntrusiveRefCntPtr<DiagnosticOptions>     DiagOpts;
    DiagnosticConsumer*                       client            = nullptr;
    bool                                      ShouldOwnClient   = true;
    FileSystemOptions                         FileSystemOpts;
    IntrusiveRefCntPtr<llvm::vfs::FileSystem> FileSystem        = nullptr;
};


ClangGen::ClangGen() {

    diagnostic_engine = new DiagnosticsEngine(
        nullptr,   
        nullptr,   
        nullptr,    
        should_own_client        
    );

    filemanager = new FileManager(FileSystemOptions(), nullptr);
    
    lang_options = new LangOptions();
    source_manager = new SourceManager(
        *diagnostic_engine, 
        *filemanager,       
        user_files_are_volatile 
    );
    identifiers = new IdentifierTable();
    selector_table = new SelectorTable();
    builtins = new Builtin::Context();

    // Create a new AST context
    context = new ASTContext(
        *lang_options,
        *source_manager,
        *identifiers,
        *selector_table, 
        *builtins
    );

    clang::TargetInfo target_info;
    context->InitBuiltinTypes(target_info, nullptr);

    log_trace = true;
}

void ClangGen::dump() {
    transunit->print(llvm::outs(), PrintingPolicy(context->getLangOpts()));
}


SourceLocation ClangGen::location() const {
    SourceManager& SM = context->getSourceManager();

    // Get the file ID for the file containing the source location
    FileID fileID = SM.getMainFileID();

    // Set the offset to 42 (for example)
    std::size_t offset = 42;

    // Combine the file ID and offset into a single 32-bit integer
    std::size_t encodedLoc = SM.getLocForStartOfFile(fileID).getRawEncoding() + offset;

    // Create a new SourceLocation from the encoded integer
    return SourceLocation::getFromRawEncoding(encodedLoc);
}



// Expression
// ----------
ExprRet ClangGen::call(Call_t* n, int depth) 
{ 
    ExprRet fun = exec(n->func, depth);

    Array<clang::Expr*> args;
    for(auto* arg: n->args) {
        args.push_back(exec(arg, depth));
    }

    // () -> int
    QualType function_type = context->getFunctionType(
          context->IntTy,
          llvm::ArrayRef<QualType>(),
          FunctionProtoType::ExtProtoInfo()
    );

    clang::CallExpr* ncall = CallExpr::Create(
        *context,
        fun,
        args,
        function_type, 
        VK_RValue,
        SourceLocation(), // RParenLoc,
        FPOptionsOverride()
        // unsigned MinNumArgs = 0,
        // ADLCallKind UsesADL = NotADL
    );

    return ncall; 
}
ExprRet ClangGen::boolop(BoolOp_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::namedexpr(NamedExpr_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::compare(Compare_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::binop(BinOp_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::unaryop(UnaryOp_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::lambda(Lambda_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::ifexp(IfExp_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::dictexpr(DictExpr_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::setexpr(SetExpr_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::listcomp(ListComp_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::generateexpr(GeneratorExp_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::setcomp(SetComp_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::dictcomp(DictComp_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::await(Await_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::yield(Yield_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::yieldfrom(YieldFrom_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::joinedstr(JoinedStr_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::formattedvalue(FormattedValue_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::constant(Constant_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::attribute(Attribute_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::subscript(Subscript_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::starred(Starred_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::name(Name_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::listexpr(ListExpr_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::tupleexpr(TupleExpr_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::slice(Slice_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::dicttype(DictType_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::arraytype(ArrayType_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::arrow(Arrow_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::builtintype(BuiltinType_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::tupletype(TupleType_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::settype(SetType_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::classtype(ClassType_t* n, int depth) { return ExprRet(); }
ExprRet ClangGen::comment(Comment_t* n, int depth) { return ExprRet(); }


StmtRet ClangGen::functiondef(FunctionDef_t* n, int depth) 
{ 
    // TODO: Nested function need to generate lambda

    // () -> int
    QualType function_type = context->getFunctionType(
          context->IntTy,
          llvm::ArrayRef<QualType>(),
          FunctionProtoType::ExtProtoInfo()
    );

    // Top Level Function Statement
    FunctionDecl* fundef = FunctionDecl::Create(
        *context,
        transunit,
        SourceLocation(),
        SourceLocation(),
        DeclarationName(&context->Idents.get(String(n->name).c_str())),
        function_type, 
        nullptr,
        SC_None
    );
    transunit->addDecl(fundef);

    // 
    Array<Stmt*> stmts;
    stmts.reserve(n->body.size());

    for(StmtNode* stmt: n->body) {
        // Add Statements to the body
        StmtRet newstmt = exec(stmt, depth);
        stmts.push_back(newstmt);
    }

    CompoundStmt* body = CompoundStmt::Create(*context, stmts, SourceLocation(), SourceLocation());
    fundef->setBody(body);

    return StmtRet(); 
}

StmtRet ClangGen::returnstmt(Return_t* n, int depth) {

    if (n->value.has_value()) {
        ExprRet expr = exec(n->value.value(), depth);

        return ReturnStmt::Create(*
            context, 
            SourceLocation(), 
            expr,
            nullptr // NRVO
        );
    }

    return ReturnStmt::CreateEmpty(*context, false);
}

StmtRet ClangGen::classdef(ClassDef_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::invalidstmt(InvalidStatement_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::deletestmt(Delete_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::assign(Assign_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::augassign(AugAssign_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::annassign(AnnAssign_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::forstmt(For_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::whilestmt(While_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::ifstmt(If_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::with(With_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::raise(Raise_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::trystmt(Try_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::assertstmt(Assert_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::global(Global_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::nonlocal(Nonlocal_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::exprstmt(Expr_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::pass(Pass_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::breakstmt(Break_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::continuestmt(Continue_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::match(Match_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::inlinestmt(Inline_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::import(Import_t* n, int depth) { return StmtRet(); }
StmtRet ClangGen::importfrom(ImportFrom_t* n, int depth) { return StmtRet(); }

PatRet ClangGen::matchvalue(MatchValue_t* n, int depth) { return PatRet(); }
PatRet ClangGen::matchsingleton(MatchSingleton_t* n, int depth) { return PatRet(); }
PatRet ClangGen::matchsequence(MatchSequence_t* n, int depth) { return PatRet(); }
PatRet ClangGen::matchmapping(MatchMapping_t* n, int depth) { return PatRet(); }
PatRet ClangGen::matchclass(MatchClass_t* n, int depth) { return PatRet(); }
PatRet ClangGen::matchstar(MatchStar_t* n, int depth) { return PatRet(); }
PatRet ClangGen::matchas(MatchAs_t* n, int depth) { return PatRet(); }
PatRet ClangGen::matchor(MatchOr_t* n, int depth) { return PatRet(); }

ModRet ClangGen::module(Module_t* stmt, int depth) { 
    // Create a new translation unit
    transunit = context->getTranslationUnitDecl(); 

    for (StmtNode* stmt: stmt->body) {

        StmtRet Decl = exec(stmt, depth);
        
        // transunit->addDecl(Decl);
    }

    return ModRet(); 
};
ModRet ClangGen::interactive(Interactive_t* n, int depth) { return ModRet(); }
ModRet ClangGen::functiontype(FunctionType_t* n, int depth) { return ModRet(); }
ModRet ClangGen::expression(Expression_t* n, int depth) { return ModRet(); }

}  // namespace lython

#endif