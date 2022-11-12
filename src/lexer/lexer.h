#pragma once

#include <cctype>
#include <ostream>

#include "ast/nodes.h"
#include "lexer/buffer.h"
#include "lexer/token.h"
#include "utilities/trie.h"

#include "dtypes.h"

#include <iostream>

/*
 *  Lexer is a stream of tokens
 *
 *      TODO:   DocString support
 */

namespace lython {

template <typename T, typename N>
bool in(T const& e, N const& v) {
    return e == v;
}

template <typename T, typename N, typename... Args>
bool in(T const& e, N const& v, Args... args) {
    return e == v || in(e, args...);
}

template <typename T, typename... Args>
bool in(T const& e, Args... args) {
    return in(e, args...);
}

struct OpConfig {
    int            precedence       = -1;
    bool           left_associative = true;
    TokenType      type             = TokenType::tok_eof;
    BinaryOperator binarykind       = BinaryOperator::None;
    UnaryOperator  unarykind        = UnaryOperator::None;
    BoolOperator   boolkind         = BoolOperator::None;
    CmpOperator    cmpkind          = CmpOperator::None;

    void print(std::ostream& out) const {
        out << to_string(type) << "(pred: " << precedence << ") "
            << "(binary: " << int(binarykind) << ") "
            << "(unary: " << int(unarykind) << ") "
            << "(bool: " << int(boolkind) << ") "
            << "(cmp: " << int(cmpkind) << ") ";
    }
};

Dict<String, OpConfig> const& default_precedence();

class LexerOperators {
    public:
    LexerOperators() {
        for (auto& c: _precedence_table) {
            _operators.insert(c.first);
        }
    }

    Trie<128> const* match(int c) const { return _operators.trie().matching(c); }

    Dict<String, OpConfig> const& precedence_table() const { return _precedence_table; }

    TokenType token_type(String const& str) const { return _precedence_table.at(str).type; }

    private:
    CoWTrie<128>           _operators;
    Dict<String, OpConfig> _precedence_table = default_precedence();
};

class AbstractLexer {
    public:
    virtual ~AbstractLexer() {}

    virtual Token const& next_token() = 0;

    virtual Token const& peek_token() = 0;

    virtual Token const& token() = 0;

    virtual const String& file_name() = 0;

    // print tokens with their info
    ::std::ostream& debug_print(::std::ostream& out);

    // print out tokens as they were inputed
    ::std::ostream& print(::std::ostream& out);

    // extract a token stream into a token vector
    Array<Token> extract_token() {
        Array<Token> v;

        Token t = next_token();
        do {
            v.push_back(t);
        } while ((t = next_token()));

        v.push_back(t);  // push eof token
        return v;
    }
};

class ReplayLexer: public AbstractLexer {
    public:
    ReplayLexer(Array<Token>& tokens): tokens(tokens) {
        Token& last = tokens[tokens.size() - 1];
        if (last.type() != tok_eof) {
            tokens.emplace_back(tok_eof, 0, 0);
        }
    }

    Token const& next_token() override final {
        if (i + 1 < tokens.size())
            i += 1;

        return tokens[i];
    }

    Token const& peek_token() override final {
        auto n = i + 1;

        if (n >= tokens.size())
            n = i;

        return tokens[n];
    }

    Token const& token() override final { return tokens[i]; }

    const String& file_name() override {
        static String fakefile = "<replay buffer>";
        return fakefile;
    }

    ~ReplayLexer() {}

    private:
    ::std::size_t i = 0;
    Array<Token>& tokens;
};

class Lexer: public AbstractLexer {
    public:
    Lexer(AbstractBuffer& reader):
        AbstractLexer(), _reader(reader), _cindent(indent()), _oindent(indent()) {}

    ~Lexer() {}

    Token const& token() override final {
        if (_count == 0) {
            return next_token();
        }
        return _token;
    }
    Token const& next_token() override final;
    Token const& peek_token() override final {
        // we can only peek ahead once
        if (_buffer.size() > 0)
            return _buffer[_buffer.size() - 1];

        // Save current token a get next
        Token current_token = _token;
        _buffer.push_back(next_token());
        _token = current_token;
        return _buffer[_buffer.size() - 1];
    }

    Token const& make_token(int8 t) {
        _token = Token(t, line(), col());
        return _token;
    }

    Token const& make_token(int8 t, const String& identifier) {
        _token              = Token(t, line(), col());
        _token.identifier() = identifier;
        return _token;
    }

    const String& file_name() override { return _reader.file_name(); }

    private:
    int             _count = 0;
    AbstractBuffer& _reader;
    Token           _token{dummy()};
    int32           _cindent;
    int32           _oindent;
    LexerOperators  _operators;
    Array<Token>    _buffer;

    // shortcuts

    int32 line() { return _reader.line(); }
    int32 col() { return _reader.col(); }
    int32 indent() { return _reader.indent(); }
    void  consume() { return _reader.consume(); }
    char  peek() { return _reader.peek(); }
    bool  empty_line() { return _reader.empty_line(); }

    // state
    bool desindent_for_comment = false;

    char nextc() {
        _reader.consume();
        return _reader.peek();
    }

    // what characters are allowed in identifiers
    bool is_identifier(char c) {
        if (::std::isalnum(c) || c == '_' || c == '?' || c == '!' || c == '-')
            return true;
        return false;
    }
};

}  // namespace lython
