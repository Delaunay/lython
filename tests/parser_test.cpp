#include "cases.h"
#include "samples.h"

#include <catch2/catch.hpp>
#include <sstream>

#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "logging/logging.h"
#include "parser/parser.h"
#include "utilities/strings.cpp"

using namespace lython;

inline String parse_it(String code) {
    StringBuffer reader(code);
    Module       module;

    Lexer  lex(reader);
    Parser parser(lex);

    Module *mod = parser.parse_module();
    assert(mod->body.size() > 0, "Should parse more than one expression");

    auto data = str(mod);

    delete mod;
    return data;
}

#define TEST_PARSING(code) \
    SECTION(#code) { REQUIRE(strip(parse_it(code())) == strip(code())); }

TEST_CASE("Parser") {
    CODE_SAMPLES(TEST_PARSING)
    IMPORT_TEST(TEST_PARSING)
}

void run_testcase(String const &name, Array<TestCase> cases) {
    info("Testing {}", name);
    for (auto &c: cases) {
        REQUIRE(strip(parse_it(c.code)) == strip(c.code));
        info("<<<<<<<<<<<<<<<<<<<<<<<< DONE");
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
