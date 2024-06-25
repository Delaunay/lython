
#include <catch2/catch_all.hpp>

// Kiwi
#include "lexer/lexer.h"
#include "utilities/strings.h"

// Testing
// #include "cases.h"
#include "cases_sample.h"
#include "libtest.h"

using namespace lython;

String lex_it(String code) {
    StringBuffer reader(code);
    Lexer        lex(reader);

    StringStream ss;
    lex.print(ss);
    auto str =  ss.str();;

    std::cout << str << "\n";
    return str;
}

#define TEST_LEXING(code) \
    SECTION(#code) { REQUIRE(strip(lex_it(code())) == strip(code())); }

TEST_CASE("Lexer") {
    CODE_SAMPLES(TEST_LEXING)
    IMPORT_TEST(TEST_LEXING)
}


TEST_CASE("Lexer_integer") {
    TEST_LEXING([](){ return "1"; })
}

TEST_CASE("Lexer_float") {
    TEST_LEXING([](){ return "1.0"; })
}

/*
void run_testcase(String const &name, Array<TestCase> cases) {
    kwinfo("Testing {}", name);
    for (auto &c: cases) {
        REQUIRE(strip(lex_it(c.code)) == strip(c.code));
        kwinfo("<<<<<<<<<<<<<<<<<<<<<<<< DONE");
    }
}

#define GENTEST(name)                                               \
    TEMPLATE_TEST_CASE("PARSE_" #name, #name, name) {               \
        run_testcase(str(nodekind<TestType>()), name##_examples()); \
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
*/