#include "token.h"
#include "../fmt.h"

namespace lython
{

String tok_to_string(int8 t){
    switch(t){
#define X(name, nb) case nb: return String(#name);
    LYTHON_TOKEN
#undef X
    default:
        String s = "' '";
        s[1] = t;
        return s;
    }
}

// this should be computed at compile time
// this is used for pretty printing
int8 tok_name_size()
{
    std::vector<String> v = {
    #define X(name, nb) #name,
        LYTHON_TOKEN
    #undef X
    };

    int8 max = 0;

    for (auto& i:v)
        max = std::max(int8(i.size()), max);

    return max;
}

std::ostream& Token::debug_print(std::ostream& out){
    out << align_right(tok_to_string(_type), tok_name_size());

    out << " =>" << " [l:" << to_string(_line, 4)
                  << ", c:" << to_string(_col, 4) << "] " << _identifier ;
    return out;
}

// could be used for code formatting
std::ostream& Token::print(std::ostream& out, int32 indent){
    // Keep track of some variable for correct printing
    static int32 indent_level = 0;
    static bool emptyline = true;   // To generate indent when needed
    static bool open_parens = false;

    if (indent > 0)
        indent_level = indent;

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
        out << std::string(std::size_t(indent_level * LYTHON_INDENT), ' ');


    String& str = keyword_as_string()[type()];

    if (str.size() > 0){
        emptyline = false;
        if (type() == tok_arrow)
            out << " ";

        return out << str;
    }

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
    else if (type() == tok_docstring)
        out << "\"\"\"" << identifier() << "\"\"\"";
    else if (type() == tok_int || type() == tok_float)
        out << identifier();
    else
        out << identifier();

    open_parens = false;
    emptyline = false;

    return out;
}

ReservedKeyword& keywords(){
    static ReservedKeyword _keywords = {
    #define X(str, tok) {str, tok},
        LYTHON_KEYWORDS
    #undef X
    };
    return _keywords;
}

KeywordToString& keyword_as_string(){
    static KeywordToString _keywords = {
    #define X(str, tok) {int(tok), String(str)},
        LYTHON_KEYWORDS
    #undef X
    };
    return _keywords;
}

}
