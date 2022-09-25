

#include "../dtypes.h"
#include "logging/logging.h"

#include "vm/tree.h"
#include "vm/builtins.h"


namespace lython {

PartialResult *TreeEvaluator::compare(Compare_t *n, int depth) {

        // a and b and c and d
    //
    PartialResult* left = exec(n->left, depth);
    Constant* left_const = cast<Constant>(left);

    Array<PartialResult*> partials;
    partials.reserve(n->comparators.size());

    bool bnative = n->native_operator.size() > 0;
    bool full_eval = true;

    for(int i = 0; i < n->comparators.size(); i++) {
        PartialResult* right = exec(n->comparators[i], depth);
        partials.push_back(right);

        Constant* right_const = cast<Constant>(right);

        if (left_const && right_const)
        {
            Constant* value = nullptr;

            if (!bnative) {
                Scope scope(bindings);

                bindings.add(StringRef(), left_const, nullptr);
                bindings.add(StringRef(), right_const, nullptr);

                value = cast<Constant>(exec(n->resolved_operator[i], depth));

            }
            else if (bnative) {
                 value = root.new_object<Constant>(
                    n->native_operator[i](left_const->value, right_const->value)
                );
            }

            assert(value != nullptr, "Must return a value if all arguments are set");

            // Should we force full_eval to shortcut ?
            if (!value->value.get<bool>()) {
                return False();
            }

            left = right;
            left_const = right_const;

        } else {
            full_eval = false;
        }
    }

    if (full_eval) {
        return True();
    }

    Compare* comp = root.new_object<Compare>();
    comp->left = n->left;
    comp->ops = n->ops;
    comp->comparators.reserve(partials.size());

    comp->resolved_operator = n->resolved_operator;
    comp->native_operator = n->native_operator;

    for(auto p: partials) {
        comp->comparators.push_back((ExprNode*) p);
    }
    return comp;
}

PartialResult *TreeEvaluator::boolop(BoolOp_t *n, int depth) {
    // a and b or c and d
    //
    PartialResult* first_value = exec(n->values[0], depth);
    Constant* first = cast<Constant>(first_value);

    Array<PartialResult*> partials;
    partials.reserve(n->values.size());
    partials.push_back(first_value);

    bool full_eval = true;

    for(int i = 1; i < n->values.size(); i++) {
        PartialResult* second_value = exec(n->values[i], depth);
        partials.push_back(second_value);

        Constant* second = cast<Constant>(second_value);

        if (first && second)
        {
            Constant* value = nullptr;

            if (n->resolved_operator) {
                Scope scope(bindings);

                bindings.add(StringRef(), first_value, nullptr);
                bindings.add(StringRef(), second_value, nullptr);

                value = cast<Constant>(exec(n->resolved_operator, depth));

            }
            else if (n->native_operator) {
                 value = root.new_object<Constant>(
                    n->native_operator(first->value, second->value)
                );
            }

            assert(value != nullptr, "Must return a value if all arguments are set");

            // Should we force full_eval to shortcut ?
            if (!value->value.get<bool>()) {
                return False();
            }

            first = second;
            first_value = second_value;

        } else {
            full_eval = false;
        }
    }

    if (full_eval) {
        return True();
    }

    BoolOp* boolop = root.new_object<BoolOp>();
    boolop->op = n->op;
    boolop->values.reserve(partials.size());
    boolop->resolved_operator = n->resolved_operator;
    boolop->native_operator = n->native_operator;

    for(auto p: partials) {
        boolop->values.push_back((ExprNode*) p);
    }
    return boolop;
}

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

    // We could not execute, just return what we could execute so far
    BinOp* binary = root.new_object<BinOp>();
    binary->op = n->op;
    binary->left = (ExprNode*) lhs;
    binary->right = (ExprNode*) rhs;
    binary->resolved_operator = n->resolved_operator;
    binary->native_operator = n->native_operator;
    return binary;
}

