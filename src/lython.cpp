#include <iostream>
#include <sstream>

#include "AbstractSyntaxTree/Expressions.h"
#include "Lexer/Buffer.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"

// #include "Lexer/Prelexer.h"

#include "revision_data.h"

using namespace lython;

int main()
{
    //auto cst = AST::Constant<int>(10, "int");
    //auto pl1 = AST::Placeholder("name", "double");
    //auto pl2 = AST::Placeholder("name", "int");

    //*
    // debug info
    std::cout << "\n"
                 "[0] Lython Interpreter \n"
                 "[0]   Compiler: " COMPILER_ID " " COMPILER_VERSION "\n"
                 "[0]     Branch: " _BRANCH "\n"
                 "[0]    Version: " _HASH "\n"
                 "[0]       Date: " _DATE "\n\n";

    //ConsoleBuffer reader;

    std::string code = "def function(test:double, test): -> double\n"
            //"    \"\"\" This is a docstring \"\"\"\n"
            "    return 1 + 1\n"
            "def function(test:int, test):\n"
            "    return 1 + 1\n";

    StringBuffer reader(code);

    ///*
    Lexer lex(reader);
    lex.print(std::cout);

    std::cout << std::endl;
    reader.reset(); //*/

    /*
    try{
        Parser par(reader);

        par.parse_one()->print(std::cout);
        std::cout << "\n\n";
        par.parse_one()->print(std::cout);
    }
    catch (lython::Exception e){
        std::cout << "Error Occured:" << std::endl;
        std::cout << "\t" << e.what() << std::endl;
    }

    // print back what the user just inputed
    //*/

    std::cout << std::endl;

    return 0;
}
