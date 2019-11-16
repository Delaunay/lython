#ifndef LYTHON_INTERPRETER_HEADER
#define LYTHON_INTERPRETER_HEADER

#include "../parser/module.h"

namespace lython{

class Value{
public:
    enum type_tag{
        Int,
        Float,
        Double,
        Function
    } tag;

    Value(int i){
        data.v_int = i;
        tag = Int;
    }

    Value(float i){
        data.v_float = i;
        tag = Float;
    }

    Value(double i){
        data.v_double = i;
        tag = Double;
    }

    Value(AST::Function* i){
        data.v_function = i;
        tag = Function;
    }

    union {
        int v_int;
        double v_double;
        float v_float;
        AST::Function* v_function;
    } data;

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


    Value eval(Value const& val){

    }

    Value value(AST::Value* val){
        return Value(-1);
    }

    Value reverse_polish_expr(AST::ReversePolishExpression* expr){
        return Value(-1);
    }

    Value ref(AST::Ref const* ref, Array<Value> env){
        return env[ref->index()];
    }

    void call(AST::Function* fun, Array<Value> args){
        assert(args.size() == fun->args().size() && "Argument size should match");

        // for recursion
        args.insert(args.begin(), fun);

        // insert all the info needed for evaluation
        Array<Tuple<String, int>>& frame = fun->frame;
        for (size_t i = args.size(); i < frame.size(); ++i) {
            int idx = std::get<1>(frame[i]);
            // Value v = eval((*module)[idx]);
            // args.push_back(v);
        }


    }

private:
    //! Global Context
    Module* module = nullptr;

    Dict<String, std::function<Value()>> _builtin_impl;
};

} // namespace lython

#endif