PartialResult *TreeEvaluator::unaryop(UnaryOp_t *n, int depth)
{
    auto operand = exec(n->operand, depth);

    // We can execute the function because both arguments got resolved
    if (operand && operand->is_instance<Constant>()) {

        // Execute function
        if (n->resolved_operator) {
            Scope scope(bindings);
            bindings.add(StringRef(), operand, nullptr);
            return exec(n->resolved_operator, depth);
        }

        if (n->native_operator)
        {
            Constant* operandc = static_cast<Constant*>(operand);

            return root.new_object<Constant>(
                n->native_operator(operandc->value)
            );
        }
    }

    // We could not execute, just return what we could execute so far
    UnaryOp* unary = root.new_object<UnaryOp>();
    unary->op = n->op;
    unary->operand = (ExprNode*) operand;
    unary->resolved_operator = n->resolved_operator;
    unary->native_operator = n->native_operator;
    return unary;
}

PartialResult *TreeEvaluator::namedexpr(NamedExpr_t *n, int depth) {
    PartialResult* value = exec(n->value, depth);

    if (value->is_instance<Constant>()) {
        bindings.add(StringRef(), value, nullptr);
        return value;
    }

    // TODO: do i put the evaluated expression or the partial expression ?
    NamedExpr* expr = root.new_object<NamedExpr>();
    expr->target = n->target;
    expr->value = (ExprNode*) value;
    bindings.add(StringRef(), value, nullptr);
    return expr;
}

PartialResult *TreeEvaluator::lambda(Lambda_t *n, int depth) {
    auto result = exec(n->body, depth);

    if (result->is_instance<Constant>())
        return result;

    // Here we should build a new lambda
    // but we have to know which args were defined and which were not
    // we can check n->args vardid and fetch them from the context
    // if they are undefined we need to forward them
    return None();
}

PartialResult *TreeEvaluator::ifexp(IfExp_t *n, int depth) {
    Constant* value = cast<Constant>(exec(n->test, depth));

    if (!value) {
        // Could not evaluate the if test
        // the entire expression cannot be evaluated
        return n;
    }

    bool btrue = value->value.get<bool>();

    if (btrue) {
        return exec(n->orelse, depth);
    }

    return exec(n->body, depth);
}


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


PartialResult *TreeEvaluator::constant(Constant_t *n, int depth) {
    Constant* cpy = root.copy(n);
    return cpy;
}


PartialResult *TreeEvaluator::name(Name_t *n, int depth) {
    Node* result = bindings.get_value(n->varid);
    debug("Looked for {} (id: {}) found {}", n->id, n->varid, str(result->kind));
    return result;
}


PartialResult *TreeEvaluator::functiondef(FunctionDef_t *n, int depth) {
    return_value = nullptr;

    debug("Executing function");

    for(StmtNode* stmt: n->body) {
        debug("Processing stmt");

        exec(stmt, depth + 1);

        if (has_exceptions()) {
            return None();
        }

        // We are returning
        if (return_value != nullptr) {
            break;
        }
    }

    return return_value;
}

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



PartialResult *TreeEvaluator::assign(Assign_t *n, int depth) {
    PartialResult* value = exec(n->value, depth);

    TupleExpr* targets = cast<TupleExpr>(n->targets[0]);
    TupleExpr* values = cast<TupleExpr>(value);

    if (values && targets) {
        assert(values->elts.size() == targets->elts.size(), "Size must match");

        for(int i = 0; i < values->elts.size(); i++) {
            bindings.add(StringRef(), values->elts[i], nullptr);
        }
    } else {
        bindings.add(StringRef(), value, nullptr);
    }

    return None();
}
PartialResult *TreeEvaluator::augassign(AugAssign_t *n, int depth) {

    // This should a Name
    auto name = cast<Name>(n->target);

    if (!name) {
        error("Assign to {}", str(n->target->kind));
        return None();
    }

    auto left = exec(n->target, depth); // load a
    auto right = exec(n->value, depth); // load b

    auto left_v = cast<Constant>(left);
    auto right_v = cast<Constant>(right);

    if (left_v && right_v) {
        PartialResult* value = nullptr;

        // Execute function
        if (n->resolved_operator) {
            Scope scope(bindings);

            bindings.add(StringRef(), left_v, nullptr);
            bindings.add(StringRef(), right_v, nullptr);

            value = exec(n->resolved_operator, depth);
        }

        if (n->native_operator)
        {
            value = root.new_object<Constant>(
                n->native_operator(left_v, right_v)
            );
        }

        bindings.set_value(name->varid, value); // store a
        return None();
    }

    AugAssign* expr = root.new_object<AugAssign>();
    // Do not use the evaluted target here
    expr->target = n->target;
    expr->op = n->op;
    expr->value = (ExprNode*) right;
    return expr;
}

