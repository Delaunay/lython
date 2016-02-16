#ifndef LYTHON_PARSER_PARSER_HEADER
#define LYTHON_PARSER_PARSER_HEADER

#include <cstdio>

#include "../Generator/Generator.h"
#include "../Lexer/Lexer.h"
#include "../AbstractSyntaxTree/Expression.h"
#include "../AbstractSyntaxTree/Operators.h"

#include "../AbstractSyntaxTree/Scope.h"
#include "Error.h"

#define PARSER_DBG 1

#ifndef MRED
#   define MRED     "\x1b[31m"
#   define MGREEN   "\x1b[32m"
#   define MYELLOW  "\x1b[33m"
#   define MBLUE    "\x1b[34m"
#   define MMAGENTA "\x1b[35m"
#   define MCYAN    "\x1b[36m"
#   define MRESET   "\x1b[0m"
#endif

// history size
#define HST_SIZE 10

#define COMM_ARGS Scope& s, int idt=0

namespace LIBNAMESPACE
{

class Parser
{
    public:

        Parser(Lexer& lex, Scope& m, std::ostream& out = std::cout);

        const int& token() const;
        const string& identifier() const; // shortcut

        const int& next_token();
        const int  precendence();
        const int& indent() const { return lexer.indent();  }

        AST::Expression* parse_multiline_expression(int idt=0);
        AST::Expression* parse_bin_op_rhs(Scope &s, int exppre, AST::Expression* lhs, int idt=0);
        AST::Expression* parse_number_expression(int idt=0);
        AST::Expression* parse_string_expression(int idt=0);
        AST::Expression* parse_parent_expression(Scope &s, int idt=0);
        AST::Expression* parse_if_expression(Scope& s, int idt=0);
        AST::Expression* parse_for_expression(Scope& s, int idt=0);
        AST::Expression* parse_unary(Scope &s, int idt=0);
        AST::Expression* parse_variable_expression(Scope &s, int idt=0);
        AST::Expression* parse_simple_expression(Scope &s, int idt=0);

        AST::Expression* parse_identifier_expression(Scope& s, int idt=0);
        AST::Expression* parse_primary(Scope &s, int idt=0);

        AST::Prototype*  parse_extern(int idt=0);
        AST::Prototype*  parse_prototype(int idt=0);
        AST::Expression* parse_top_level_expression(Scope &s, int idt=0);
        AST::Function*   parse_definition(Scope &s, int idt=0);
        AST::Expression* parse_class(Scope &s, int idt=0);

        AST::ScopedExpression* handle_definition(Scope &s, int idt=0);
        AST::ScopedExpression* handle_extern(int idt=0);
        AST::Expression* handle_top_level_expression(Scope &s, int idt=0);
        AST::ScopedExpression* handle_class(Scope &s, int idt=0);

        void parse();

        // 0 return current item
        // 1 return past item
        const int& past_token(const int& i)
        {
            return _past_token[std::max((_tk_idx - i - 1) % HST_SIZE, 0)];
        }

    protected:

    #if LLVM_CODEGEN
        Generator     _gen;
    #endif

        Operators     _op;
        int           _tk_idx;
        int           _token;
        Lexer          lexer;

        bool          _print_out;
        std::ostream& _out;
        int           _indent;

        Scope&        _module;
        vector<int>   _past_token;

#if PARSER_DBG
        Traceback     _intern_trace;
#endif
        // internal trace not used yet
//        Traceback     _lython_trace;
};
}

#endif
