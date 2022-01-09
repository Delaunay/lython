
#include "vm/tree.h"

namespace lython {

PartialResult *TreeEvaluator::boolop(BoolOp *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::namedexpr(NamedExpr *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::binop(BinOp *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::unaryop(UnaryOp *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::lambda(Lambda *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::ifexp(IfExp *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::dictexpr(DictExpr *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::setexpr(SetExpr *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::listcomp(ListComp *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::generateexpr(GeneratorExp *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::setcomp(SetComp *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::dictcomp(DictComp *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::await(Await *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::yield(Yield *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::yieldfrom(YieldFrom *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::compare(Compare *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::call(Call *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::joinedstr(JoinedStr *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::formattedvalue(FormattedValue *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::constant(Constant *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::attribute(Attribute *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::subscript(Subscript *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::starred(Starred *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::name(Name *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::listexpr(ListExpr *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::tupleexpr(TupleExpr *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::slice(Slice *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::dicttype(DictType *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::arraytype(ArrayType *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::tupletype(TupleType *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::arrow(Arrow *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::classtype(ClassType *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::settype(SetType *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::builtintype(BuiltinType *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::functiondef(FunctionDef *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::classdef(ClassDef *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::returnstmt(Return *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::deletestmt(Delete *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::assign(Assign *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::augassign(AugAssign *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::annassign(AnnAssign *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::forstmt(For *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::whilestmt(While *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::ifstmt(If *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::with(With *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::raise(Raise *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::trystmt(Try *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::assertstmt(Assert *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::import(Import *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::importfrom(ImportFrom *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::global(Global *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::nonlocal(Nonlocal *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::exprstmt(Expr *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::pass(Pass *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::breakstmt(Break *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::continuestmt(Continue *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::match(Match *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::inlinestmt(Inline *n, int depth) { return nullptr; }

// Match
// -----
PartialResult *TreeEvaluator::matchvalue(MatchValue *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::matchsingleton(MatchSingleton *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::matchsequence(MatchSequence *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::matchmapping(MatchMapping *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::matchclass(MatchClass *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::matchstar(MatchStar *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::matchas(MatchAs *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::matchor(MatchOr *n, int depth) { return nullptr; }

} // namespace lython