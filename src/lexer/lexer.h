#pragma once

#include <ostream>

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

namespace lython{

struct OpConfig{
    int precedence = -1;
    bool left_associative = true;
    TokenType type;
};

inline
Dict<String, OpConfig> default_precedence() {
    static Dict<String, OpConfig> val = {
        {"+" , {2, true , tok_operator}}, // Predecence, Left Associative
        {"-" , {2, true , tok_operator}},
        {"%" , {1, true , tok_operator}},
        {"*" , {3, true , tok_operator}},
        {"/" , {3, true , tok_operator}},
        {".*", {2, true , tok_operator}},
        {"./", {2, true , tok_operator}},
        {"." , {5, true , tok_operator}},
        {"=" , {5, true , tok_operator}},
        {"^" , {4, false, tok_operator}},
        // Not an operator but we use same data structure for parsing
        {"->", {1, false, tok_arrow}}
    };
    return val;
}

class LexerOperators{
public:
    LexerOperators(){
        for (auto& c : _precedence_table) {
            _operators.insert(c.first);
        }
    }

    Trie<128> const *match(int c) const {
        return _operators.trie().matching(c);
    }

    Dict<String, OpConfig> const&precedence_table() const {
        return _precedence_table;
    }

    TokenType token_type(String const& str) const {
        return _precedence_table.at(str).type;
    }

private:
    CoWTrie<128> _operators;
    Dict<String, OpConfig> _precedence_table =
        default_precedence();
};


class Lexer
{
public:

    Lexer(AbstractBuffer& reader):
        _reader(reader), _cindent(indent()), _oindent(indent())
    {}

    // shortcuts
    const String& file_name()   {   return _reader.file_name();  }
    int32  line()      {    return _reader.line();      }
    int32  col()       {    return _reader.col();       }
    int32  indent()    {    return _reader.indent();    }
    void   consume()   {    return _reader.consume();   }
    char   peek()      {    return _reader.peek();      }
    bool   empty_line(){    return _reader.empty_line();}
    Token& token()     {    return _token;              }
    char   nextc(){
        _reader.consume();
        return _reader.peek();
    }

    // what characters are allowed in identifiers
    bool is_identifier(char c){
        if (std::isalnum(c) || c == '_' || c == '?' || c == '!' || c == '-')
            return true;
        return false;
    }


    Token next_token(){
        // if we peeked ahead return that one
        if (_buffered_token){
            _buffered_token = false;
            _token = _buffer;
            return _token;
        }

        char c = peek();

        // newline
        if (c == '\n'){
            // Only reset current indentation once in case of double new_lines
            if (_cindent != 0){
                _oindent = _cindent;
                _cindent = 0;
            }
            consume();
            return make_token(tok_newline);
        }

        if (c == EOF)
            return make_token(tok_eof);

        // indent
        if (c == ' ' && empty_line()){
            int k = 1;
            do{
                c = nextc();
                k++;

                if (k == LYTHON_INDENT && c == ' '){
                    consume();
                    break;
                }
            } while(c == ' ');

            _cindent += LYTHON_INDENT;

            // if current indent is the same do nothing
            if (_cindent <= _oindent)
                return next_token();

            // else increase indent
            return make_token(tok_indent);
        }

        if (_cindent < _oindent){
            _oindent -= LYTHON_INDENT;
            return make_token(tok_desindent);
        }

        // remove white space
        while (c == ' '){
            c = nextc();
        }

        // Operators & Arrow
        auto* trie_iter = _operators.match(c);
        if (trie_iter != nullptr){
            String opr;
            while (trie_iter != nullptr){
                opr.push_back(c);
                c = nextc();
                trie_iter = trie_iter->matching(c);
            }

            // if this is an operator then go ahead
            auto const& op_config = _operators.precedence_table().at(opr);
            if (op_config.precedence > -1){
                return make_token(op_config.type, opr);
            } else { // FIXME
                return make_token(tok_identifier, opr);
            }
        }

        // Identifiers
        if (isalpha(c)){
            String ident;
            ident.push_back(c);

            while(is_identifier(c = nextc())){
                ident.push_back(c);
            }

            TokenType t = keywords()[ident];
            if (t)
                return make_token(t);

            return make_token(tok_identifier, ident);
        }

        // Numbers
        if (std::isdigit(c)){
            String num;
            TokenType ntype = tok_int;

            while(std::isdigit(c)){
                num.push_back(c);
                c = nextc();
            }

            if (c == '.'){
                num.push_back(c);
                c = nextc();
                while(std::isdigit(c)){
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
        if (c == '"'){
            String str;
            TokenType tok = tok_string;
            char c2 = nextc();
            char c3 = '\0';

            if (c2 == '"'){
                char c3 = nextc();
                if (c3 == '"'){
                    tok = tok_docstring;
                } else{
                    str.push_back(c2);
                    str.push_back(c3);
                }
            } else{
                str.push_back(c2);
            }

            if (tok == tok_string)
                while((c = nextc()) != '"'){
                    str.push_back(c);
                }
            else{
                while(true){
                    c = nextc();
                    if (c == '"'){
                        c2 = nextc();
                        if (c2 == '"'){
                            c3 = nextc();
                            if (c3 == '"'){
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
                    } else{
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

    Token make_token(int8 t){
        _token = Token(t, line(), col());
        return _token;
    }

    Token make_token(int8 t, const String& identifier){
        _token = Token(t, line(), col());
        _token.identifier() = identifier;
        return _token;
    }

    Token peek_token(){
        // we can only peek ahead once
        if (_buffered_token)
            return _buffer;

        // Save current token a get next
        Token current_token = _token;
        _buffer = next_token();
        _token = current_token;
        _buffered_token = true;
        return _buffer;
    }

private:
    AbstractBuffer& _reader;
    Token           _token{dummy()};
    int32           _cindent;
    int32           _oindent;
    bool            _buffered_token = false;
    Token           _buffer{dummy()};
    LexerOperators  _operators;

// debug
public:

    // print tokens with their info
    std::ostream& debug_print(std::ostream& out);

    // print out tokens as they were inputed
    std::ostream& print(std::ostream& out);

    // extract a token stream into a token vector
    Array<Token> extract_token(){
        Array<Token> v;

        Token t = next_token();
        do {
            v.push_back(t);
        } while((t = next_token()));

        v.push_back(t); // push eof token
        return v;
    }
};



}
