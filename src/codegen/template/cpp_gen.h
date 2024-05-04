#ifndef LYTHON_CPP_GEN_HEADER
#define LYTHON_CPP_GEN_HEADER

#include "ast/ops.h"
#include "ast/visitor.h"
#include "sema/bindings.h"
#include "sema/builtin.h"
#include "sema/errors.h"
#include "utilities/printing.h"
#include "utilities/strings.h"

namespace lython {

struct CPPGenVisitorTrait {
    using StmtRet = void;
    using ExprRet = void;
    using ModRet  = void;
    using PatRet  = void;
    using IsConst = std::false_type;
    using Trace   = std::true_type;

    enum
    { MaxRecursionDepth = LY_MAX_VISITOR_RECURSION_DEPTH };
};

/*
 */
struct CPPGen: BaseVisitor<CPPGen, false, CPPGenVisitorTrait> {
    using Super = BaseVisitor<CPPGen, false, CPPGenVisitorTrait>;

    using StmtRet = Super::StmtRet;
    using ExprRet = Super::ExprRet;
    using ModRet  = Super::ModRet;
    using PatRet  = Super::PatRet;

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

    Bindings              bindings;
    bool                  forwardpass = false;
    Array<StmtNode*>      nested;
    Array<String>         namespaces;
    Dict<StringRef, bool> flags;

    public:
    virtual ~CPPGen() {}

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
};

}  // namespace lython

#endif