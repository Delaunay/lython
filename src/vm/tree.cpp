
#include "vm/tree.h"
#include "ast/values/exception.h"
#include "dependencies/formatter.h"
#include "dtypes.h"
#include "logging/logging.h"
#include "parser/parsing_error.h"
#include "utilities/guard.h"
#include "sema/importlib.h"

#define MAKE_NAME(type, name) ((StringStream() << type << (name)).str())

namespace lython {
template<typename T>
void pop(Array<T>& array, CodeLocation const& loc) {
    outlog().debug(
        loc,
        "Poping {} {}", (void*)&array, array.size()
    );
    array.pop_back();
}
}

#define KW_NEW_SCOPE                    \
    KW_DEFERRED([&] (std::size_t size){ \
        variables.resize(size);         \
    }, variables.size())               


#define KW_EXEC_BLOCK_BODY(body, start, dbname, tryhandler, withhandler)            \
    {                                                                               \
        auto* blocks = get_blocks();                                                \
        ExecBlock& _block = blocks->emplace_back();                                 \
        kwdebug(outlog(), "Insert block {} {}", (void*)blocks, blocks->size());     \
        _block.block      = (body);                                                 \
        _block.name       = (dbname);                                               \
        _block.exception_handler = tryhandler;                                      \
        _block.resources         = withhandler;                                     \
        _block.i = start;                                                           \
        for (int i = start; i < body.size(); i++) {                                 \
            StmtNode* stmt = body[i];                                               \
            exec(stmt, depth);                                                      \
            _block.i = i + 1;                                                       \
            if (has_exceptions()) {                                                 \
                pop(*get_blocks(), LOC);                                            \
                return flag::done();                                                \
            }                                                                       \
                                                                                    \
            if (has_returned()) {                                                   \
                if (!yielding) {                                                    \
                    pop(*get_blocks(), LOC);                                        \
                    return flag::done();                                            \
                }                                                                   \
                return flag::paused();                                              \
            }                                                                       \
        }                                                                           \
        pop(*get_blocks(), LOC);  \
    }


#define EXEC_BODY(body, start, dbname) KW_EXEC_BLOCK_BODY(body, start, dbname, nullptr, {})

struct EvaluationResult {};

struct _done {};
struct _paused {};

namespace lython {

template<typename T>
Value quickval() {
    return make_value<T>();
}

namespace flag {
    Value done() {
        return quickval<_done>();
    }

