#include <catch2/catch.hpp>

#include <sstream>

#include "samples.h"

#include "ast/magic.h"
#include "lexer/buffer.h"
#include "parser/parser.h"
#include "utilities/strings.h"

using namespace lython;

inline String parse_it(String code) {
    StringBuffer reader(code);
    Module       module;

    Lexer  lex(reader);
    Parser parser(lex);

    Module *mod = parser.parse_module();
    assert(mod->body.size() > 0, "Should parse more than one expression");

    return str(mod);
}

#define TEST_PARSING(code) \
    SECTION(#code) { REQUIRE(strip(parse_it(code())) == strip(code())); }

TEST_CASE("Parser") {
    CODE_SAMPLES(TEST_PARSING)
    IMPORT_TEST(TEST_PARSING)
}
