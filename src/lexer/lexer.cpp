#include "lexer.h"
#include "unlex.h"
#include "utilities/strings.h"

namespace lython {

Array<OpConfig> const& all_operators() {
    // clang-format off
    static Array<OpConfig> ops = {
        // Predecence, Left Associative, is_binary, is_bool, can_be_unary, kind
        // Arithmetic
        {"+",       20, true , tok_operator, BinaryOperator::Add, UnaryOperator::UAdd},
        {"-",       20, true , tok_operator, BinaryOperator::Sub, UnaryOperator::USub},
        {"%",       10, true , tok_operator, BinaryOperator::Mod},
        {"*",       30, true , tok_operator, BinaryOperator::Mult},
        {"**",      40, true , tok_operator, BinaryOperator::Pow},
        {"/",       30, true , tok_operator, BinaryOperator::Div},
        {"//",      30, true , tok_operator, BinaryOperator::FloorDiv},
        {".*",      20, true , tok_operator, BinaryOperator::EltMult},
        {"./",      20, true , tok_operator, BinaryOperator::EltDiv},
        //*/ Shorthand
        {"+=",      50, true , tok_augassign, BinaryOperator::Add},
        {"-=",      50, true , tok_augassign, BinaryOperator::Sub},
        {"*=",      50, true , tok_augassign, BinaryOperator::Mult},
        {"/=",      50, true , tok_augassign, BinaryOperator::Div},
        {"%=",      50, true , tok_augassign, BinaryOperator::Mod},
        {"**=",     50, true , tok_augassign, BinaryOperator::Pow},
        {"//=",     50, true , tok_augassign, BinaryOperator::FloorDiv},
        //*/
        // Assignment
        {"=",       50, true , tok_assign},
        // Logic
        {"~",       40, false, tok_operator, BinaryOperator::None, UnaryOperator::Invert},
        {"<<",      40, false, tok_operator, BinaryOperator::LShift},
        {">>",      40, false, tok_operator, BinaryOperator::RShift},
        {"^",       40, false, tok_operator, BinaryOperator::BitXor},
        {"&",       40, true , tok_operator, BinaryOperator::BitAnd},
        {"and",     40, true , tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::And},
        {"|",       40, true , tok_operator, BinaryOperator::BitOr},
        {"or",      40, true , tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::Or},
        {"!",       40, true , tok_operator, BinaryOperator::None, UnaryOperator::Not},
        {"not",     40, true , tok_operator, BinaryOperator::None, UnaryOperator::Not},
        // Comparison
        {"==",      40, true , tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::None, CmpOperator::Eq},
        {"!=",      40, true , tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::None, CmpOperator::NotEq},
        {">=",      40, true , tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::None, CmpOperator::GtE},
        {"<=",      40, true , tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::None, CmpOperator::LtE},
        {">",       40, true , tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::None, CmpOperator::Gt},
        {"<",       40, true , tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::None, CmpOperator::Lt},
        // membership
        {"in",      40, false, tok_in      , BinaryOperator::None, UnaryOperator::None, BoolOperator::None, CmpOperator::In},
        {"not in",  40, false, tok_in      , BinaryOperator::None, UnaryOperator::None, BoolOperator::None, CmpOperator::NotIn},
        // identity
        {"is",      40, false, tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::None, CmpOperator::Is},
        {"is not",  40, false, tok_operator, BinaryOperator::None, UnaryOperator::None, BoolOperator::None, CmpOperator::IsNot},
        // Not an operator but we use same data structure for parsing
        {"->",      10, false, tok_arrow},
        {":=",      10, false, tok_walrus},
        {":",       10, false, (TokenType)':'},
        {".",       60, true , tok_dot}
    };
    // clang-format on
    return ops;
}

std::ostream& operator<<(std::ostream& out, OpConfig const& op) {
        return out << to_string(op.type) << "(pred: " << op.precedence << ") "
            << "(binary: " << int(op.binarykind) << ") "
            << "(unary: " << int(op.unarykind) << ") "
            << "(bool: " << int(op.boolkind) << ") "
            << "(cmp: " << int(op.cmpkind) << ") ";
    }

Dict<String, OpConfig> _make_op_dict() {
    Dict<String, OpConfig> ops;
    for(auto const& op: all_operators()) {
        ops[op.operator_name] = op;
    }
    return ops;
}

Dict<String, OpConfig> const& default_precedence() 
{    
    static Dict<String, OpConfig> val = _make_op_dict();
    return val;
}

std::ostream& AbstractLexer::debug_print(std::ostream& out) 
{
    Token t = next_token();
    int   k = 1;
    do {
        out << fmt::format("{:4}", k) << "  ";
        t.debug_print(out) << std::endl;
        k += 1;
    } while ((t = next_token()));

    out << fmt::format("{:4}", k) << "  ";
    t.debug_print(out) << std::endl;  // eof

    return out;
}

// print out tokens as they were inputed
std::ostream& AbstractLexer::print(std::ostream& out) {

    Token t = next_token();
    Unlex unlex;

    do {
        unlex.format(out, t);
    } while ((t = next_token()));

    // send eof for reset
    unlex.format(out, t);

    return out;
}

int Lexer::get_mode() const {
    return int(_fmtstr);
}

void Lexer::set_mode(int mode) {
    _fmtstr = mode > 0;
}

Token const& Lexer::format_tokenizer() {
    char c = peek();
    nextc();
    return make_token(c);
}


Token const& Lexer::next_token() {
    _count += 1;

    // if we peeked ahead return that one
    if (_buffer.size() > 0) {
        _token = _buffer[_buffer.size() - 1];
        _buffer.pop_back();
        return _token;
    }

    if (_fmtstr) {
        return format_tokenizer();
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

// only broadcast desindent on actual code
// comments have no impacts on our indentation level
//
// Doing it here brings another problem:
//  - now comment indentation is going to change
//    this could be a good thing as it forces comment
//    to be at the "right" indentation
//
// but if you write a comment after a class its indentation is going to be wrong
//
//  class X:
//  # comment
//      def __init__(self):
//          ...
//
// becomes
//
//  class X:
//      # comment
//      def __init__(self):
//          ...
//
//  and
//
//  for i in range(10):
//      ...
//  # comment
//
// becomes
//
//  for i in range(10):
//      ...
//      # comment
//
//  1) is ok, the comment was written inside a statement block
//  2) is problematic, the comment was written outside the block
//  but we cannot tell until we reached a desindent block
//  which happens AFTER the comment
//
// SOLUTION: make the parser associate comment with the comming statement
#define FORCE_COMMENT_INDENT(X) X

    bool desindent_comment = _cindent < _oindent && c == tok_comment;

    if (_cindent < _oindent) {
        // TODO: this behaviour is not good for the Unlexer
        // but it is fine for the parser
        if (FORCE_COMMENT_INDENT(c != tok_comment)) {
            _oindent -= LYTHON_INDENT;
            return make_token(tok_desindent);
        } else {
            // reset current indent to match previous indentation level
            // because comment indentation do not matter
            _cindent = _oindent;
        }
    }

    // remove white space
    while (c == ' ') {
        c = nextc();
    }

    // Identifiers
    // -----------
    if ((isalpha(c) || c == '_')) {
        String identifier;

        // FIXME: check that ident can be an identifier
        identifier.push_back(c);

        while (is_identifier(c = nextc())) {
            identifier.push_back(c);
        }

        // is it a string operator (is, not, in, and, or) ?
        {
            auto result = default_precedence().find(identifier);
            if (result != default_precedence().end()) {
                OpConfig const& conf = result->second;
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

                if (identifier == "not" && tok.operator_name() == "in") {
                    return make_token(conf.type, "not in");
                }

                _buffer.push_back(tok);
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

        // is it followed by a quote
        if (peek() == '"' || peek() == '\'') {
            return make_token(tok_formatstr, identifier);
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
                    OpConfig const& conf = result->second;
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

    // Regular string
    // --------------
    if (c == '"' || c == '\'') {
        char      end = c;
        String    str;
        TokenType tok = tok_string;
        char      c2  = nextc();
        char      c3  = '\0';

        if (c2 == end) {
            char c3 = nextc();
            if (c3 == end) {
                tok = tok_docstring;
            } else {
                str.push_back(c2);
                str.push_back(c3);
            }
        } else {
            str.push_back(c2);
        }

        if (tok == tok_string)
            while ((c = nextc()) != end && c != EOF) {
                str.push_back(c);
            }
        else {
            while (c != EOF) {
                c = nextc();

                if (c == end) {
                    c2 = nextc();
                    if (c2 == end) {
                        c3 = nextc();
                        if (c3 == end) {
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

    c = peek();
    if (c == tok_comment) {
        String comment;
        comment.reserve(128);

        // eat the comment token
        c = nextc();

        // eat all characters until the newline
        while (c != '\n' && c != EOF) {
            comment.push_back(c);
            c = nextc();
        };

        return make_token(tok_comment, comment);
    }

    // get next char
    c = peek();
    consume();

    if (c > 0) {
        return make_token(c);
    }
    return make_token(tok_incorrect);
}

}  // namespace lython