    Value paused() {
        return quickval<_paused>();
    }
}

// Value TreeEvaluator::condjump(CondJump_t* n, int depth) {
//     return Value(); 
// }


Value TreeEvaluator::execbody(Array<StmtNode*>& body, Array<StmtNode*>& newbod, int depth) {
    ExecBlock& block = get_blocks()->emplace_back();
    block.block      = newbod;
    block.name = "here";
    auto* blocks = get_blocks();

    for (int i = 0; i < body.size(); i++) {
        block.i          = i + 1;
        StmtNode* stmt   = body[i];
        Value     result = exec(stmt, depth);

        // if (result->family() == NodeFamily::Statement) {
        //     newbod.push_back((StmtNode*)(result));
        // }

        // We cannot proceed, essentially found a compile time issue
        if (has_exceptions()) {
            pop(*blocks, LOC);
            return flag::done();
        }

        // We have reached a value to return
        // check if partial or not
        // if partial then maybe we do not have sufficient info to return right away
        // if not we can return right away
        if (has_returned()) {
            if (!yielding) {
                    pop(*blocks, LOC);
                    return flag::done();
                }
                return flag::paused();
        }
    }
    pop(*blocks, LOC);
    return flag::done();
}
void TreeEvaluator::raise_exception(Value exception, Value cause) {
    // Create the exception object

    _LyException* except = root.new_object<_LyException>(traces);
    except->custom       = exception;
    except->cause        = cause;
    except->traces       = traces;

    exceptions.push_back(except);
}

Value TreeEvaluator::exported(Exported* n, int depth) { return nullptr; }

Value TreeEvaluator::compare(Compare_t* n, int depth) {

    // a and b and c and d
    //
    Value left = exec(n->left, depth);

    Array<Value> partials;
    partials.reserve(n->comparators.size());

    bool bnative   = !n->native_operator.empty();
    bool full_eval = true;
    bool result    = true;

    for (int i = 0; i < n->comparators.size(); i++) {
        Value right = exec(n->comparators[i], depth);
        partials.push_back(right);

        if (is_concrete(left) && is_concrete(right)) {
            Value value;

            if (!bnative) {
                auto KW_IDT(_) = new_scope();
                add_variable(StringRef(), left);
                add_variable(StringRef(), right);
                value = exec(n->resolved_operator[i], depth);

            } else if (bnative) {
                Function native = n->native_operator[i];
                lyassert(native, "Operator needs to be set");

                value = binary_invoke((void*)this, native, left, right);
                kwdebug(
                    treelog, "{} = {} {} {}", str(value), str(left), str(n->ops[i]), str(right));
            }

            result = result && value.as<bool>();

            // One comparison is false so the entire thing does not work
            if (!result) {
                return Value(false);
            }

            left = right;
        } else {
            full_eval = false;
        }
    }

    if (full_eval) {
        return Value(true);
    }

    return Value();

    // Compare* comp = root.new_object<Compare>();
    // comp->left    = n->left;
    // comp->ops     = n->ops;
    // comp->comparators.reserve(partials.size());

    // comp->resolved_operator = n->resolved_operator;
    // comp->native_operator   = n->native_operator;

    // for (auto* p: partials) {
    //     comp->comparators.push_back((ExprNode*)p);
    // }
    // return comp;
}

Value TreeEvaluator::boolop(BoolOp_t* n, int depth) {
    // a and b or c and d
    //
    Value first = exec(n->values[0], depth);

    Array<Value> partials;
    partials.reserve(n->values.size());
    partials.push_back(first);

    bool                            full_eval = true;
    bool                            result    = false;
    std::function<bool(bool, bool)> reduce    = [](bool a, bool b) -> bool { return a || b; };

    if (n->op == BoolOperator::And) {
        result = true;
        reduce = [](bool a, bool b) -> bool { return a && b; };
    }

    for (int i = 1; i < n->values.size(); i++) {
        Value second = exec(n->values[i], depth);
        partials.push_back(second);

        if (is_concrete(first) && is_concrete(second)) {
            Value value;

            if (n->resolved_operator != nullptr) {
                auto KW_IDT(_) = new_scope();

                add_variable(StringRef(), first);
                add_variable(StringRef(), second);
                value = exec(n->resolved_operator, depth);

            } else if (n->native_operator != nullptr) {
                value = binary_invoke((void*)this, n->native_operator, first, second);
            }

            result = reduce(result, value.as<bool>());

            // Shortcut
            if (n->op == BoolOperator::And && !result) {
                return Value(false);
            }

            if (n->op == BoolOperator::Or && result) {
                return Value(true);
            }

            first = second;
        } else {
            full_eval = false;
        }
    }

    if (result) {
        return Value(true);
    } else {
        return Value(false);
    }

    // BoolOp* boolop = root.new_object<BoolOp>();
    // boolop->op     = n->op;
    // boolop->values.reserve(partials.size());
    // boolop->resolved_operator = n->resolved_operator;
    // boolop->native_operator   = n->native_operator;

    // for (auto* p: partials) {
    //     boolop->values.push_back((ExprNode*)p);
    // }
    // return boolop;
}

Value TreeEvaluator::binop(BinOp_t* n, int depth) {

    auto lhs = exec(n->left, depth);
    auto rhs = exec(n->right, depth);

    // TODO: if they evaluate to constant that belong to the value root
    // we can free them as soon as we finish combining the values

    // We can execute the function because both arguments got resolved
    if (is_concrete(lhs) && is_concrete(rhs)) {
        Value result;

        // Execute function
        if (n->resolved_operator != nullptr) {
            auto KW_IDT(_) = new_scope();

            add_variable(StringRef(), lhs);
            add_variable(StringRef(), rhs);
            result = exec(n->resolved_operator, depth);
            return result;
        }

        else if (n->native_operator != nullptr) {
            auto r = binary_invoke((void*)this, n->native_operator, lhs, rhs);
            kwdebug(treelog, "{} = {} {} {}", str(r), str(lhs), str(n->op), str(rhs));
            return r;
        }
    }

    // // We could not execute, just return what we could execute so far
    // BinOp* binary = root.new_object<BinOp>();

    // binary->op    = n->op;
    // binary->left  = (ExprNode*)lhs;
    // binary->right = (ExprNode*)rhs;

    // binary->resolved_operator = n->resolved_operator;
    // binary->native_operator   = n->native_operator;

    // // The partial operator becomes the owner of the partial results
    // lhs->move(binary);
    // rhs->move(binary);

    // return binary;
    return nullptr;
}

Value TreeEvaluator::unaryop(UnaryOp_t* n, int depth) {
    auto operand = exec(n->operand, depth);

    // We can execute the function because both arguments got resolved
    if (is_concrete(operand)) {

        // Execute function
        if (n->resolved_operator != nullptr) {
            auto KW_IDT(_) = new_scope();

            add_variable(StringRef(), operand);
            return exec(n->resolved_operator, depth);
        }

        if (n->native_operator != nullptr) {
            return unary_invoke((void*)this, n->native_operator, operand);
        }
    }

    // We could not execute, just return what we could execute so far
    // UnaryOp* unary           = root.new_object<UnaryOp>();
    // unary->op                = n->op;
    // unary->operand           = (ExprNode*)operand;
    // unary->resolved_operator = n->resolved_operator;
    // unary->native_operator   = n->native_operator;
    // return unary;
    return nullptr;
}

Value TreeEvaluator::namedexpr(NamedExpr_t* n, int depth) {
    Value value = exec(n->value, depth);

    StringRef name;
    if (Name* name_expr = cast<Name>(n->target)) {
        name = name_expr->id;
    }

    if (is_concrete(value)) {
        add_variable(name, value);
        return value;
    }

    // TODO: do i put the evaluated expression or the partial expression ?
    // NamedExpr* expr = root.new_object<NamedExpr>();
    // expr->target    = n->target;
    // expr->value     = (ExprNode*)value;
    // bindings.add(name, value, nullptr);
    // return expr;
    return nullptr;
}

Value TreeEvaluator::lambda(Lambda_t* n, int depth) {
    auto result = exec(n->body, depth);

    if (is_concrete(result))
        return result;

    // Here we should build a new lambda
    // but we have to know which args were defined and which were not
    // we can check n->args vardid and fetch them from the context
    // if they are undefined we need to forward them
    return Value();
}

Value TreeEvaluator::ifexp(IfExp_t* n, int depth) {
    Value value = exec(n->test, depth);

    bool btrue = value.as<bool>();

    if (btrue) {
        return exec(n->body, depth);
    }

    return exec(n->orelse, depth);
}

Value TreeEvaluator::call_native(Call_t* call, FunctionDef_t* function, int depth) {
    Array<Value> args;
    StackTrace&  trace = traces[traces.size() - 1];

    Value self;
    if (auto attr = cast<Attribute>(call->func)) {
        args.reserve(call->args.size() + 1);
        self = exec(attr->value, depth);
        args.push_back(self);
    } else {
        args.reserve(call->args.size());
    }

    // Array<Constant*>      value_args;
    
    trace.args.reserve(call->args.size());

    bool compile_time = true;

    if (false) {}

    for (int i = 0; i < call->args.size(); i++) {
        Value arg = exec(call->args[i], depth);
        args.push_back(arg);

        if (!is_concrete(arg)) {
            // trace.args.push_back(value);
        }
        compile_time = compile_time && is_concrete(arg);
    }

    if (compile_time) {
        return function->native(&root, args);
        // ConstantValue result = function->native_function(&root, value_args);
        // ret_result = function->native(&root, trace.args);
        // ret_result           = root.new_object<Constant>(result);
    } else {
        // FIXME: we probably need the context here
        // ret_result = function->native_macro(&root, args);
    }

    for (Value arg: args) {
        // root.remove_child_if_parent(arg, true);
    }

    return Value();
}
Value TreeEvaluator::call_script(Call_t* call, FunctionDef_t* function, int depth) {
    auto KW_IDT(_) = new_scope();

    bool partial_call = false;

    // insert arguments to the context
    for (int i = 0; i < call->args.size(); i++) {
        Value arg = exec(call->args[i], depth);

        if (is_concrete(arg)) {
            partial_call = true;
        }

        StringRef arg_name = function->args.args[i].arg;
        add_variable(arg_name, arg);
    }

    partial.push_back(partial_call);
    EXEC_BODY(function->body, 0, MAKE_NAME("call ", str(function->name)));
    partial.pop_back();
    return returned();
}

struct ScriptObject {
    // Name - Value
    // We would like to get rid of the name if possible
    // during SEMA we can resolve the name to the ID
    // so we would never have to lookup by name
    // If some need the name at runtime this is reflection stuff
    // and that would be handled by a different datastructure
    // for now it will have to do
    //
    // Usually methods will not be stored there
    // but it can happen when the code assign method as attributes
    //
    Array<Tuple<String, Value>> attributes;

