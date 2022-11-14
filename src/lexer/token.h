#pragma once

#include <algorithm>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "dtypes.h"
#include "logging/logging.h"

/*
 *  incorrect is used when the input is known to be wrong
 *  but we want to parse as much as we can anyway
 *
 *  incorrect is also the first token and eof the last
 *
 *  I could add an error policy arg
 *
 *  I think those would be nice to have
 *      Hex: 0x000A9F
 *      bin: bx010101
 */
#define LYTHON_INDENT 4
#define LYTHON_TOKEN(X)    \
    X(tok_identifier, -1)  \
    X(tok_float, -2)       \
    X(tok_string, -3)      \
    X(tok_int, -4)         \
    X(tok_newline, -5)     \
    X(tok_indent, -6)      \
    X(tok_desindent, -7)   \
    X(tok_incorrect, -8)   \
    X(tok_eof, -9)         \
    X(tok_def, -10)        \
    X(tok_docstring, -11)  \
    X(tok_arrow, -12)      \
    X(tok_struct, -13)     \
    X(tok_return, -14)     \
    X(tok_yield, -15)      \
    X(tok_async, -16)      \
    X(tok_operator, -17)   \
    X(tok_import, -18)     \
    X(tok_match, -19)      \
    X(tok_as, -20)         \
    X(tok_from, -21)       \
    X(tok_if, -22)         \
    X(tok_elif, -23)       \
    X(tok_else, -24)       \
    X(tok_try, -25)        \
    X(tok_except, -26)     \
    X(tok_raise, -27)      \
    X(tok_case, -28)       \
    X(tok_for, -29)        \
    X(tok_while, -30)      \
    X(tok_with, -31)       \
    X(tok_class, -32)      \
    X(tok_assert, -33)     \
    X(tok_global, -34)     \
    X(tok_del, -35)        \
    X(tok_pass, -36)       \
    X(tok_break, -37)      \
    X(tok_continue, -38)   \
    X(tok_parens, '(')     \
    X(tok_square, '[')     \
    X(tok_dot, '.')        \
    X(tok_assign, '=')     \
    X(tok_curly, '{')      \
    X(tok_star, '*')       \
    X(tok_augassign, -39)  \
    X(tok_annassign, -40)  \
    X(tok_walrus, -41)     \
    X(tok_boolop, -42)     \
    X(tok_binaryop, -43)   \
    X(tok_compareop, -44)  \
    X(tok_unaryop, -45)    \
    X(tok_await, -46)      \
    X(tok_lambda, -47)     \
    X(tok_fstring, -48)    \
    X(tok_yield_from, -49) \
    X(tok_in, -50)         \
    X(tok_finally, -51)    \
    X(tok_nonlocal, -52)   \
    X(tok_comma, ',')      \
    X(tok_none, -53)       \
    X(tok_true, -54)       \
    X(tok_false, -55)      \
    X(tok_is, -56)         \
    X(tok_not, -57)        \
    X(tok_and, -58)        \
    X(tok_or, -59)         \
    X(tok_decorator, '@')  \
    X(tok_comment, '#')

#define LYTHON_KEYWORDS(X)      \
    X("def", tok_def)           \
    X("->", tok_arrow)          \
    X("struct", tok_struct)     \
    X("return", tok_return)     \
    X("yield", tok_yield)       \
    X("async", tok_async)       \
    X("import", tok_import)     \
    X("from", tok_from)         \
    X("as", tok_as)             \
    X("if", tok_if)             \
    X("elif", tok_elif)         \
    X("else", tok_else)         \
    X("try", tok_try)           \
    X("except", tok_except)     \
    X("match", tok_match)       \
    X("raise", tok_raise)       \
    X("case", tok_case)         \
    X("while", tok_while)       \
    X("for", tok_for)           \
    X("with", tok_with)         \
    X("class", tok_class)       \
    X("assert", tok_assert)     \
    X("global", tok_global)     \
    X("del", tok_del)           \
    X("pass", tok_pass)         \
    X("break", tok_break)       \
    X("continue", tok_continue) \
    X("await", tok_await)       \
    X("lambda", tok_lambda)     \
    X("in", tok_in)             \
    X("finally", tok_finally)   \
    X("nonlocal", tok_nonlocal) \
    X("None", tok_none)         \
    X("True", tok_true)         \
    X("False", tok_false)       \
    X("not", tok_not)           \
    X("is", tok_is)             \
    X("or", tok_or)             \
    X("and", tok_and)

namespace lython {

enum TokenType
{
#define X(name, nb) name = nb,
    LYTHON_TOKEN(X)
#undef X
};

String to_human_name(int8 t);
String to_string(int8 t);

inline void print(TokenType const& t, std::ostream& out) { out << to_string(t); }

using ReservedKeyword = Dict<String, TokenType>;
using KeywordToString = Dict<int, String>;

ReservedKeyword& keywords();
KeywordToString& keyword_as_string();

int8 tok_name_size();

class Token {
    public:
    Token(TokenType t, int32 l, int32 c): _type(t), _line(l), _col(c) {}

    Token(int8 t, int32 l, int32 c): _type(t), _line(l), _col(c) {}

    int8  type() const { return _type; }
    int32 line() const { return _line; }

    int32 begin_col() const { return _col - int32(identifier().size()); }
    int32 end_col() const { return _col; }
    int32 col() const { return _col; }

    int32 end_line() const { return col(); }
    int32 begin_line() const { return col() - int32(identifier().size()); }

    String&       operator_name() { return _identifier; }
    String const& operator_name() const { return _identifier; }
    String&       identifier() { return _identifier; }
    String const& identifier() const { return _identifier; }

    float64 as_float() const { return std::stod(_identifier.c_str()); }

    int64  as_integer() const { return std::strtoll(_identifier.c_str(), nullptr, 10); }
    uint64 as_uint64() const { return std::strtoull(_identifier.c_str(), nullptr, 10); }

    operator bool() const { return _type != tok_eof; }

    int compare(Token const& tok) {
        if (_line != tok._line)
            return _line - tok._line;
        return _col - tok._col;
    }

    bool isbefore(Token const& tok) { return compare(tok) < 0; }
    bool isafter(Token const& tok) { return compare(tok) > 0; }
    bool isbetween(Token const& begin, Token const& end) { return isafter(begin) && isbefore(end); }

    bool operator==(Token const& tok) const {
        return (_type == tok._type) && (_line == tok._line) && (_col == tok._col);
    }

    private:
    int8  _type = tok_incorrect;
    int32 _line = -1;
    int32 _col  = -1;

    // Data
    String _identifier;

    public:
    // print all tokens and their info
    std::ostream& debug_print(std::ostream& out) const;

    std::ostream& print(std::ostream& out) const;
};

inline Token& dummy() {
    static Token dy = Token(tok_incorrect, 0, 0);
    return dy;
}

// Make something that look like clang's error underlining.
// offset is used if you need to print multiple underline on a same line
inline std::ostream& underline(std::ostream& out, Token& t, int32 offset = 0) {
    int32 start = t.begin_line() - offset;
    if (start > 0) {
        out << std::string(uint32(start), ' ');

        if (t.identifier().size() > 0)
            out << std::string(t.identifier().size(), '~');
        else
            out << "~";
    }

    return out;
}

}  // namespace lython
