#include <gtest/gtest.h>

#include <sstream>

#include "Lexer/Buffer.h"
#include "Parser/Parser.h"

using namespace lython;


std::string parse_it(std::string code){
    StringBuffer reader(code);
    Parser par(reader);

    std::stringstream ss;
    par.parse_one()->print(ss);
    return ss.str();
}

TEST(Parser, Function){
    std::string code =
            "def my_function(a: b, c: d) -> e:\n"
            "    return 1\n";

    ASSERT_EQ(parse_it(code), code);
}

TEST(Parser, FunctionNoArgs){
    std::string code =
            "def my_function() -> e:\n"
            "    return 1\n";

    ASSERT_EQ(parse_it(code), code);
}

TEST(Parser, FunctionDocString){
    std::string code =
            "def my_function(a: b, c: d) -> e:\n"
            "    \"\"\"This is a docstring\"\"\"\n"
            "    return 1\n";

    ASSERT_EQ(parse_it(code), code);
}

TEST(Parser, Struct){
    std::string code =
            "struct a:\n"
            "    b: c\n";

    ASSERT_EQ(parse_it(code), code);
}

TEST(Parser, StructDocString){
    std::string code =
            "struct a:\n"
            "    \"\"\"This is a docstring\"\"\"\n"
            "    b: c\n";

    ASSERT_EQ(parse_it(code), code);
}
