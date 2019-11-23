#include "samples.h"

#include <sstream>

#include "lexer/buffer.h"
#include "parser/parser.h"

using namespace lython;


String parse_it(String code){
    StringBuffer reader(code);
    Module module;

    Parser par(reader, &module);

    StringStream ss;
    par.parse_one()->print(ss);
    return ss.str();
}

#define TEST_PARSING(code)\
    SECTION(#code){\
        REQUIRE(parse_it(code()) == code());\
    }

TEST_CASE("Parser"){
    CODE_SAMPLES(TEST_PARSING)
}
