// 
#include <catch2/catch_all.hpp>
#include <sstream>

// Kiwi
#include "utilities/printing.h"
#include "lexer/buffer.h"
#include "parser/parser.h"
#include "revision_data.h"
#include "sema/sema.h"
#include "utilities/strings.h"
#include "logging/logging.h"
#include "lowering/SSA.h"

// Test
#include "libtest.h"
//#include "cases.h"

// Path to repository on current system

using namespace lython;

String test_modules_path() { 
    return String(_SOURCE_DIRECTORY) + "/code"; 
}

void run_testcase(String const& folder, String const& name, Array<TestCase> cases);

TEST_CASE("SEMA_FunctionDef_Typing") {
    run_testcase("sema", "FunctionDef", get_test_cases("sema", "FunctionDef"));
}

TEST_CASE("SEMA_Match_Details") {}

TEST_CASE("SEMA_ClassDef_Attribute") {
    run_testcase("sema", "ClassDef_3", get_test_cases("sema", "ClassDef_3"));
}

TEST_CASE("SEMA_IfStmt") {
    run_testcase("sema", "Conditional_Assign", get_test_cases("sema", "Conditional_Assign"));
}

TEST_CASE("SEMA_Unpacking") {
    run_testcase("sema", "Unpacking", get_test_cases("sema", "Unpacking"));
}

TEST_CASE("SEMA_ClassDef_Static_Attribute") {
    run_testcase("sema", "ClassDef_1", get_test_cases("sema", "ClassDef_1"));
}

TEST_CASE("SEMA_ClassDef_Magic_BoolOperator") {
    run_testcase("sema", "ClassDef_0", get_test_cases("sema", "ClassDef_0"));
}


FILE* get_fuzz_file() {
    static FILE* file = fopen("fizz.ly", "w");
    return file;
}

void write_fuzz_file(String const& name, String const& code) {
    String path = String("in/") + name + String(".ly");

    FILE* file = fopen(path.c_str(), "w");

    if (file) {
        fprintf(file, "%s\n", code.c_str());
        fclose(file);
    }
}

inline Tuple<TypeExpr*, Array<String>> sema_it(String code, Module*& mod) {

    StringBuffer reader(code);
    Lexer        lex(reader);
    Parser       parser(lex);

    kwinfo(outlog(), "{}", "Parse");
    mod = parser.parse_module();
    lyassert(mod->body.size() > 0, "Should parse more than one expression");

    kwinfo(outlog(), "{}", "Sema");

    ImportLib::instance()->add_to_path(test_modules_path());

    SemanticAnalyser sema;
    sema.exec(mod, 0);

    BindingEntry& entry = sema.bindings.bindings.back();

    Array<String> errors;
    for (auto& err: sema.errors) {
        errors.push_back(err->what());
    }

    return std::make_tuple(entry.type, errors);
}

void run_testcase(String const& folder, String const& name, Array<TestCase> cases) {
    kwinfo(outlog(), "Testing {}", name);

    Array<String> errors;
    TypeExpr*     deduced_type = nullptr;

    // cases = get_test_cases(folder, name, cases);

    int i = 0;
    for (auto& c: cases) {
        StringStream ss;
        ss << name << "_" << i;
        String filename = ss.str();
        write_fuzz_file(ss.str(), c.code);

        SECTION(filename.c_str()) {
            Module* mod;

            std::tie(deduced_type, errors) = sema_it(c.code, mod);


            if (!c.call.empty()) {
                StringBuffer reader(c.call);
                Lexer        lex(reader);
                Parser       parser(lex);
                parser.parse_to_module(mod);

                SemanticAnalyser sema;
                sema.exec(mod, 0);

                StmtNode* stmt = mod->body[int(mod->body.size()) - 1];
                std::cout << c.call << "\n";
                REQUIRE(str(stmt) == c.call);
            }


            StaticSingleAssignment ssa;
            Module* ssa_mod = cast<Module>(ssa.module(mod, 0));

            REQUIRE(errors == c.errors);

            if (c.expected_type != "") {
                REQUIRE(c.expected_type == str(deduced_type));
            }
            delete mod;

            kwinfo(outlog(), "<<<<<<<<<<<<<<<<<<<<<<<< DONE");

        }
        i += 1;
    }
}

/*
TEST_CASE("Class_Attribute_Lookup") {
    // Futures tests cases
    run_testcase("sema", "ClassDef_New", sema_cases());
}*/



#if 1
#define GENTEST(name)                                               \
    TEMPLATE_TEST_CASE("SEMA_" #name, #name, name) {                \
        auto cases = get_test_cases("cases", #name);\
        run_testcase("sema", str(nodekind<TestType>()), cases); \
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