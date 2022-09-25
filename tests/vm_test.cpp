#include "ast/magic.h"
#include "lexer/buffer.h"
#include "parser/parser.h"
#include "revision_data.h"
#include "sema/sema.h"
#include "vm/tree.h"
#include "utilities/strings.h"

#include <catch2/catch.hpp>
#include <sstream>

#include "logging/logging.h"


#include <iostream>
#include <catch2/catch.hpp>

using namespace lython;





String test_modules_path() { return String(_SOURCE_DIRECTORY) + "/code"; }

String eval_it(String code, String expr, Module *&mod) {
    StringBuffer reader(code);
    Lexer        lex(reader);
    Parser       parser(lex);

    info("{}", "Parse");
    mod = parser.parse_module();
    assert(mod->body.size() > 0, "Should parse more than one expression");

    info("{}", "Sema");
    SemanticAnalyser sema;
    sema.paths.push_back(test_modules_path());
    sema.exec(mod, 0);

    StringBuffer expr_reader(expr);
    Lexer        expr_lex(expr_reader);
    Parser       expr_parser(expr_lex);

    auto emod = expr_parser.parse_module();
    auto stmt = emod->body[0];

    // Sema the code with module sema
    sema.exec(stmt, 0);

    info("{}", "Eval");
    TreeEvaluator eval(sema.bindings);
    auto partial = str(eval.exec(stmt, 0));

    delete emod;
    return partial;
}

void run_test_case(String code, String expr, String expected) {
    Module* mod = nullptr;

    auto result = eval_it(code, expr, mod);

    delete mod;
    REQUIRE(result == expected);

}

TEST_CASE("VM_Tree") {
    // run_test_case(
    //     "def fun(a: i32) -> i32:\n"
    //     "    return a\n",
    //     "fun(1)",
    //     "1"
    // );
    run_test_case(
        "def fun(a: i32) -> i32:\n"
        "    return a + 1\n",
        "fun(1)",
        "2"
    );
}