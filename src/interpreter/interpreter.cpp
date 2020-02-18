#include "interpreter.h"
#include "ast/visitor.h"

namespace lython {

Value builtin_sin(Array<Value>& args);
Value builtin_max(Array<Value>& args);
Value builtin_div(Array<Value>& args);
Value builtin_mult(Array<Value>& args);


struct InterpreterImpl: public ConstVisitor<InterpreterImpl, Value>{
    //! Execution environment
    Array<Value> env;

    //! Builtin function defined in C++
    Dict<String, Value::BuiltinImpl> builtins = {
        {"max", builtin_max},
        {"sin", builtin_sin},
        {"/", builtin_div},
        {"*", builtin_mult},
    };

    InterpreterImpl(Module& m){
        // Eval the module en create the environment for the interpreter
        for(int i = 0; i < m.size(); ++i){
            debug("{}", m.get_name(i).c_str());
            Expression exp = m[i];
            auto v = eval(exp);
            env.push_back(v);
        }
        debug("Module evaluated (env: {})", env.size());

        debug("Dumping env");
        for(auto& val: env){
            std::cout << val.str() << '\n';
        }
        debug("---");
    }

    Value eval(Expression const expr, std::size_t d=0){
        return visit(expr, d);
    }

    Array<Value> eval(Array<Expression> const & exprs, size_t depth){
        trace_start(depth, "eval_args");

        Array<Value> values;
        values.reserve(exprs.size());

        for (auto& expr: exprs)
            values.push_back(eval(expr, depth + 1));

        return values;
    }

    Value undefined(Node_t, std::size_t){
        return Value("undefined");
    }

    Value parameter(Parameter_t, std::size_t) {
        return Value("parameter");
    }

    Value unary(UnaryOperator_t, size_t d) {
        return Value("unary");
    }

    Value binary(BinaryOperator_t, std::size_t d) {
        return Value("binary");
    }

    Value sequential(SeqBlock_t seq, std::size_t depth) {
        trace_start(depth, "seq_block");

        size_t n = size_t(std::max(int(seq->blocks.size()) - 1, 0));

        for(size_t i = 0; i < n; ++i)
            eval(seq->blocks[i], depth + 1);

        // returns last line of sequential block
        return eval(seq->blocks[n], depth + 1);
    }

    Value unparsed(UnparsedBlock_t, std::size_t) {
        return Value("unparsed");
    }

    Value reverse_polish(ReversePolish_t rev, std::size_t d) {
        auto iter = std::begin(rev->stack);
        return eval_rpe(iter, d);
    }

    Value statement(Statement_t stmt, std::size_t depth) {
        trace_start(depth, "%d", stmt->statement);
        return eval(stmt->expr, depth);
    }

    Value reference(Reference_t ref, std::size_t depth) {
        trace_start(depth, "{}: {}, {} | {}",
                    ref->name,
                    ref->index,
                    ref->length,
                    env.size());

        assert(env.size() > ref->index && "Environment should hold the ref");
        auto n = ref->index;

        debug("found {}", env[n].str());
        return env[n];
    }

    Value builtin(Builtin_t blt, std::size_t depth) {
        trace_start(depth, "{}", blt->name);
        auto fun = builtins[blt->name.str()];
        return Value(fun, Array<Value>());
    }

    Value type(Type_t type, std::size_t) {
        return Value(type->name);
    }

    Value value(Value_t val, std::size_t depth) {
        trace_start(depth, "value");
        return val->value;
    }

    Value struct_type(Struct_t cstruct, std::size_t) {
        return Value(cstruct);
    }

    Value call(Call_t call, std::size_t depth) {
        trace_start(depth, "call");
        Value closure = eval(call->function, depth + 1);
        switch (closure.tag) {
        // standard function
        case ValueKind::obj_closure: return fun_call(closure, call, depth);

        // Calling a struct type
        case ValueKind::obj_object: return struct_call(closure, call, depth);
        }

        return Value("Unsupported");
    }

    Value struct_call(Value closure, Call_t call, std::size_t depth){
        Value v = new_object();
        value::Struct& data = *v.get<value::Struct*>();

        const AST::Struct* cstruct = closure.get<value::Class*>()->fun;

        assert(call->arguments.size() == cstruct->attributes.size()
               && "arguments should match attributes");

        // for all positional arguments
        auto n = call->arguments.size();
        for(auto i = 0u; i < n; ++i){
            StringRef name = std::get<0>((*cstruct).attributes[i]);
            data.attributes[name] = eval(call->arguments[i], depth);
        }

        for(auto& item: call->kwargs){
            data.attributes[item.first] = eval(item.second, depth);
        }

        // v.v_object.attributes;
        return v;
    }

