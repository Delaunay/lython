#include <iostream>
#include <fstream>

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"

using namespace std;

//===----------------------------------------------------------------------===//
// "Library" functions that can be "extern'd" from user code.
//===----------------------------------------------------------------------===//

/// putchard - putchar that takes a double and returns 0.
extern "C" double putchard(double X)
{
    putchar((char)X);
    return 0;
}

using namespace LIBNAMESPACE;

#define LEXER  1
#define PARSER 1
#define MODULE 1

int main()
{
    //lython::StandardInputBuffer buf;
    //lython::FileBuffer buf("../code/code.ly");
    //lython::FileBuffer buf("../code/if_test.ly");
    //lython::FileBuffer buf("../code/test5.ly");
    //lython::FileBuffer buf("../code/for_test.ly");

    //lython::FileBuffer buf("../code/unary_op.ly");
    //lython::FileBuffer buf("../code/return_stmt.ly");
    lython::FileBuffer buf("../code/class_test.ly");

#if LEXER
    lython::Lexer      lex(buf);
#endif

    std::ofstream file("../log/lython_parser.log");

    Scope module("class_test");

#if PARSER
    lython::Parser     par(lex, module, file);
#endif

    buf.restart();
    buf.print(file);

#if LEXER
    lex.print(file);
#endif

#if PARSER
    par.parse();
#endif

#if MODULE
    std::cout << "\n\n";
//    module.print(std::cout);
    module.print(file);
#endif

    return 0;
}

