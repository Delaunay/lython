#include <spdlog/fmt/bundled/core.h>

#include "lexer.h"

namespace lython {

Dict<String, OpConfig> const &default_precedence() {
    static Dict<String, OpConfig> val = {
        // Predecence, Left Associative, is_binary, is_bool, can_be_unary, kind
        // Arithmetic
        {"+", {20, true, tok_operator, BinaryOperator::Add, UnaryOperator::UAdd}},
        {"-", {20, true, tok_operator, BinaryOperator::Sub, UnaryOperator::USub}},
        {"%", {10, true, tok_operator, BinaryOperator::Mod}},
        {"*", {30, true, tok_operator, BinaryOperator::Mult}},
        {"**", {40, true, tok_operator, BinaryOperator::Pow}},
        {"/", {30, true, tok_operator, BinaryOperator::Div}},
        {"//", {30, true, tok_operator, BinaryOperator::FloorDiv}},
        {".*", {20, true, tok_operator, BinaryOperator::EltMult}},
        {"./", {20, true, tok_operator, BinaryOperator::EltDiv}},
        // Shorthand
        {"+=", {50, true, tok_operator, BinaryOperator::Add}},
        {"-=", {50, true, tok_operator, BinaryOperator::Sub}},
        {"*=", {50, true, tok_operator, BinaryOperator::Mult}},
        {"/=", {50, true, tok_operator, BinaryOperator::Div}},
        {"%=", {50, true, tok_operator, BinaryOperator::Mod}},
        {"**=", {50, true, tok_operator, BinaryOperator::Pow}},
        {"//=", {50, true, tok_operator, BinaryOperator::FloorDiv}},
        // Assignment
        {"=", {50, true, tok_assign}},
        // Logic
        {"~", {40, false, tok_operator, BinaryOperator::None, UnaryOperator::Invert}},
        {"<<", {40, false, tok_operator, BinaryOperator::LShift}},
        {">>", {40, false, tok_operator, BinaryOperator::RShift}},
        {"^", {40, false, tok_operator, BinaryOperator::BitXor}},
        {"&", {40, true, tok_operator, BinaryOperator::BitAnd}},
        {"and",
         {40, true, tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::And}},
        {"|", {40, true, tok_operator, BinaryOperator::BitOr}},
        {"or",
         {40, true, tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::Or}},
        {"!", {40, true, tok_operator, BinaryOperator::None, UnaryOperator::Not}},
        {"not", {40, true, tok_operator, BinaryOperator::None, UnaryOperator::Not}},
        // Comparison
        {"==",
         {40, true, tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::None,
          CmpOperator::Eq}},
        {"!=",
         {40, true, tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::None,
          CmpOperator::NotEq}},
        {">=",
         {40, true, tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::None,
          CmpOperator::GtE}},
        {"<=",
         {40, true, tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::None,
          CmpOperator::LtE}},
        {">",
         {40, true, tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::None,
          CmpOperator::Gt}},
        {"<",
         {40, true, tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::None,
          CmpOperator::Lt}},
        // membership
        {"in",
         {40, false, tok_in, BinaryOperator::None, UnaryOperator::None, BoolOperator::None,
          CmpOperator::In}},
        // identity
        {"is",
         {40, false, tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::None,
          CmpOperator::Is}},
        // Not an operator but we use same data structure for parsing
        {"->", {10, false, tok_arrow}},
        // Self lookup
        {".", {60, true, tok_dot}},
    };
    return val;
}

std::ostream &AbstractLexer::debug_print(std::ostream &out) {

    Token t = next_token();
    int   k = 1;
    do {
        out << fmt::format("{:4}", k) << "  ";
        t.debug_print(out) << std::endl;
        k += 1;
    } while ((t = next_token()));

    out << fmt::format("{:4}", k) << "  ";
    t.debug_print(out) << std::endl; // eof

    return out;
}

// print out tokens as they were inputed
std::ostream &AbstractLexer::print(std::ostream &out) {

    Token t = next_token();
    do {
        t.print(out);
    } while ((t = next_token()));

    // send eof for reset
    t.print(out);

    return out;
}

Token const &Lexer::next_token() {
    // if we peeked ahead return that one
    if (_buffered_token) {
        _buffered_token = false;
        _token          = _buffer;
        return _token;
    }

    char c = peek();

    // newline
    if (c == '\n') {
        // Only reset current indentation once in case of double new_lines
        if (_cindent != 0) {
            _oindent = _cindent;
            _cindent = 0;
        }
        consume();
        return make_token(tok_newline);
    }

    if (c == EOF)
        return make_token(tok_eof);

    // indent
    if (c == ' ' && empty_line()) {
        int k = 1;
        do {
            c = nextc();
            k++;

            if (k == LYTHON_INDENT && c == ' ') {
                consume();
                break;
            }
        } while (c == ' ');

        _cindent += LYTHON_INDENT;

        // if current indent is the same do nothing
        if (_cindent <= _oindent)
            return next_token();

        // else increase indent
        return make_token(tok_indent);
    }

    if (_cindent < _oindent) {
        _oindent -= LYTHON_INDENT;
        return make_token(tok_desindent);
    }

    // remove white space
    while (c == ' ') {
        c = nextc();
    }

    // Operators & Arrow
    auto *previous = _operators.match(c);
    auto  next     = previous;

    String ident;
    while (next != nullptr) {
        previous = next;

        ident.push_back(c);
        c    = nextc();
        next = previous->matching(c);
    }

    // if valid operator return that
    if (previous != nullptr && previous->leaf()) {
        try {
            auto const &op_config = _operators.precedence_table().at(ident);
            return make_token(op_config.type, ident);
        } catch (std::exception) {

            return make_token(tok_incorrect, ident);
        }
    }
    // else it might be an identifier

    // Identifiers
    if (ident.size() > 0 || isalpha(c)) {

        // FIXME: check that ident can be an identifier
        if (isalpha(c)) {
            ident.push_back(c);

            while (is_identifier(c = nextc())) {
                ident.push_back(c);
            }
        }

        auto result = keywords().find(ident);
        if (result != keywords().end()) {
            return make_token(result->second);
        }

        return make_token(tok_identifier, ident);
    }

    // Numbers
    if (std::isdigit(c)) {
        String    num;
        TokenType ntype = tok_int;

        while (std::isdigit(c)) {
            num.push_back(c);
            c = nextc();
        }

        if (c == '.') {
            ntype = tok_float;
            num.push_back(c);
            c = nextc();
            while (std::isdigit(c)) {
                num.push_back(c);
                c = nextc();
            }
        }

        /*/ Incorrect Numbers
            while (c != ' ' && c != '\n' && c != EOF){
                num.push_back(c);
                c = nextc();
                ntype = tok_incorrect;
            }*/

        // std::cout << '"' << num << '"' << ntype << ',' << tok_incorrect << std::endl;
        // throw 0;
        return make_token(ntype, num);
    }

    // strings
    if (c == '"') {
        String    str;
        TokenType tok = tok_string;
        char      c2  = nextc();
        char      c3  = '\0';

        if (c2 == '"') {
            char c3 = nextc();
            if (c3 == '"') {
                tok = tok_docstring;
            } else {
                str.push_back(c2);
                str.push_back(c3);
            }
        } else {
            str.push_back(c2);
        }

        if (tok == tok_string)
            while ((c = nextc()) != '"') {
                str.push_back(c);
            }
        else {
            while (true) {
                c = nextc();
                if (c == '"') {
                    c2 = nextc();
                    if (c2 == '"') {
                        c3 = nextc();
                        if (c3 == '"') {
                            break;
                        } else {
                            str.push_back(c);
                            str.push_back(c2);
                            str.push_back(c3);
                        }
                    } else {
                        str.push_back(c);
                        str.push_back(c2);
                    }
                } else {
                    str.push_back(c);
                }
            }
        }
        consume();
        return make_token(tok, str);
    }

    // get next char
    c = peek();
    consume();
    return make_token(c);
}

} // namespace lython
