

#include "../dtypes.h"
#include "logging/logging.h"

#include "vm/tree.h"
#include "vm/builtins.h"


namespace lython {

PartialResult *TreeEvaluator::boolop(BoolOp_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::namedexpr(NamedExpr_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::binop(BinOp_t *n, int depth) {

    auto lhs = exec(n->left, depth);
    auto rhs = exec(n->right, depth);

    // We can execute the function because both arguments got resolved
    if (lhs && lhs->is_instance<Constant>() && rhs && rhs->is_instance<Constant>()) {

        // Execute function
        if (n->resolved_operator) {
            Scope scope(bindings);

            bindings.add(StringRef(), lhs, nullptr);
            bindings.add(StringRef(), rhs, nullptr);

            return exec(n->resolved_operator, depth);
        }

        if (n->native_operator)
        {
            Constant* lhsc = static_cast<Constant*>(lhs);
            Constant* rhsc = static_cast<Constant*>(rhs);

            return root.new_object<Constant>(
                n->native_operator(lhsc->value, rhsc->value)
            );
        }
    }

    // We could not execute, just return itself
    return n;
}
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

    // fetch the function we need to call
    auto function = exec(n->func, depth);
    assert(function, "Function should be found");

    // TODO: if function is a FunctionDef/Lambda/Callable we can populate the binding name

    // insert arguments to the context
    for(int i = 0; i < n->args.size(); i++) {
        PartialResult* arg = exec(n->args[i], depth);
        bindings.add(StringRef(), arg, nullptr);
    }

    // execute function
    // NB: function can both be a Statement and and Expression (lambda)
    auto returned = exec<PartialResult*>(function, depth);
    return returned;
}

PartialResult *TreeEvaluator::joinedstr(JoinedStr_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::formattedvalue(FormattedValue_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::constant(Constant_t *n, int depth) {
    Constant* cpy = root.copy(n);
    return cpy;
}
PartialResult *TreeEvaluator::attribute(Attribute_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::subscript(Subscript_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::starred(Starred_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::name(Name_t *n, int depth) {
    Node* result = bindings.get_value(n->varid);
    debug("Looked for {} (id: {}) found {}", n->id, n->varid, str(result->kind));
    return result;
}
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
PartialResult *TreeEvaluator::functiondef(FunctionDef_t *n, int depth) {
    return_value = nullptr;

    debug("Executing function");

    for(StmtNode* stmt: n->body) {
        debug("Processing stmt");

        exec(stmt, depth + 1);

        // We are returning
        if (return_value != nullptr) {
            break;
        }
    }

    return return_value;
}
PartialResult *TreeEvaluator::classdef(ClassDef_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::returnstmt(Return_t *n, int depth) {
    debug("Returning {}", str(n));

    if (n->value.has_value()) {
        return_value = exec(n->value.value(), depth);
        debug("Returning {}", str(return_value));
        return return_value;
    }
    return_value = None();
    return return_value;
}


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
PartialResult *TreeEvaluator::exprstmt(Expr_t *n, int depth) {
    return exec(n->value, depth);
}
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