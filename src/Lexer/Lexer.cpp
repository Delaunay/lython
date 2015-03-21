#include "Lexer.h"

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
    _identifier(string("")), c(' ')
    //_indent(0)
{}

const char& Lexer::nextc()
{
    return _buffer.nextc();
}

// cut the buffer into words
int Lexer::get_token()
{
    if (c == '\n')
    {
        c = nextc();
        return tok_newline;
    }

    // Skip any whitespace.
    while (isspace(c) )
    {
        c = nextc();

        if (c == '\n')
            return tok_newline;
    }

    // identifier: [a-zA-Z][a-zA-Z0-9]*
    if (isalpha(c))
    {
        _identifier = c;

        c = nextc();

        while (isalnum(c))
        {
            _identifier += c;
            c = nextc();
        }

        // put the language keyword into a dictionary {identifier=> tok_number}
        // make those lines disapear
        if (_identifier == "def")
            return tok_def;

        if (_identifier == "extern")
            return tok_extern;

        if (_identifier  == "if")
            return tok_if;

        if (_identifier  == "then")
            return tok_then;

        if (_identifier  == "else")
            return tok_else;

        if (_identifier  == "for")
            return tok_for;

        if (_identifier  == "in")
            return tok_in;

        return tok_identifier;
    }

    // Number: [0-9.]+
    if (isdigit(c) || c == '.')
    {
        string numstr;

        do
        {
            numstr += c;
            c = nextc();

        } while (isdigit(c) || c == '.');

        _value = strtod(numstr.c_str(), 0);

        return tok_number;
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

    // Otherwise, just return the character as its ascii value.
    char tc = c;
    c = nextc();

    // printf("\n '%c' '%c' \n", c, tc);
    return tc;
}

const double& Lexer::value() const {   return _value;  }
const string& Lexer::identifier() const {  return _identifier; }

void Lexer::print(std::ostream& str)
{
    // save cursor position
    int curs = _buffer.cursor();

    // restart
    _buffer.restart();

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
            str << BLUE  "def" RESET ": '" << identifier() << "'";
            break;

        case tok_extern:
            str << BLUE  "ext" RESET ": '" << identifier() << "'";
            break;

        case tok_identifier:
            str << GREEN "ide" RESET ": '" << identifier() << "'";
            break;

        case tok_then:
            str << GREEN "ide" RESET ": '" << identifier() << "'";
            break;

        case tok_else:
            str << GREEN "ide" RESET ": '" << identifier() << "'";
            break;

        case tok_if:
            str << GREEN "ide" RESET ": '" << identifier() << "'";
            break;

        case tok_newline:
            str << GREEN "ide" RESET ": '\\n'";
            break;

        case tok_number:
            str << YELLOW "val" RESET ": '" << value() << "'";
            break;

        case tok_eof:
            str << GREEN "ide" RESET ": 'EOF'";
            break;

        default:
            str << RED "chr" RESET ": '" << char(t) << "'";
            break;
        }

        str << ")\n";
        /*
        str << "\tpos: [line: " << _buffer.current_line()
            << ", col: "    << _buffer.current_col()
            << ", cursor: " << _buffer.cursor() << "]), \n";*/

    } while(t != lython::tok_eof);

    str << "] (size: " << k << ")\n";

    // put the cursor back
    _buffer.set_cursor(curs);
}

}
