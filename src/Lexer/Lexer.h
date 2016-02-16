#ifndef LYTHON_LEXER_LEXER_HEADER
#define LYTHON_LEXER_LEXER_HEADER

#include <string>
#include <unordered_map>

using namespace std;

#include "Token.h"
#include "Buffer.h"

namespace LIBNAMESPACE{

/*! Lexer - This provides a simple interface that turns a text buffer into a
 *  stream of tokens.  This provides no support for file reading or buffering,
 *  or buffering/seeking of tokens, only forward lexing is supported.
 */
class Lexer
{
public:

    enum LexerMode
    {
        Norm,       // Skip Comments
        Comments,   // Don't Skip Comments
    };

    Lexer(AbstractBuffer& buf);

    const char& nextc();
    const int& indent() const   {   return _buffer.indent();    }
    const bool& empty_line() const {    return _buffer.is_line_empty();    }

    const int& line() const {   return _buffer.current_line(); }
    const int& col() const {   return _buffer.current_col(); }

    // cut the buffer into words
    int get_token();

    const double& value() const;
    const string& identifier() const;

    void print(std::ostream& str);

    string type()
    {
        switch(_type)
        {
        case Integer:
            return "Integer";
        case Float:
            return "Float";
        default:
            return "String";
        }
    }

    void discard_identifier()
    {
        _identifier = "";
    }

    unordered_map<char, int> special_char;

    // const int& lookahead(const int& k)

protected:

    char            c;
    double          _value;
    string          _identifier;
    AbstractBuffer& _buffer;
    IdentifierType  _type;
    unsigned int    _previous_indent;
    unordered_map<string, int> _keywords;

};

}


#endif
