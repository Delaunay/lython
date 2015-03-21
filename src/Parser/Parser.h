#ifndef LYTHON_PARSER_PARSER_HEADER
#define LYTHON_PARSER_PARSER_HEADER

#include "../Lexer/Lexer.h"
#include "../AbstractSyntaxTree/Expression.h"
#include "../AbstractSyntaxTree/Operators.h"

#include "ObjectManager.h"

#include <cstdio>

#include "../Generator/Generator.h"

namespace lython
{
    template<typename T>
    T* error(const char* str)
    {
        printf("Error: %s \n", str);
        return 0;
    }

class Parser
{
    public:

        Parser(Lexer& lex);

        const int& token() const;
        const int& next_token();
        const int  precendence();

        AST::Expression* parse_expression();
        AST::Expression* parse_bin_op_rhs(int exppre, AST::Expression* lhs);
        AST::Expression* parse_number_expression();
        AST::Expression* parse_parent_expression();
        AST::Expression* parse_if_expression();

        // identifier '(' expression* ')'
        AST::Expression* parse_identifier_expression();
        AST::Expression* parse_primary();

        AST::Prototype*  parse_extern();
        AST::Prototype*  parse_prototype();
        AST::Function*   parse_top_level_expression();
        AST::Function*   parse_definition();

        void handle_definition();
        void handle_extern();
        void handle_top_level_expression();

        void parse();

    protected:

    #if LLVM_CODEGEN
        Generator     _gen;
    #endif
        Operators     _op;
        int           _tk_idx;
        ObjectManager _object;
        int           _token;
        Lexer          lexer;
};
}

#endif
