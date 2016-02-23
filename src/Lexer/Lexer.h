#pragma once

#include <ostream>

#include "Buffer.h"
#include "Tokens.h"

#include "../Types.h"


/*
 *  Lexer is a stream of tokens
 *
 *      TODO:   DocString support
 */

namespace lython{

class Lexer
{
public:

    Lexer(AbstractBuffer& reader):
        _reader(reader)
    {}

    // shortcuts
    const std::string& file_name()   {   return _reader.file_name();  }
    uint32 line()      {    return _reader.line();      }
    uint32 col()       {    return _reader.col();       }
    uint32 indent()    {    return _reader.indent();    }
    char   nextc()     {    return _reader.nextc();     }
    bool   empty_line(){    return _reader.empty_line();}
    Token& token()     {    return _token;              }

    // what characters are allowed in identifiers
    bool is_identifier(char c){
        if (std::isalnum(c) || c == '-' || c == '_' || c == '?' || c == '!')
            return true;
        return false;
    }

    //
    Token next_token(){
        // get first char
        // humm I am not sure it is a good idea
        // I think those value will be shared with all Lexer Instances (bad)
        static char c = nextc();
        static uint32 oindent = indent();
        static uint32 cindent = indent();

        // newline
        if (c == '\n'){
            oindent = cindent;
            cindent = 0;
            c = nextc();
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

                if (k == LYTHON_INDENT){
                    c = nextc();
                    break;
                }
            } while(c == ' ');

            cindent += LYTHON_INDENT;

            // if current indent is the same do nothing
            if (cindent <= oindent)
                return next_token();

            // else increase indent
            return make_token(tok_indent);
        }

        if (cindent < oindent){
            oindent -= LYTHON_INDENT;
            return make_token(tok_desindent);
        }

        // remove white space
        while (c == ' '){
            c = nextc();
        }

        // Identifiers
        if (isalpha(c)){
            std::string ident;
            ident.push_back(c);

            while(is_identifier(c = nextc())){
                ident.push_back(c);
            }

            TokenType t = keywords()[ident];
            if (t)  return make_token(t);
            return make_token(tok_identifier, ident);
        }

        // Numbers
        if (std::isdigit(c)){
            std::string num;
            TokenType ntype = tok_int;

            while(c != ' ' && c != EOF && c != '\n')
            {
                num.push_back(c);

                // if it is not a digit
                if (!std::isdigit(c) && c != ' '){
                    if (c == '.'){
                        if (ntype == tok_int)
                            ntype = tok_float;
                        else
                            ntype = tok_incorrect;    // 12.12.12 is an incorrect token
                    }
                    else
                        ntype = tok_incorrect;        // 1abc is an incorrect token
                }

                c = nextc();
            }

            return make_token(ntype, num);
        }

        // strings
        if (c == '"'){
            std::string str;

            while((c = nextc()) != '"'){
                str.push_back(c);
            }

            c = nextc();
            return make_token(tok_string, str);
        }

        // get next char
        make_token(c);
        c = nextc();

        // and return the char as a token;
        return _token;
    }

    Token make_token(int8 t){
        _token = Token(t, line(), col());
        return _token;
    }

    Token make_token(int8 t, const std::string& identifier){
        _token = Token(t, line(), col());
        _token.identifier() = identifier;
        return _token;
    }

private:

    AbstractBuffer& _reader;
    Token           _token{dummy()};

// debug
public:

    // print tokens with their info
    std::ostream& debug_print(std::ostream& out);

    // print out tokens as they were inputed
    std::ostream& print(std::ostream& out);

    // extract a token stream into a token vector
    std::vector<Token> extract_token(){
        std::vector<Token> v;

        Token t = next_token();
        do{
            v.push_back(t);
        }while(t = next_token());

        v.push_back(t); // push eof token
        return v;
    }
};



}
