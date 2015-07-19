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

using namespace lython;

#define LEXER  1
#define PARSER 1

int main()
{
    //lython::StandardInputBuffer buf;
    //lython::FileBuffer buf("../code/code.ly");
    //lython::FileBuffer buf("../code/if_test.ly");
    //lython::FileBuffer buf("../code/test5.ly");
    //lython::FileBuffer buf("../code/for_test.ly");

//    lython::FileBuffer buf("../code/unary_op.ly");
//    lython::FileBuffer buf("../code/return_stmt.ly");
    lython::FileBuffer buf("../code/python_test.ly");

#if LEXER
    lython::Lexer      lex(buf);
//    cout << "here" << "\n";
#endif

    std::ofstream file("../log/lython_parser.log");

#if PARSER
    lython::Parser     par(lex, file);
#endif

    buf.restart();
    buf.print(file);

#if LEXER
//    printf("\n");
    lex.print(file);
#endif

#if PARSER
//    printf("\n");
    par.parse();
#endif


    return 0;
}

