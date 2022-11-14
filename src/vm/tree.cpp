
#include "../dtypes.h"
#include "ast/values/exception.h"
#include "ast/values/generator.h"
#include "ast/values/object.h"
#include "logging/logging.h"
#include "parser/parsing_error.h"
#include "utilities/guard.h"

#include "vm/tree.h"

namespace lython {

void TreeEvaluator::raise_exception(PartialResult* exception, PartialResult* cause) {
    // Create the exception object

    // Constant* cause_value = cast<Constant>(cause);
    // // cause should be an exception
    // NativeObject* cause_except = cause_value->value.get<NativeObject*>();

    // if (cause != nullptr && cause_except) {
    //     // TODO:
    // }

    lyException* except = root.new_object<lyException>(traces);
    // Constant*  except_value = root.new_object<Constant>(except);

    exceptions.push_back(except);
}

PartialResult* TreeEvaluator::compare(Compare_t* n, int depth) {

    // a and b and c and d
    //
    PartialResult* left       = exec(n->left, depth);
    Constant*      left_const = cast<Constant>(left);

    Array<PartialResult*> partials;
    partials.reserve(n->comparators.size());

    bool bnative   = n->native_operator.size() > 0;
    bool full_eval = true;
    bool result    = true;

    for (int i = 0; i < n->comparators.size(); i++) {
        PartialResult* right = exec(n->comparators[i], depth);
        partials.push_back(right);

        Constant* right_const = cast<Constant>(right);

        if (left_const && right_const) {
            Constant* value = nullptr;

            if (!bnative) {
                Scope scope(bindings);

                bindings.add(StringRef(), left_const, nullptr);
                bindings.add(StringRef(), right_const, nullptr);

                value = cast<Constant>(exec(n->resolved_operator[i], depth));

            } else if (bnative) {
                auto native = n->native_operator[i];
                assert(native, "Operator needs to be set");

                ConstantValue v = native(left_const->value, right_const->value);
                result          = result && v.get<bool>();
            }

            // One comparison is false so the entire thing does not work
            if (!result) {
                return False();
            }

            left       = right;
            left_const = right_const;

        } else {
            full_eval = false;
        }
    }

    if (full_eval) {
        return True();
    }

    Compare* comp = root.new_object<Compare>();
    comp->left    = n->left;
    comp->ops     = n->ops;
    comp->comparators.reserve(partials.size());

    comp->resolved_operator = n->resolved_operator;
    comp->native_operator   = n->native_operator;

    for (auto p: partials) {
        comp->comparators.push_back((ExprNode*)p);
    }
    return comp;
}

PartialResult* TreeEvaluator::boolop(BoolOp_t* n, int depth) {
    // a and b or c and d
    //
    PartialResult* first_value = exec(n->values[0], depth);
    Constant*      first       = cast<Constant>(first_value);

    Array<PartialResult*> partials;
    partials.reserve(n->values.size());
    partials.push_back(first_value);

    bool                            full_eval = true;
    bool                            result    = false;
    std::function<bool(bool, bool)> reduce    = [](bool a, bool b) -> bool { return a || b; };

    if (n->op == BoolOperator::And) {
        result = true;
        reduce = [](bool a, bool b) -> bool { return a && b; };
    }

    for (int i = 1; i < n->values.size(); i++) {
        PartialResult* second_value = exec(n->values[i], depth);
        partials.push_back(second_value);

        Constant* second = cast<Constant>(second_value);

        if (first && second) {
            Constant* value = nullptr;

            if (n->resolved_operator) {
                Scope scope(bindings);

                bindings.add(StringRef(), first_value, nullptr);
                bindings.add(StringRef(), second_value, nullptr);

                value = cast<Constant>(exec(n->resolved_operator, depth));

                result = reduce(result, value->value.get<bool>());

            } else if (n->native_operator) {
                ConstantValue v = n->native_operator(first->value, second->value);
                result          = reduce(result, v.get<bool>());
            }

            // Shortcut
            if (n->op == BoolOperator::And && result == false) {
                return False();
            }

            if (n->op == BoolOperator::Or && result == true) {
                return True();
            }

            first       = second;
            first_value = second_value;

        } else {
            full_eval = false;
        }
    }

    if (result) {
        return True();
    } else {
        return False();
    }

    BoolOp* boolop = root.new_object<BoolOp>();
    boolop->op     = n->op;
    boolop->values.reserve(partials.size());
    boolop->resolved_operator = n->resolved_operator;
    boolop->native_operator   = n->native_operator;

    for (auto p: partials) {
        boolop->values.push_back((ExprNode*)p);
    }
    return boolop;
}

PartialResult* TreeEvaluator::binop(BinOp_t* n, int depth) {

    auto lhs = exec(n->left, depth);
    auto rhs = exec(n->right, depth);

    // TODO: if they evaluate to constant that belong to the value root
    // we can free them as soon as we finish combining the values

    // We can execute the function because both arguments got resolved
    if (lhs && lhs->is_instance<Constant>() && rhs && rhs->is_instance<Constant>()) {
        PartialResult* result = nullptr;

        // Execute function
        if (n->resolved_operator) {
            Scope scope(bindings);

            bindings.add(StringRef(), lhs, nullptr);
            bindings.add(StringRef(), rhs, nullptr);

            result = exec(n->resolved_operator, depth);
        }

        else if (n->native_operator) {
            Constant* lhsc = static_cast<Constant*>(lhs);
            Constant* rhsc = static_cast<Constant*>(rhs);

            result = root.new_object<Constant>(n->native_operator(lhsc->value, rhsc->value));
        }

        if (result) {
            // Free the temporary values because we were able to combine them into a result
            // lhs/rhs could be constant created by the parser, in that case their parent are not
            // the root node and they should not be destroyed

            // This is not valid, the constant could have been allocated by a function call
            // they need to be freed when the call ends
            // root.remove_child_if_parent(lhs, true);
            // root.remove_child_if_parent(rhs, true);

            return result;
        }
    }

    // We could not execute, just return what we could execute so far
    BinOp* binary = root.new_object<BinOp>();

    binary->op    = n->op;
    binary->left  = (ExprNode*)lhs;
    binary->right = (ExprNode*)rhs;

    binary->resolved_operator = n->resolved_operator;
    binary->native_operator   = n->native_operator;

    // The partial operator becomes the owner of the partial results
    lhs->move(binary);
    rhs->move(binary);

    return binary;
}

PartialResult* TreeEvaluator::unaryop(UnaryOp_t* n, int depth) {
    auto operand = exec(n->operand, depth);

    // We can execute the function because both arguments got resolved
    if (operand && operand->is_instance<Constant>()) {

        // Execute function
        if (n->resolved_operator) {
            Scope scope(bindings);
            bindings.add(StringRef(), operand, nullptr);
            return exec(n->resolved_operator, depth);
        }

        if (n->native_operator) {
            Constant* operandc = static_cast<Constant*>(operand);
            return root.new_object<Constant>(n->native_operator(operandc->value));
        }
    }

    // We could not execute, just return what we could execute so far
    UnaryOp* unary           = root.new_object<UnaryOp>();
    unary->op                = n->op;
    unary->operand           = (ExprNode*)operand;
    unary->resolved_operator = n->resolved_operator;
    unary->native_operator   = n->native_operator;
    return unary;
}

PartialResult* TreeEvaluator::namedexpr(NamedExpr_t* n, int depth) {
    PartialResult* value = exec(n->value, depth);

    if (value->is_instance<Constant>()) {
        bindings.add(StringRef(), value, nullptr);
        return value;
    }

    // TODO: do i put the evaluated expression or the partial expression ?
    NamedExpr* expr = root.new_object<NamedExpr>();
    expr->target    = n->target;
    expr->value     = (ExprNode*)value;
    bindings.add(StringRef(), value, nullptr);
    return expr;
}

PartialResult* TreeEvaluator::lambda(Lambda_t* n, int depth) {
    auto result = exec(n->body, depth);

    if (result->is_instance<Constant>())
        return result;

    // Here we should build a new lambda
    // but we have to know which args were defined and which were not
    // we can check n->args vardid and fetch them from the context
    // if they are undefined we need to forward them
    return None();
}

PartialResult* TreeEvaluator::ifexp(IfExp_t* n, int depth) {
    Constant* value = cast<Constant>(exec(n->test, depth));

    if (!value) {
        // Could not evaluate the if test
        // the entire expression cannot be evaluated
        return n;
    }

    bool btrue = value->value.get<bool>();

    if (btrue) {
        return exec(n->body, depth);
    }

    return exec(n->orelse, depth);
}

PartialResult* TreeEvaluator::call_native(Call_t* call, BuiltinType_t* function, int depth) {
    Array<PartialResult*> args;
    Array<Constant*>      value_args;
    args.reserve(call->args.size());
    value_args.reserve(call->args.size());

    bool compile_time = true;

    for (int i = 0; i < call->args.size(); i++) {
        PartialResult* arg = exec(call->args[i], depth);
        args.push_back(arg);

        Constant* value = cast<Constant>(arg);
        if (value) {
            value_args.push_back(value);
        }
        compile_time = compile_time && value != nullptr;
    }

    PartialResult* ret_result = nullptr;

    if (compile_time) {
        ConstantValue result = function->native_function(value_args);
        ret_result           = root.new_object<Constant>(result);
    } else {
        // FIXME: we probably need the context here
        ret_result = function->native_macro(args);
    }

    for (PartialResult* arg: args) {
        root.remove_child_if_parent(arg, true);
    }

    return ret_result;
}
PartialResult* TreeEvaluator::call_script(Call_t* call, FunctionDef_t* function, int depth) {
    Scope scope(bindings);

    // TODO: free the references held by the binding to save sapce
    Array<PartialResult*> to_be_freed;
    to_be_freed.reserve(call->args.size());

    // insert arguments to the context
    for (int i = 0; i < call->args.size(); i++) {
        PartialResult* arg = exec(call->args[i], depth);
        to_be_freed.push_back(arg);
        bindings.add(StringRef(), arg, nullptr);
    }

    for (StmtNode* stmt: function->body) {
        exec(stmt, depth + 1);

        if (has_exceptions()) {
            return None();
        }

        // We are returning
        if (return_value != nullptr) {
            break;
        }
    }

    // TODO: check if the execution was partial or full
    // Actually; if we take ownership of the arguments
    // when we generate partial nodes then we can always try to free regardless
    for (PartialResult* arg: to_be_freed) {
        root.remove_child_if_parent(arg, true);
    }

    return return_value;
}

PartialResult* TreeEvaluator::call_constructor(Call_t* call, ClassDef_t* cls, int depth) {
    return nullptr;
}

Constant* object__new__(GCObject* parent, ClassDef* class_t) {
    Constant* value = parent->new_object<Constant>();

    Object* obj = value->new_object<Object>();
    obj->attributes.resize(class_t->attributes.size());

    value->value = ConstantValue(obj);
    return value;
}

Constant* TreeEvaluator::make(ClassDef* class_t, Array<Constant*> args, int depth) {
    // TODO: this should be generated inside the SEMA
    //
    String __new__  = class_t->cls_namespace + "." + "__new__";
    String __init__ = class_t->cls_namespace + "." + "__init__";

    int varid_new_fun  = bindings.get_varid(__new__);
    int varid_init_fun = bindings.get_varid(__init__);

    FunctionDef* new_fun  = cast<FunctionDef>(bindings.get_value(varid_new_fun));
    FunctionDef* init_fun = cast<FunctionDef>(bindings.get_value(varid_init_fun));

    Constant* self = nullptr;

    if (new_fun) {
        Scope _(bindings);
        bindings.add(StringRef(), class_t, nullptr);
        for (auto& arg: args) {
            bindings.add(StringRef(), arg, nullptr);
        }

        for (auto& stmt: new_fun->body) {
            exec(stmt, depth);

            if (return_value) {
                self = cast<Constant>(return_value);
                break;
            }

            if (has_exceptions()) {
                break;
            }
        }
    }

    if (init_fun) {
        Scope _(bindings);

        bindings.add(StringRef(), self, nullptr);
        for (auto& arg: args) {
            bindings.add(StringRef(), arg, nullptr);
        }

        for (auto& stmt: init_fun->body) {
            exec(stmt, depth);

            if (has_exceptions()) {
                break;
            }
        }
    }

    return self;
}

PartialResult* TreeEvaluator::make_generator(Call_t* call, FunctionDef_t* n, int depth) {

    Generator* gen = root.new_object<Generator>();
    gen->scope     = bindings;

    for (int i = 0; i < call->args.size(); i++) {
        PartialResult* arg = exec(call->args[i], depth);
        gen->scope.add(StringRef(), arg, nullptr);
    }

    Constant* val = root.new_object<Constant>();
    val->value    = ConstantValue(gen);

    return val;
}

PartialResult* TreeEvaluator::call(Call_t* n, int depth) {

    using TraceGuard = PopGuard<Array<StackTrace>, StackTrace>;

    // Populate current stack with the expression that will branch out
    get_trace().expr = n;

    TraceGuard  _(traces);
    StackTrace& trace = traces.emplace_back();

    // fetch the function we need to call
    auto function = exec(n->func, depth);
    assert(function, "Function should be found");

    if (FunctionDef_t* fun = cast<FunctionDef>(function)) {
        if (fun->generator) {
            return make_generator(n, fun, depth);
        }
        return call_script(n, fun, depth);
    }

    if (ClassDef_t* cls = cast<ClassDef_t>(function)) {
        return call_constructor(n, cls, depth);
    }

    if (BuiltinType_t* fun = cast<BuiltinType>(function)) {
        return call_native(n, fun, depth);
    }

    /*
    if (Coroutine_t* fun = cast<Coroutine>(function)) {
        return call_coroutine(n, fun, depth);
    }
    */

    // function could not be resolved at compile time
    // return self ?
    return nullptr;
}

PartialResult* TreeEvaluator::constant(Constant_t* n, int depth) {
    Constant* cpy = root.copy(n);
    return cpy;
}

PartialResult* TreeEvaluator::comment(Comment_t* n, int depth) { return nullptr; }

PartialResult* TreeEvaluator::name(Name_t* n, int depth) {

    Node* result = nullptr;
    int   varid  = -1;

    if (n->dynamic) {
        // Local variables | Arguments
        assert(n->offset != -1, "Reference should have a reverse lookup offset");
        varid  = int(bindings.bindings.size()) - n->offset;
        result = bindings.get_value(varid);
    } else {
        // Global variables
        result = bindings.get_value(n->varid);
    }

    assert(result != nullptr, "Could not find variable");
    // bindings.dump(std::cout);

    String kindstr = "";
    if (result) {
        kindstr = str(result->kind);
    }

    String strresult;
    if (result && result->kind == NodeKind::Constant) {
        strresult = str(result);
    }
    debug("Name (varid: {}), (size: {}) (offset: {}) resolved: {}",
          n->varid,
          n->size,
          n->offset,
          varid);
    debug("Looked for {} (id: {}) found {}: {}", n->id, n->varid, kindstr, strresult);
    return result;
}

PartialResult* TreeEvaluator::functiondef(FunctionDef_t* n, int depth) {
    return_value = nullptr;

    for (StmtNode* stmt: n->body) {
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

PartialResult* TreeEvaluator::invalidstmt(InvalidStatement_t* n, int depth) {
    // FIXME: raise exception here
    raise_exception(nullptr, nullptr);
    return nullptr;
}

PartialResult* TreeEvaluator::returnstmt(Return_t* n, int depth) {
    debug("Compute return {}", str(n));

    if (n->value.has_value()) {
        set_return_value(exec(n->value.value(), depth));
        debug("Returning {}", str(return_value));
        return return_value;
    }

    set_return_value(None());
    return return_value;
}

PartialResult* TreeEvaluator::assign(Assign_t* n, int depth) {
    PartialResult* value = exec(n->value, depth);

    TupleExpr* targets = cast<TupleExpr>(n->targets[0]);
    TupleExpr* values  = cast<TupleExpr>(value);

    if (values && targets) {
        assert(values->elts.size() == targets->elts.size(), "Size must match");

        for (int i = 0; i < values->elts.size(); i++) {
            bindings.add(StringRef(), values->elts[i], nullptr);
        }
    } else {
        bindings.add(StringRef(), value, nullptr);
    }

    return None();
}
PartialResult* TreeEvaluator::augassign(AugAssign_t* n, int depth) {

    // This should a Name
    auto name = cast<Name>(n->target);

    if (!name) {
        error("Assign to {}", str(n->target->kind));
        return None();
    }

    auto left  = exec(n->target, depth);  // load a
    auto right = exec(n->value, depth);   // load b

    auto left_v  = cast<Constant>(left);
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

        else if (n->native_operator) {
            auto result = n->native_operator(left_v->value, right_v->value);
            value       = root.new_object<Constant>(result);
        } else {
            error("Operator does not have implementation!");
        }

        bindings.set_value(name->varid, value);  // store a
        return None();
    }

    AugAssign* expr = root.new_object<AugAssign>();
    // Do not use the evaluted target here
    expr->target = n->target;
    expr->op     = n->op;
    expr->value  = (ExprNode*)right;
    return expr;
}

PartialResult* TreeEvaluator::annassign(AnnAssign_t* n, int depth) {
    PartialResult* value = None();
    if (n->value.has_value()) {
        value = exec(n->value.value(), depth);
    }

    bindings.add(StringRef(), value, nullptr);
    return None();
}

PartialResult* TreeEvaluator::forstmt(For_t* n, int depth) {

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

        for (StmtNode* stmt: n->body) {
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

    for (StmtNode* stmt: n->orelse) {
        exec(stmt, depth);

        if (has_exceptions()) {
            return None();
        }
    }

    return None();
}
PartialResult* TreeEvaluator::whilestmt(While_t* n, int depth) {

    bool broke = false;

    while (true) {
        Constant* value = cast<Constant>(exec(n->test, depth));
        // TODO if it is not a value that means we could not evaluate it so we should return the
        // node itself
        assert(value, "While test should return a boolean");

        bool bcontinue = value && value->value.get<bool>();

        if (!bcontinue || broke) {
            break;
        }

        for (StmtNode* stmt: n->body) {
            exec(stmt, depth);

            if (has_exceptions()) {
                return None();
            }

            if (loop_break) {
                broke = true;
                break;
            }

            if (loop_continue) {
                break;
            }
        }

        // reset
        loop_break    = false;
        loop_continue = false;
    }

    for (StmtNode* stmt: n->orelse) {
        exec(stmt, depth);

        if (has_exceptions()) {
            return None();
        }
    }
    return None();
}

PartialResult* TreeEvaluator::ifstmt(If_t* n, int depth) {

    Array<StmtNode*>& body = n->orelse;
    // Chained
    if (n->tests.size() > 0) {
        for (int i = 0; i < n->tests.size(); i++) {
            Constant* value = cast<Constant>(exec(n->test, depth));
            assert(value, "If test should return a boolean");

            bool btrue = value->value.get<bool>();
            if (btrue) {
                body = n->bodies[i];
                break;
            }
        }

        for (StmtNode* stmt: body) {
            exec(stmt, depth);

            if (has_exceptions()) {
                return None();
            }
        }

        return None();
    }

    // Simple
    PartialResult* test  = exec(n->test, depth);
    Constant*      value = cast<Constant>(test);
    assert(value, "If test should return a boolean");

    bool btrue = value->value.get<bool>();

    if (btrue) {
        body = n->body;
    }

    for (StmtNode* stmt: body) {
        exec(stmt, depth);

        if (has_exceptions()) {
            return None();
        }

        if (return_value != nullptr) {
            break;
        }
    }

    return None();
}

PartialResult* TreeEvaluator::assertstmt(Assert_t* n, int depth) {
    PartialResult* btest = exec(n->test, depth);
    Constant*      value = cast<Constant>(btest);
    if (value) {
        if (!value->value.get<bool>()) {
            // make_ref(n, "AssertionError") n->msg
            raise_exception(nullptr, nullptr);
            return None();
        }

        // All Good
        return None();
    }

    Assert* expr = root.new_object<Assert>();
    expr->test   = (ExprNode*)btest;
    expr->msg    = n->msg;
    return expr;
}

PartialResult* TreeEvaluator::exprstmt(Expr_t* n, int depth) { return exec(n->value, depth); }
PartialResult* TreeEvaluator::pass(Pass_t* n, int depth) {
    // return itself in case the pass is required for correctness
    return n;
}
PartialResult* TreeEvaluator::breakstmt(Break_t* n, int depth) {
    loop_break = true;
    return n;
}
PartialResult* TreeEvaluator::continuestmt(Continue_t* n, int depth) {
    loop_continue = true;
    return n;
}

PartialResult* TreeEvaluator::inlinestmt(Inline_t* n, int depth) {

    for (StmtNode* stmt: n->body) {
        exec(stmt, depth);

        if (has_exceptions()) {
            return None();
        }
    }
    return None();
}

PartialResult* TreeEvaluator::raise(Raise_t* n, int depth) {

    if (n->exc.has_value()) {
        // create a new exceptins
        auto obj = exec(n->exc.value(), depth);

        if (n->cause.has_value()) {
            cause = exec(n->cause.value(), depth);
        }

        raise_exception(obj, cause);
    }

    // FIXME: this re-reraise current exception
    // check what happens if no exception exists
    exceptions.push_back(nullptr);
    return nullptr;
}

void TreeEvaluator::execute_loop_body(Array<StmtNode*>& body, int depth) {
    for (StmtNode* stmt: body) {
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

void TreeEvaluator::execute_body(Array<StmtNode*>& body, int depth) {
    for (StmtNode* stmt: body) {
        exec(stmt, depth);

        if (has_exceptions()) {
            break;
        }
    }
}

PartialResult* TreeEvaluator::trystmt(Try_t* n, int depth) {

    bool received_exception = false;

    for (StmtNode* stmt: n->body) {
        exec(stmt, depth);

        if (has_exceptions()) {
            received_exception = true;
            break;
        }
    }

    if (received_exception) {
        // start handling all the exceptions we received
        auto _ = HandleException(this);

        ExceptHandler const* matched          = nullptr;
        lyException*         latest_exception = exceptions[exceptions.size() - 1];

        for (ExceptHandler const& handler: n->handlers) {
            // match the exception type to the one we received

            // catch all
            if (!handler.type.has_value()) {
                matched = &handler;
                break;
            }

            // FIXME: we do not have the type at runtime!!!
            else if (equal(handler.type.value(), latest_exception->type)) {
                matched = &handler;
                break;
            }
        }

        if (matched) {
            Scope    _(bindings);
            Constant exception;

            // Execute Handler
            if (matched->name.has_value()) {
                exception.value = latest_exception->custom;
                bindings.add(matched->name.value(), &exception, nullptr);
            }

            for (StmtNode* stmt: matched->body) {
                exec(stmt, depth);

                // new exception
                if (has_exceptions()) {
                    return None();
                }

                if (return_value != nullptr) {
                    break;
                }
            }

            // Exception was handled!
            exceptions.pop_back();
            cause = nullptr;
        }
        // Exception was NOT handled
        // leave the exception as is so we continue moving back

    } else {
        for (StmtNode* stmt: n->orelse) {
            exec(stmt, depth);

            if (has_exceptions()) {
                return None();
            }

            if (return_value != nullptr) {
                break;
            }
        }
    }

    auto _ = HandleException(this);

    for (StmtNode* stmt: n->finalbody) {
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

/*
https://stackoverflow.com/questions/60926323/can-i-raise-an-exception-in-exit

manager = (EXPRESSION)
enter = type(manager).__enter__
exit = type(manager).__exit__
value = enter(manager)
hit_except = False

try:
    TARGET = value
    SUITE
except:
    hit_except = True
    if not exit(manager, *sys.exc_info()):
        raise
finally:
    if not hit_except:
        exit(manager, None, None, None)

*/
PartialResult* TreeEvaluator::with(With_t* n, int depth) {
    // Call enter
    for (auto& item: n->items) {
        auto ctx    = exec(item.context_expr, depth);
        auto result = call_enter(ctx, depth);

        if (item.optional_vars.has_value()) {
            bindings.add(StringRef(""), result, nullptr);
        }
    }

    for (StmtNode* stmt: n->body) {
        exec(stmt, depth);

        // New exceptions
        if (has_exceptions()) {
            return None();
        }
    }

    // Call exit regardless of exception status
    auto _ = HandleException(this);

    for (auto& item: n->items) {
        auto ctx = exec(item.context_expr, depth);
        call_exit(ctx, depth);
    }

    return nullptr;
}

PartialResult* TreeEvaluator::match(Match_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::import(Import_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::importfrom(ImportFrom_t* n, int depth) { return nullptr; }

PartialResult* TreeEvaluator::dictexpr(DictExpr_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::setexpr(SetExpr_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::listcomp(ListComp_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::generateexpr(GeneratorExp_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::setcomp(SetComp_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::dictcomp(DictComp_t* n, int depth) { return nullptr; }

PartialResult* TreeEvaluator::yield(Yield_t* n, int depth) {
    if (n->value.has_value()) {
        auto value = exec(n->value.value(), depth);
        // Get the top level functions and create a lambda
        yielding     = true;
        return_value = value;
        return value;
    }
    return None();
}
PartialResult* TreeEvaluator::yieldfrom(YieldFrom_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::joinedstr(JoinedStr_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::formattedvalue(FormattedValue_t* n, int depth) { return nullptr; }

PartialResult* TreeEvaluator::starred(Starred_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::listexpr(ListExpr_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::tupleexpr(TupleExpr_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::deletestmt(Delete_t* n, int depth) { return nullptr; }

PartialResult* TreeEvaluator::await(Await_t* n, int depth) { return nullptr; }

// Objects
PartialResult* TreeEvaluator::slice(Slice_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::attribute(Attribute_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::subscript(Subscript_t* n, int depth) { return nullptr; }

// Call __next__ for a given object
PartialResult* TreeEvaluator::get_next(Node* iterator, int depth) {
    // FIXME: implement me
    return nullptr;
}

PartialResult* TreeEvaluator::call_enter(Node* ctx, int depth) {
    // Call __enter__
    return nullptr;
}
PartialResult* TreeEvaluator::call_exit(Node* ctx, int depth) {
    // Call __exit__
    return nullptr;
}

// Types
// -----
PartialResult* TreeEvaluator::classdef(ClassDef_t* n, int depth) { return nullptr; }

PartialResult* TreeEvaluator::dicttype(DictType_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::arraytype(ArrayType_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::tupletype(TupleType_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::arrow(Arrow_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::classtype(ClassType_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::settype(SetType_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::builtintype(BuiltinType_t* n, int depth) {
    // return self because it also holds the native function to use
    return n;
}

// Match
// -----
PartialResult* TreeEvaluator::matchvalue(MatchValue_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::matchsingleton(MatchSingleton_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::matchsequence(MatchSequence_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::matchmapping(MatchMapping_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::matchclass(MatchClass_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::matchstar(MatchStar_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::matchas(MatchAs_t* n, int depth) { return nullptr; }
PartialResult* TreeEvaluator::matchor(MatchOr_t* n, int depth) { return nullptr; }

PartialResult* TreeEvaluator::global(Global_t* n, int depth) {
    // we don't really need it right now, we are not enforcing this
    // might be sema business anyway
    return nullptr;
}
PartialResult* TreeEvaluator::nonlocal(Nonlocal_t* n, int depth) {
    // we don't really need it right now, we are not enforcing this
    // might be sema business anyway
    return nullptr;
}

#define PRINT(msg) std::cout << msg << "\n";

void printtrace(StackTrace& trace) {
    String file   = "<input>";
    int    line   = -1;
    String parent = "<module>";
    String expr   = "";

    if (trace.stmt) {
        line   = trace.stmt->lineno;
        parent = shortprint(get_parent(trace.stmt));
        expr   = shortprint(trace.stmt);
    } else if (trace.expr) {
        line   = trace.expr->lineno;
        parent = shortprint(trace.stmt);
        expr   = shortprint(trace.stmt);
    }

    fmt::print("  File \"{}\", line {}, in {}\n", file, line, parent);
    fmt::print("    {}\n", expr);
}

PartialResult* TreeEvaluator::eval(StmtNode_t* stmt) {
    auto result = exec(stmt, 0);

    if (has_exceptions()) {
        lyException* except = exceptions[exceptions.size() - 1];

        assert(except != nullptr, "Exception is null");

        fmt::print("Traceback (most recent call last):\n");
        for (StackTrace& st: except->traces) {
            printtrace(st);
        }

        String exception_type = "AssertionError";
        String exception_msg  = "Very bad";
        fmt::print("{}: {}\n", exception_type, exception_msg);
    }

    return result;
}

}  // namespace lython