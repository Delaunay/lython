#pragma once

#include <ostream>
#include <string>
#include <vector>
#include <algorithm>

#include "../Types.h"

/*
 *  incorrect is used when the input is known to be wrong
 *  but we want to parse the project anyway
 *
 *  Add?
 *      Hex: 0x000A9F
 *      bin: bx010101
 */
#define LYTHON_INDENT 4
#define LYTHON_TOKEN \
    X(tok_identifier, -1)\
    X(tok_float,      -2)\
    X(tok_string,     -3)\
    X(tok_int,        -4)\
    X(tok_newline,    -5)\
    X(tok_indent,     -6)\
    X(tok_desindent,  -7)\
    X(tok_incorrect,  -8)\
    X(tok_eof,        -9)
    
namespace lython{

enum TokenType{
#define X(name, nb) name = nb,
    LYTHON_TOKEN
#undef X
};

uint8 tok_name_size();
std::string tok_to_string(int8 t);

class Token{
public:
    Token(TokenType t, uint32 l, uint32 c):
        _type(t), _line(l), _col(c)
    {}

    Token(int8 t, uint32 l, uint32 c):
        _type(t), _line(l), _col(c)
    {}

     int8  type()   {   return _type;   }
    uint32 line()   {   return _line;   }
    uint32 col()    {   return _col;    }

    int32 end_line() { return col() - 1;} // Lexer is one char in advance
    int32 begin_line() {   return col() - identifier().size() - 1; }

    std::string& identifier() { return _identifier; }
    float64      as_float()   { return std::stod(_identifier); }
    int32        as_integer() { return std::stoi(_identifier); }

    operator bool(){
        return _type != tok_eof;
    }

private:
    int8    _type;
    uint32  _line;
    uint32  _col;

    // Data
    std::string _identifier;

public:
    // print all tokens and their info
    std::ostream& debug_print(std::ostream& out);

    // could be used for code formatting
    std::ostream& print(std::ostream& out);
};

inline
Token& dummy(){
    static Token dy = Token(tok_incorrect, -1, -1);
    return dy;
}

// Make something that look like clang's error underlining.
// offset is used if you need to print multiple underline on a same line
inline
std::ostream& underline(std::ostream& out, Token& t, int32 offset = 0){
    int32 start = t.begin_line() - offset;
    if (start > 0){
        out << std::string(start, ' ');

        if (t.identifier().size() > 0)
            out << std::string(t.identifier().size(), '~');
        else
            out << "~";
    }

    return out;
}

}
