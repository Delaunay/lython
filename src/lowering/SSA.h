
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


struct StaticSingleAssignment: public TreeWalk<Lowering, false, LoweringVisitorTrait> {
    using Super = TreeWalk<Lowering, false, LoweringVisitorTrait>;


    using StmtRet = Super::StmtRet;
    using ExprRet = Super::ExprRet;
    using ModRet  = Super::ModRet;
    using PatRet  = Super::PatRet;

    ExprNode* name(Name_t, int depth);

    StmtRet classdef(ClassDef_t* n, int depth);

    // Might need SSA change for unpacking
    //a = b
    //StmtRet assign(Assign_t* n, int depth);

    // a += b; => a1 = a + b;
    StmtRet augassign(Assign_t* n, int depth);
    
    // a: b = c
    //StmtRet annassign(Assign_t* n, int depth);

    // fun1(a + b, arg2, arg3) => fun1 = fun; arg1 = a + b; arg21 = arg2; arg31 = arg3; fun1(arg1, arg21, arg31);
    ExprNode* call(Call_t* n, int depth);
    
    ExprNode* load(ExprNode* expr);
    ExprNode* new_store(ExprNode* original);
    ExprNode* maybe_new_assign(Node* parent, ExprNode* target, ExprNode* value, int depth);

    ExprNode* new_assign(Node* parent, ExprNode* target, ExprNode* value);

    ExprNode* split(ExprNode* original, int depth) {
        if (Name* name = cast<Name>(expr)) {
            return original;
        }
        return new_assign(original->parent, exec(original, depth), original);
    }

    struct Renamed{
        String original;
        String new_name;
    };

    int unique_count = 0;
    Array<Renamed> renamed;
};

}