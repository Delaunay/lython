#include "Lexer.h"
#include "../config.h"

//#include <cctype>
//#include <cstdio>
//#include <cstdlib>
//#include <map>
//#include <string>
//#include <vector>

namespace lython
{

Lexer::Lexer(AbstractBuffer& buf):
    _buffer(buf), _value(0),
    _identifier(string("")), _type(String), c(' '),
    _previous_indent(indent())
    //_indent(0)
{
    _keywords["def"]    = tok_def;
    _keywords["extern"] = tok_extern;
    _keywords["if"]     = tok_if;
    _keywords["else"]   = tok_else;
    _keywords["for"]    = tok_for;
    _keywords["unary"]  = tok_unary;
    _keywords["binary"] = tok_binary;
    _keywords["in"]     = tok_in;
    _keywords["class"]  = tok_class;

    special_char['+'] = 1;
    special_char['-'] = 1;
    special_char['/'] = 1;
    special_char['*'] = 1;
    special_char['='] = 1;
    special_char['^'] = 1;
    special_char['.'] = 1;
}

const char& Lexer::nextc()
{
    return _buffer.nextc();
}

// cut buffer into words
int Lexer::get_token()
{
//    static char c = ' ';

    if (c == '\n')
    {
        c = nextc();
        return tok_newline;
    }

    // Skip any whitespace.
    while (isspace(c))
    {
        c = nextc();

        if (c == '\n')
        {
            c = nextc();
            return tok_newline;
        }
    }

    // if the line is empty the indentation change does not matter
    if (_previous_indent < indent() && !empty_line())
    {
        _previous_indent = indent();
        return tok_indent;
    }

    if (_previous_indent > indent() && !empty_line())
    {
        _previous_indent = indent();
        return tok_desindent;
    }

    // Group digits/words
    if (isalpha(c) || isdigit(c))
    {
        if (isalpha(c))
            _type = String;
        else if (isdigit(c))
            _type = Integer;

        _identifier = c;

        c = nextc();

        while (isalnum(c) || c == '.' || c == '_')
        {
            // handles the case 1.2.3 which is considered a string
            if (c == '.')
            {
                if (_type == Integer)
                    _type = Float;
                else
                    _type = String;
            }

            // case where 1000g
            if (isalpha(c))
                _type = String;

           _identifier += c;
           c = nextc();
        }

        if (_type == Float || _type == Integer)
        {
            _value = strtod(_identifier.c_str(), 0);
            return tok_number;
        }

        if (_keywords.count(_identifier) > 0)
            return _keywords[_identifier];
        else
            return tok_identifier; //TOK_IDENTIFIER;
    }

    if (c == '#')
    {
        // Comment until end of line.
        do
        {
            c = nextc();
        }
        while (c != EOF && c != '\n' && c != '\r');

        if (c != EOF)
            return get_token();
    }

    // Check for end of file.  Don't eat the EOF.
    if (c == EOF)
        return tok_eof;

    // group Special character
    // Special Character are character that should be grouped together
    // for example when writting '++' '+=' '=/' '--' they should be grouped
    if (special_char.count(c) > 0)
    {
        _identifier = c;
        c = nextc();

        while(special_char.count(c) > 0)
        {
            _identifier += c;
            c = nextc();
        }
        return tok_identifier;
    }

    // String litteral
    if (c == '\'' || c == '"')
    {
        char c_end = c;
        _identifier = "";
        c = nextc();

        while(1)
        {
            _identifier += c;
            c = nextc();

            if (c == c_end)
                break;
        }

        // Eat c_end
        c = nextc();
        return tok_string_lit;
    }

    // Otherwise, just return the character as its ascii value.
    char tc = c;
    c = nextc();

    return tc;
}

const double& Lexer::value() const {   return _value;  }
const string& Lexer::identifier() const {  return _identifier; }

void Lexer::print(std::ostream& str)
{
    // save cursor position
    //int curs = _buffer.cursor();

    // restart
    _buffer.restart();
    c = ' ';

    int k = 0,
        t = 0;

    str << "===============================================================================\n"
           "                   Lexer\n"
           "===============================================================================\n\n";

    str << "Tokens = [\n";

    do
    {
        k++;
        t = get_token();
        str << "    ";

        if (k < 10)
            str << "000";
        else if (k < 100)
            str << "00";
        else if (k < 1000)
            str << "0";

        str << k << ": (tok: [" << t << ", '" << char(t) << "'], ";

        switch(t)
        {
        case tok_def:
            str << MBLUE  "def" MRESET ": '" << identifier() << "'";
            break;

        case tok_string_lit:
            str << MBLUE  "string" MRESET ": '" << identifier() << "'";
            break;

        case tok_extern:
            str << MBLUE  "ext" MRESET ": '" << identifier() << "'";
            break;

        case tok_identifier:
        case tok_if:
//        case tok_then:
//        case tok_var:
        case tok_else:
        case tok_in:
        case tok_for:
        case tok_unary:
        case tok_binary:

            str << MGREEN "ide" MRESET ": '" << identifier() << "'";
            break;

        case tok_indent:
            str << MGREEN "Indent" MRESET ": '" << indent() << "'";
            break;

        case tok_desindent:
            str << MGREEN "Desindent" MRESET ": '" << indent() << "'";
            break;

        case tok_newline:
            str << MGREEN "Newline" MRESET;
            break;

        case tok_number:
            str << MYELLOW "Value" MRESET ": '" << value() << "'";
            break;

        case tok_eof:
            str << MGREEN "EOF" MRESET;
            break;

        default:
            str << MRED "chr" MRESET ": '" << char(t) << "'";
            break;
        }

        str << ")\n";
        /*
        str << "\tpos: [line: " << _buffer.current_line()
            << ", col: "    << _buffer.current_col()
            << ", cursor: " << _buffer.cursor() << "]), \n";//*/

    } while(t != lython::tok_eof);

    str << "] (size: " << k << ")\n";

    // put the cursor back
    //_buffer.set_cursor(curs);
    _buffer.restart();
    c = ' ';
}

}
