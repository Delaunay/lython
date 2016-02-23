#ifndef PARSER_H
#define PARSER_H

#include "../Lexer/Lexer.h"
#include "Module.h"
#include <iostream>

/*
 *  TODO:
 *      Handle tok_incorrect so the parser does not stop
 *      I need a more generic way to parse tokens since
 *      I will need to parse vector<Token> too
 *
 *      Some statistic about compiler memory usage would be nice
 *
 *  if definition 1 is incorrect
 *  The parser should be able to parse definition 2 (which is correct)
 *  correctly
 *
 * if a body is incorrect it has not impact if the function is not used
 *
 */

#define EAT(tok) if (token().type() == tok){ next_token();   }
#define EXPECT(tok, msg) if(token().type() != tok) throw msg;
//assert(token().type() == tok && msg)
#define TRACE(out)  out << "function " << std::endl;
#define CHECK_TYPE(type) type
#define CHECK_NAME(name) name
#define PARSE_ERROR(msg) std::cout << msg;

namespace lython{

class Parser
{
public:
    Parser(AbstractBuffer& buffer):
        _lex(buffer)
    {

    }

    // Shortcut
    Token next_token()  {   return _lex.next_token();   }
    Token token()       {   return _lex.token();    }

    ST::Expr get_unparsed_block(){

    }

    // currently type is a string
    // nevertheless we want to support advanced typing
    // which means type will be an expression too
    Type parse_type(){

    }

    /*  Keep parsing if a token is incorrect */
    std::string& get_identifier(){
        switch (token().type()){
        case tok_identifier:
            return token().identifier();
        case tok_incorrect:
            PARSE_ERROR("Incorrect Identifier");
            return token().identifier();
        default:
            assert("An Identifier was expected");
        }
    }

    // Parsing routines
    ST::Expr parse_function(){
        EAT(tok_def);

        // Get Name
        EXPECT(tok_identifier, "An Identifier was expected");
        AST::Function* fun = new AST::Function(token().identifier());
        next_token();

        // Parse Args
        EXPECT('(', "( was expected"); EAT('(');
        while(token().type() != ')' && token()){

            std::string vname = CHECK_NAME(get_identifier());
            std::string type = "void";
            next_token();

            // type declaration
            if(token().type() == ':'){
                next_token();
                type = CHECK_TYPE(get_identifier());
                next_token();
            }

            if (token().type() == ','){ next_token();   }

            // Add parameter
            fun->args().push_back(AST::Placeholder(vname, type));
        }
                                                      EAT(')');
        EXPECT(':', ": was expected");                EAT(':');
        EXPECT(tok_newline, "new line was expected"); EAT(tok_newline);
        EXPECT(tok_indent, "indent was expected"); // EAT(tok_indent);

        /*  Dont think to much about what the function does */
        AST::UnparsedBlock* body = new AST::UnparsedBlock();

        while(token().type() != tok_desindent && token()){
            body->tokens().push_back(next_token());
        }

        fun->body() = ST::Expr(body);
        return ST::Expr(fun);
    }

    // return One Top level Expression (Functions)
    ST::Expr parse_one(){
        if (tok_incorrect)
            next_token();

        switch(token().type()){
        case tok_def:
            return parse_function();
        default:
            assert("Unknown Token");
        }
    }

    //
    BaseScope parse_all(){
        // first token is tok_incorrect
        while(token()){
            _scope.insert(parse_one());
        }
    }

private:

    Lexer _lex;
    BaseScope _scope;
};

}

#endif // PARSER_H
