#pragma once

#include "ast/nodes.h"
#include "ast/ops/treewalk.h"

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
struct Lowering: public TreeWalk<Lowering, false, LoweringVisitorTrait> {
    using Super = TreeWalk<Lowering, false, LoweringVisitorTrait>;

    using StmtRet = Super::StmtRet;
    using ExprRet = Super::ExprRet;
    using ModRet  = Super::ModRet;
    using PatRet  = Super::PatRet;

    StmtRet classdef(ClassDef_t* n, int depth);
};

}