PartialResult *TreeEvaluator::annassign(AnnAssign_t *n, int depth) {
    PartialResult* value = None();
    if (n->value.has_value()){
        value = exec(n->value.value(), depth);
    }

    bindings.add(StringRef(), value, nullptr);
    return None();
}

PartialResult *TreeEvaluator::forstmt(For_t *n, int depth) {

    // insert target into the context
    // exec(n->target, depth);
    int targetid = bindings.add(StringRef(), None(), nullptr);

    PartialResult* iterator = exec(n->iter, depth);

    while (true) {
        // Python does not create a new scope for `for`
        // Scope scope(bindings);

        // Get the value of the iterator
        PartialResult* value = get_next(iterator, depth);

        // or value == StopIteration
        if (value == nullptr) {
            break;
        }

        bindings.set_value(targetid, value);

        for(StmtNode* stmt: n->body)
        {
            exec(stmt, depth);

            if (has_exceptions()) {
                return None();
            }

            if (loop_break) {
                break;
            }

            if (loop_continue) {
                continue;
            }
        }
    }

    for(StmtNode* stmt: n->orelse) {
        exec(stmt, depth);
    }

    return None();
}
PartialResult *TreeEvaluator::whilestmt(While_t *n, int depth) {

    while (true) {
        Constant* value = cast<Constant>(exec(n->test, depth));
        // TODO if it is not a value that means we could not evaluate it so we should return the node
        // itself
        assert(value, "While test should return a boolean");

        bool bcontinue = value && value->value.get<bool>();

        if (!bcontinue) {
            break;
        }

        for(StmtNode* stmt: n->body)
        {
            exec(stmt, depth);

            if (has_exceptions()){
                return None();
            }

            if (loop_break) {
                break;
            }

            if (loop_continue) {
                continue;
            }
        }
    }

    for(StmtNode* stmt: n->orelse) {
        exec(stmt, depth);
    }
    return None();
}

PartialResult *TreeEvaluator::ifstmt(If_t *n, int depth) {
    // Chained
    Array<StmtNode *>& body = n->orelse;

    for(int i = 0; i < n->tests.size(); i++) {
        Constant* value = cast<Constant>(exec(n->test, depth));
        assert(value, "If test should return a boolean");

        bool btrue = value->value.get<bool>();
        if (btrue) {
            body = n->bodies[i];
            break;
        }
    }

    for(StmtNode* stmt: body)
    {
        exec(stmt, depth);

        if (has_exceptions()) {
            return None();
        }
    }

    return None();

#if 0
    // Simple
    Constant* value = cast<Constant>(exec(n->test, depth));
    assert(value, "If test should return a boolean");

    bool btrue = value->value.get<bool>();

    Array<StmtNode *>& body = n->orelse;
    if (btrue) {
        body = n->body;
    }

    for(StmtNode* stmt: body)
    {
        exec(stmt, depth);
    }

    return None();
#endif
}

PartialResult *TreeEvaluator::assertstmt(Assert_t *n, int depth) {
    PartialResult* btest = exec(n->test, depth);
    Constant* value = cast<Constant>(btest);
    if (value) {
        if (!value->value.get<bool>()) {
            // raise AssertionError with the assertion message
            return None();
        }

        // All Good
        return None();
    }

    Assert* expr = root.new_object<Assert>();
    expr->test = (ExprNode*) btest;
    expr->msg = n->msg;
    return expr;
}


PartialResult *TreeEvaluator::exprstmt(Expr_t *n, int depth) {
    return exec(n->value, depth);
}
PartialResult *TreeEvaluator::pass(Pass_t *n, int depth) {
    // return itself in case the pass is required for correctness
    return n;
}
PartialResult *TreeEvaluator::breakstmt(Break_t *n, int depth) {
    loop_break = true;
    return n;
}
PartialResult *TreeEvaluator::continuestmt(Continue_t *n, int depth) {
    loop_continue = true;
    return n;
}

PartialResult *TreeEvaluator::inlinestmt(Inline_t *n, int depth) {

    for(StmtNode* stmt: n->body)
    {
        exec(stmt, depth);

        if (has_exceptions()) {
            return None();
        }
    }
    return None();
}

