
#include "vm/tree.h"

namespace lython {

PartialResult *TreeEvaluator::boolop(BoolOp_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::namedexpr(NamedExpr_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::binop(BinOp_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::unaryop(UnaryOp_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::lambda(Lambda_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::ifexp(IfExp_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::dictexpr(DictExpr_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::setexpr(SetExpr_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::listcomp(ListComp_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::generateexpr(GeneratorExp_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::setcomp(SetComp_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::dictcomp(DictComp_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::await(Await_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::yield(Yield_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::yieldfrom(YieldFrom_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::compare(Compare_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::call(Call_t *n, int depth) { 
    Scope scope(bindings);
    
    // insert arguments to the context
    for(int i = 0; i < n->args.size(); i++) {
        arg = exec(n->args[i], depth);
        bindings.add("", arg, nullptr);
    }

    // not sure if we need 2 exec for this
    // maybe the fetch should exec too
    // fetch the function we need to call
    auto function = exec(n->func, depth);

    // execute function
    return exec(function, depth); 
}
PartialResult *TreeEvaluator::joinedstr(JoinedStr_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::formattedvalue(FormattedValue_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::constant(Constant_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::attribute(Attribute_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::subscript(Subscript_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::starred(Starred_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::name(Name_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::listexpr(ListExpr_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::tupleexpr(TupleExpr_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::slice(Slice_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::dicttype(DictType_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::arraytype(ArrayType_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::tupletype(TupleType_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::arrow(Arrow_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::classtype(ClassType_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::settype(SetType_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::builtintype(BuiltinType_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::functiondef(FunctionDef_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::classdef(ClassDef_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::returnstmt(Return_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::deletestmt(Delete_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::assign(Assign_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::augassign(AugAssign_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::annassign(AnnAssign_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::forstmt(For_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::whilestmt(While_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::ifstmt(If_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::with(With_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::raise(Raise_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::trystmt(Try_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::assertstmt(Assert_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::import(Import_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::importfrom(ImportFrom_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::global(Global_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::nonlocal(Nonlocal_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::exprstmt(Expr_t *n, int depth) { return exec(n->value, depth); }
PartialResult *TreeEvaluator::pass(Pass_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::breakstmt(Break_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::continuestmt(Continue_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::match(Match_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::inlinestmt(Inline_t *n, int depth) { return nullptr; }

// Match
// -----
PartialResult *TreeEvaluator::matchvalue(MatchValue_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::matchsingleton(MatchSingleton_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::matchsequence(MatchSequence_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::matchmapping(MatchMapping_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::matchclass(MatchClass_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::matchstar(MatchStar_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::matchas(MatchAs_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::matchor(MatchOr_t *n, int depth) { return nullptr; }

} // namespace lython