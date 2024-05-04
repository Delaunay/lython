#ifndef LYTHON_CLANG_GEN_HEADER
#define LYTHON_CLANG_GEN_HEADER

#if WITH_CLANG_CODEGEN

#include "utilities/printing.h"
#include "ast/ops.h"
#include "ast/visitor.h"
#include "sema/bindings.h"
#include "sema/builtin.h"
#include "sema/errors.h"
#include "utilities/strings.h"

// Clang
#include <clang/Basic/SourceLocation.h>

namespace clang {
    class ASTContext;
    class TranslationUnitDecl;
    class CompoundStmt;
    class Stmt;
    class Expr;
    class DiagnosticsEngine;
    class FileManager;
    class SourceManager;
    class IdentifierTable;
    class SelectorTable;
    class IdentifierTable;
    class LangOptions;

    namespace Builtin {
        class Context;
    }
}


namespace lython {

struct ClangGenVisitorTrait {
    using StmtRet = clang::Stmt*;
    using ExprRet = clang::Expr*;
    using ModRet  = void;
    using PatRet  = void;
    using IsConst = std::false_type;
    using Trace   = std::true_type;

    enum
    { MaxRecursionDepth = LY_MAX_VISITOR_RECURSION_DEPTH };
};

/* Use Clang AST to generate C++ code
 *
 * What we can do:
 *  - Add support for C++ including Kiwi
 *  - Add support for Kiwi including C++
 * 
 */
struct ClangGen: BaseVisitor<ClangGen, false, ClangGenVisitorTrait> {
    using Super = BaseVisitor<ClangGen, false, ClangGenVisitorTrait>;

    using StmtRet = Super::StmtRet;
    using ExprRet = Super::ExprRet;
    using ModRet  = Super::ModRet;
    using PatRet  = Super::PatRet;

    bool should_own_client = true;
    bool user_files_are_volatile = false;
    clang::LangOptions*         lang_options        = nullptr;
    clang::ASTContext*          context             = nullptr;
    clang::TranslationUnitDecl* transunit           = nullptr;
    clang::CompoundStmt*        current_body        = nullptr;
    clang::DiagnosticsEngine*   diagnostic_engine   = nullptr;
    clang::FileManager*         filemanager         = nullptr;
    clang::Builtin::Context*    builtins            = nullptr;
    clang::SourceManager*       source_manager      = nullptr;
    clang::SelectorTable*       selector_table      = nullptr;
    clang::IdentifierTable*     identifiers         = nullptr;

    ClangGen();
    void dump();
    clang::SourceLocation location() const;

#define TYPE_GEN(rtype) using rtype##_t = Super::rtype##_t;

#define X(name, _)
#define SSECTION(name)
#define EXPR(name, fun)  TYPE_GEN(name)
#define STMT(name, fun)  TYPE_GEN(name)
#define MOD(name, fun)   TYPE_GEN(name)
#define MATCH(name, fun) TYPE_GEN(name)

    NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH
#undef TYPE_GEN

#define FUNCTION_GEN(name, fun, ret) virtual ret fun(name##_t* n, int depth);

#define X(name, _)
#define SSECTION(name)
#define MOD(name, fun)   FUNCTION_GEN(name, fun, ModRet)
#define EXPR(name, fun)  FUNCTION_GEN(name, fun, ExprRet)
#define STMT(name, fun)  FUNCTION_GEN(name, fun, StmtRet)
#define MATCH(name, fun) FUNCTION_GEN(name, fun, PatRet)

    NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef FUNCTION_GEN

    virtual ~ClangGen() {}
};

}  // namespace lython

#endif
#endif