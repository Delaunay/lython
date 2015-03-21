#ifndef LYTHON_LEXER_TOKEN_HEADER
#define LYTHON_LEXER_TOKEN_HEADER

#include <string>

using namespace std;

namespace lython{

enum TokenEnum
{
    tok_eof = -1,

    // commands
    tok_def = -2,
    tok_extern = -3,

    // primary
    tok_identifier = -4,
    tok_number = -5,

    // control
    tok_if = -6,
    tok_then = -7,  // can be deleted
    tok_else = -8,

    tok_newline = -9,

    // loop
    tok_for = -10,
    tok_in = -11,
};

class Token
{
    public:
        Token()
        {}

        bool has_value()
        {}

        double          _value;
        string          _identifier;
};
}

#endif
