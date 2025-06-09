#include "lexer/buffer.h"
#include "parser/parser.h"
#include "revision_data.h"
#include "sema/native_module.h"
#include "sema/sema.h"
#include "utilities/printing.h"
#include "utilities/strings.h"
#include "vm/tree.h"

#include <catch2/catch_all.hpp>
#include <sstream>

#include "logging/logging.h"

#include <catch2/catch_all.hpp>
#include <iostream>

// #include "cases_vm.h"
#include "libtest.h"

using namespace lython;

double native_add(double a, double b) { return a + b; }

struct Pnt {
    int x;
    int y;

    Pnt(int x = 0, int y = 0): x(x), y(y) {}

    Pnt add(Pnt* a) { return Pnt(x + a->x, y + a->y); }
};

template <>
struct lython::meta::ReflectionTrait<Pnt> {
    static int register_members() {
        //lython::meta::new_member<Pnt, int>("x");
        //lython::meta::new_member<Pnt, int>("y");
        // lython::meta::new_method("add", &Pnt::add);
        // lython::meta::new_method("sum", &Pnt::sum);

        return 1;
    }
};

int get_x(Pnt* pnt) { return pnt->x; }

String test_modules_path() { return String(_SOURCE_DIRECTORY) + "/code"; }

int compile_time_add(int a, int b) { return a + b; }

void make_native_module() {
#if 1
    ImportLib&          imported = *ImportLib::instance();
    NativeModuleBuilder nativemodule("nmodule", imported);

    int (*fun)(int, int)              = [](int a, int b) -> int { return a + b; };
    std::function<int(int, int)> fun2 = [](int a, int b) -> int { return a + b; };
    Pnt* (*stuff)(Pnt*, Pnt*)         = [](Pnt* a, Pnt* b) -> Pnt* {
        return new Pnt(a->x + b->x, a->y + b->y);
    };

    nativemodule
        // Compile time function
        .function<decltype(compile_time_add)>("native_add")
        .set<compile_time_add>()
        .args("a", "b")
        .end()

        // Simplified
        .def<&compile_time_add>("new_add", "a", "b")

        // Runtime function
        // This could only work if I move away from a C function pointer
        // which I am not sure I want to
        //.function<decltype(fun)>("runtime_add")
        //    .set(fun)
        //.end()
        // .function("add2", [](int a, int b) -> int { return a + b; })
        // .function("add3", fun2)

        .klass<Pnt>("Point")
            .constructor<int, int>({"x", "y"})
            .attribute<int>("x")
            .attribute<float>("y")
            .def<&Pnt::add>("add", "self", "b")

            // .method<decltype(&Pnt::add)>("add")
            // .set<&Pnt::add>()
        .end();

    imported.add_module("nmodule", nativemodule.module);

    meta::TypeRegistry::instance().dump(std::cout);
#endif
}

String eval_it(String const& code, String const& expr, Module*& mod) {
    std::cout << ">>>>>> Start\n";

    StringBuffer reader(code);
    Lexer        lex(reader);
    Parser       parser(lex);

    kwinfo(outlog(), "Code:\n{}", code.c_str());
    kwinfo(outlog(), "Expr: {}", expr.c_str());
    kwinfo(outlog(), "{}", "Parse");
    mod = parser.parse_module();

    if (code != "") {
        lyassert(mod->body.size() > 0, "Should parse more than one expression");
        REQUIRE(parser.has_errors() == false);
    }
    parser.show_diagnostics(std::cout);

    kwinfo(outlog(), "{}", "Sema");
    make_native_module();
    SemanticAnalyser sema;

    // Add Native first
    // register_native_function(mod, sema.bindings, "add", native_add);
    // register_native_object<Pnt, int, int>(mod, sema.bindings, "Pnt");
    // register_native_function(mod, sema.bindings, "get_x", get_x);

    // execute script
    ImportLib::instance()->add_to_path(test_modules_path());

    sema.exec(mod, 0);
    sema.show_diagnostic(std::cout);
    REQUIRE(sema.has_errors() == false);

    // Parse the expression itself
    StringBuffer expr_reader(expr);
    Lexer        expr_lex(expr_reader);
    Parser       expr_parser(expr_lex);

    auto* emod = expr_parser.parse_module();
    expr_parser.show_diagnostics(std::cout);
    REQUIRE(expr_parser.has_errors() == false);

    auto* stmt = emod->body[0];

    // Sema the code with module sema
    sema.exec(stmt, 0);
    sema.bindings.dump(std::cout);
    sema.show_diagnostic(std::cout);
    REQUIRE(sema.has_errors() == false);

    kwinfo(outlog(), "{}", "Eval");
    TreeEvaluator eval;
    eval.module(mod, 0);
    auto partial = str(eval.eval(stmt));

    std::cout << "Value Tree\n";
    eval.root.dump(std::cout);

    std::cout << "Module dump\n";
    emod->dump(std::cout);
    delete emod;

    std::cout << "<<<<<< End\n";

    return partial;
}

