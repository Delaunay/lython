#include "sema/sema.h"

namespace lython {

TypeExpr *SemanticAnalyser::boolop(BoolOp *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::namedexpr(NamedExpr *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::binop(BinOp *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::unaryop(UnaryOp *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::lambda(Lambda *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::ifexp(IfExp *n, int depth) {
    exec(n->test, depth);
    exec(n->body, depth);
    exec(n->orelse, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::dictexpr(DictExpr *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::setexpr(SetExpr *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::listcomp(ListComp *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::generateexpr(GeneratorExp *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::setcomp(SetComp *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::dictcomp(DictComp *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::await(Await *n, int depth) {
    exec(n->value, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::yield(Yield *n, int depth) {
    exec<TypeExpr>(n->value, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::yieldfrom(YieldFrom *n, int depth) {
    exec(n->value, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::compare(Compare *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::call(Call *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::joinedstr(JoinedStr *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::formattedvalue(FormattedValue *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::constant(Constant *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::attribute(Attribute *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::subscript(Subscript *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::starred(Starred *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::name(Name *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::listexpr(ListExpr *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::tupleexpr(TupleExpr *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::slice(Slice *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::functiondef(FunctionDef *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::classdef(ClassDef *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::returnstmt(Return *n, int depth) {
    exec<TypeExpr>(n->value, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::deletestmt(Delete *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::assign(Assign *n, int depth) {
    exec<TypeExpr>(n->targets, depth);
    exec(n->value, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::augassign(AugAssign *n, int depth) {
    exec(n->target, depth);
    exec(n->value, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::annassign(AnnAssign *n, int depth) {
    exec(n->target, depth);
    // TODO: type check here
    exec<TypeExpr>(n->value, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::forstmt(For *n, int depth) {
    exec(n->target, depth);
    exec(n->iter, depth);
    exec<TypeExpr>(n->body, depth);
    exec<TypeExpr>(n->orelse, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::whilestmt(While *n, int depth) {
    exec(n->test, depth);
    exec<TypeExpr>(n->body, depth);
    exec<TypeExpr>(n->orelse, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::ifstmt(If *n, int depth) {
    exec(n->test, depth);
    exec<TypeExpr>(n->body, depth);

    for (int i = 0; i < n->tests.size(); i++) {
        exec(n->tests[i], depth);
        exec<TypeExpr>(n->bodies[i], depth);
    }
    return nullptr;
}
TypeExpr *SemanticAnalyser::with(With *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::raise(Raise *n, int depth) {
    exec<TypeExpr>(n->exc, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::trystmt(Try *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::assertstmt(Assert *n, int depth) {
    exec(n->test, depth);
    exec<TypeExpr>(n->msg, depth + 1);
    return nullptr;
}
TypeExpr *SemanticAnalyser::import(Import *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::importfrom(ImportFrom *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::global(Global *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::nonlocal(Nonlocal *n, int depth) {
    // n->names
    return nullptr;
}
TypeExpr *SemanticAnalyser::exprstmt(Expr *n, int depth) {
    exec(n->value, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::pass(Pass *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::breakstmt(Break *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::continuestmt(Continue *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::match(Match *n, int depth) {
    exec(n->subject, depth);

    for (auto &b: n->cases) {
        exec(b.pattern, depth + 1);
        exec<TypeExpr>(b.guard, depth + 1);
        exec<TypeExpr>(b.body, depth + 1);
    }

    return nullptr;
}
TypeExpr *SemanticAnalyser::inlinestmt(Inline *n, int depth) {
    exec<TypeExpr>(n->body, depth);
    return nullptr;
}

TypeExpr *SemanticAnalyser::matchvalue(MatchValue *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::matchsingleton(MatchSingleton *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::matchsequence(MatchSequence *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::matchmapping(MatchMapping *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::matchclass(MatchClass *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::matchstar(MatchStar *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::matchas(MatchAs *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::matchor(MatchOr *n, int depth) { return nullptr; }

TypeExpr *SemanticAnalyser::dicttype(DictType *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::arraytype(ArrayType *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::arrow(Arrow *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::builtintype(BuiltinType *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::tupletype(TupleType *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::settype(SetType *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::classtype(ClassType *n, int depth) { return nullptr; }

} // namespace lython
