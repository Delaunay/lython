#include <iostream>
#include <sstream>

#include "AbstractSyntaxTree/Expressions.h"
#include "Lexer/Buffer.h"
#include "Lexer/Lexer.h"

#include "Lexer/Prelexer.h"

#include "revision_data.h"

/*
 *  One of the problem of using streams instead of vector of tokens
 *  is that i becomes harder to add intermediate steps between
 *      Prelexing and lexing
 *
 *  The lexer can extract its Token into a vector if needed.
 */

using namespace lython;

int main()
{
    std::cout << "\n"
                 "[0] Lython Interpreter \n"
                 "[0]   Compiler: " COMPILER_ID " " COMPILER_VERSION "\n"
                 "[0]     Branch: " _BRANCH "\n"
                 "[0]    Version: " _HASH "\n"
                 "[0]       Date: " _DATE "\n\n";

    std::string code(
        "%% Preblock\n"
        "{\n"
        "    % this is a block\n"
        "    \"The answer is 42\";\n"
        "    "
        "    a = 3;\n"
        "    \n"
        "    {\n"
        "        \"The answer is 42\";\n"
        "    \n"
        "        a = 3;\n"
        "    }\n"
        "}\n"
        ""
        "%% Prestring\n"
        "%% --> String variable\n"
        "\"The answer is 42\";\n"
        "\n"
        "%% Pretoken\n"
        "%% --> Everything else\n"
        "a = 3;\n"
        "b = 2.12;"
    );

    std::cout << "\n\n====\n"
                 "     PreLexer - PreToken\n"
                 "==============================" << std::endl;

    StringBuffer reader(code);

    Prelexer pl(reader);
    pl.debug_print(std::cout);

    std::cout << "\n\n====\n"
                 "     Lexer - Sexp\n"
                 "==============================" << std::endl;

    reader.reset();

    Lexer l(reader);
    l.debug_print(std::cout);

    std::cout << "\n\n====\n"
                 "     Parsing - Pexp\n"
                 "==============================" << std::endl;




    //for(int i = 0; i < 10; ++i)
    //    std::cout << reader.nextc() << std::endl;

    //

    //pl.next_pretoken().debug_print(std::cout);

    //auto cst = AST::Constant<int>(10, "int");
    //auto pl1 = AST::Placeholder("name", "double");
    //auto pl2 = AST::Placeholder("name", "int");

    /*
    // debug info


    ConsoleBuffer reader;

    Lexer lex(reader);

    // print back what the user just inputed
    lex.print(std::cout);*/

    return 0;
}
