#ifndef LYTHON_INTERPRETER_HEADER
#define LYTHON_INTERPRETER_HEADER

#include "../parser/module.h"
#include "../logging/logging.h"

#include <cmath>

namespace lython{

class InterpreterException: public Exception{
public:
    InterpreterException(String str):
        Exception(std::move(str))
    {}
};

inline
Value builtin_sin(Array<Value>& args){
    assert(args.size() == 2 && "expected 1 arguments");

    auto v = args[1].get<float64>();

    // std::cout << "sin(" << v << ")";
    return std::sin(v);
}

inline
Value builtin_max(Array<Value>& args){
    assert(args.size() == 3 && "expected 2 arguments");

    auto a = args[1].get<float64>();
    auto b = args[2].get<float64>();

    // std::cout << "max(" << a << ", " << b << ")";
    return std::max(a, b);
}

inline
Value builtin_div(Array<Value>& args){
    assert(args.size() == 3 && "expected 2 arguments");

    auto a = args[1].get<float64>();
    auto b = args[2].get<float64>();

    //std::cout << "div(" << a << ", " << b << ")";
    return a / b;
}

inline
Value builtin_mult(Array<Value>& args){
    assert(args.size() == 3 && "expected 2 arguments");

    auto a = args[1].get<float64>();
    auto b = args[2].get<float64>();

    //std::cout << "mult(" << a << ", " << b << ")";
    return a * b;
}

class Interpreter{
private:
    Dict<String, Value::BuiltinImpl> builtins = {
        {"max", builtin_max},
        {"sin", builtin_sin},
        {"/", builtin_div},
        {"*", builtin_mult},
    };

public:
    Interpreter(Module* m):
        module(m)
    {}


    Value eval(ST::Expr expr, size_t depth=0){
        trace_start(depth, "");
        switch(expr->kind()){
        case AST::Expression::KindFunction:{
            AST::Function* fun = static_cast<AST::Function*>(expr.get());

            // Create the closure here with its environment setup
            Array<Value> env;
            env.reserve(100);

            return Value(fun, env);
        }
        case AST::Expression::KindCall:{
            AST::Call* cl = static_cast<AST::Call*>(expr.get());
            return call(cl, depth + 1);
        }
        case AST::Expression::KindSeqBlock:{
            AST::SeqBlock* block = static_cast<AST::SeqBlock*>(expr.get());
            return seq_block(block, depth + 1);
        }
        case AST::Expression::KindValue:{
            AST::ValueExpr* val = static_cast<AST::ValueExpr*>(expr.get());
            return value(val, depth + 1);
        }
        case AST::Expression::KindStatement:{
            AST::Statement* stmt = static_cast<AST::Statement*>(expr.get());
            return statement(stmt, depth + 1);
        }
        case AST::Expression::KindReversePolish:{
            AST::ReversePolishExpression* rpe = static_cast<AST::ReversePolishExpression*>(expr.get());
            auto iter = std::begin(rpe->stack);
            return eval_rpe(iter, depth + 1);
        }
        case AST::Expression::KindReference: {
            AST::Ref* ref = static_cast<AST::Ref*>(expr.get());
            return eval_ref(ref, depth + 1);
        }
        case AST::Expression::KindBuiltin: {
            AST::Builtin* blt = static_cast<AST::Builtin*>(expr.get());
            return eval_builtin(blt, depth + 1);
        }
        default: {
            info("Ignoring %d", expr->kind());
            return Value("MainEval Not Implemented");
        }
        }
    }

    Value eval_builtin(AST::Builtin* blt, size_t depth){
        trace_start(depth, "%s", blt->name.c_str());
        auto fun = builtins[blt->name];
        return Value(fun, Array<Value>());
    }

    Value eval_ref(AST::Ref* ref, size_t depth){
        trace_start(depth, "%s: %d | %d | %d",
                    ref->name().c_str(),
                    ref->index(),
                    module->size(),
                    env.size());

        return env[ref->index()];

        auto expr = module->get_item(ref->index());
        return eval(expr, depth + 1);
    }

