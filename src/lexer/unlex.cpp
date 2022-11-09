
#include "lexer/unlex.h"

namespace lython {

std::ostream& Unlex::format(std::ostream& out, Array<Token> const& tokens) {
    for (auto& tok: tokens) {
        format(out, tok);

        if (should_stop) {
            should_stop = false;
            break;
        }
    }
    return out;
}

std::ostream& Unlex::format(std::ostream& out, Token const& token, int indent) {
    if (indent > 0)
        indent_level = indent;

    // because indent_level is static we need a token to reset the value
    if (token.type() == tok_eof) {
        indent_level = 0;
        emptyline    = true;
        open_parens  = false;
        prev_is_op   = false;
        return out;
    }

    // Invisible Token
    if (token.type() == tok_indent) {
        indent_level += 1;
        return out;
    }

    if (token.type() == tok_desindent) {
        indent_level -= 1;
        return out;
    }

    if (token.type() == tok_newline) {
        emptyline = true;

        if (stop_on_newline) {
            should_stop = stop_on_newline;
            return out;
        }

        out << std::endl;
        return out;
    }

    if (token.type() == tok_operator || token.type() == tok_dot || token.type() == tok_in ||
        token.type() == tok_assign) {
        if (token.operator_name() == "." || token.operator_name() == "@") {
            out << token.operator_name();
        } else {
            out << " " << token.operator_name() << " ";
        }

        prev_is_op = true;
        return out;
    }

    // Indentation
    if (emptyline && indent_level > 0)
        out << String(std::size_t(indent_level * LYTHON_INDENT), ' ');

    String const& str = keyword_as_string()[token.type()];

    if (str.size() > 0) {
        emptyline = false;

        switch (token.type()) {
        case tok_arrow:
        case tok_as: out << " "; break;

        case tok_import:
            if (!emptyline) {
                out << " ";
            }
            break;
        }

        return out << str;
    }

    // Single Char
    if (token.type() > 0) {

        if (token.type() == '(' || token.type() == '[')
            open_parens = true;
        else
            open_parens = false;

        emptyline = false;
        out << token.type();
        return out;
    }

    // Everything else
    // we printout what the lexer read in identifier (no risk of error)

    // no space if the line is empty and no space if it is just after
    // a open parens
    if (!prev_is_op && !emptyline && !open_parens) {
        out << ' ';
    }
    prev_is_op = false;

    if (token.type() == tok_string)
        out << '"' << token.identifier() << '"';
    else if (token.type() == tok_docstring)
        out << "\"\"\"" << token.identifier() << "\"\"\"";
    else if (token.type() == tok_int || token.type() == tok_float)
        out << token.identifier();
    else
        out << token.identifier();

    open_parens = false;
    emptyline   = false;
    prev_is_op  = false;

    return out;
}

void Unlex::reset() {
    indent_level = 0;
    emptyline    = true;
    open_parens  = false;
    prev_is_op   = false;
}
}  // namespace lython