#include <catch2/catch.hpp>

#include "samples.h"

#include "lexer/buffer.h"
#include "parser/parser.h"
#include "interpreter/interpreter.h"

using namespace lython;

template<typename... Args>
void insert_arg(AST::Call* call, Args... v){
    insert_arg(call, v...);
}

template<typename T, typename... Args>
void insert_arg(AST::Call* call, T a, Args... v){
    call->arguments().push_back(ST::Expr(new AST::ValueExpr(a, nullptr)));
    insert_arg(call, v...);
}

void insert_arg(AST::Call*){}

template<typename... Args>
Value interpret_it(String code, String fun_name, Args... v){
    StringBuffer reader(code);

    // Parse code
    Module module;
    Parser par(reader, &module);

    ST::Expr expr = nullptr;
    do {
        expr = par.parse_one(module);
    } while(expr != nullptr);
    // ---

    Interpreter vm(&module);

    // Make Fun Call
    AST::Call* call = new AST::Call();
    auto fun = module.find(fun_name);

    if (fun == nullptr)
        return Value("Unknown function");

    call->function() = fun;
    insert_arg(call, v...);

    // return value
    return vm.eval(ST::Expr(call));
}

#define TEST_INTERPRETER(code, value)\
    SECTION(#code){\
        REQUIRE(interpret_it(code(), #code) == value);\
    }

#define TEST_INTERPRETER_ARGS(code, value, ...)\
    SECTION(#code){\
        REQUIRE(interpret_it(code(), #code, __VA_ARGS__) == value);\
    }


TEST_CASE("Interpreter"){
    TEST_INTERPRETER(simple_function_noargs, Value(1.0))
    // TEST_INTERPRETER_ARGS(simple_function_return_args, Value(1.0), 1.0)
}
