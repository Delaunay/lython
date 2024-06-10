
#include "SSA.h"

#include "utilities/magic.h"

namespace lython {

using StmtRet = Lowering::StmtRet;
using ExprRet = Lowering::ExprRet;
using ModRet  = Lowering::ModRet;
using PatRet  = Lowering::PatRet;


ExprNode* StaticSingleAssignment::load(ExprNode* expr) {
    if (Name* name = cast<Name>(expr)) {
        expr->ctx = ExprContext::Load;
    }
    return expr;
}

ExprNode* StaticSingleAssignment::new_store(ExprNode* original) 
{    
    String name = "var";
    if (Name* name = cast<Name>(expr)) {
        name = str(expr->id);
    }

    Name* new_name = original->new_object<Name>();
    new_name->ctx = ExprContext::Store;
    StringStream ss;
    ss << name << "_#" << unique_count;

    new_name->id = StringRef(ss.str());

    unique_count += 1;
    return new_name;
};

ExprNode* StaticSingleAssignment::maybe_new_assign(Node* parent, ExprNode* target, ExprNode* value, int depth) {
    switch (value->kind) {
        case NodeKind::Name: return value;
        case NodeKind::Constant: return value;
        default: break;
    }
    ExprNode* new_val = exec(value, depth);
    return new_assign(parent, target, new_val)
}

ExprNode* StaticSingleAssignment::new_assign(Node* parent, ExprNode* target, ExprNode* value) {
    AugAssign* resolved = parent->new_object<AugAssign>();
    (*body_stack.rbegin()).push_back(new_obj);

    // create a new unique name, maybe using target as hint
    resolved->target = new_store(target);
    resolved_fun->value = value;

    return resolved->target;
}

StmtRet StaticSingleAssignment::classdef(ClassDef_t* n, int depth) {
    for(int i = 0; i < n->body.size(); i++) {
        auto* stmt = n->body[i];

        if (FunctionDef* def = cast<FunctionDef>(stmt)) {

        }
    }
    return n;
}

StmtRet annassign(Assign_t* n, int depth);


StmtRet StaticSingleAssignment::augassign(AugAssign_t* n, int depth) {
    Node* parent = n->parent;

    // SSA the value expression
    ExprNode* value = exec(n->value, depth);

    BinOp* binop = parent->new_object<BinOp>();
    binop->op = n->op;
    binop->left = load(n->target); // Load target
    binop->right = value;

    // Store target
    return new_assign(parent, target, binop, depth);;
}


ExprNode* StaticSingleAssignment::call(Call_t* n, int depth) {
    Node* parent = n->parent;

    ExprNode* func = maybe_new_assign(parent, n->func, n->func, depth);

    Call* new_call = parent->new_object<Call>();
    new_call->func = func;
    new_call->args.reserve(n->args.size());

    for(ExprNode* arg: n->args) {
        ExprNode* new_arg = maybe_new_assign(parent, arg, arg, depth);
        new_call->args.push_back(new_arg);
    }

    for(Keyword& arg: n->keywords) {
        ExprNode* new_arg = maybe_new_assign(parent, arg.value, arg.value, depth);
        new_call->keywords.push_back({
            arg.key, 
            resolved_fun->target
        });
    }

    for(ExprNode* arg: n->varargs) {
        ExprNode* new_arg = maybe_new_assign(parent, arg, arg, depth);
        new_call->varargs.push_back(new_arg);
    }

    return new_call;
}

}