void run_test_case(VMTestCase const& testcase) {
    Module* mod = nullptr;

    auto result = eval_it(testcase.code, testcase.call, mod);

    delete mod;
    REQUIRE(result == testcase.expected_type);
}

void run_test_case(String const& code, String const& expr, String const& expected) {
    VMTestCase original(code, expr, expected);
    run_test_case(original);
}

void run_vm_testcases(String const& name, Array<VMTestCase> const& cases) {
    kwinfo(outlog(), "Testing {}", name);

    auto new_cases = transition("vm", name, cases);

    int i = 0;
    for (auto& c: new_cases) {
        SECTION(c.name.c_str()) {
            Module* mod;

            String result = eval_it(c.get_code(), c.get_call(), mod);

            REQUIRE(result == c.get_result());

            delete mod;

            kwinfo(outlog(), "<<<<<<<<<<<<<<<<<<<<<<<< DONE");
            i += 1;
        }
    }
}

#ifndef EXPERIMENT

TEST_CASE("VM_FunctionDef") {
    run_vm_testcases("VM_FunctionDef", get_test_cases("vm", "VM_FunctionDef"));
}

TEST_CASE("NativeModule") {
    run_vm_testcases("NativeModule", get_test_cases("cases", "NativeModule"));
}

TEST_CASE("VM_BinOp") { run_vm_testcases("VM_BinOp", get_test_cases("vm", "VM_BinOp")); }

TEST_CASE("VM_Bool") { run_vm_testcases("VM_Bool", get_test_cases("vm", "VM_Bool")); }

TEST_CASE("VM_Compare") { run_vm_testcases("VM_Compare", get_test_cases("vm", "VM_Compare")); }

TEST_CASE("VM_IfStmt") { run_vm_testcases("VM_IfStmt", get_test_cases("vm", "VM_IfStmt")); }

TEST_CASE("VM_assert") { run_vm_testcases("VM_assert", get_test_cases("vm", "VM_assert")); }

TEST_CASE("VM_UnaryOp") { run_vm_testcases("VM_UnaryOp", get_test_cases("vm", "VM_UnaryOp")); }

TEST_CASE("VM_assign") { run_vm_testcases("VM_assign", get_test_cases("vm", "VM_assign")); }

TEST_CASE("VM_pass") { run_vm_testcases("VM_pass", get_test_cases("vm", "VM_pass")); }

TEST_CASE("VM_inline_stmt") {
    run_vm_testcases("VM_inline_stmt", get_test_cases("vm", "VM_inline_stmt"));
}

TEST_CASE("VM_exception") {
    run_vm_testcases("VM_exception", get_test_cases("vm", "VM_exception"));
}

TEST_CASE("VM_AugAssign") {
    run_vm_testcases("VM_AugAssign", get_test_cases("vm", "VM_AugAssign"));
}

TEST_CASE("VM_While") { run_vm_testcases("VM_While", get_test_cases("vm", "VM_While")); }

TEST_CASE("VM_AnnAssign") {
    run_vm_testcases("VM_AnnAssign", get_test_cases("vm", "VM_AnnAssign"));
}

TEST_CASE("VM_ifexp") { run_vm_testcases("VM_ifexp", get_test_cases("vm", "VM_ifexp")); }

TEST_CASE("VM_NamedExpr") {
    run_vm_testcases("VM_NamedExpr", get_test_cases("vm", "VM_NamedExpr"));
}

TEST_CASE("VM_ClassDef") { run_vm_testcases("VM_ClassDef", get_test_cases("vm", "VM_ClassDef")); }

TEST_CASE("VM_Generator") {
    run_vm_testcases("VM_Generator", get_test_cases("vm", "VM_Generator"));
}

#endif

#if EXPERIMENTAL_TESTS
#define GENTEST(name)                                                      \
    TEMPLATE_TEST_CASE("VM_" #name, #name, name) {                         \
        run_vm_testcases(str(nodekind<TestType>()), name##_vm_examples()); \
    }

#define X(name, _)
#define SSECTION(name)
#define EXPR(name, _) GENTEST(name)
#define STMT(name, _) GENTEST(name)
#define MOD(name, _)
#define MATCH(name, _)

NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef GENTEST
#endif