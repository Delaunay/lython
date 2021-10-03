#include "sema/sema.h"

namespace lython {

ExprType *SemanticAnalyser::boolop(BoolOp *n, int depth) {}
ExprType *SemanticAnalyser::namedexpr(NamedExpr *n, int depth) {}
ExprType *SemanticAnalyser::binop(BinOp *n, int depth) {}
ExprType *SemanticAnalyser::unaryop(UnaryOp *n, int depth) {}
ExprType *SemanticAnalyser::lambda(Lambda *n, int depth) {
    n->args;
    exec(n->body, depth);
    return nullptr;
}
ExprType *SemanticAnalyser::ifexp(IfExp *n, int depth) {
    exec(n->test, depth);
    exec(n->body, depth);
    exec(n->orelse, depth);
    return nullptr;
}
ExprType *SemanticAnalyser::dictexpr(DictExpr *n, int depth) {
    auto type = n->new_object<DictType>();

    return type;
}
ExprType *SemanticAnalyser::setexpr(SetExpr *n, int depth) {
    auto type = n->new_object<SetType>();

    return type;
}
ExprType *SemanticAnalyser::listcomp(ListComp *n, int depth) {
    auto type = n->new_object<ArrayType>();

    return type;
}
ExprType *SemanticAnalyser::generateexpr(GeneratorExp *n, int depth) {
    auto type = n->new_object<ArrayType>();

    return type;
}
ExprType *SemanticAnalyser::setcomp(SetComp *n, int depth) {
    auto type = n->new_object<SetType>();

    return type;
}
ExprType *SemanticAnalyser::dictcomp(DictComp *n, int depth) {
    auto type = n->new_object<DictType>();

    return type;
}
ExprType *SemanticAnalyser::await(Await *n, int depth) { return exec(n->value, depth); }
ExprType *SemanticAnalyser::yield(Yield *n, int depth) {
    auto r = exec(n->value, depth);
    if (r.has_value()) {
        return r.value();
    }
    return nullptr;
}
ExprType *SemanticAnalyser::yieldfrom(YieldFrom *n, int depth) { return exec(n->value, depth); }
ExprType *SemanticAnalyser::compare(Compare *n, int depth) {}
ExprType *SemanticAnalyser::call(Call *n, int depth) {
    auto type = exec(n->func, depth);

    for (auto &arg: n->args) {
        exec(arg, depth);
    }

    for (auto &kw: n->keywords) {
        exec(kw.value, depth);
    }

    // type check argument with function def
    // fetch return type inside arrow
    return type;
}
ExprType *SemanticAnalyser::joinedstr(JoinedStr *n, int depth) {}
ExprType *SemanticAnalyser::formattedvalue(FormattedValue *n, int depth) {}
ExprType *SemanticAnalyser::constant(Constant *n, int depth) { return nullptr; }
ExprType *SemanticAnalyser::attribute(Attribute *n, int depth) {}
ExprType *SemanticAnalyser::subscript(Subscript *n, int depth) {}
ExprType *SemanticAnalyser::starred(Starred *n, int depth) {}
ExprType *SemanticAnalyser::name(Name *n, int depth) {
    n->varid = get_varid(n->id);
    return get_type(n->varid);
}
ExprType *SemanticAnalyser::listexpr(ListExpr *n, int depth) {
    auto type = n->new_object<ArrayType>();

    return type;
}
ExprType *SemanticAnalyser::tupleexpr(TupleExpr *n, int depth) {
    auto type = n->new_object<TupleType>();

    return type;
}
ExprType *SemanticAnalyser::slice(Slice *n, int depth) {}
ExprType *SemanticAnalyser::functiondef(FunctionDef *n, int depth) {
    auto id = add(n->name, n, nullptr);
    enter();

    auto return_effective = exec_body(n->body, depth);

    auto type = n->new_object<Arrow>();
    if (n->returns.has_value()) {
        type->returns = n->returns.value();
    }
    type->args = Array<ExprNode *>();

    exit();
    set_type(id, type);
    return nullptr;
}
ExprType *SemanticAnalyser::classdef(ClassDef *n, int depth) {
    auto id = add(n->name, n, nullptr);
    enter();
    exec(n->body, depth);
    exit();
    return nullptr;
}
ExprType *SemanticAnalyser::returnstmt(Return *n, int depth) {
    auto v = exec(n->value, depth);
    if (v.has_value()) {
        return v.value();
    }
    return nullptr;
}
ExprType *SemanticAnalyser::deletestmt(Delete *n, int depth) {}
ExprType *SemanticAnalyser::assign(Assign *n, int depth) {
    exec(n->targets, depth);
    exec(n->value, depth);
    return nullptr;
}
ExprType *SemanticAnalyser::augassign(AugAssign *n, int depth) {
    exec(n->target, depth);
    exec(n->value, depth);
    return nullptr;
}
ExprType *SemanticAnalyser::annassign(AnnAssign *n, int depth) {
    exec(n->target, depth);
    // TODO: type check here
    exec(n->value, depth);
    return nullptr;
}
ExprType *SemanticAnalyser::forstmt(For *n, int depth) {
    exec(n->target, depth);
    exec(n->iter, depth);
    exec(n->body, depth);
    exec(n->orelse, depth);
    return nullptr;
}
ExprType *SemanticAnalyser::whilestmt(While *n, int depth) {
    exec(n->test, depth);
    exec(n->body, depth);
    exec(n->orelse, depth);
    return nullptr;
}
ExprType *SemanticAnalyser::ifstmt(If *n, int depth) {
    exec(n->test, depth);
    exec(n->body, depth);

    for (int i = 0; i < n->tests.size(); i++) {
        exec(n->tests[i], depth);
        exec(n->bodies[i], depth);
    }
}
ExprType *SemanticAnalyser::with(With *n, int depth) {}
ExprType *SemanticAnalyser::raise(Raise *n, int depth) {
    exec(n->exc, depth);
    return nullptr;
}
ExprType *SemanticAnalyser::trystmt(Try *n, int depth) {}
ExprType *SemanticAnalyser::assertstmt(Assert *n, int depth) {
    exec(n->test, depth);
    exec(n->msg, depth + 1);
    return nullptr;
}
ExprType *SemanticAnalyser::import(Import *n, int depth) {}
ExprType *SemanticAnalyser::importfrom(ImportFrom *n, int depth) {}
ExprType *SemanticAnalyser::global(Global *n, int depth) {}
ExprType *SemanticAnalyser::nonlocal(Nonlocal *n, int depth) {
    // n->names
    return nullptr;
}
ExprType *SemanticAnalyser::exprstmt(Expr *n, int depth) {
    exec(n->value, depth);
    return nullptr;
}
ExprType *SemanticAnalyser::pass(Pass *n, int depth) { return nullptr; }
ExprType *SemanticAnalyser::breakstmt(Break *n, int depth) { return nullptr; }
ExprType *SemanticAnalyser::continuestmt(Continue *n, int depth) { return nullptr; }
ExprType *SemanticAnalyser::match(Match *n, int depth) {
    exec(n->subject, depth);

    for (auto &b: n->cases) {
        exec(b.pattern, depth + 1);
        exec(b.guard, depth + 1);
        exec(b.body, depth + 1);
    }

    return nullptr;
}
ExprType *SemanticAnalyser::inlinestmt(Inline *n, int depth) {
    exec(n->body, depth);
    return nullptr;
}

ExprType *SemanticAnalyser::matchvalue(MatchValue *n, int depth) { return nullptr; }
ExprType *SemanticAnalyser::matchsingleton(MatchSingleton *n, int depth) { return nullptr; }
ExprType *SemanticAnalyser::matchsequence(MatchSequence *n, int depth) { return nullptr; }
ExprType *SemanticAnalyser::matchmapping(MatchMapping *n, int depth) { return nullptr; }
ExprType *SemanticAnalyser::matchclass(MatchClass *n, int depth) { return nullptr; }
ExprType *SemanticAnalyser::matchstar(MatchStar *n, int depth) { return nullptr; }
ExprType *SemanticAnalyser::matchas(MatchAs *n, int depth) { return nullptr; }
ExprType *SemanticAnalyser::matchor(MatchOr *n, int depth) { return nullptr; }

} // namespace lython
