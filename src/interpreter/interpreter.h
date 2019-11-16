#ifndef LYTHON_INTERPRETER_HEADER
#define LYTHON_INTERPRETER_HEADER

#include "../parser/module.h"
#include "../logging/logging.h"

namespace lython{

class InterpreterException: public Exception{
public:
    InterpreterException(String str):
        Exception(std::move(str))
    {}
};


class Value{
public:
    enum type_tag{
        TInt,
        TFloat,
        TDouble,
        TClosure
    } tag;

    struct Closure{
        AST::Function* fun = nullptr;
        Array<Value> env;
    };

    Value(int i){
        data.v_int = i;
        tag = TInt;
    }

    Value(float i){
        data.v_float = i;
        tag = TFloat;
    }

    Value(double i){
        data.v_double = i;
        tag = TDouble;
    }

    Value(AST::Function* fun, Array<Value>   env){
        v_closure = {fun, env};
        tag = TClosure;
    }

    ~Value(){}

    union Union{
        int v_int;
        double v_double;
        float v_float;

    } data;

    std::ostream& print(std::ostream& out){
        switch (tag) {
        case TClosure: return out << "closure";
        case TDouble: return out << "double " << data.v_double;
        case TInt: return out << "int " << data.v_int;
        }
        return out;
    }

    Closure v_closure;
    String name;
};

class Interpreter{
public:
    Interpreter(Module* m):
        module(m)
    {}

    /*
    void eval(AST::Call* call){
        // Value fun = eval(call->function());
        AST::Expression* fun_expr = call->function().get();
        if (fun_expr->kind() != AST::Expression::KindFunction){
            return;
        }

        auto fun = static_cast<AST::Function*>(fun_expr);
    }*/

    Value eval(ST::Expr expr){
        info("eval");

        if (expr->kind() == AST::Expression::KindFunction){
            AST::Function* fun = static_cast<AST::Function*>(expr.get());

            // Create the closure here with its environment setup
            Array<Value> env;
            env.reserve(100);

            return Value(fun, env);
        }

        else if (expr->kind() == AST::Expression::KindCall){
            AST::Call* cl = static_cast<AST::Call*>(expr.get());
            return call(cl);
        }

        else if (expr->kind() == AST::Expression::KindSeqBlock){
            AST::SeqBlock* block = static_cast<AST::SeqBlock*>(expr.get());
            return seq_block(block);
        }

        else if (expr->kind() == AST::Expression::KindValue){
            AST::Value* val = static_cast<AST::Value*>(expr.get());
            return value(val);
        }

        else if (expr->kind() == AST::Expression::KindStatement){
            AST::Statement* stmt = static_cast<AST::Statement*>(expr.get());
            return statement(stmt);
        }
        else if (expr->kind() == AST::Expression::KindReversePolish){
            AST::ReversePolishExpression* rpe = static_cast<AST::ReversePolishExpression*>(expr.get());
            auto iter = std::begin(rpe->stack);
            return eval_rpe(iter);
        }

        else {
            info("Ignoring %d", expr->kind());
        }

        return Value(0);
    }

    Value eval_rpe(Stack<AST::MathNode>::Iterator &iter){
        AST::MathNode op = *iter;
        iter++;

        switch (op.kind){
        case AST::MathKind::Value:{
           StringStream ss(op.name);
           double d;
           ss >> d;
           return Value(d);
        }
        case AST::MathKind::Operator:{
            auto rhs = eval_rpe(iter);
            auto lhs = eval_rpe(iter);
        }
        case AST::MathKind::Function:{

        }
        case AST::MathKind::VarRef:{

        }
        case AST::MathKind::None:{

        }
        }
    }

    Value statement(AST::Statement* stmt){
        return eval(stmt->expr());
    }

    Array<Value> eval(Array<ST::Expr> exprs){
        info("eval_map");
        Array<Value> values;
        values.reserve(exprs.size());

        for (auto expr: exprs)
            values.push_back(eval(expr));

        return values;
    }

    Value value(AST::Value* val){
        info("value");
        if (val->tag == AST::Value::CTInt)
            return Value(val->v_int);

        if (val->tag == AST::Value::CTDouble)
            return Value(val->v_double);

        return Value(0);
    }

    Value seq_block(AST::SeqBlock* val){
        info("seq_block");
        int n = int(val->blocks().size()) - 1;
        for(int i = 0; i < n; ++i)
            eval(val->blocks()[i]);

        return eval(val->blocks()[val->blocks().size() - 1]);
    }

    Value reverse_polish_expr(AST::ReversePolishExpression* expr){
        return Value(-1);
    }

    Value ref(AST::Ref const* ref, Array<Value> env){
        return env[ref->index()];
    }

    Value call(AST::Call* call){
        info("call");
        Value closure = eval(call->function());
        Array<Value> arguments = eval(call->arguments());

        closure.print(std::cout) << std::endl;
        for(auto arg: arguments)
            arg.print(std::cout) << std::endl;

        assert(closure.tag == Value::TClosure);
        AST::Function* fun = closure.v_closure.fun;

        auto old = env;
        env = closure.v_closure.env;
        Value returned_value = eval(fun->body());
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
