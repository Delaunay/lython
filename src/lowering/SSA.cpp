
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
    String name = "LY";
    if (Name* nm = cast<Name>(original)) {
        name = str(nm->id);
    }

    Name* new_name = new_object<Name>();
    new_name->ctx = ExprContext::Store;
    StringStream ss;
    ss << name << "_" << unique_count;

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
    ExprNode* value = exec(n->value, depth);
    Array<ExprNode*> elements = {
        value
    };

    if (TupleExpr* tuple = cast<TupleExpr>(value)) {
        elements = tuple->elts;
    }
    else if (ListExpr* tuple = cast<ListExpr>(value)) {
        elements = tuple->elts;
    } 

    if (elements.size() != n->targets.size()) {
        //
        return nullptr;
    }

    //
    for(int i = 0; i < elements.size(); i++) {
        // would this be working
        // raw unpacking
        // new_assign(n->targets[i], elements[i]);

        // target = getitem(tuple, i)
        Call* getitem = new_object<Call>();
        Name* nm = new_object<Name>();
        getitem->func = nm;
        getitem->args.push_back(value);
        Constant* cst = new_object<Constant>();
        cst->value = i;
        getitem->args.push_back(cst); 

        new_assign(n->targets[i], getitem);
    }
    
    return nullptr;
}


StmtRet StaticSingleAssignment::augassign(AugAssign_t* n, int depth) {
    // SSA the value expression
    ExprNode* value = exec(n->value, depth);

    BinOp* binop = new_object<BinOp>();
    binop->op = n->op;
    binop->left = load(n->target); // Load target
    binop->right = value;

    // Store target
    return new_assign(n->target, binop);
}

ExprNode* StaticSingleAssignment::unaryop(UnaryOp_t* n, int depth) {
    ExprNode* operand = maybe_new_assign(n->operand, n->operand, depth);

    UnaryOp* unary = Super::new_object<UnaryOp>();
    unary->operand = operand;
    unary->op = n->op;
    return unary;
}

ExprNode* StaticSingleAssignment::boolop(BoolOp_t* n, int depth) {
    ExprNode* left = maybe_new_assign(n->values[0], n->values[0], depth);

    Name* name = Super::new_object<Name>();
    name->id = "__builtin_" + str(n->op);

    for(int i = 1; i < n->values.size(); i++) {
        ExprNode* right = maybe_new_assign(n->values[i], n->values[i], depth);

        #if 1
        Call* binary = Super::new_object<Call>();
        
        binary->func = name;
        binary->args.push_back(left);
        binary->args.push_back(right);
        #else
        BinOp* binary = Super::new_object<BinOp>();
        binary->left = left;
        binary->right = right;
        //
        //binary->op = op;
        #endif

        left = maybe_new_assign(binary, binary, depth);
    }
    
    return left;
}
ExprNode* StaticSingleAssignment::compare(Compare_t* n, int depth) {
    ExprNode* left = maybe_new_assign(n->left, n->left, depth);

    for(int i = 0; i < n->ops.size(); i++) {
        CmpOperator op = n->ops[i];
        ExprNode* right = maybe_new_assign(n->comparators[i], n->comparators[i], depth);

        #if 1
        Call* binary = Super::new_object<Call>();
        Name* name = Super::new_object<Name>();
        name->id = "__builtin_" + str(op);
        binary->func = name;
        binary->args.push_back(left);
        binary->args.push_back(right);
        #else
        BinOp* binary = Super::new_object<BinOp>();
        binary->left = left;
        binary->right = right;
        //
        //binary->op = op;
        #endif

        left = maybe_new_assign(binary, binary, depth);
    }

    return left;
}
ExprNode* StaticSingleAssignment::binop(BinOp_t* n, int depth) {
    ExprNode* left = maybe_new_assign(n->left, n->left, depth);
    ExprNode* right = maybe_new_assign(n->right, n->right, depth);

    BinOp* binary = Super::new_object<BinOp>();
    binary->left = left;
    binary->right = right;
    binary->op = n->op;
    return binary;
}

StmtRet StaticSingleAssignment::returnstmt(Return_t* n, int depth) {
    Return* cl = Super::new_object<Return>();
    if (n->value.has_value()) {
        cl->value = maybe_new_assign(
            n->value.value(), 
            n->value.value(), 
            depth
        );
    }
    return cl;
}

// binop to function call ?
// a and b ==> _builtin_and(a, b)

// a.c(1, 2, 3)
//
// fun = getattr(a, "c")
// fun(a, 1, 2, 3)
ExprNode* StaticSingleAssignment::attribute(Attribute_t* n, int depth) {
    Call* cl = new_object<Call>();
    Name* name = new_object<Name>();
    name->id = "getattr";
    cl->func = name;

    cl->args.push_back(n->value);
    Constant* cst = new_object<Constant>();
    cst->value = make_value<String>(n->attr);
    cl->args.push_back(cst);
    return cl;
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