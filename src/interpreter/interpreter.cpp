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

    std::ostream& dump_env(std::ostream& out){
        out << String(40, '-') << '\n';

        for(auto i = 0ul; i < env.size(); ++i){
            out << fmt::format("{:4d} | {}\n", i, env[i]);
        }

        out << String(40, '-') << '\n';
        return out;
    }

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
        dump_env(std::cout);
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
            values.push_back(eval(expr, depth));

        return values;
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
            debug("retrieved {}", obj);

            dump_env(std::cout);

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
                env.push_back(rhs);
                return Value("none");
            }
            // assign to object
            case AST::NodeKind::KBinaryOperator: {
                debug("Assign to an object attribute");

                auto rhs = eval(bin->rhs, d);
                AST::BinaryOperator const* attr = bin->lhs.ref<AST::BinaryOperator>();

                // fetch the p in p.x
                auto obj = eval(attr->lhs, d);
                assert(obj.tag == ValueKind::obj_object, "Assign to an object");

                // set x to lhs
                value::Struct& data = *obj.get<value::Struct*>();
                data.set_attribute(attr->rhs.ref<AST::Ref>()->name, rhs);
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
        trace_start(depth, "%d", stmt->statement);
        return eval(stmt->expr, depth);
    }

    Value reference(Reference_t ref, std::size_t depth) {
        trace_start(depth, "{}: {}, {} | {}",
                    ref->name,
                    ref->index,
                    ref->length,
                    env.size());

        assert(env.size() > ref->index, "Environment should hold the ref");
        auto n = ref->index;

        // debug("found {}", env[n].str());
        auto r = env[n];
        // debug("return {}", r);
        return r;
    }

    Value builtin(Builtin_t blt, std::size_t depth) {
        trace_start(depth, "{}", blt->name);
        auto fun = builtins[blt->name.str()];
        return Value(fun, Array<Value>());
    }

    Value value(Value_t val, std::size_t depth) {
        trace_start(depth, "value {}", val->value);
        return val->value;
    }

    Value struct_type(Struct_t cstruct, std::size_t) {
        return Value(cstruct);
    }

    Value call(Call_t call, std::size_t depth) {
        trace_start(depth, "call");
        Value closure = eval(call->function, depth);
        switch (closure.tag) {
        // standard function
        case ValueKind::obj_closure: return fun_call(closure, call, depth);

        // Calling a struct type
        case ValueKind::obj_class: return struct_call(closure, call, depth);
        }

        return Value("Unsupported");
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

        // v.v_object.attributes;
        return v;
    }

    Value fun_call(Value closure, Call_t call, std::size_t depth){
        trace_start(depth, "fun_call");

        auto on = std::ptrdiff_t(env.size());
        for (auto& expr: call->arguments)
            env.push_back(eval(expr, depth + 1));

        Value returned_value = eval_closure(closure, depth);

        env.erase(env.begin() + on, env.end());
        // debug("return {}", returned_value);
        return returned_value;
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
        return Value(fun, env);
    }

    // Helpers
    // -------
    Value eval_closure(Value fun, std::size_t depth){
        trace_start(depth, "closure {}", fun.tag);

        if (fun.tag == ValueKind::obj_closure){
            value::Closure* clo = fun.get<value::Closure*>();

            if (!clo->fun){
                assert(clo->builtin, "Closure is undefined");
                auto v = clo->builtin(env);
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

Value builtin_sin(Array<Value>& args){
    assert(args.size() == 2, "expected 1 arguments");

    auto v = args[1].get<float64>();

    // std::cout << "sin(" << v << ")";
    return std::sin(v);
}


Value builtin_max(Array<Value>& args){
    debug("calling max");
    assert(args.size() >= 3, "expected 2 arguments");

    auto n = args.size();

    auto a = args[n - 2];
    auto b = args[n - 1];

    return std::max(a.get<float64>(), b.get<float64>());
}


Value builtin_div(Array<Value>& args){
    assert(args.size() >= 3, "expected 2 arguments");

    auto a = args[1].get<float64>();
    auto b = args[2].get<float64>();

    //std::cout << "div(" << a << ", " << b << ")";
    return a / b;
}


Value builtin_mult(Array<Value>& args){
    assert(args.size() >= 3, "expected 2 arguments");

    auto a = args[1].get<float64>();
    auto b = args[2].get<float64>();

    //std::cout << "mult(" << a << ", " << b << ")";
    return a * b;
}

}
