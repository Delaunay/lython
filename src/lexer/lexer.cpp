#include <spdlog/fmt/bundled/core.h>

#include "lexer.h"
#include "utilities/strings.h"

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
        //*/ Shorthand
        {"+=", {50, true, tok_augassign, BinaryOperator::Add}},
        {"-=", {50, true, tok_augassign, BinaryOperator::Sub}},
        {"*=", {50, true, tok_augassign, BinaryOperator::Mult}},
        {"/=", {50, true, tok_augassign, BinaryOperator::Div}},
        {"%=", {50, true, tok_augassign, BinaryOperator::Mod}},
        {"**=", {50, true, tok_augassign, BinaryOperator::Pow}},
        {"//=", {50, true, tok_augassign, BinaryOperator::FloorDiv}},
        //*/
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
        {"not in",
         {40, false, tok_in, BinaryOperator::None, UnaryOperator::None, BoolOperator::None,
          CmpOperator::NotIn}},
        // identity
        {"is",
         {40, false, tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::None,
          CmpOperator::Is}},
        {"is not",
         {40, false, tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::None,
          CmpOperator::IsNot}},
        // Not an operator but we use same data structure for parsing
        {"->", {10, false, tok_arrow}},
        {":=", {10, false, tok_walrus}},
        {":", {10, false, (TokenType)':'}},
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

    // Indentation
    // --------------------------------
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

    // Identifiers
    // -----------
    if (isalpha(c) && peek() != '"') {
        String identifier;

        // FIXME: check that ident can be an identifier
        identifier.push_back(c);

        while (is_identifier(c = nextc())) {
            identifier.push_back(c);
        }

        if (c == 'f') {
            goto strings;
        }

        // is it a string operator (is, not, in, and, or) ?
        {
            auto result = default_precedence().find(identifier);
            if (result != default_precedence().end()) {
                OpConfig const &conf = result->second;
                Token           tok  = dummy();

                // combine is not & not in right now
                if (identifier == "is" || identifier == "not") {
                    tok = next_token();
                } else {
                    return make_token(conf.type, identifier);
                }

                if (identifier == "is" && tok.operator_name() == "not") {
                    return make_token(conf.type, "is not");
                }

                if (identifier == "not" && tok.operator_name() == "int") {
                    return make_token(conf.type, "not int");
                }

                _buffered_token = true;
                _buffer         = tok;
                return make_token(conf.type, identifier);
            }
        }

        // is it a keyword ?
        {
            auto result = keywords().find(identifier);
            if (result != keywords().end()) {
                return make_token(result->second);
            }
        }

        // then it must be an identifier
        return make_token(tok_identifier, identifier);
    }

    // Operators
    // -----------------------------------------------
    // c is not alpha num
    {
        auto next = _operators.match(c);
        if (next != nullptr) {
            String op;
            op.reserve(6);
            auto prev = next;

            while (next != nullptr) {
                op.push_back(c);
                c    = nextc();
                prev = next;
                next = prev->matching(c);
            }

            if (prev->leaf()) {
                op          = strip(op);
                auto result = default_precedence().find(op);
                if (result != default_precedence().end()) {
                    OpConfig const &conf = result->second;
                    return make_token(conf.type, op);
                }
            }
        }
    }

    // Numbers
    // -----------------------------------------------
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

// Strings
// --------------------------------------------------
strings:

    // Formated string
    // ---------------
    // upper/lower and combinaison are available
    // fr & rf & br & rb
    if (c == 'f' && peek() == '"') {
    }

    // Raw string
    if (c == 'r' && peek() == '"') {
    }

    // byte string
    if (c == 'b' && peek() == '"') {
    }

    // Regular string
    // --------------
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
