#include <gtest.h>

#include <sstream>

#include "AbstractSyntaxTree/Expressions.h"
#include "Lexer/Buffer.h"
#include "Lexer/Lexer.h"

using namespace lython;

TEST(Lexer, Lexer){

    std::string code = "        def my_function(a: b, c: d):\n"
                       "            return 1\n"         // tok_int
                       "    def my_function():\n"       // correct indent management
                       "        return 1.1\n"           // tok_float
                       "a = \"2 + 2\"\n"            // tok_identifier '=' tok_string
                       "b = 1yy\n"                  // tok_identifier '=' tok_incorrect
                       "c = 1.1.1\n";               // tok_identifier '=' tok_incorrect
                       
    StringBuffer reader(code);

    Lexer lex(reader);

    // print out read tokens
    std::stringstream ss;

    lex.print(ss);

    std::string result = ss.str();
    
    ASSERT_EQ(result, ss);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();

    return 0;
}
