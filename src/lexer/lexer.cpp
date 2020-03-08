#include "lexer.h"
#include "../fmt.h"

namespace lython{

Dict<String, OpConfig> const& default_precedence() {
    static Dict<String, OpConfig> val = {
        // Arithmetic
        {"+"  , {20, true , tok_operator}}, // Predecence, Left Associative
        {"-"  , {20, true , tok_operator}},
        {"%"  , {10, true , tok_operator}},
        {"*"  , {30, true , tok_operator}},
        {"**" , {40, true , tok_operator}},
        {"/"  , {30, true , tok_operator}},
        {"//" , {30, true , tok_operator}},
        {".*" , {20, true , tok_operator}},
        {"./" , {20, true , tok_operator}},
        // Shorthand
        {"+=" , {50, true , tok_operator}},
        {"-=" , {50, true , tok_operator}},
        {"*=" , {50, true , tok_operator}},
        {"/=" , {50, true , tok_operator}},
        {"%=" , {50, true , tok_operator}},
        {"**=", {50, true , tok_operator}},
        {"//=", {50, true , tok_operator}},
        // Assignment
        {"="  , {50, true , tok_operator}},
        // Logic
        {"~"  , {40, false, tok_operator}},
        {"<<" , {40, false, tok_operator}},
        {">>" , {40, false, tok_operator}},
        {"^"  , {40, false, tok_operator}},
        {"&"  , {40, true , tok_operator}},
        {"and", {40, true , tok_operator}},
        {"|"  , {40, true , tok_operator}},
        {"or" , {40, true , tok_operator}},
        {"!"  , {40, true , tok_operator}},
        {"not", {40, true , tok_operator}},
        // Comparison
        {"==" , {40, true , tok_operator}},
        {"!=" , {40, true , tok_operator}},
        {">=" , {40, true , tok_operator}},
        {"<=" , {40, true , tok_operator}},
        {">"  , {40, true , tok_operator}},
        {"<"  , {40, true , tok_operator}},
        // membership
        {"in" , {40, false, tok_operator}},
        // identity
        {"is" , {40, false, tok_operator}},
        // Not an operator but we use same data structure for parsing
        {"->",  {10, false, tok_arrow}},
        // Self lookup
        {"."  , {60, true , tok_operator}},
        };
    return val;
}

std::ostream& Lexer::debug_print(std::ostream& out){

    Token t = next_token();
    int k = 1;
    do{
        out << to_string(k, 4) << "  ";
        t.debug_print(out) << std::endl;
        k += 1;
    } while((t = next_token()));

    out << to_string(k, 4) << "  ";
    t.debug_print(out) << std::endl;    // eof

    return out;
}
// print out tokens as they were inputed
std::ostream& Lexer::print(std::ostream& out){

    Token t = next_token();
    do{
        t.print(out);
    }while((t = next_token()));

    // send eof for reset
    t.print(out);

    return out;
}


Token Lexer::next_token(){
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
    auto* previous = _operators.match(c);
    auto next = previous;

    String ident;
    while (next != nullptr){
        previous = next;

        ident.push_back(c);
        c = nextc();
        next = previous->matching(c);
    }

    // if valid operator return that
    if(previous != nullptr && previous->leaf()){
        auto const& op_config = _operators.precedence_table().at(ident);

        return make_token(op_config.type, ident);
    }
    // else it might be an identifier

    // Identifiers
    if (ident.size() > 0 || isalpha(c)){

        // FIXME: check that ident can be an identifier
        if (isalpha(c)){
            ident.push_back(c);

            while(is_identifier(c = nextc())){
                ident.push_back(c);
            }
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

}
