#include "interpreter.h"
#include "ast/visitor.h"

#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

namespace lython {

Value builtin_sin(State& args);
Value builtin_max(State& args);
Value builtin_div(State& args);
Value builtin_mult(State& args);

// Execute function upon destruction
template <typename Exit>
struct Guard{
    Guard(Exit fun):
        on_exit(fun)
    {}

    ~Guard(){
        on_exit();
    }

    Exit  on_exit;
};

template <typename Exit>
Guard<Exit> guard(Exit fun){
    return Guard(fun);
}


#define RESULT(X) (std::make_tuple(X, &main))

struct InterpreterImpl: public ConstVisitor<InterpreterImpl, Value>{
    // Current environment
    State* env = nullptr;
    std::size_t env_switch = 0;

    // Keep track of all the dependencies we load
    List<State> module;

    //! Builtin function defined in C++
    Dict<String, Value::BuiltinImpl> builtins = {
        {"max", builtin_max},
        {"sin", builtin_sin},
        {"/", builtin_div},
        {"*", builtin_mult},
    };

    // Eval a module to generate an execution context
    State* eval_module(Module const& m) {
        State& state = module.emplace_back();

        for(int i = 0; i < m.size(); ++i){
            debug("{}", m.get_name(i).c_str());
            Expression exp = m[i];
            auto v = eval(exp);

            state.push(v, m.get_name(i));
        }

        return &state;
    }

    void push(Value v, String const& name = ""){
        env->push(v, name);
    }

    void pop(std::ptrdiff_t n){
        env->pop(n);
    }

    InterpreterImpl(Module& m){
        // Eval the main module
        env = eval_module(m);
        env->dump(std::cout);
    }

    Value eval(Expression const expr, std::size_t d=0){
        return visit(expr, d);
    }

    Value imported_expression(ImportedExpr_t imp, std::size_t d){
        assert(imp->import.kind() == AST::NodeKind::KImport, "Need to be an import");
        auto package = imp->import.ref<AST::Import>();

        State* old_env = env;

        // Save module by path
        // package->module_path();
        env = eval_module(package->module);

        // fetch
        if (!imp->ref){
            return Value("<nullptr>");
        }

        auto v = eval(imp->ref, d + 1);
        env = old_env;

        return v;
    }

    Value import(Import_t import, size_t){
        // this should not be called
        return  String("import_error");
    }

    Value undefined(Node_t, std::size_t){
        return Value("undefined");
    }

    Value parameter(Parameter_t, std::size_t) {
        return Value("parameter");
    }

    Value unary(UnaryOperator_t, size_t) {
        return Value("unary");
    }

    Value binary(BinaryOperator_t bin, std::size_t d) {
        trace_start(d, "{} {} {}", bin->lhs, bin->op, bin->rhs);

        // StringDatabase::instance().report(std::cout);

        // retrieval attribute
        if (bin->op == get_string(".")){
            auto obj = eval(bin->lhs, d);
            debug("retrieve attribute {}", obj);
            assert(obj.tag == ValueKind::obj_object, "retrieve attribute");
            value::Struct& data = *obj.get<value::Struct*>();
            return data.get_attributes(bin->rhs.ref<AST::Ref>()->name);
        }

        // assignment
        if (bin->op == get_string("=")){
            switch(bin->lhs.kind()){
            // assign to variable
            case AST::NodeKind::KReference: {
                debug("Assign to a Reference");

                auto rhs = eval(bin->rhs, d);
                push(rhs, bin->lhs.ref<AST::Ref>()->name.str());
                return Value("none");
            }
            // assign to object
            case AST::NodeKind::KBinaryOperator: {
                debug("Assign to an object attribute");

                auto new_value = eval(bin->rhs, d);

                AST::BinaryOperator const* attr = bin->lhs.ref<AST::BinaryOperator>();
                assert(attr->op == get_string("."), "Expected getattr operator");

                // fetch the p in p.x
                auto obj = eval(attr->lhs, d);
                auto attr_name = attr->rhs.ref<AST::Ref>()->name;
                assert(obj.tag == ValueKind::obj_object, "Assign to an object");

                // set x to lhs
                value::Struct& data = *obj.get<value::Struct*>();
                data.set_attribute(attr_name, new_value);

                // dump_env(std::cout);
                return Value("none");
            }

            default:
                return Value("Unsupported");
            }
        } else {
            auto lhs = eval(bin->lhs, d);
            auto rhs = eval(bin->rhs, d);
        }

        return Value("binary");
    }

    Value sequential(SeqBlock_t seq, std::size_t depth) {
        trace_start(depth, "seq_block");

        size_t n = size_t(std::max(int(seq->blocks.size()) - 1, 0));

        for(size_t i = 0; i < n; ++i)
            eval(seq->blocks[i], depth + 1);

        // returns last line of sequential block
        return eval(seq->blocks[n], depth);
    }

