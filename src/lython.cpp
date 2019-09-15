#include <iostream>
#include <sstream>

#include "AbstractSyntaxTree/Expressions.h"
#include "Lexer/Buffer.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"

#include "Logging/logging.h"

// #include "Lexer/Prelexer.h"

#include "revision_data.h"

using namespace lython;

int main()
{
    info("Enter");

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

    std::string code =
            "def my_function1() -> e:\n"
            "    return 1\n"
            "\n"

            "def function2(test: double, test) -> double:\n"
            "    \"\"\"This is a docstring\"\"\"\n"
            "    return add(1, 1)\n\n"

            "def function3(test: int, test) -> e:\n"
            "    return add(1, 1)\n\n"

            "struct Object:\n"
            "    \"\"\"This is a docstring\"\"\"\n"
            "    a: Type\n";

    StringBuffer reader(code);

    {
        Lexer lex(reader);
        lex.print(std::cout);
        //lex.debug_print(std::cout);
        std::cout << std::endl;
    }

    reader.reset();
    Dict<std::string, ST::Expr> module;

    try{
        Parser par(reader, module);

        auto expr1 = par.parse_one();
        std::cout << "--\n\n";
        expr1->print(std::cout) << "\n";

        auto expr2 = par.parse_one();
        std::cout << "--\n\n";
        expr2->print(std::cout) << "\n";

        auto expr3 = par.parse_one();
        std::cout << "--\n\n";
        expr3->print(std::cout) << "\n";

        auto expr4 = par.parse_one();
        std::cout << "--\n\n";
        expr4->print(std::cout) << "\n";
    }
    catch (lython::Exception e){
        std::cout << "Error Occured:" << std::endl;
        std::cout << "\t" << e.what() << std::endl;
    }

    std::cout << std::string(80, '-') << '\n';

    for(auto expr: module){
        std::cout << expr.first << ":\n";
        expr.second->print(std::cout) << "\n\n";
    }

    // print back what the user just inputed
    //*/

    std::cout << std::endl;

    return 0;
}
