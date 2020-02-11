#include <catch2/catch.hpp>

#include "samples.h"

#include "lexer/buffer.h"
#include "parser/parser.h"
#include "interpreter/interpreter.h"

using namespace lython;

template<typename T>
void insert_arg(AST::Call* call, T a){
    call->arguments().emplace_back(Expression::make<AST::ValueExpr>(a, Expression()));
}

template<typename... Args>
Value interpret_call(String const& code, String fun_name, Args... v){
    StringBuffer reader(code);

    // Parse code
    Module module;
    Parser par(reader, &module);

    Expression expr;
    do {
        expr = par.parse_one(module);
    } while(expr);
    // ---

    Interpreter vm(&module);

    // Make Fun Call
    auto fun = module.find(fun_name);

    assert(fun);

    expr = Expression::make<AST::Call>();
    auto* call = expr.ref<AST::Call>();
    call->function() = fun;

    (insert_arg(call, std::forward<Args>(v)), ...);

    // return value
    return vm.eval(expr);
}

Value interpret_code(String const& code){
    StringBuffer reader(code);

    // Parse code
    Module module;
    Parser par(reader, &module);

    Expression expr;
    do {
        expr = par.parse_one(module);
    } while(expr);
    // ---

    Interpreter vm(&module);

    // return value
    return vm.eval(expr);
}

#define TEST_INTERPRETER(code, value)\
    SECTION(#code){\
        REQUIRE(interpret_call(code(), #code) == value);\
    }

#define TEST_INTERPRETER_ARGS(code, value, ...)\
    SECTION(#code){\
        REQUIRE(interpret_call(code(), #code, __VA_ARGS__) == value);\
    }


TEST_CASE("Interpreter"){
    TEST_INTERPRETER(simple_function_noargs, Value(1.0))
    TEST_INTERPRETER_ARGS(max_alias, Value(2.0), 2.0, 1.0)
}
