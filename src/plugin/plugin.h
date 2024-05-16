#ifndef LYTHON_PLUGIN_VISITOR_HEADER
#define LYTHON_PLUGIN_VISITOR_HEADER

#include "ast/visitor.h"

namespace lython {

struct VisitorPluginTrait {
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
struct VisitorPlugin: BaseVisitor<VisitorPlugin, false, VisitorPluginTrait> {
public:
    using Super = BaseVisitor<VisitorPlugin, false, VisitorPluginTrait>;

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
#define VM(name, fun)

    NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH, VM)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH
#undef VM
#undef TYPE_GEN

    virtual ~VisitorPlugin() {}

#define FUNCTION_GEN(name, fun, ret) virtual ret fun(name##_t* n, int depth) = 0;

#define X(name, _)
#define SSECTION(name)
#define MOD(name, fun)   FUNCTION_GEN(name, fun, ModRet)
#define EXPR(name, fun)  FUNCTION_GEN(name, fun, ExprRet)
#define STMT(name, fun)  FUNCTION_GEN(name, fun, StmtRet)
#define MATCH(name, fun) FUNCTION_GEN(name, fun, PatRet)
#define VM(name, fun)

    NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH, VM)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH
#undef VM

#undef FUNCTION_GEN
};


}  // namespace lythoncode

extern "C" {
    typedef void* VisitorPlugin_C;
    extern VisitorPlugin_C make_plugin();

    extern void free_plugin(VisitorPlugin_C* plugin);
}

#endif