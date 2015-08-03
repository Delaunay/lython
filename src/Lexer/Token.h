#ifndef LYTHON_LEXER_TOKEN_HEADER
#define LYTHON_LEXER_TOKEN_HEADER

#include <string>

#include "../config.h"

using namespace std;

namespace LIBNAMESPACE{

enum TokenEnum
{
    tok_eof         = - 1,
    tok_newline     = - 2,
    tok_indent      = - 3,
    tok_desindent   = - 4,

    tok_extern      = - 5,  // Not Python
    tok_def         = - 6,
    tok_class       = - 7,
    tok_import      = - 8,
    tok_from        = - 9,

    // primary
    tok_identifier  = -10,
    tok_number      = -11,
    tok_string_lit  = -12,

    // control
    tok_if          = -13,
    tok_elif        = -14,
    tok_else        = -15,

    // loop
    tok_for         = -16,
    tok_while       = -17,
    tok_do          = -18,
    tok_in          = -19,

    // Not Python
    // User defined operators
    tok_binary      = -20,
    tok_unary       = -21,

    // Variable this must be deleted
    // tok_var         = -14,
};

// Number Type
enum IdentifierType
{
    String,
    Integer,
    Float
    // Binary and Hex ??
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
