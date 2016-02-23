#ifndef PARSER_H
#define PARSER_H

#include "../Lexer/Lexer.h"
#include "Grammar.h"
#include "pexp.h"

namespace lython{

/*
class SexpParser
{
public:
};*/

class SexpParser
{
public:
    SexpParser(AbstractBuffer& buffer):
        _lex(buffer)
    {}

    // Shortcut
    Token next_token()  {   return _lex.next_token();   }

    // Parsing routines
    Sexp next_sexp(){

        Token tok = next_token();

        // if Symbol check if operator
        switch(tok->type()){
        case Sexp_Symbol:

            Precedence pre = _g[get_str(tok)];

            if (pre){

            }else{

            }

        }

        // Type must be epsilon or Node
    }

    // return One Top level Expression (Functions)


private:
    Grammar _g;
    Lexer _lex;
};

}

#endif // PARSER_H

/*
 *  // Can't use RTTI in a swith
        switch(tok->type()){
        // Pimm
        case Sexp_Block:
        case Sexp_Integer:
        case Sexp_Float:
        case Sexp_String:
            return Pexp(new Pimm(tok));
        // Symbol
        case Sexp_Symbol:
            return Pexp(new Pvar(tok));
        }*/


