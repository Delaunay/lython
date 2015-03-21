#include <iostream>

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"

using namespace std;

//===----------------------------------------------------------------------===//
// "Library" functions that can be "extern'd" from user code.
//===----------------------------------------------------------------------===//

/// putchard - putchar that takes a double and returns 0.
extern "C" double putchard(double X) {
  putchar((char)X);
  return 0;
}

int main()
{
    //lython::StandardInputBuffer buf;
    //lython::FileBuffer buf("../code/code.ly");
    //lython::FileBuffer buf("../code/if_test.ly");
    //lython::FileBuffer buf("../code/if_test.ly");

    lython::FileBuffer buf("../code/test5.ly");
    lython::Lexer      lex(buf);
    lython::Parser     par(lex);

    buf.restart();
    buf.print(cout);

    printf("\n");

    lex.print(cout);

    printf("\n");

    par.parse();


    return 0;
}

