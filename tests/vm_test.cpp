#include "ast/magic.h"
#include "lexer/buffer.h"
#include "parser/parser.h"
#include "revision_data.h"
#include "sema/sema.h"
#include "utilities/strings.h"
#include "vm/tree.h"

#include <catch2/catch.hpp>
#include <sstream>

#include "logging/logging.h"

#include <catch2/catch.hpp>
#include <iostream>

using namespace lython;

String test_modules_path() { return String(_SOURCE_DIRECTORY) + "/code"; }

String eval_it(String code, String expr, Module*& mod) {
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
    auto          partial = str(eval.eval(stmt));

    eval.root.dump(std::cout);
    emod->dump(std::cout);
    delete emod;
    return partial;
}

void run_test_case(String code, String expr, String expected) {
    Module* mod = nullptr;

    auto result = eval_it(code, expr, mod);

    delete mod;
    REQUIRE(result == expected);
}

#ifndef EXPERIMENT

TEST_CASE("VM_FunctionDef") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    return a\n",
                  "fun(1)",
                  "1");
}

TEST_CASE("VM_FunctionDef_with_temporaries") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    b = a + 1\n"
                  "    if b == 0:\n"
                  "        return 0\n"
                  "    return fun(a - 1) + 1\n",
                  "fun(10)",
                  "11");
}

TEST_CASE("VM_FunctionDef_recursive") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    if a == 0:\n"
                  "        return 0\n"
                  "    return fun(a - 1) + 1\n",
                  "fun(10)",
                  "10");
}

TEST_CASE("VM_BinOp_Add_i32") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    return a + 1\n",
                  "fun(1)",
                  "2");
}

TEST_CASE("VM_BoolAnd_True") {
    run_test_case("def fun() -> bool:\n"
                  "    return (1 < 2) and (2 < 3)\n",
                  "fun()",
                  "True");
}

TEST_CASE("VM_BoolAnd_False") {
    run_test_case("def fun() -> bool:\n"
                  "    return (1 > 2) and (2 > 3)\n",
                  "fun()",
                  "False");
}

TEST_CASE("VM_Compare_True") {
    run_test_case("def fun() -> bool:\n"
                  "    return 1 < 2 < 3 < 4 < 5\n",
                  "fun()",
                  "True");
}

TEST_CASE("VM_Compare_False") {
    run_test_case("def fun() -> bool:\n"
                  "    return 1 > 2 > 3 > 4 > 5\n",
                  "fun()",
                  "False");
}

TEST_CASE("VM_IfStmt_True") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    if a > 0:\n"
                  "        return 1\n"
                  "    else:\n"
                  "        return 2\n",
                  "fun(1)",
                  "1");
}

TEST_CASE("VM_assert_True") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    assert True, \"all good\"\n",
                  "fun(0)",
                  "None");
}

// FIXME: tree should raise exception
// Traceback (most recent call last):
//   File "main.py", line 7, in <module>
//     fun(0)
//   File "main.py", line 5, in fun
//     assert False, "false"
// AssertionError: false
TEST_CASE("VM_assert_False") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    assert False, \"Very bad\"\n",
                  "fun(0)",
                  "None");
}

TEST_CASE("VM_IfStmt_False") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    if a > 0:\n"
                  "        return 1\n"
                  "    else:\n"
                  "        return 2\n",
                  "fun(0)",
                  "2");
}

TEST_CASE("VM_UnaryOp_USub_i32") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    return - a\n",
                  "fun(-1)",
                  "1");
}

TEST_CASE("VM_assign") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    b = 3\n"
                  "    b = a * b\n"
                  "    return b\n"
                  "",
                  "fun(2)",
                  "6");
}

TEST_CASE("VM_pass") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    pass\n",
                  "fun(0)",
                  "None");
}

TEST_CASE("VM_inline_stmt") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    a += 1; return a\n",
                  "fun(0)",
                  "1");
}

// Traceback (most recent call last):
//   File "<input>", line -2, in fun(2)
//     fun(2)
//   File "<input>", line 4, in fun
//     return fun(a - 1)
//   File "<input>", line 4, in fun
//     return fun(a - 1)
//   File "<input>", line 3, in fun
//     assert False, "Very bad"
// AssertionError: Very bad
TEST_CASE("VM_exception_stop_recursion") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    if a == 0:\n"
                  "        assert False, \"Very bad\"\n"
                  "    return fun(a - 1)\n",
                  "fun(2)",
                  "None");
}

