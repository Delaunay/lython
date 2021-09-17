#include <spdlog/fmt/bundled/core.h>
#include "token.h"

namespace lython
{

String to_string(int8 t){
    switch(t){
    #define X(name, nb)\
        case nb: return String(#name);

        LYTHON_TOKEN(X)
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
        LYTHON_TOKEN(X)
    #undef X
    };

    int8 max = 0;

    for (auto& i:v)
        max = std::max(int8(i.size()), max);

    return max;
}

std::ostream& Token::debug_print(std::ostream& out){
    out << fmt::format("{:>20}", to_string(_type));

		out << " =>" << " [l:" << fmt::format("{:4}", _line)
				<< ", c:" << fmt::format("{:4}", _col) << "] `" << _identifier << "`";
    return out;
}

// could be used for code formatting
std::ostream& Token::print(std::ostream& out, int32 indent) const {
    // Keep track of some variable for correct printing
    static int32 indent_level = 0;
    static bool emptyline = true;   // To generate indent when needed
    static bool open_parens = false;
    static bool prev_is_op = false;

    if (indent > 0)
        indent_level = indent;

    // because indent_level is static we need a token to reset the value
    if (type() == tok_eof){
        indent_level = 0;
        emptyline = true;
        open_parens = false;
        prev_is_op = false;
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

    if (type() == tok_operator){
        if (operator_name() == "."){
            out << operator_name();
        } else {
            out << " " << operator_name() << " ";
        }

        prev_is_op = true;
        return out;
    }

    // Indentation
    if (emptyline && indent_level > 0)
        out << String(std::size_t(indent_level * LYTHON_INDENT), ' ');

    String const& str = keyword_as_string()[type()];

    if (str.size() > 0){
        emptyline = false;

        switch (type()) {
        case tok_arrow:
        case tok_as:
            out << " ";
            break;

        case tok_import:
            if (!emptyline){
                out << " ";
            }
            break;
        }

        return out << str;
    }

    // Single Char
    if (type() > 0){

        if (type() == '(' || type() == '[')
            open_parens = true;
        else
            open_parens = false;

        emptyline = false;
        out << type();
        return out;
    }

    // Everything else
    // we printout what the lexer read in identifier (no risk of error)

    // no space if the line is empty and no space if it is just after
    // a open parens
    if (!prev_is_op && !emptyline && !open_parens){
        out << ' ';
    }
    prev_is_op = false;

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
    prev_is_op = false;

    return out;
}

ReservedKeyword& keywords(){
    static ReservedKeyword _keywords = {
    #define X(str, tok) {str, tok},
        LYTHON_KEYWORDS(X)
    #undef X
    };
    return _keywords;
}

KeywordToString& keyword_as_string(){
    static KeywordToString _keywords = {
    #define X(str, tok) {int(tok), String(str)},
        LYTHON_KEYWORDS(X)
    #undef X
    };
    return _keywords;
}

}