    ScriptObject(std::size_t reserve) { attributes.reserve(reserve); }
};

Value object__new__(GCObject* parent, ClassDef* class_t) {
    // Move this to sema
    ValuePrinter printer = [](std::ostream& out, Value const& val) {
        Array<int> attributes;

        auto& obj = val.as<ScriptObject const&>();
        for (int i = 0; i < obj.attributes.size(); i++) {
            if (std::get<1>(obj.attributes[i]).is_valid<Node*>()) {
                continue;
            }
            attributes.push_back(i);
        }

        int n = int(attributes.size()) - 1;
        out << "(";
        for (int i = 0; i < attributes.size(); i++) {
            auto& attr = obj.attributes[attributes[i]];
            out << std::get<0>(attr) << "=" << std::get<1>(attr);
            if (i < n) {
                out << ", ";
            }
        }
        out << ")";
    };
    register_value<ScriptObject>(printer);

    if (class_t->type_id < 0) {
        class_t->type_id = meta::_new_type();
    }
    // <<<

    // Create a new runtime object of a specific type
    auto val = make_value<ScriptObject>(class_t->attributes.size());
    ScriptObject& obj   = val.as<ScriptObject&>();

    for (int i = 0; i < class_t->attributes.size(); i++) {
        auto  attr = class_t->attributes[i];
        Value value;
        if (FunctionDef* def = cast<FunctionDef>(attr.stmt)) {
            value = make_value<Node*>(def);
        }
        obj.attributes.emplace_back(attr.name, value);
    }

    //
    // Note maybe we could save a template object and simply copy it every time
    //
    return val;
}

Value TreeEvaluator::call_constructor(Call_t* call, ClassDef_t* cls, int depth) {

    // std::cout << "wtf" << std::endl;
    FunctionDef* ctor = nullptr;
    for (auto stmt: cls->body) {
        if (FunctionDef* def = cast<FunctionDef>(stmt)) {
            if (def->name == StringRef("__init__")) {
                ctor = def;
                break;
            }
        }
    }

    if (ctor == nullptr) {
        static StringRef name("__init__");
        for (StmtNode* stmt: cls->body) {
            if (FunctionDef* def = cast<FunctionDef>(stmt)) {
                if (def->name == name) {
                    ctor = def;
                }
            }
        }

    }

    if (ctor && ctor->native) {
        Array<Value> value_args;
        value_args.reserve(call->args.size());

        bool compile_time = true;

        for (int i = 0; i < call->args.size(); i++) {
            Value arg = exec(call->args[i], depth);
            if (is_concrete(arg)) {
                value_args.push_back(arg);
            }
            compile_time = compile_time && is_concrete(arg);
        }

        return ctor->native(this, value_args);
        // Constant* ret = ctor->native(&root, value_args);
        // return ret;
    }

    // execute function
    auto KW_IDT(_) = new_scope();

    // Create the object
    Value obj = object__new__(&root, cls);

    if (ctor != nullptr) {
        add_variable(ctor->args.args[0].arg, obj);

        int i = 0;
        for (auto& arg: call->args) {
            add_variable(ctor->args.args[i + 1].arg, exec(arg, depth));
            i += 1;
        }

        for (auto& stmt: ctor->body) {
            exec(stmt, depth);

            if (has_exceptions()) {
                break;
            }
        }

        return obj;
    }

    return obj;
}

Value TreeEvaluator::make(ClassDef* class_t, Array<Value> args, int depth) {
    // TODO: this should be generated inside the SEMA
    //
    String __new__  = class_t->cls_namespace + "." + "__new__";
    String __init__ = class_t->cls_namespace + "." + "__init__";

    BindingEntry* new_fun_entry  = nullptr;  // bindings.find(StringRef(__new__));
    BindingEntry* init_fun_entry = nullptr;  //.find(StringRef(__init__));

    FunctionDef* new_fun  = cast<FunctionDef>(new_fun_entry->value);
    FunctionDef* init_fun = cast<FunctionDef>(init_fun_entry->value);

    Value self;

    if (new_fun != nullptr) {
        // Scope _(bindings);
        add_variable(StringRef("cls"), class_t);
        for (auto& arg: args) {
            add_variable(StringRef(), arg);
        }

        for (auto& stmt: new_fun->body) {
            exec(stmt, depth);

            if (has_returned()) {
                self = returned();
                break;
            }

            if (has_exceptions()) {
                break;
            }
        }
    }

    if (init_fun != nullptr) {
        // Scope _(bindings);

        add_variable(StringRef("self"), self);

        for (auto& arg: args) {
            add_variable(StringRef(), arg);
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

void TreeEvaluator::show_variables(std::ostream& out, Variables& variables) {
    for (auto var: variables) {
        StringStream ss;
        var.value.debug_print(ss);

        out << fmt::format("{:>30} - {}\n", var.name, ss.str());
    }
}

Value TreeEvaluator::make_generator(Call_t* call, FunctionDef_t* n, int depth) {
    Generator* gen = root.new_object<Generator>();
    gens.push_back(gen);

    auto KW_IDT(_) = new_scope();

    // insert arguments to the context
    for (int i = 0; i < call->args.size(); i++) {
        Value arg = exec(call->args[i], depth);

        StringRef arg_name = n->args.args[i].arg;
        add_variable(arg_name, arg);
    }

    // Save the execution state for resuming
    gen->environment = variables;
    gen->blocks      = *get_blocks();
    gen->function    = n;
    gen->blocks.push_back(ExecBlock{0, n->body, MAKE_NAME("generator ", n->name)});

    show_variables(std::cout, gen->environment);

    // Call to function that yields only create the generator
    // to fetch values from it
    // execute the body
    // EXEC_BODY(n->body, 0, MAKE_NAME("call ", str(function->name)));

    gens.pop_back();
    return make_value<Generator*>(gen);
}


Value TreeEvaluator::call(Call_t* n, int depth) {
    // Populate current stack with the expression that will branch out
    get_trace().expr = n;

    StackTrace& trace = traces.emplace_back();
    KW_DEFERRED([&]{
        kwdebug(treelog, "Stack pop {}", traces.size());
        traces.pop_back();
    });

    // fetch the function we need to call
    // the issue is that we need the object called in the case of a method
    Value function = exec(n->func, depth);

    if (function.is_valid<Function>()) {
        return Value();
    }

    if (function.is_valid<Node*>()) {
        reset();

        Node* node = function.as<Node*>();

        if (FunctionDef_t* fun = cast<FunctionDef>(node)) {
            if (fun->generator) {
                return make_generator(n, fun, depth);
            }
            if (fun->native) {
                return call_native(n, fun, depth);
            }
            return call_script(n, fun, depth);
        }

        if (ClassDef_t* cls = cast<ClassDef_t>(node)) {
            return call_constructor(n, cls, depth);
        }
    }

    /*
    if (BuiltinType_t* fun = cast<BuiltinType>(function)) {
        return call_native(n, fun, depth);
    }*/

    /*
    if (Coroutine_t* fun = cast<Coroutine>(function)) {
        return call_coroutine(n, fun, depth);
    }
    */

    // function could not be resolved at compile time
    // return self ?
    return Value();
}

Value TreeEvaluator::placeholder(Placeholder_t* n, int depth) { return nullptr; }

Value TreeEvaluator::constant(Constant_t* n, int depth) { return n->value; }

Value TreeEvaluator::comment(Comment_t* n, int depth) { return nullptr; }

Value* TreeEvaluator::fetch_name(Name_t* n, int depth) {
    lyassert(variables.size() > 0, "");
    int last = int(variables.size()) - 1;

    for (int i = last; i >= 0; i--) {
        ValuePair& entry = variables[i];
        if (n->id == entry.name) {
            return &entry.value;
        }
    }

    kwwarn(treelog, "Could not find variable");
    return nullptr;
}

Value TreeEvaluator::name(Name_t* n, int depth) {

    if (n->ctx == ExprContext::Store) {
        return add_variable(n->id, Value());
    }

    return *fetch_name(n, depth);
}

Value TreeEvaluator::functiondef(FunctionDef_t* n, int depth) {
    // this should not be called
    // return_value = nullptr;
    // EXEC_BODY(n->body, 0, MAKE_NAME("call ", str(function->name)));
    add_variable(n->name, make_value<Node*>(n));
    return flag::done();
}

Value TreeEvaluator::invalidstmt(InvalidStatement_t* n, int depth) {
    // FIXME: raise exception here
    raise_exception(nullptr, nullptr);
    return flag::done();
}

Value TreeEvaluator::returnstmt(Return_t* n, int depth) {
    kwdebug(treelog, "Compute return {}", str(n));

    if (n->value.has_value()) {
        set_return_value(exec(n->value.value(), depth));
        kwdebug(treelog, "Returning {}", str(return_value));
        return return_value;
    }

    set_return_value(Value());
    return return_value;
}

Value TreeEvaluator::assign(Assign_t* n, int depth) {
    Value value = exec(n->value, depth);

    // Unpacking a, b, c = ...
    TupleExpr* targets = nullptr;  // cast<TupleExpr>(n->targets[0]);
    TupleExpr* values  = nullptr;  // cast<TupleExpr>(value);

    if (values != nullptr && targets != nullptr) {
        lyassert(values->elts.size() == targets->elts.size(), "Size must match");

        // this probably does not work quite right in some cases
        for (int i = 0; i < values->elts.size(); i++) {
            ExprNode* target = targets->elts[i];
            StringRef name;

            // create a new variable
            if (Name* target_name = cast<Name>(target)) {
                name = target_name->id;
                add_variable(name, values->elts[i]);
            }

            // Update attrubyte
            if (Attribute* attr = cast<Attribute>(target)) {
                Value& val = fetch_attribute(attr, depth);
                val     = values->elts[i];
            }
        }
        return flag::done();
    }

    if (!n->targets.empty()) {
        auto* target = n->targets[0];

        if (Attribute* attr = cast<Attribute>(target)) {
            Value val = fetch_attribute(attr, depth);
            (val)     = value;
        }

        if (Name* name = cast<Name>(target)) {
            add_variable(name->id, value);
        }

        return flag::done();
    }

    return flag::done();
}

Value* TreeEvaluator::fetch_store_target(ExprNode* n, int depth) {
    if (Attribute* attr = cast<Attribute>(n)) {
        return &fetch_attribute(attr, depth);
    }

    if (Name* name = cast<Name>(n)) {
        return fetch_name(name, depth);
    }

    kwerror(treelog, "{} is not a valid target", str(n));
    return nullptr;
}

Value TreeEvaluator::augassign(AugAssign_t* n, int depth) {
    Value* left  = fetch_store_target(n->target, depth);  // load a
    Value  right = exec(n->value, depth);                 // load b

    if (is_concrete(left) && is_concrete(right)) {
        Value value = nullptr;

        // Execute function
        if (n->resolved_operator != nullptr) {
            auto KW_IDT(_) = new_scope();

            // Fetch the argument name from the operator
            add_variable(StringRef(), (*left));
            add_variable(StringRef(), right);
            (*left) = exec(n->resolved_operator, depth);
        } else if (n->native_operator != nullptr) {
            Value oldleft = (*left);

            (*left) = binary_invoke((void*)this, n->native_operator, oldleft, right);
            kwdebug(treelog, "{} = {} {} {}", str(*left), str(oldleft), str(n->op), str(right));
        } else {
            kwerror(treelog, "Operator does not have implementation!");
        }
        return flag::done();
    }

    // AugAssign* expr = root.new_object<AugAssign>();
    // // Do not use the evaluted target here
    // expr->target = n->target;
    // expr->op     = n->op;
    // expr->value  = (ExprNode*)right;
    // return expr;

    return flag::done();
}

Value TreeEvaluator::annassign(AnnAssign_t* n, int depth) {
    Value value = Value();
    if (n->value.has_value()) {
        value = exec(n->value.value(), depth);
    }

    StringRef name;
    if (Name* node_name = cast<Name>(n->target)) {
        name = node_name->id;
    }

    add_variable(name, value);
    return flag::done();
}

StringRef TreeEvaluator::get_name(ExprNode* expression) {
    if (Name* name = cast<Name>(expression)) {
        return name->id;
    }
    return StringRef();
}

Value TreeEvaluator::forstmt(For_t* n, int depth) {

    // insert target into the context
    // exec(n->target, depth);
    StringRef target_name = get_name(n->target);
    int       value_idx   = int(variables.size());
    Value*    target      = add_variable(target_name, Value());

    Value iterator = exec(n->iter, depth);

    bool broke    = false;
    loop_break    = false;
    loop_continue = false;

    while (true) {
        // Python does not create a new scope for `for`
        // auto KW_IDT(_) = new_scope();

        // Get the value of the iterator
        // (*target) = get_next(iterator, depth);

        variables[value_idx].value = get_next(iterator, depth);

        // Technically here we would catch StopIteration
        if (is<_done>(variables[value_idx].value)) {
            break;
        }

        show_variables(std::cout, variables);

        for (StmtNode* stmt: n->body) {
            exec(stmt, depth);

            if (has_exceptions()) {
                return flag::done();
            }

            if (has_returned()) {
                if (yielding) {
                    return flag::paused();
                }
                return flag::done();
            }

            if (loop_break) {
                broke = true;
                break;
            }

            if (loop_continue) {
                break;
            }
        }

        loop_break    = false;
        loop_continue = false;

        if (broke) {
            break;
        }
    }

    EXEC_BODY(n->orelse, 0, MAKE_NAME("for ", "orlese"));
    return flag::done();
}
Value TreeEvaluator::whilestmt(While_t* n, int depth) {

    bool broke    = false;
    loop_break    = false;
    loop_continue = false;

    while (true) {
        Value value     = exec(n->test, depth);
        bool  bcontinue = value.as<bool>();

        if (!bcontinue || broke) {
            break;
        }

        auto*      blocks = get_blocks();
        ExecBlock& _block = blocks->emplace_back();
        _block.block      = n->body;
        _block.name       = "while body";
        kwdebug(outlog(), "Insert block {} {} + 1", (void*)blocks, blocks->size());

        _block.i = 0;
        for (StmtNode* stmt: n->body) {
            exec(stmt, depth);
            _block.i += 1;

            if (has_exceptions()) {
                pop(*blocks, LOC);
                return Value();
            }

            if (has_returned()) {
                if (!yielding) {
                    pop(*blocks, LOC);
                    return flag::done();
                }
                return flag::paused();
            }

            if (loop_break) {
                broke = true;
                break;
            }

            if (loop_continue) {
                break;
            }
        }
        pop(*blocks, LOC);

        // reset
        loop_break    = false;
        loop_continue = false;

        if (broke) {
            break;
        }
    }

    EXEC_BODY(n->orelse, 0, MAKE_NAME("while ", "orelse"));
    return flag::done();
}

Value TreeEvaluator::ifstmt(If_t* n, int depth) {

    Array<StmtNode*>& body = n->orelse;
    // Chained
    if (!n->tests.empty()) {
        for (int i = 0; i < n->tests.size(); i++) {
            Value value = exec(n->test, depth);

            bool btrue = value.as<bool>();
            if (btrue) {
                body = n->bodies[i];
                break;
            }
        }

        EXEC_BODY(body, 0, MAKE_NAME("if ", "body"));

        return flag::done();
    }

    // Simple
    Value test  = exec(n->test, depth);
    bool  btrue = test.as<bool>();

    // Skip the test on resume
    // ic() += 1;

    if (btrue) {
        body = n->body;
    }

    EXEC_BODY(body, 0, MAKE_NAME("if ", "body"));
    return flag::done();
}

Value assert_error(Value message) {
    auto v        = make_value<ScriptObject>(2);
    ScriptObject& self = v.as<ScriptObject&>();

    auto t = make_value<String>("AssertionError");

    self.attributes.emplace_back("type", t);
    self.attributes.emplace_back("message", message);
    return v;
}

Value TreeEvaluator::assertstmt(Assert_t* n, int depth) {
    Value btest = exec(n->test, depth);

    if (is_concrete(btest)) {
        if (!btest.as<bool>()) {
            Value msg;
            if (n->msg.has_value()) {
                msg = exec(n->msg.value(), depth);
            }
            raise_exception(assert_error(msg), Value());
            return flag::done();
        }

        // All Good
        return flag::done();
    }

    // Assert* expr = root.new_object<Assert>();
    // expr->test   = (ExprNode*)btest;
    // expr->msg    = n->msg;
    // return expr;
    return nullptr;
}

Value TreeEvaluator::exprstmt(Expr_t* n, int depth) { return exec(n->value, depth); }
Value TreeEvaluator::pass(Pass_t* n, int depth) {
    // return itself in case the pass is required for correctness
    return n;
}
Value TreeEvaluator::breakstmt(Break_t* n, int depth) {
    loop_break = true;
    return n;
}
Value TreeEvaluator::continuestmt(Continue_t* n, int depth) {
    loop_continue = true;
    return n;
}

Value TreeEvaluator::inlinestmt(Inline_t* n, int depth) {

    EXEC_BODY(n->body, 0, MAKE_NAME("inline ", "body"));

    return flag::done();
}

Value TreeEvaluator::raise(Raise_t* n, int depth) {

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

Value TreeEvaluator::except(Try_t* n, int depth) {
    if (has_exceptions()) {
        // start handling all the exceptions we received
        auto _ = HandleException(this);

        ExceptHandler const* matched          = nullptr;
        _LyException*        latest_exception = exceptions[exceptions.size() - 1];

        for (ExceptHandler const& handler: n->handlers) {
            // match the exception type to the one we received

            bool found_matcher = false;

            // catch all
            if (!handler.type.has_value()) {
                matched       = &handler;
                found_matcher = true;
            }

            // FIXME: we do not have the type at runtime!!!
            else if (equal(handler.type.value(), latest_exception->type)) {
                matched       = &handler;
                found_matcher = true;
            }

            if (found_matcher) {
                break;
            }
        }

        if (matched != nullptr) {
            // Scope    _(bindings);
            Constant exception;

            // Execute Handler
            if (matched->name.has_value()) {
                exception.value = latest_exception->custom;
                add_variable(matched->name.value(), &exception);
            }

            EXEC_BODY(matched->body, 0, MAKE_NAME("match ", "body"));

            // Exception was handled!
            exceptions.pop_back();
            cause = nullptr;
        }
        // Exception was NOT handled
        // leave the exception as is so we continue moving back

    } else {
        EXEC_BODY(n->orelse, 0, MAKE_NAME("match ", "orelse"));
    }

    auto _ = HandleException(this);

    {   //
        EXEC_BODY(n->finalbody, 0, MAKE_NAME("match ", "finalbody")); 
        //
    }
    // we are not handling exception anymore
    handling_exceptions = 0;
    return flag::done();
}

Value TreeEvaluator::trystmt(Try_t* n, int depth) {
    // FIXME: how do I pause/resume exception handling ?
    // the block needs to be aware of the exceptions

    {   //
        KW_EXEC_BLOCK_BODY(n->body, 0, MAKE_NAME("trystmt ", "body"), n, {}); 
        //    
    }

    if (!yielding) {
        except(n, depth);
    }

    return Value();
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
    if not exit(manager, *sys.exc_kwinfo()):
        raise
finally:
    if not hit_except:
        exit(manager, None, None, None)

*/
Value TreeEvaluator::with_exit(With_t* n, Array<Value>& contexts, int depth) {
    // Call exit regardless of exception status
    auto _ = HandleException(this);

    for (Value& val: contexts) {
        call_exit(val, depth);
    }
    return flag::done();
}

Value TreeEvaluator::with(With_t* n, int depth) {
    // Call enter
    Array<Value> contexts;

    for (auto& item: n->items) {
        auto ctx = exec(item.context_expr, depth);
        contexts.push_back(ctx);

        auto result = call_enter(ctx, depth);

        StringRef name;
        if (item.optional_vars.has_value()) {
            // name = item.options_vars.value();
        }

        add_variable(name, result);
    }

    KW_EXEC_BLOCK_BODY(n->body, 0, MAKE_NAME("with ", "body"), nullptr, contexts);

    if (!yielding) {
        with_exit(n, contexts, depth);
    }
    return flag::done();
}

Value TreeEvaluator::match(Match_t* n, int depth) {
    //
    return flag::done();
}
Value TreeEvaluator::import(Import_t* n, int depth) {
    //
    for (Alias const& name: n->names) {
        StringRef binding_name = name.name;
        if (name.asname.has_value()) {
            binding_name = name.asname.value();
        }
        add_variable(binding_name, Value());
    }
    return flag::done();
}
Value TreeEvaluator::importfrom(ImportFrom_t* n, int depth) {
    //
    ImportLib* importsys = ImportLib::instance();
    ImportLib::ImportedLib* imported = nullptr;

    if (n->module.has_value()) {
        imported = importsys->importfile(n->module.value());
    }

    if (imported) {
        for (Alias const& name: n->names) {
            StringRef binding_name = name.name;
            StmtNode* val = find(imported->mod->body, binding_name);
            
            if (name.asname.has_value()) {
                binding_name = name.asname.value();
            }
            add_variable(binding_name, make_value<Node*>(val));
        }
    }
    return flag::done();
}

Value TreeEvaluator::dictexpr(DictExpr_t* n, int depth) { 


    return nullptr; 
}
Value TreeEvaluator::setexpr(SetExpr_t* n, int depth) { 
    return nullptr; 
}
Value TreeEvaluator::listcomp(ListComp_t* n, int depth) { return nullptr; }
Value TreeEvaluator::generateexpr(GeneratorExp_t* n, int depth) { return nullptr; }
Value TreeEvaluator::setcomp(SetComp_t* n, int depth) { return nullptr; }
Value TreeEvaluator::dictcomp(DictComp_t* n, int depth) { return nullptr; }

Value TreeEvaluator::yield(Yield_t* n, int depth) {
    if (n->value.has_value()) {
        auto value = exec(n->value.value(), depth);

        Generator* gen   = (*gens.rbegin());
        gen->environment = variables;
        gen->blocks      = *get_blocks();

        // Get the top level functions and create a lambda
        yielding     = true;
        return_value = value;
        return flag::done();
    }
    return flag::done();
}
Value TreeEvaluator::yieldfrom(YieldFrom_t* n, int depth) { return nullptr; }
Value TreeEvaluator::joinedstr(JoinedStr_t* n, int depth) { return nullptr; }
Value TreeEvaluator::formattedvalue(FormattedValue_t* n, int depth) { return nullptr; }

Value TreeEvaluator::starred(Starred_t* n, int depth) { return nullptr; }
Value TreeEvaluator::listexpr(ListExpr_t* n, int depth) { return nullptr; }
Value TreeEvaluator::tupleexpr(TupleExpr_t* n, int depth) { return nullptr; }
Value TreeEvaluator::deletestmt(Delete_t* n, int depth) { return nullptr; }

Value TreeEvaluator::await(Await_t* n, int depth) { return nullptr; }

// Objects
Value TreeEvaluator::slice(Slice_t* n, int depth) { return nullptr; }

Value& TreeEvaluator::fetch_attribute(Attribute_t* n, int depth) {
    Value obj = exec(n->value, depth);

    meta::ClassMetadata const& meta = meta::classmeta(obj.tag);

    int member_id = -1;
    int i = 0;
    for(meta::Property const& member: meta.members) {
        if (StringRef(member.name.c_str()) == n->attr) {
            member_id = i;
            break;
        }
        i += 1;
    }

    if (i >= 0) {
        Value property;
        meta::Property const& member = meta.members[member_id];
        int type = member.type;
        int refid = meta::classmeta(type).weakref_type_id;

        property.tag = refid;
        if (meta.size <= sizeof(Value::Holder) && meta.is_trivially_copyable) {
            property.value.obj = ((char*)&obj) + member.offset;
        }
        return property;
    }

    kwassert(obj.tag == meta::type_id<ScriptObject>(), "Attribute should be an object");
    ScriptObject&         dat  = obj.as<ScriptObject&>();

    if (dat.attributes.size() > 0) {
        Tuple<String, Value>& attr = dat.attributes[n->attrid];
        return std::get<1>(attr);
    }

    static Value invalid;
    return invalid;
}

Value TreeEvaluator::attribute(Attribute_t* n, int depth) { 
    Value& val = fetch_attribute(n, depth);

    if (val.tag != meta::type_id<_Invalid>()) {
        return val;
    }

    kwdebug(treelog, "Attribute not found {}", n->attr);
    return Value(_None());
}
Value TreeEvaluator::subscript(Subscript_t* n, int depth) { return nullptr; }

// Call __next__ for a given object
Value TreeEvaluator::get_next(Value v, int depth) {
    if (v.tag == meta::type_id<Generator*>()) {
        return resume(v.as<Generator*>(), depth);
    }
    // FIXME: implement me
    return Value();
}

Value TreeEvaluator::call_enter(Value ctx, int depth) {
    // Call __enter__
    return nullptr;
}
Value TreeEvaluator::call_exit(Value ctx, int depth) {
    // Call __exit__
    return nullptr;
}

// Types
// -----
Value TreeEvaluator::classdef(ClassDef_t* n, int depth) {
    auto v = make_value<Node*>(n);
    add_variable(n->name, v);
    return v;
}

Value TreeEvaluator::dicttype(DictType_t* n, int depth) { return nullptr; }
Value TreeEvaluator::arraytype(ArrayType_t* n, int depth) { return nullptr; }
Value TreeEvaluator::tupletype(TupleType_t* n, int depth) { return nullptr; }
Value TreeEvaluator::arrow(Arrow_t* n, int depth) { return nullptr; }
Value TreeEvaluator::classtype(ClassType_t* n, int depth) { return nullptr; }
Value TreeEvaluator::settype(SetType_t* n, int depth) { return nullptr; }
Value TreeEvaluator::builtintype(BuiltinType_t* n, int depth) {
    // return self because it also holds the native function to use
    return n;
}

// Match
// -----
Value TreeEvaluator::matchvalue(MatchValue_t* n, int depth) { return nullptr; }
Value TreeEvaluator::matchsingleton(MatchSingleton_t* n, int depth) { return nullptr; }
Value TreeEvaluator::matchsequence(MatchSequence_t* n, int depth) { return nullptr; }
Value TreeEvaluator::matchmapping(MatchMapping_t* n, int depth) { return nullptr; }
Value TreeEvaluator::matchclass(MatchClass_t* n, int depth) { return nullptr; }
Value TreeEvaluator::matchstar(MatchStar_t* n, int depth) { return nullptr; }
Value TreeEvaluator::matchas(MatchAs_t* n, int depth) { return nullptr; }
Value TreeEvaluator::matchor(MatchOr_t* n, int depth) { return nullptr; }

Value TreeEvaluator::global(Global_t* n, int depth) {
    // we don't really need it right now, we are not enforcing this
    // might be sema business anyway
    return nullptr;
}
Value TreeEvaluator::nonlocal(Nonlocal_t* n, int depth) {
    // we don't really need it right now, we are not enforcing this
    // might be sema business anyway
    return nullptr;
}

#define PRINT(msg) std::cout << (msg) << "\n";

void printkwtrace(std::ostream& out, StackTrace const& trace) {
    String file   = "<input>";
    int    line   = -1;
    String parent = "<module>";
    String expr;

    if (trace.stmt != nullptr) {
        line   = trace.stmt->lineno;
        parent = shortprint(get_parent(trace.stmt));
        expr   = shortprint(trace.stmt);
    } else if (trace.expr != nullptr) {
        line   = trace.expr->lineno;
        parent = shortprint(trace.stmt);
        expr   = shortprint(trace.stmt);
    }

    fmt::print(out, "  File \"{}\", line {}, in {}\n", file, line, parent);
    fmt::print(out, "    {}\n", expr);
}

Value TreeEvaluator::module(Module* stmt, int depth) {
    for (auto* stmt: stmt->body) {
        exec(stmt, depth);
    }
    return Value();
}

Value TreeEvaluator::eval(StmtNode_t* stmt) {
    Value result = exec(stmt, 0);

    if (has_exceptions()) {
        _LyException* except = exceptions[exceptions.size() - 1];

        ValuePrinter printer = [](std::ostream& out, Value const& error) {
            _LyException const* except = error.as<_LyException const*>();

            lyassert(except != nullptr, "Exception is null");
            fmt::print(out, "Traceback (most recent call last):\n");
            for (StackTrace const& st: except->traces) {
                printkwtrace(out, st);
            }

            String exception_type = str(except->type);
            String exception_msg  = "";

            if (except->custom.is_valid<ScriptObject const&>()) {
                ScriptObject const& obj = except->custom.as<ScriptObject const&>();
                exception_type          = std::get<1>(obj.attributes[0]).as<String>();
                exception_msg           = std::get<1>(obj.attributes[1]).as<String>();
            }

            fmt::print(out, "{}: {}\n", exception_type, exception_msg);
        };

        register_value<_LyException*>(printer);
        auto v = make_value<_LyException*>(except);
        return v;
    }

    // return_value;
    return result;
}

int TreeEvaluator::eval() {
    StmtNode* __init__ = nullptr;

    // exec()
    // auto n    = bindings.bindings.size();
    // auto last = bindings.bindings[n - 1];

    // auto call = last.value->new_object<Call>();
    // auto name = last.value->new_object<Name>();

    // name->id = "__init__";
    // // name->varid = bindings.get_varid(StringRef("__init__"));
    // call->func = name;

    // exec(call, 0);

    // cast<FunctionDef>(last.value);

    // for(auto& b: bindings.bindings) {
    //     kwdebug("eval {}", str(b.value));

    // switch(b.value->family()) {
    //     case NodeFamily::Expression:    exec(cast<ExprNode_t>(b.value), 0);
    //     case NodeFamily::Statement:     exec(cast<StmtNode_t>(b.value), 0);
    //     // case NodeFamily::Module:        exec(cast<ModNode_t>(b.value, 0));
    // }
    // }
    return 0;
}

Value TreeEvaluator::resume(Generator* n, int depth) {
    // Save current state
    Variables previous;
    std::swap(variables, previous);

    // Restore generator state
    std::swap(variables, n->environment);

    int   finished_block = 0;
    Value result;
    {
        using TraceGuard = PopGuard<Array<StackTrace>, StackTrace>;

        // Insert a call
        TraceGuard  _(traces);
        StackTrace& trace = traces.emplace_back();
        trace.blocks      = n->blocks;

        gens.emplace_back(n);

        //kwdebug(treelog, "Resuming generator");
        //show_variables(std::cout, variables);

        auto execbloc = [&]() -> Value {
            Array<ExecBlock>& blocks = trace.blocks;

            for (int k = int(blocks.size()) - 1; k >= 0; k--) {
                ExecBlock& block = blocks[k];
                auto& body = block.block;

                kwdebug(treelog, "Resume {} at {}", block.name, block.i);

                if (!has_exceptions()) {
                    for (int i = block.i; i < body.size(); i++) {
                        StmtNode* stmt = body[i];
                        Value flag = exec(stmt, depth);

                        // We cannot always increase like this
                        // if stmt is a while loop, the while might not be done
                        block.i = i + int(flag.tag != meta::type_id<_paused>());

                        if (has_exceptions()) {
                            // we should probably break here and
                            // go through all the blocks looking for an exception handler
                            // and/or resources to free
                            break;
                        }
                        if (has_returned()) {
                            // n->environment = variables;
                            n->blocks      = *get_blocks();
                            
                            //
                            // So yield does it for us
                            // BUT when yield save the blocks it has not advanced past the yield
                            // so when resuming it should jump past it
                            //
                            // So this does not work because it does not handle nested blocks
                            // n->blocks[int(n->blocks.size()) -  1].i += 1;
                            return returned();
                        }
                    }
                }

                // try block
                if (block.exception_handler != nullptr) {
                    except(block.exception_handler, depth);
                }

                // with block
                if (!block.resources.empty()) {
                    with_exit(nullptr, block.resources, depth);
                }
                pop(blocks, LOC);
            }

            // Technically in python we would raise StopIteration
            return flag::done();
        };

        result = execbloc();

        // result = fun();
        StringStream ss;
        result.debug_print(ss);
        kwdebug(treelog, "Yielded {}", ss.str());

        // if (finished_block > 0) {
        //    n->blocks.erase(n->blocks.begin() + k, n->blocks.end());
        ///}
        gens.pop_back();
    }

    show_variables(std::cout, variables);

    yielding = false;
    return_value = Value();

    // Restore state
    std::swap(variables, previous);
    return result;
}


}  // namespace lython
