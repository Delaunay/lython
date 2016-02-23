#pragma once

#include <unordered_set>
#include <queue>
#include <list> // Queue is not a real container and it was badly design to not support vector

#include "Prelexer.h"
#include "Tokens.h"

#include "../Types.h"

/*
 *  Lexer is a stream of tokens
 *
 *      Add lython keywords in a map
 *
 */

namespace lython{

enum NumType
{
    integer_num,
    exp_num,
    float_num,
    not_num
};

#define SAFE_ACT(act)  index += 1;\
                        if (index >= n){\
                            act;}

#define SAFE_INCREMENT  index += 1;\
                        if (index >= n){\
                            return next_token();}

#define SAFE_INCREMENT_BK   index += 1;\
                            if (index >= n){\
                                break;}

class Lexer
{
public:
    typedef std::list<Sexp> TokenVector;
    typedef std::queue<Sexp, TokenVector> TokenQueue;

    Lexer(AbstractBuffer& reader):
        _plex(reader), _ptok()
    {}

    void print_error(const char* msg, int err_code){
        printf("[Lexer ERROR] [l:%d c:%d]  EC %d: %s\n",
               line(), col(), err_code, msg);
    }

    Token next_token(){

        if (_pending.size()){
            Token tok = _pending.front();
            _pending.pop();
            return tok;
        }

        //
        PreToken tok = next_pretoken();

        // Preblock
        if (tok.type() == pretok_preblock){
            return Sexp(new Block(tok));
        }

        // PreString
        if (tok.type() == pretok_prestring){
            return Sexp(new String(tok));
        }

        // PreToken Where the lex work is done
        if (tok.type() != pretok_pretok)
            print_error("Expected Pretok Pretoken", -1);

        // Get Data
        std::string& val = tok.as_string();
        uint32 line = tok.line_begin();
        uint32 col = tok.col();
        uint32 n = val.size();
        NumType t = integer_num;
        int index = 0;

        while(index < n){
            // read number
            if (isdigit(val[index] /*|| +/- */)){
                // Create a substring
                std::string num;
                num.push_back(val[index]);
                index += 1;

                while(index < n){

                    if (val[index] == '.'){
                        if (t == integer_num)
                            t = float_num;
                        else // we already found a '.'
                            t = not_num;

                        num.push_back(val[index]);
                        SAFE_INCREMENT_BK
                        continue;
                    }

                    if (val[index] == 'e' || val[index] == 'E'){
                        if (t == integer_num || t == float_num)
                            t = exp_num;
                        else // we already found a 'e'
                            t = not_num;

                        num.push_back(val[index]);
                        SAFE_INCREMENT_BK
                        continue;
                    }

                    if (val[index] == '+' || val[index] == '-'){

                        if (t == exp_num && (num[num.size() - 1] == 'e' ||
                                             num[num.size() - 1] == 'E')){
                            num.push_back(val[index]);
                            SAFE_INCREMENT_BK
                            continue;
                        }
                        // Number is done reading
                        else
                            break;
                    }

                    if (std::isdigit(val[index])){
                        num.push_back(val[index]);
                        SAFE_INCREMENT_BK
                    }
                    else
                        break;
                }

                switch(t){
                case integer_num:
                    _pending.push(Sexp(new Integer(std::stoi(num), line, col + index)));
                    break;
                case float_num:
                case exp_num:
                    _pending.push(Sexp(new Float(std::stod(num), line, col + index)));
                    break;
                default:
                    print_error("Not a correct Number", -2);
                    // we still produce a symbol || Ocaml Code does not handle this
                    _pending.push(Sexp(new Symbol(std::stod(num), line, col + index)));
                }
            }

            if (index >= n)
                break;

            // Separator
            if (_default_stt.count(val[index]) > 0){
                _pending.push(Sexp(new Symbol(val[index], line, col + index)));
                SAFE_INCREMENT
            }
            else{

                std::string sym;
                sym.push_back(val[index]);
                index += 1;

                while(index < n && _default_stt.count(val[index]) <= 0){
                    sym.push_back(val[index]);
                    SAFE_INCREMENT_BK
                }

                _pending.push(Sexp(new Symbol(sym, line, col + index)));
            }
        }

        // return pending tokens
        if (_pending.size()){
            Token tok = _pending.front();
            _pending.pop();
            return tok;
        }

        // just in case
        print_error("No More Tokens", -3);
    }

    PreToken& pretok()  {   return _ptok;   }
    PreToken& next_pretoken() {
        _ptok = _plex.next_pretoken();
        return _ptok;
    }

    uint32 line() { return _plex.line();   }
    uint32 col()  { return _plex.col();    }

    operator bool(){
        return _pending.size() || bool(_plex);
    }

private:

    PreToken _ptok;
    Prelexer _plex;     // AbstractBuffer& _reader;

    std::unordered_set<char> _default_stt{';', ',', '(', ')'};

    //std::vector<Sexp> _pending;
    TokenQueue    _pending;

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
            t = next_token();
        }while((*this));

        v.push_back(t); // push eof token
        return v;
    }
};



}