    Value fun_call(Value closure, Call_t call, std::size_t depth){
        auto on = env.size();
        for (auto& expr: call->arguments)
            env.push_back(eval(expr, depth + 1));

        // clo.env = eval(call->arguments(), depth);
        // clo.env.insert(std::begin(clo.env), Value(0));

        const AST::Function* fun = closure.get<value::Closure*>()->fun;

        //auto old = env;
        //env = closure.v_closure.env;
        Value returned_value = eval(fun->body, depth);
        // env = old;

        env.erase(env.begin() + on, env.end());
        // returned_value.print(std::cout) << std::endl;
        // throw InterpreterException("Not Implemented");
        return returned_value;
    }

    Value arrow(Arrow_t aw, std::size_t d) {
        return Value("arrow");
    }

    Value function(Function_t fun, std::size_t d) {
        return Value(fun, env);
    }

    // Helpers
    // -------
    Value closure(Value fun, std::size_t depth){
        trace_start(depth, "closure");
        if (fun.tag == ValueKind::obj_closure){
            value::Closure* clo = fun.get<value::Closure*>();

            if (!clo->fun){
                auto v = clo->builtin(env);
                return v;
            }
        }

        return Value("closure");
    }

    Value eval_rpe(Stack<AST::MathNode>::ConstIterator &iter, size_t depth){
        AST::MathNode op = *iter;
        iter++;

        switch (op.kind){
        case AST::MathKind::Value:{
            trace_start(depth, "value {}", op.name.c_str());
            StringStream ss(op.name);
            double d; ss >> d;
            return Value(d);
        }
        case AST::MathKind::Operator:{
            trace_start(depth, "operator {}", op.name.c_str());
            auto rhs = eval_rpe(iter, depth + 1);
            auto lhs = eval_rpe(iter, depth + 1);

            // Create a new closure
            // a few steps could be remove here to improve perf if needed
            auto fun = builtins[op.name];
            // value 0 should be self fun for recursions
            Array<Value> args = {Value(0), lhs, rhs};
            //
            return closure(Value(fun, args), depth + 1);
        }
        case AST::MathKind::Function:{
            trace_start(depth, "function (name: {}) (arg_count: {}) (env: {})", op.name, op.arg_count, env.size());
            Value clo = eval(op.ref, depth + 1);
            assert(clo.tag == ValueKind::obj_closure);

            debug("Retrieve closure");
            // Value::Closure& clo = closure.get_closure();
            int n = int(env.size());

            for(int i = 0; i < op.arg_count; ++i){
                debug("eval argument: {}", i);
                env.push_back(eval_rpe(iter, depth + 1));
            }

            env.push_back(Value(0)); // should be self fun for recursion

            // only reverse the added arguments
            std::reverse(std::begin(env) + n, std::end(env));
            return closure(clo, depth + 1);
        }
        case AST::MathKind::VarRef:{
            trace_start(depth, "varref");
            return eval(op.ref, depth + 1);
        }
        case AST::MathKind::None:{
            trace_start(depth, "none");
            return Value("none");
        }
        }

        return Value("None");
        throw std::runtime_error("unreachable");
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

Value builtin_sin(Array<Value>& args){
    assert(args.size() == 2 && "expected 1 arguments");

    auto v = args[1].get<float64>();

    // std::cout << "sin(" << v << ")";
    return std::sin(v);
}


Value builtin_max(Array<Value>& args){
    assert(args.size() >= 3 && "expected 2 arguments");

    auto n = args.size();

    auto a = args[n - 2];
    auto b = args[n - 1];

    std::cout << "max(" << a.str() << ", " << b.str() << ")\n";

    return std::max(a.get<float64>(), b.get<float64>());
}


Value builtin_div(Array<Value>& args){
    assert(args.size() >= 3 && "expected 2 arguments");

    auto a = args[1].get<float64>();
    auto b = args[2].get<float64>();

    //std::cout << "div(" << a << ", " << b << ")";
    return a / b;
}


Value builtin_mult(Array<Value>& args){
    assert(args.size() >= 3 && "expected 2 arguments");

    auto a = args[1].get<float64>();
    auto b = args[2].get<float64>();

    //std::cout << "mult(" << a << ", " << b << ")";
    return a * b;
}

}