    Value unparsed(UnparsedBlock_t, std::size_t) {
        return Value("unparsed");
    }

    Value statement(Statement_t stmt, std::size_t depth) {
        trace_start(depth, "{} {}", stmt->statement, stmt->expr);
        return eval(stmt->expr, depth);
    }

    Value reference(Reference_t ref, std::size_t depth) {
        trace_start(depth, "{}: {}, length={} | size={}",
                    ref->name,
                    ref->index,
                    ref->length,
                    env->size());
        return (*env)[ref->index];
    }

    Value builtin(Builtin_t blt, std::size_t depth) {
        trace_start(depth, "{}", blt->name);
        auto fun = builtins[blt->name.str()];
        return Value(fun);
    }

    Value value(Value_t val, std::size_t depth) {
        trace_start(depth, "value {}", val->value);
        return val->value;
    }

    Value struct_type(Struct_t cstruct, std::size_t) {
        return Value(cstruct);
    }

    Value call(Call_t call, std::size_t depth) {
        trace_start(depth, "{}", call->function);

        /*/ Restore previous environment
        State* old_env = env;
        auto restore_env = guard([&](){
            if (env != old_env){
                env_switch += 1;
            }
            env = old_env;
        }); */

        // This might load a custom environment
        // (if function is an imported expression)
        Value closure = eval(call->function, depth);

        switch (closure.tag) {
        // standard function
        case ValueKind::obj_closure: return fun_call(closure, call, depth);

        // Calling a struct type
        case ValueKind::obj_class: return struct_call(closure, call, depth);

        default: return closure;
        }
    }

    Value struct_call(Value closure, Call_t call, std::size_t depth){
        trace_start(depth, "struct constructor");

        AST::Struct const* cstruct = closure.get<value::Class*>()->fun;
        Value v = new_object(cstruct);
        value::Struct& data = *v.get<value::Struct*>();

        assert(call->arguments.size() == cstruct->attributes.size(),
               "arguments should match attributes");

        // for all positional arguments
        auto n = call->arguments.size();

        for(auto i = 0u; i < n; ++i){
            data.set_attribute(i, eval(call->arguments[i], depth));
        }

        for(auto& item: call->kwargs){
           data.set_attribute(item.first, eval(item.second, depth));
        }

        return v;
    }

    Value fun_call(Value closure, Call_t call, std::size_t depth){
        trace_start(depth, "fun_call");

        // Clean arguments from the exec environment
        auto on = env->size();
        auto clean_env = guard([&]{
            pop(on);
        });

        for (auto& expr: call->arguments)
            push(eval(expr, depth));

        return eval_closure(closure, depth);

    }

    Value arrow(Arrow_t aw, std::size_t d) {
        // this should not be called at runtime
        // Note that there is a compile time interpreter running as well
        return Value("arrow");
    }

    Value type(Type_t type, std::size_t) {
        return Value(type->name);
    }

    Value function(Function_t fun, std::size_t) {
        // make a closure out a function
        return Value(fun);
    }

    // Helpers
    // -------
    Value eval_closure(Value fun, std::size_t depth){
        trace_start(depth, "{}", fun);

        if (fun.tag == ValueKind::obj_closure){
            value::Closure* clo = fun.get<value::Closure*>();

            if (!clo->fun){
                assert(clo->builtin, "Closure is undefined");
                auto v = clo->builtin(*env);
                return v;
            } else {
                const AST::Function* fun_decl = fun.get<value::Closure*>()->fun;
                assert(fun_decl, "fun should be defined");
                return eval(fun_decl->body, depth);
            }
        }


        return Value("NotImplemented");
    }
};

Interpreter::Interpreter(Module& m):
    ptr(new InterpreterImpl(m))
{}

Interpreter::~Interpreter(){
    delete ptr;
}

Value Interpreter::eval(Expression const expr){
    return ptr->eval(expr);
}

Value builtin_sin(State& args){
    auto v = args[1].get<float64>();

    // std::cout << "sin(" << v << ")";
    return std::sin(v);
}


Value builtin_max(State& args){
    debug("calling max");
    auto a = args[2];
    auto b = args[1];

    return std::max(a.get<float64>(), b.get<float64>());
}


Value builtin_div(State& args){
    auto a = args[2].get<float64>();
    auto b = args[1].get<float64>();

    //std::cout << "div(" << a << ", " << b << ")";
    return a / b;
}


Value builtin_mult(State& args){
    auto a = args[2].get<float64>();
    auto b = args[1].get<float64>();

    //std::cout << "mult(" << a << ", " << b << ")";
    return a * b;
}

}
