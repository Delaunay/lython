
#include "SSA.h"

#include "utilities/magic.h"

namespace lython {

using StmtRet = StaticSingleAssignment::StmtRet;
using ExprRet = StaticSingleAssignment::ExprRet;
using ModRet  = StaticSingleAssignment::ModRet;
using PatRet  = StaticSingleAssignment::PatRet;


ExprNode* StaticSingleAssignment::load(ExprNode* expr) {
    if (Name* nm = cast<Name>(expr)) {
        nm->ctx = ExprContext::Load;
    }
    return expr;
}

ExprNode* StaticSingleAssignment::new_store(ExprNode* original) 
{    
    String name = "var";
    if (Name* nm = cast<Name>(original)) {
        name = str(nm->id);
    }

    Name* new_name = new_object<Name>();
    new_name->ctx = ExprContext::Store;
    StringStream ss;
    ss << name << "_#" << unique_count;

    new_name->id = StringRef(ss.str());

    unique_count += 1;
    return new_name;
};

ExprNode* StaticSingleAssignment::maybe_new_assign(ExprNode* target, ExprNode* value, int depth) {
    switch (value->kind) {
        case NodeKind::Name: return value;
        case NodeKind::Constant: return value;
        default: break;
    }
    ExprNode* new_val = exec(value, depth);
    return new_assign(target, new_val)->target;
}

AnnAssign* StaticSingleAssignment::new_assign(ExprNode* target, ExprNode* value) {
    AnnAssign* resolved = new_object<AnnAssign>();
    body_append(resolved);

    // create a new unique name, maybe using target as hint
    resolved->target = new_store(target);
    resolved->value = value;

    return resolved;
}

StmtRet StaticSingleAssignment::classdef(ClassDef_t* n, int depth) {
    for(int i = 0; i < n->body.size(); i++) {
        auto* stmt = n->body[i];

        if (FunctionDef* def = cast<FunctionDef>(stmt)) {

        }
    }
    return n;
}

StmtRet StaticSingleAssignment::annassign(Assign_t* n, int depth) {
    return n;
}


StmtRet StaticSingleAssignment::augassign(AugAssign_t* n, int depth) {
    // SSA the value expression
    ExprNode* value = exec(n->value, depth);

    BinOp* binop = new_object<BinOp>();
    binop->op = n->op;
    binop->left = load(n->target); // Load target
    binop->right = value;

    // Store target
    new_assign(n->target, binop);
    return nullptr;
}


ExprNode* StaticSingleAssignment::call(Call_t* n, int depth) {
    ExprNode* func = maybe_new_assign(n->func, n->func, depth);

    Call* new_call = Super::new_object<Call>();
    new_call->func = func;
    new_call->args.reserve(n->args.size());

    for(ExprNode* arg: n->args) {
        ExprNode* new_arg = maybe_new_assign(arg, arg, depth);
        new_call->args.push_back(new_arg);
    }

    for(Keyword& arg: n->keywords) {
        ExprNode* new_arg = maybe_new_assign(arg.value, arg.value, depth);
        lython::Keyword kw;
        kw.arg = arg.arg;
        kw.value = new_arg;
        new_call->keywords.push_back(kw);
    }

    for(ExprNode* arg: n->varargs) {
        ExprNode* new_arg = maybe_new_assign(arg, arg, depth);
        new_call->varargs.push_back(new_arg);
    }

    return new_call;
}

}