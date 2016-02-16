#include "Tokens.h"
#include "../fmt.h"

namespace lython
{

std::string tok_to_string(int8 t){
    switch(t){
#define X(name, nb) case nb: return std::string(#name);
    LYTHON_TOKEN
#undef X
    default:
        std::string s = "' '";
        s[1] = t;
        return s;
    }
}

// this should be computed at compile time
// this is used for pretty printing
uint8 tok_name_size()
{
    std::vector<std::string> v = {
    #define X(name, nb) #name,
        LYTHON_TOKEN
    #undef X
    };

    std::string::size_type max = 0;

    for (auto& i:v)
        max = std::max(i.size(), max);

    return max;
}

std::ostream& Token::debug_print(std::ostream& out){
    out << align_right(tok_to_string(_type), tok_name_size());

    out << " =>" << " [l:" << to_string(_line, 4)
                  << ", c:" << to_string(_col, 4) << "] " << _identifier ;
    return out;
}

// could be used for code formatting
std::ostream& Token::print(std::ostream& out){
    // Keep track of some variable for correct printing
    static int8 indent_level = 0;
    static bool emptyline = true;   // To generate indent when needed
    static bool open_parens = false;

    // because indent_level is static we need a token to reset the value
    if (type() == tok_eof){
        indent_level = 0;
        emptyline = true;
        open_parens = false;
        return out;
    }

    // Invisible Token
    if (type() == tok_indent){
        indent_level += 1;
        return out;
    }

    if (type() == tok_desindent){
        indent_level -= 1;
        return out;
    }

    if (type() == tok_newline){
        out << std::endl;
        emptyline = true;
        return out;
    }

    // Indentation
    if (emptyline && indent_level > 0)
        out << std::string(indent_level * LYTHON_INDENT, ' ');

    // Single Char
    if (type() > 0){
        if (type() == '(' || type() == '[')
            open_parens = true;
        else
            open_parens = false;

        if (type() == '=')
            out << ' ';

        emptyline = false;
        out << type();
        return out;
    }

    // Everything else
    // we printout what the lexer read in identifier (no risk of error)

    // no space if the line is empty and no space if it is just after
    // a open parens
    if (!emptyline && !open_parens)
        out << ' ';

    if (type() == tok_string)
        out << '"' << identifier() << '"' ;
    else
        out << identifier();

    open_parens = false;
    emptyline = false;

    return out;
}

}
