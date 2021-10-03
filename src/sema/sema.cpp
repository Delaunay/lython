#include "sema/sema.h"

namespace lython {

BoolOp *   SemanticAnalyser::boolop(BoolOp *n, int depth) {}
NamedExpr *SemanticAnalyser::namedexpr(NamedExpr *n, int depth) {}
BinOp *    SemanticAnalyser::binop(BinOp *n, int depth) {}
UnaryOp *  SemanticAnalyser::unaryop(UnaryOp *n, int depth) {}
Lambda *   SemanticAnalyser::lambda(Lambda *n, int depth) {}
IfExp *    SemanticAnalyser::ifexp(IfExp *n, int depth) {
    exec(n->test, depth);
    exec(n->body, depth);
    exec(n->orelse, depth);
    return n;
}
DictExpr *    SemanticAnalyser::dictexpr(DictExpr *n, int depth) {}
SetExpr *     SemanticAnalyser::setexpr(SetExpr *n, int depth) {}
ListComp *    SemanticAnalyser::listcomp(ListComp *n, int depth) {}
GeneratorExp *SemanticAnalyser::generateexpr(GeneratorExp *n, int depth) {}
SetComp *     SemanticAnalyser::setcomp(SetComp *n, int depth) {}
DictComp *    SemanticAnalyser::dictcomp(DictComp *n, int depth) {}
Await *       SemanticAnalyser::await(Await *n, int depth) {
    exec(n->value, depth);
    return n;
}
Yield *SemanticAnalyser::yield(Yield *n, int depth) {
    exec(n->value, depth);
    return n;
}
YieldFrom *SemanticAnalyser::yieldfrom(YieldFrom *n, int depth) {
    exec(n->value, depth);
    return n;
}
Compare *       SemanticAnalyser::compare(Compare *n, int depth) {}
Call *          SemanticAnalyser::call(Call *n, int depth) {}
JoinedStr *     SemanticAnalyser::joinedstr(JoinedStr *n, int depth) {}
FormattedValue *SemanticAnalyser::formattedvalue(FormattedValue *n, int depth) {}
Constant *      SemanticAnalyser::constant(Constant *n, int depth) { return n; }
Attribute *     SemanticAnalyser::attribute(Attribute *n, int depth) {}
Subscript *     SemanticAnalyser::subscript(Subscript *n, int depth) {}
Starred *       SemanticAnalyser::starred(Starred *n, int depth) {}
Name *          SemanticAnalyser::name(Name *n, int depth) {}
ListExpr *      SemanticAnalyser::listexpr(ListExpr *n, int depth) {}
TupleExpr *     SemanticAnalyser::tupleexpr(TupleExpr *n, int depth) {}
Slice *         SemanticAnalyser::slice(Slice *n, int depth) {}
FunctionDef *   SemanticAnalyser::functiondef(FunctionDef *n, int depth) {}
ClassDef *      SemanticAnalyser::classdef(ClassDef *n, int depth) {}
Return *        SemanticAnalyser::returnstmt(Return *n, int depth) {
    exec(n->value, depth);
    return n;
}
Delete *SemanticAnalyser::deletestmt(Delete *n, int depth) {}
Assign *SemanticAnalyser::assign(Assign *n, int depth) {
    exec(n->targets, depth);
    exec(n->value, depth);
    return n;
}
AugAssign *SemanticAnalyser::augassign(AugAssign *n, int depth) {
    exec(n->target, depth);
    exec(n->value, depth);
    return n;
}
AnnAssign *SemanticAnalyser::annassign(AnnAssign *n, int depth) {
    exec(n->target, depth);
    // TODO: type check here
    exec(n->value, depth);
    return n;
}
For *SemanticAnalyser::forstmt(For *n, int depth) {
    exec(n->target, depth);
    exec(n->iter, depth);
    exec(n->body, depth);
    exec(n->orelse, depth);
    return n;
}
While *SemanticAnalyser::whilestmt(While *n, int depth) {
    exec(n->test, depth);
    exec(n->body, depth);
    exec(n->orelse, depth);
    return n;
}
If *SemanticAnalyser::ifstmt(If *n, int depth) {
    exec(n->test, depth);
    exec(n->body, depth);

    for (int i = 0; i < n->tests.size(); i++) {
        exec(n->tests[i], depth);
        exec(n->bodies[i], depth);
    }
}
With * SemanticAnalyser::with(With *n, int depth) {}
Raise *SemanticAnalyser::raise(Raise *n, int depth) {
    exec(n->exc, depth);
    return n;
}
Try *   SemanticAnalyser::trystmt(Try *n, int depth) {}
Assert *SemanticAnalyser::assertstmt(Assert *n, int depth) {
    exec(n->test, depth);
    exec(n->msg, depth + 1);
    return n;
}
Import *    SemanticAnalyser::import(Import *n, int depth) {}
ImportFrom *SemanticAnalyser::importfrom(ImportFrom *n, int depth) {}
Global *    SemanticAnalyser::global(Global *n, int depth) {}
Nonlocal *  SemanticAnalyser::nonlocal(Nonlocal *n, int depth) {
    // n->names
    return n;
}
Expr *SemanticAnalyser::exprstmt(Expr *n, int depth) {
    exec(n->value, depth);
    return n;
}
Pass *    SemanticAnalyser::pass(Pass *n, int depth) { return n; }
Break *   SemanticAnalyser::breakstmt(Break *n, int depth) { return n; }
Continue *SemanticAnalyser::continuestmt(Continue *n, int depth) { return n; }
Match *   SemanticAnalyser::match(Match *n, int depth) {
    exec(n->subject, depth);

    for (auto &b: n->cases) {
        exec(b.pattern, depth + 1);
        exec(b.guard, depth + 1);
        exec(b.body, depth + 1);
    }

    return n;
}
Inline *SemanticAnalyser::inlinestmt(Inline *n, int depth) {
    exec(n->body, depth);
    return n;
}

MatchValue *    SemanticAnalyser::matchvalue(MatchValue *n, int depth) { return n; }
MatchSingleton *SemanticAnalyser::matchsingleton(MatchSingleton *n, int depth) { return n; }
MatchSequence * SemanticAnalyser::matchsequence(MatchSequence *n, int depth) { return n; }
MatchMapping *  SemanticAnalyser::matchmapping(MatchMapping *n, int depth) { return n; }
MatchClass *    SemanticAnalyser::matchclass(MatchClass *n, int depth) { return n; }
MatchStar *     SemanticAnalyser::matchstar(MatchStar *n, int depth) { return n; }
MatchAs *       SemanticAnalyser::matchas(MatchAs *n, int depth) { return n; }
MatchOr *       SemanticAnalyser::matchor(MatchOr *n, int depth) { return n; }

} // namespace lython
