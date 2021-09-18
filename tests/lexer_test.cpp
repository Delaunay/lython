#include <catch2/catch.hpp>

#include "lexer/lexer.h"
#include "samples.h"
#include "utilities/strings.h"

using namespace lython;

String lex_it(String code) {
    StringBuffer reader(code);
    Lexer        lex(reader);

    StringStream ss;
    lex.print(ss);
    return ss.str();
}

#define TEST_LEXING(code) \
    SECTION(#code) { REQUIRE(strip(lex_it(code())) == strip(code())); }

TEST_CASE("Lexer") {
    CODE_SAMPLES(TEST_LEXING)
    IMPORT_TEST(TEST_LEXING)
}
