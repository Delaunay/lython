#include "ast/magic.h"
#include "lexer/buffer.h"
#include "parser/parser.h"
#include "revision_data.h"
#include "sema/sema.h"
#include "utilities/strings.h"
#include "codegen/llvm/llvm_gen.h"

#include <catch2/catch_all.hpp>
#include <sstream>

#include "logging/logging.h"

#include <catch2/catch_all.hpp>
#include <iostream>

#include "vm_cases.cpp"

#if WITH_LLVM_CODEGEN

using namespace lython;

String test_modules_path() { return String(_SOURCE_DIRECTORY) + "/code"; }

String eval_it(String const& code, String const& expr, Module*& mod) {
    std::cout << ">>>>>> Start\n";

    StringBuffer reader(code);
    Lexer        lex(reader);
    Parser       parser(lex);


    std::cout << code << std::endl;

    kwdebug("Code:\n{}", code.c_str());
    kwdebug("Expr: {}", expr.c_str());
    kwdebug("{}", "Parse");

    mod = parser.parse_module();
    lyassert(mod->body.size() > 0, "Should parse more than one expression");
    parser.show_diagnostics(std::cout);
    if(parser.has_errors()) {
        return "";
    }

    kwinfo("{}", "Sema");
    ImportLib::instance()->add_to_path(test_modules_path());
    SemanticAnalyser sema;
    // sema.paths.push_back(test_modules_path());
    sema.exec(mod, 0);
    sema.show_diagnostic(std::cout);
    if (sema.has_errors()) {
        return "";
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
        return "";
    }
    auto* stmt = emod->body[0];

    // Sema the code with module sema
    sema.exec(stmt, 0);

    sema.show_diagnostic(std::cout);
    
    // FIXME
    // REQUIRE(sema.has_errors() == false);
    if (sema.has_errors()) {
        return "";
    }
    kwinfo("{}", "Eval");

    LLVMGen generator;
    generator.exec(mod, 0);
    generator.dump();

    // JITEVal eval();
    // TreeEvaluator eval(sema.bindings);
    // auto          partial = str(eval.eval(stmt));

    std::cout << "Value Tree\n";
    //eval.root.dump(std::cout);

    std::cout << "Module dump\n";
    emod->dump(std::cout);
    delete emod;

    std::cout << "<<<<<< End\n";

    return "";
}

void run_testcases(String const& name, Array<VMTestCase> const& cases) {
    kwinfo("Testing {}", name);

    Array<String> errors;
    TypeExpr*     deduced_type = nullptr;

    int i = 0;
    for (auto& c: cases) {
        Module* mod;

        StringStream ss;
        ss << "_" << i;

        // write_fuzz_file(name + ss.str(), c.code);
        String result = eval_it(c.code, c.call, mod);

        // REQUIRE(errors == c.errors);

        // FIXME: check for correctness
        delete mod;

        kwinfo("<<<<<<<<<<<<<<<<<<<<<<<< DONE");
        i += 1;
    }
}
#define GENTEST(name)                                                   \
    TEMPLATE_TEST_CASE("LLVM_" #name, #name, name) {                      \
        run_testcases(str(nodekind<TestType>()), name##_vm_examples()); \
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
