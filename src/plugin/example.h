#ifndef LYTHON_PLUGIN_EXAMPLE_HEADER
#define LYTHON_PLUGIN_EXAMPLE_HEADER

#include "plugin/plugin.h"


struct ExampleVisitor: public lython::VisitorPlugin {
public:
    using Super = lython::VisitorPlugin;

    using StmtRet = Super::StmtRet;
    using ExprRet = Super::ExprRet;
    using ModRet  = Super::ModRet;
    using PatRet  = Super::PatRet;

    ExampleVisitor() {}

    virtual ~ExampleVisitor() {}

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

#define FUNCTION_GEN(name, fun, ret) ret fun(name##_t* n, int depth) override;

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


#endif