    Value eval_rpe(Stack<AST::MathNode>::Iterator &iter, size_t depth){
        AST::MathNode op = *iter;
        iter++;

        switch (op.kind){
        case AST::MathKind::Value:{
            trace_start(depth, "value %s", op.name.c_str());
            StringStream ss(op.name);
            double d; ss >> d;
            return Value(d);
        }
        case AST::MathKind::Operator:{
            trace_start(depth, "operator %s", op.name.c_str());
            auto rhs = eval_rpe(iter, depth + 1);
            auto lhs = eval_rpe(iter, depth + 1);

            // Create a new closure
            // a few steps could be remove here to improve perf if needed
            auto fun = builtins[op.name];
            // value 0 should be self fun for recursions
            Array<Value> args = {Value(0), lhs, rhs};
            //
            return eval_closure(Value(fun, args), depth + 1);
        }
        case AST::MathKind::Function:{
            trace_start(depth, "function (arg_count: %d)", op.arg_count);
            Value closure = eval(op.ref, depth + 1);
            assert(closure.tag == obj_closure);

            Value::Closure& clo = closure.get();
            int n = int(clo.env.size());

            for(int i = 0; i < op.arg_count; ++i){
                clo.env.push_back(eval_rpe(iter, depth + 1));
            }
            clo.env.push_back(Value(0)); // should be self fun for recursion

            // only reverse the added arguments
            std::reverse(std::begin(clo.env) + n, std::end(clo.env));
            return eval_closure(closure, depth + 1);
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

        throw std::runtime_error("unreachable");
    }

    Value eval_closure(Value fun, size_t depth){
        if (fun.tag == obj_closure){
            if (!fun.v_closure.fun){
                auto v = fun.v_closure.builtin(fun.v_closure.env);
                // std::cout << "result: "; v.print(std::cout) << std::endl;
                return v;
            }
        }

        return Value("closure");
    }

    Value statement(AST::Statement* stmt, size_t depth){
        trace_start(depth, "%d", stmt->statement());
        return eval(stmt->expr(), depth + 1);
    }

    Array<Value> eval(Array<ST::Expr> exprs, size_t depth){
        trace_start(depth, "");

        Array<Value> values;
        values.reserve(exprs.size());

        for (auto expr: exprs)
            values.push_back(eval(expr, depth + 1));

        return values;
    }

    Value value(AST::ValueExpr* val, size_t depth){
        trace_start(depth, "");
        return val->value;
    }

    Value seq_block(AST::SeqBlock* val, size_t depth){
        trace_start(depth, "");

        size_t n = size_t(std::max(int(val->blocks().size()) - 1, 0));

        for(size_t i = 0; i < n; ++i)
            eval(val->blocks()[i], depth + 1);

        // returns last line of sequential block
        return eval(val->blocks()[n], depth + 1);
    }

    Value ref(AST::Ref const* ref, Array<Value> env, size_t depth){
        trace_start(depth, "");
        return env[ref->index()];
    }

    Value call(AST::Call* call, size_t depth){
        trace_start(depth, "");
        Value closure = eval(call->function(), depth + 1);
        assert(closure.tag == obj_closure);
        Value::Closure& clo = closure.get();

        clo.env = eval(call->arguments(), depth);
        clo.env.insert(std::begin(clo.env), Value(0));

        AST::Function* fun = closure.v_closure.fun;

        auto old = env;
        env = closure.v_closure.env;
        Value returned_value = eval(fun->body(), depth);
        env = old;

        returned_value.print(std::cout) << std::endl;
        throw InterpreterException("Not Implemented");
    }

private:
    //! Global Context
    Module* module = nullptr;

    Array<Value> env;
    Dict<String, std::function<Value()>> _builtin_impl;
};

} // namespace lython

#endif
