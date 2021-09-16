#include <catch2/catch.hpp>

#include <sstream>

#include "samples.h"

#include "lexer/buffer.h"
#include "parser/parser.h"
#include "utilities/strings.h"

using namespace lython;


inline String parse_it(String code){
    StringBuffer reader(code);
    Module module;

    Lexer lex(reader);
    Parser parser(lex);

    StringStream ss;

    Expression expr;
    int k = 0;
    do {
        expr = parser.parse_one(module);

        if (expr){
            parse_sublocks(expr, module);

            expr.print(ss) << "\n\n";
            k += 1;
        }
    } while(expr);

    assert(k > 0, "Should parse more than one expression");
    return ss.str();
}

#define TEST_PARSING(code)\
    SECTION(#code){\
        REQUIRE(strip(parse_it(code())) == strip(code()));\
    }

TEST_CASE("Parser"){
    CODE_SAMPLES(TEST_PARSING)

    IMPORT_TEST(TEST_PARSING)
}