PartialResult *TreeEvaluator::raise(Raise_t *n, int depth) {

    if (n->exc.has_value()) {
        exceptions.push_back(exec(n->exc.value(), depth));

        if (n->cause.has_value()) {
            cause = n->cause.value();
        }
    }

    exceptions.push_back(None());
    return nullptr;
 }

 void TreeEvaluator::execute_loop_body(Array<StmtNode *>& body, int depth) {
    for(StmtNode* stmt: body)
    {
        exec(stmt, depth);

        if (has_exceptions()) {
            break;
        }

        if (loop_break) {
            break;
        }

        if (loop_continue) {
            continue;
        }
    }
 }

 void TreeEvaluator::execute_body(Array<StmtNode *>& body, int depth) {
    for(StmtNode* stmt: body)
    {
        exec(stmt, depth);

        if (has_exceptions()) {
            break;
        }
    }
 }


PartialResult *TreeEvaluator::trystmt(Try_t *n, int depth) {

    bool received_exception = false;

    for(StmtNode* stmt: n->body)
    {
        exec(stmt, depth);

        if (has_exceptions()) {
            received_exception = true;
            break;
        }
    }

    if (received_exception) {
        // start handling all the exceptions we received
        handling_exceptions = exceptions.size();

        ExceptHandler const* matched = nullptr;
        PartialResult* latest_exception = exceptions[exceptions.size() - 1];

        for(ExceptHandler const& handler: n->handlers) {
            // match the exception type to the one we received

            // catch all
            if (!handler.type.has_value()) {
                matched = &handler;
                break;
            }

            // FIXME: we do not have the type at runtime!!!
            else if (equal(handler.type.value(), latest_exception)) {
                matched = &handler;
                break;
            }
        }

        if (matched) {
            // Execute Handler
            if (matched->name.has_value()) {
                bindings.add(matched->name.value(), latest_exception, nullptr);
            }

            for(StmtNode* stmt: matched->body)
            {
                exec(stmt, depth);

                // new exception
                if (has_exceptions()) {
                    return None();
                }
            }

            // Exception was handled!
            exceptions.pop_back();
            cause = nullptr;
        }
        // Exception was NOT handled
        // leave the exception as is so we continue moving back

    } else {
        for(StmtNode* stmt: n->orelse)
        {
            exec(stmt, depth);

            if (has_exceptions()) {
                return None();
            }
        }
    }

    // this some clean up code, acknowledge we have exceptions
    // but this code needs to run regardless, it will stop if new exceptions are raised
    handling_exceptions = exceptions.size();

    for(StmtNode* stmt: n->finalbody)
    {
        exec(stmt, depth);

        // New exceptions
        if (has_exceptions()) {
            return None();
        }
    }

    // we are not handling exception anymore
    handling_exceptions = 0;

    return None();
}

PartialResult *TreeEvaluator::with(With_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::match(Match_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::import(Import_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::importfrom(ImportFrom_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::global(Global_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::nonlocal(Nonlocal_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::dictexpr(DictExpr_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::setexpr(SetExpr_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::listcomp(ListComp_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::generateexpr(GeneratorExp_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::setcomp(SetComp_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::dictcomp(DictComp_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::await(Await_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::yield(Yield_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::yieldfrom(YieldFrom_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::joinedstr(JoinedStr_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::formattedvalue(FormattedValue_t *n, int depth) { return nullptr; }

PartialResult *TreeEvaluator::starred(Starred_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::listexpr(ListExpr_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::tupleexpr(TupleExpr_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::slice(Slice_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::deletestmt(Delete_t *n, int depth) { return nullptr; }


// Objects
PartialResult *TreeEvaluator::attribute(Attribute_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::subscript(Subscript_t *n, int depth) { return nullptr; }

// Call __next__ for a given object
PartialResult *TreeEvaluator::get_next(Node *iterator, int depth) {
    // FIXME: implement me
    return nullptr;
}

// Types
// -----
PartialResult *TreeEvaluator::classdef(ClassDef_t *n, int depth) { return nullptr; }

PartialResult *TreeEvaluator::dicttype(DictType_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::arraytype(ArrayType_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::tupletype(TupleType_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::arrow(Arrow_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::classtype(ClassType_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::settype(SetType_t *n, int depth) { return nullptr; }
PartialResult *TreeEvaluator::builtintype(BuiltinType_t *n, int depth) { return nullptr; }

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