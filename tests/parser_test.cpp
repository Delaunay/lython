#include "samples.h"

#include "ast/magic.h"
#include "lexer/buffer.h"
#include "parser/parser.h"
#include "utilities/strings.h"

#include <catch2/catch.hpp>
#include <sstream>

#include "logging/logging.h"

using namespace lython;

template <typename T>
Array<String> const &examples() {
    static Array<String> ex = {};
    return ex;
}

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

#define GENTEST(name)                                         \
    TEMPLATE_TEST_CASE("Parse_" #name, #name, name) {         \
        info("Testing {}", str(nodekind<TestType>()));        \
        Array<String> const &examples = TestType::examples(); \
                                                              \
        for (auto &code: examples) {                          \
            REQUIRE(strip(parse_it(code)) == strip(code));    \
            info("<<<<<<<<<<<<<<<<<<<<<<<< DONE");            \
        }                                                     \
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
