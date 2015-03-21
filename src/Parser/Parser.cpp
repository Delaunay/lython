#include "Parser.h"

#define NEW_EXPR(x) _object.new_expr(new x)
#define NEW_FUNCT(x) _object.new_function(new x)
#define NEW_PROTO(x) _object.new_prototype(new x)
#define NEW_GEN(x) _object.new_gen_function(x)

//#define PARSER_DBG 1


#define EAT_TOK(x, err) if (token() != x) \
                            return error<AST::Expression>(err); \
                        next_token()\

namespace lython
{

const int& Parser::token() const
{
    return _token;
}

const int& Parser::next_token()
{
    _tk_idx++;
    _token = lexer.get_token();
    return _token;
}

Parser::Parser(Lexer& lex):
    lexer(lex), _token(0), _tk_idx(0)
{}

AST::Expression* Parser::parse_number_expression()
{
#if PARSER_DBG
    //      parse parent expressiong
    printf("Parse number expr       %d, %c \n", token(), token());
#endif

    AST::Expression* r = NEW_EXPR(AST::DoubleExpression(lexer.value()));

    next_token();

#if PARSER_DBG_PRINT
    r->print(cout);
#endif

    return r;
}

AST::Expression* Parser::parse_parent_expression()
{
#if PARSER_DBG
    //      parse parent expressiong
    printf("parse parent expression %d, %c \n", token(), token());
#endif

    next_token();

    AST::Expression* r = parse_expression();

    if (!r)
        return 0;

    if (token() != ')')
        return error<AST::Expression>("expected ')'");

    next_token();

    return r;
}

// identifier '(' expression* ')'
AST::Expression* Parser::parse_identifier_expression()
{
#if PARSER_DBG
    //      parse parent expressiong
    printf("parse identifier        %d, %c \n", token(), token());
#endif

    string idname = lexer.identifier();

    next_token();

    // then it is not a function
    // it is a Variable
    if (token() != '(')
        return NEW_EXPR(AST::VariableExpression(idname));

    next_token(); // '('

    AST::CallExpression::Arguments args;

    if (token() != ')')
    {
        // extract arguments
        while(1)
        {
            AST::Expression* arg = parse_expression();

            if (!arg)
                return 0;

            args.push_back(arg);

            if (token() == ')')
                break;

            if (token() != ',')
                return error<AST::Expression>("Expected ')' or ',' "
                                              "in argument list");

            next_token();
        }
    }

    next_token(); // ')'

    return NEW_EXPR(AST::CallExpression(idname, args));
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
AST::Expression* Parser::parse_primary()
{
#if PARSER_DBG
    //      parse parent expressiong
    printf("Parse primary           %d, %c \n", token(), token());
#endif

    if (token() == tok_newline)
        next_token();

    switch(token())
    {
        case tok_identifier: // - 4
            return parse_identifier_expression();

        case tok_number:     // - 5
            return parse_number_expression();

        case '(':
            return parse_parent_expression();

        case tok_if:
            return parse_if_expression();

        default:
            printf("%c, %d\n", token(), token());
            return error<AST::Expression>("unknown token when "
                                          "expecting an expression");
    }
}


const int Parser::precendence()
{
    if (!isascii(token()))
        return -1;

    int tokpre = _op[token()]; //binpre[token()];

    if (tokpre <= 0)
        return -1;

    return tokpre;
}

AST::Expression* Parser::parse_expression()
{
#if PARSER_DBG
    //      parse parent expressiong
    printf("Parse expression        %d, %c \n", token(), token());
#endif

    AST::Expression* lhs = parse_primary();

#if PARSER_DBG_PRINT
    lhs->print(cout);
#endif

    if (!lhs)
        return 0;

    return parse_bin_op_rhs(0, lhs);
}

AST::Expression* Parser::parse_bin_op_rhs(int exppre, AST::Expression* lhs)
{
    while(1)
    {
        #if PARSER_DBG
        //      parse parent expressiong
        printf("Parse bin op rhs        %d, %c \n", token(), token());
        #endif

        int tokpre = precendence();

        if (tokpre < exppre)
            return lhs;

        int binop = token();

        next_token();

        AST::Expression* rhs = parse_primary();

        if (!rhs)
            return 0;

        int nextpre = precendence();

        if (tokpre < nextpre)
        {
            rhs = parse_bin_op_rhs(tokpre + 1, rhs);

            if (rhs == 0)
                return 0;
        }

        lhs = NEW_EXPR(AST::BinaryExpression(binop, lhs, rhs));

        #if PARSER_DBG_PRINT
        lhs->print(cout);
        #endif
    }
}

AST::Expression* Parser::parse_if_expression()
{
    next_token();

    AST::Expression* cond = parse_expression();

    //printf("Parse cond \n");
    //cond->print(std::cout);
    //printf("\n");

    if (!cond)
        return 0;

    //if (token() != tok_then)
    //    return error<AST::Expression>("expected then");

    if (token() != ':')
        return error<AST::Expression>("expected ':'");

    next_token();

    AST::Expression* then = parse_expression();

    if (then == 0)
        return 0;

    // eat new line
    while (token() == tok_newline)
        next_token();

    EAT_TOK(tok_else, "expected else");

    EAT_TOK(':', "expected ':'");

    EAT_TOK(tok_newline, "expected '\\n'");

    AST::Expression* el = parse_expression();

    if (!el)
        return 0;

    return NEW_EXPR(AST::IfExpression(cond, then, el));
}

AST::Prototype* Parser::parse_prototype()
{
#if PARSER_DBG
    //      parse parent expressiong
    printf("parse prototype         %d, %c \n", token(), token());
#endif

    if (token() != tok_identifier)
        return error<AST::Prototype>("Expected function name in prototype");

    std::string fnname = lexer.identifier();

    // get parens
    next_token();

    if (token() != '(')
        return error<AST::Prototype>("Expected '(' in prototype");

    // Read the list of argument names.
    AST::Prototype::Arguments argname;

    next_token();
    // extract arguments
    while (token() == tok_identifier || token() == ',')
    {
        if (token() == ',')
            next_token();
        else
        {
            argname.push_back(lexer.identifier());
            next_token();
        }
    }

    // printf("%c %d %s\n", token(), token(), lexer.identifier().c_str());

    // close parens
    if (token() != ')')
        return error<AST::Prototype>("Expected ')' in prototype");

    // success.
    next_token();  // eat ')'

    // ':' is optional =(
    if (token() == ':' || token() == tok_newline)
    {
        next_token(); // eat ':'

        if (token() == tok_newline) // eat '\n'
            next_token();
    }
    else
        return error<AST::Prototype>("Expected ':' in prototype");

    return NEW_PROTO(AST::Prototype(fnname, argname));
}

AST::Function* Parser::parse_definition()
{
    // Previous tokken is 'def'
    // get function name || identifier
    next_token();

#if PARSER_DBG
    //      parse parent expressiong
    printf("Parse prototype         %d, %c \n", token(), token());
#endif

    // get the prototype
    AST::Prototype* proto = parse_prototype();

    if (proto == 0)
        return 0;

    // get the function body
    AST::Expression* e = parse_expression();

    if (e)
        return NEW_FUNCT(AST::Function(proto, e));

    return 0;
}

AST::Prototype* Parser::parse_extern()
{
#if PARSER_DBG
    //      parse parent expressiong
    printf("Parse extern            %d, %c \n", token(), token());
#endif

    next_token();
    return parse_prototype();
}

AST::Function* Parser::parse_top_level_expression()
{
#if PARSER_DBG
    //      parse parent expressiong
    printf("Parse top level         %d, %c \n", token(), token());
#endif

    AST::Expression* e = parse_expression();

    if (e)
    {
        AST::Prototype* proto = NEW_PROTO(AST::Prototype("",
                                          AST::Prototype::Arguments()));

        return NEW_FUNCT(AST::Function(proto, e));
    }

    return 0;
}

#include<iostream>

void Parser::parse()
{
    std::cout << "===============================================================================\n"
                 "                   Parser\n"
                 "===============================================================================\n\n";

    if (_tk_idx == 0)
        next_token();

    while(1)
    {
#if PARSER_DBG
        //      parse parent expressiong
        printf("Parse                   %d, %c \n", token(), token());
#endif

        switch (token())
        {
            case tok_eof:
                return;

            case ';':
            case tok_newline:
                next_token();
                break;  // ignore top-level semicolons.

            case tok_def:
                handle_definition();
                break;

            case tok_extern:
                handle_extern();
                break;

            default:
                handle_top_level_expression();
                break;
        }

        // printf("%d, %c \n", token(), token());
    }
}

void Parser::handle_definition()
{
#if PARSER_DBG
    //      parse parent expressiong
    printf("Handle def              %d, %c \n", token(), token());
#endif

    AST::Function* f = parse_definition();

    if(f)
    {
        f->print(cout);
        #if LLVM_CODEGEN
        llvm::Function* lf = NEW_GEN(f->code_gen(_gen));

        if (lf)
            lf->dump();
        #endif
    }
    else
    {
        next_token();
    }

}

void Parser::handle_extern()
{
#if PARSER_DBG
    //      parse parent expressiong
    printf("handle Extern           %d, %c \n", token(), token());
#endif

    AST::Prototype* p = parse_extern();

    if (p)
    {
        p->print(cout);

        #if LLVM_CODEGEN
        llvm::Function *lf = NEW_GEN(p->code_gen(_gen));

        if(lf)
            lf->dump();
        #endif
    }
    else
    {
        next_token();
    }
}

void Parser::handle_top_level_expression()
{
#if PARSER_DBG
    //      parse parent expressiong
    printf("handle Top Level        %d, %c \n", token(), token());
#endif

    AST::Function* f = parse_top_level_expression();

    if (f)
    {
        f->print(cout);

        #if LLVM_CODEGEN
        llvm::Function *lf = NEW_GEN(f->code_gen(_gen));

        if(lf)
            lf->dump();
        #endif
    }
    else
    {
        next_token();
    }
}

}
