#include <gtest/gtest.h>

#include <sstream>

#include "AbstractSyntaxTree/Expressions.h"
#include "Lexer/Buffer.h"
#include "Lexer/Lexer.h"

using namespace lython;

std::string lex_it(std::string code){
    StringBuffer reader(code);
    Lexer lex(reader);

    std::stringstream ss;
    lex.print(ss);
    return ss.str();
}

TEST(Lexer, Function){
    std::string code =
            "def my_function(a: b, c: d) -> e:\n"
            "    return 1\n"
            "\n";

    ASSERT_EQ(lex_it(code), code);
}

TEST(Lexer, FunctionDocString){
    std::string code =
            "def my_function(a: b, c: d) -> e:\n"
            "    \"\"\"This is a docstring\"\"\"\n"
            "    return 1\n"
            "\n";

    ASSERT_EQ(lex_it(code), code);
}

TEST(Lexer, Struct){
    std::string code =
            "struct a:\n"
            "    b: c\n"
            "\n";

    ASSERT_EQ(lex_it(code), code);
}

TEST(Lexer, StructDocString){
    std::string code =
            "struct a:\n"
            "    \"\"\"This is a docstring\"\"\"\n"
            "    b: c\n"
            "\n";

    ASSERT_EQ(lex_it(code), code);
}

TEST(Lexer, Misc){
    std::string code =
       "def my_function():\n"       // correct indent management
       "    return 1.1\n"           // tok_float

       "a = \"2 + 2\"\n"            // tok_identifier '=' tok_string
       "b = 1yy\n"                  // tok_identifier '=' tok_incorrect
       "c = 1.1.1\n";               // tok_identifier '=' tok_incorrect

    ASSERT_EQ(lex_it(code), code);
}


