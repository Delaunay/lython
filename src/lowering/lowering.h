#pragma once

#include "ast/nodes.h"
#include "ast/visitor.h"

namespace lython {


struct LoweringVisitorTrait {
    using StmtRet = StmtNode*;
    using ExprRet = ExprNode*;
    using ModRet  = ModNode*;
    using PatRet  = Pattern*;
    using IsConst = std::false_type;
    using Trace   = std::true_type;

    enum {
        MaxRecursionDepth = LY_MAX_VISITOR_RECURSION_DEPTH
    };
};


/*
 * Remove syntatic sugar from the ast and generate a canonical/simplifier representation
 *
 * Examples
 * --------
 *
 * Method to functions
 *
 * .. code-block::
 *
 *    class Name:
 *        def __add__(self, a):
 *            return
 *
 *    // data only
 *    class Name:
 *        pass
 *
 *    // function only
 *    def Name#002__add__(self, a):
 *        return
 *
 * .. code-block::
 *
 *    a = Name()
 *    a = Name#001__ctor__()      <= Unique name that cannot clash with
 *                                   Anything in scope
 *
 *  .. code-block::
 *
 *     a + 2
 *     Name#002__add__(a, 2)
 *
 */
struct Lowering: public BaseVisitor<Lowering, false, LoweringVisitorTrait> {
    using Super = BaseVisitor<Lowering, false, LoweringVisitorTrait>;

    using StmtRet = Super::StmtRet;
    using ExprRet = Super::ExprRet;
    using ModRet  = Super::ModRet;
    using PatRet  = Super::PatRet;

#define FUNCTION_GEN(name, fun, rtype)                                          \
    LY_INLINE rtype fun(name##_t* node, int depth);

#define X(name, _)
#define SSECTION(name)
#define EXPR(name, fun)  FUNCTION_GEN(name, fun, ExprRet)
#define STMT(name, fun)  FUNCTION_GEN(name, fun, StmtRet)
#define MOD(name, fun)   FUNCTION_GEN(name, fun, ModRet)
#define MATCH(name, fun) FUNCTION_GEN(name, fun, PatRet)
#define VM(name, fun)    FUNCTION_GEN(name, fun, StmtRet)

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

}