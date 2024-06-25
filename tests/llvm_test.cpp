
#include <catch2/catch_all.hpp>
#include <fstream>
#include <iostream>

// Kiwi
#include "lexer/buffer.h"
#include "parser/parser.h"
#include "revision_data.h"
#include "sema/sema.h"
#include "utilities/printing.h"
#include "utilities/strings.h"
#include "codegen/llvm/llvm_gen.h"
#include "logging/logging.h"

// Test
#include "libtest.h"
// #include "cases_vm.h"

#if WITH_LLVM_CODEGEN

using namespace lython;

String test_modules_path() { return String(_SOURCE_DIRECTORY) + "/code"; }
String reg_modules_path() { return String(_SOURCE_DIRECTORY) + "/code/llvm"; }

String
llvm_codegen_it(llvm::raw_fd_ostream& out, String const& code, String const& expr, Module*& mod) {
    std::cout << ">>>>>> Start\n";

    StringBuffer reader(code);
    Lexer        lex(reader);
    Parser       parser(lex);

    std::cout << code << std::endl;

    kwdebug(outlog(), "Code:\n{}", code.c_str());
    kwdebug(outlog(), "Expr: {}", expr.c_str());
    kwdebug(outlog(), "{}", "Parse");

    mod = parser.parse_module();
    lyassert(mod->body.size() > 0, "Should parse more than one expression");
    parser.show_diagnostics(std::cout);
    if (parser.has_errors()) {
        return "bad_parse";
    }

    kwinfo(outlog(), "{}", "Sema");
    ImportLib::instance()->add_to_path(test_modules_path());
    SemanticAnalyser sema;
    // sema.paths.push_back(test_modules_path());
    sema.exec(mod, 0);
    sema.show_diagnostic(std::cout);
    if (sema.has_errors()) {
        return "bad_sema";
    }
    // REQUIRE(sema.has_errors() == false);

    // Parse the expression itself
    StringBuffer expr_reader(expr);
    Lexer        expr_lex(expr_reader);
    Parser       expr_parser(expr_lex);

    auto* emod = expr_parser.parse_module();
    expr_parser.show_diagnostics(std::cout);
    // REQUIRE(expr_parser.has_errors() == false);

    if (expr_parser.has_errors()) {
        return "bad_parse";
    }
    auto* stmt = emod->body[0];

    // Sema the code with module sema
    sema.exec(stmt, 0);

    sema.show_diagnostic(std::cout);

    // FIXME
    // REQUIRE(sema.has_errors() == false);
    if (sema.has_errors()) {
        return "bad_sema";
    }
    kwinfo(outlog(), "{}", "Eval");

    LLVMGen generator;
    generator.exec(mod, 0);
    generator.dump();

    // JITEVal eval();
    // TreeEvaluator eval(sema.bindings);
    // auto          partial = str(eval.eval(stmt));

    std::cout << "Value Tree\n";
    // eval.root.dump(std::cout);

    std::cout << "Module dump\n";

    generator.llmodule->print(out, nullptr);

    emod->dump(std::cout);
    delete emod;

    std::cout << "<<<<<< End\n";

    return "";
}

void run_testcases(String const& name, Array<VMTestCase> const& cases) {
    kwinfo(outlog(), "Testing {}", name);

    if (cases.size() <= 0) {
        return;
    }

    Array<String> errors;
    TypeExpr*     deduced_type = nullptr;

    String current = [&]() {
        StringStream sspath;
        sspath << reg_modules_path() << "/current/" << name << ".ll";
        return sspath.str();
    }();

    String expected = [&]() {
        StringStream sspath;
        sspath << reg_modules_path() << "/" << name << ".ll";
        return sspath.str();
    }();

    {
        std::error_code      EC;
        llvm::raw_fd_ostream out(current.c_str(), EC);

        auto write_example = [&](llvm::raw_fd_ostream& out, int i, String const& code) {
            std::istringstream iss(code.c_str());
            std::string        line;
            out << ">>>>>>>\n";
            out << "; Example " << i << "\n";
            out << "; ------------\n";

            while (std::getline(iss, line)) {
                // Process each line here
                out << "; " << line << "\n";
            };

            out << "\n";
        };

        int i = 0;

        for (auto& c: cases) {
            Module* mod;

            write_example(out, i, c.code);

            String result = llvm_codegen_it(out, c.code, c.call, mod);

            out << "<<<<<<\n\n";

            REQUIRE(result == "");

            // FIXME: check for correctness
            delete mod;
            kwinfo(outlog(), "<<<<<<<<<<<<<<<<<<<<<<<< DONE");
            i += 1;
        }
    }

    // Regression
    {
        std::ifstream fp_current(current.c_str());
        std::ifstream fp_expected(expected.c_str());

        std::string line_expected, line_current;

        int diff = 0;
        while (std::getline(fp_current, line_current) && std::getline(fp_expected, line_expected)) {
            if (line_current != line_expected) {
                diff += 1;
            }
        }

        REQUIRE(diff == 0);
    }
}

#define GENTEST(name)                                                                         \
    TEMPLATE_TEST_CASE("LLVM_" #name, #name, name) {                                          \
        auto cases = get_test_cases("LLVM", str(nodekind<TestType>())); \
        run_testcases(str(nodekind<TestType>()), cases);                                      \
    }

#define X(name, _)
#define SSECTION(name)
#define EXPR(name, _) GENTEST(name)
#define STMT(name, _) GENTEST(name)
#define MOD(name, _)
#define MATCH(name, _)
#define VM(n, m)

NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH, VM)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH
#undef VM

#undef GENTEST
#endif
