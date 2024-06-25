#include <catch2/catch_all.hpp>

// Kiwi
#include "ast/ops.h"
#include "parser/parser.h"
#include "utilities/printing.h"
#include "utilities/strings.h"
#include "logging/logging.h"

// Test
//#include "cases.h"
#include "cases_sample.h"
#include "libtest.h"

using namespace lython;

#define TEST_PARSING(code) \
    SECTION(#code) { REQUIRE(strip(parse_it(code())) == strip(code())); }

void run_testcase_notequal();

TEST_CASE("Equality_notequal") { run_testcase_notequal(); }

Array<Array<TestCase>> const& Not_Equal_examples() {
    static Array<Array<TestCase>> ex = {
        {
            // node kind is different
            {"continue"},
            {"pass"},
        },
        {
            // value is different
            {"1"},
            {"2"},
        },
        {
            /// type is different
            {"1"},
            {"1.2"},
        },
        {
            // variable name is different
            {"a"},
            {"b"},
        },
    };
    return ex;
}

// Equality is longest when checking for code that are equals
// Unequal code will fail very fast as node kind will be different
inline bool equal_it(String code_a, String code_b) {

    StringBuffer reader1(code_a);
    Lexer        lex1(reader1);
    Parser       parser1(lex1);
    Module*      mod1 = parser1.parse_module();
    lyassert(mod1->body.size() > 0, "Should parse more than one expression");

    StringBuffer reader2(code_b);
    Lexer        lex2(reader2);
    Parser       parser2(lex2);
    Module*      mod2 = parser2.parse_module();
    lyassert(mod2->body.size() > 0, "Should parse more than one expression");

    auto a        = mod1->body[0];
    auto b        = mod2->body[0];
    auto is_equal = equal(a, b);

    delete mod1;
    delete mod2;
    return is_equal;
}

inline bool equal_it(String code_a) { return equal_it(code_a, code_a); }

void run_testcase_notequal() {
    auto& cases = Not_Equal_examples();

    kwinfo(outlog(), "Testing {}", "NotEqual");
    for (auto& c: cases) {
        auto a = c[0];
        auto b = c[1];

        REQUIRE(equal_it(a.code, b.code) == false);
        kwinfo(outlog(), "<<<<<<<<<<<<<<<<<<<<<<<< DONE");
    }
}

void run_testcase(String const& name, Array<TestCase> cases) {
    kwinfo(outlog(), "Testing {}", name);
    for (auto& c: cases) {
        REQUIRE(equal_it(c.code) == true);
        kwinfo(outlog(), "<<<<<<<<<<<<<<<<<<<<<<<< DONE");
    }
}

#define GENTEST(name)                                               \
    TEMPLATE_TEST_CASE("EQUAL_" #name, #name, name) {               \
        auto cases = get_test_cases("cases", #name);\
        run_testcase(str(nodekind<TestType>()), cases); \
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
