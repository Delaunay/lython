
#pragma once

#include "ast/nodes.h"
#include "ast/ops/treewalk.h"

namespace lython {


struct SSAVisitorTrait {
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


struct StaticSingleAssignment: public TreeWalk<StaticSingleAssignment, false, SSAVisitorTrait> {
    using Super = TreeWalk<StaticSingleAssignment, false, SSAVisitorTrait>;

    using StmtRet = Super::StmtRet;
    using ExprRet = Super::ExprRet;
    using ModRet  = Super::ModRet;
    using PatRet  = Super::PatRet;

    ExprNode* name(Name_t, int depth);

    StmtRet classdef(ClassDef_t* n, int depth);
    StmtRet returnstmt(Return_t* n, int depth);

    // Might need SSA change for unpacking
    //a = b
    //StmtRet assign(Assign_t* n, int depth);

    // a += b; => a1 = a + b;
    StmtRet augassign(AugAssign_t* n, int depth);
    
    // a: b = c
    StmtRet annassign(Assign_t* n, int depth);

    // fun1(a + b, arg2, arg3) => fun1 = fun; arg1 = a + b; arg21 = arg2; arg31 = arg3; fun1(arg1, arg21, arg31);
    ExprNode* call(Call_t* n, int depth);
    
    ExprNode* load(ExprNode* expr);
    ExprNode* new_store(ExprNode* original);
    ExprNode* maybe_new_assign(ExprNode* target, ExprNode* value, int depth);

    ExprNode* unaryop(UnaryOp_t* n, int depth);
    ExprNode* boolop(BoolOp_t* n, int depth);
    ExprNode* compare(Compare_t* n, int depth);
    ExprNode* binop(BinOp_t* n, int depth);
    ExprNode* attribute(Attribute_t* n, int depth);

    AnnAssign* new_assign(ExprNode* target, ExprNode* value);

    ExprNode* split(ExprNode* original, int depth) {
        if (Name* name = cast<Name>(original)) {
            return original;
        }
        return new_assign(original, exec(original, depth))->target;
    }

    struct Renamed{
        String original;
        String new_name;
    };

    int unique_count = 0;
    Array<Renamed> renamed;
};

}