TEST_CASE("VM_exception_stop_loop") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    while True:\n"
                  "        assert False, \"Very bad\"\n",
                  "fun(2)",
                  "None");
}

TEST_CASE("VM_raise") {
    // This fails because of SEMA, we need a real exception
    // run_test_case(
    //     "def fun(a: i32) -> i32:\n"
    //     "    raise 'Ohoh'\n"
    //     "    return a\n",
    //     "fun(-1)",
    //     "1"
    // );
}

TEST_CASE("VM_Try_AllGood") {
    // Increase once in try
    // another inside the else (no exception)
    // and final one inside the finally
    /*
    run_test_case(
        "def fun(a: i32) -> i32:\n"
        "    try:\n"
        "        a += 1\n"
        "    except:\n"
        "        a += 1\n"
        "    else:\n"
        "        a += 1\n"
        "    finally:\n"
        "        a += 1\n",
        "    return a\n"
        "fun(0)",
        "3"
    );
    //*/
}

TEST_CASE("VM_Aug_Add_i32") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    a += 1\n"
                  "    return a\n",
                  "fun(0)",
                  "1");
}

TEST_CASE("VM_While") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    b = 0\n"
                  "    while a > 0:\n"
                  "        a -= 1\n"
                  "        b += 2\n"
                  "    return b\n"
                  "",

                  "fun(2)",
                  "4");
}

// TEST_CASE("VM_yield_for_generator") {
//     run_test_case("def range(a: i32) -> i32:\n"
//                   "    b = 0\n"
//                   "    while b < a:\n"
//                   "        yield b\n"
//                   "        b += 1\n"
//                   "    \n"
//                   "\n"
//                   "def fun(a: i32) -> i32:\n"
//                   "    s = 0\n"
//                   "    for i in range(a):\n"
//                   "        s += i\n"
//                   "    return s\n"
//                   "",

//                   "fun(2)",
//                   "3");
// }

TEST_CASE("VM_While_break") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    b = 0\n"
                  "    while a > 0:\n"
                  "        a -= 1\n"
                  "        b += 1\n"
                  "        break\n"
                  "        b += 1\n"
                  "    return b\n"
                  "",

                  "fun(10)",
                  "1");
}

TEST_CASE("VM_While_continue") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    b = 0\n"
                  "    while a > 0:\n"
                  "        a -= 1\n"
                  "        b += 1\n"
                  "        continue\n"
                  "        b += 1\n"
                  "    return b\n"
                  "",

                  "fun(10)",
                  "10");
}

TEST_CASE("VM_AnnAssign") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    b: i32 = 3\n"
                  "    b: i32 = a * b\n"
                  "    return b\n"
                  "",
                  "fun(2)",
                  "6");
}

TEST_CASE("VM_ifexp_True") {
    // FIXME: wrong syntax
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    return 1 if a > 0 else 0\n"
                  "",
                  "fun(2)",
                  "1");
}

TEST_CASE("VM_ifexp_False") {
    // FIXME: wrong syntax
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    return 1 if a > 0 else 0\n"
                  "",
                  "fun(-2)",
                  "0");
}

// TEST_CASE("VM_ifexp_ext_True") {
//     // FIXME: wrong syntax
//     run_test_case("def fun(a: i32) -> i32:\n"
//                   "    return if a > 0: 1 else 0\n"
//                   "",
//                   "fun(2)",
//                   "1");
// }

// TEST_CASE("VM_ifexp_ext_False") {
//     // FIXME: wrong syntax
//     run_test_case("def fun(a: i32) -> i32:\n"
//                   "    return if a > 0: 1 else 0\n"
//                   "",
//                   "fun(-2)",
//                   "0");
// }

TEST_CASE("VM_NamedExpr") {
    run_test_case("def fun(a: i32) -> i32:\n"
                  "    if (b := a + 1) > 0:\n"
                  "        return b\n"
                  "    return 0\n"
                  "",
                  "fun(0)",
                  "1");
}

// TEST_CASE("VM_ClassDef") {
//     run_test_case("class Point:\n"
//                   "    def __init__(self, x: f64, y: f64):\n"
//                   "        self.x = x\n"
//                   "        self.y = y\n"
//                   "\n"
//                   "def fun(p: Point) -> f64:\n"
//                   "    return p.x + p.y\n"
//                   "",
//                   "fun(Point(1.0, 2.0))",
//                   "3");
// }
#endif
