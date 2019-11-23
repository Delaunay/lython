#include <catch2/catch.hpp>

#include "samples.h"

#include "lexer/buffer.h"
#include "parser/parser.h"
#include "interpreter/interpreter.h"

using namespace lython;


Value interpret_it(String code, String fun_name){
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

    // return value
    return vm.eval(ST::Expr(call));
}

#define TEST_INTERPRETER(code, value)\
    SECTION(#code){\
        REQUIRE(interpret_it(code(), #code) == value);\
    }

TEST_CASE("Interpreter"){
    TEST_INTERPRETER(simple_function_noargs, Value(1.0))
}
