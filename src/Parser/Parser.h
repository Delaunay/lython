#ifndef PARSER_H
#define PARSER_H

#include "../Lexer/Lexer.h"

namespace lython{

class Parser
{
public:
    Parser(AbstractBuffer& buffer):
        _lex(buffer)
    {}

    // Shortcut
    Token next_token()  {   return _lex.next_token();   }

    // Parsing routines


    // return One Top level Expression (Functions)


private:

    Lexer _lex;
};

}

#endif // PARSER_H
