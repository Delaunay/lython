#include "Parser.h"


namespace lython {

/*
<function-definition> ::= {<declaration-specifier>}* <declarator> {<declaration>}* <compound-statement>

<declaration> ::=  {<declaration-specifier>}+ {<init-declarator>}* ;

<direct-declarator> ::= <identifier>
                      | ( <declarator> )
                      | <direct-declarator> [ {<constant-expression>}? ]
                      | <direct-declarator> ( <parameter-type-list> )
                      | <direct-declarator> ( {<identifier>}*

* Body
<compound-statement> ::= { {<declaration>}* {<statement>}* }

<parameter-list> ::= <parameter-declaration>
                   | <parameter-list> , <parameter-declaration>

<parameter-declaration> ::= {<declaration-specifier>}+ <declarator>
                          | {<declaration-specifier>}+ <abstract-declarator>
                          | {<declaration-specifier>}+


<declarator> ::= {<pointer>}? <direct-declarator>

<declaration-specifier> ::= <storage-class-specifier>
                          | <type-specifier>
                          | <type-qualifier>

<storage-class-specifier> ::= auto
                            | register
                            | static
                            | extern
                            | typedef

<pointer> ::= * {<type-qualifier>}* {<pointer>}?
<type-qualifier> ::= const
                   | volatile

<type-specifier> ::= void
                   | char
                   | short
                   | int
                   | long
                   | float
                   | double
                   | signed
                   | unsigned
                   | <struct-or-union-specifier>
                   | <enum-specifier>
                   | <typedef-name>

*/

// def <declarator>(<parameter-list>) -> <declaration-specifier>:\n
//      tok_indent
//      <compound-statement>

AST::ParameterList Parser::parse_parameter_list(std::size_t depth){
    TRACE_START();
    EXPECT('(', "Expected start of parameter list"); EAT('(');

    AST::ParameterList list;
    while (token().type() != ')' && token()) {

        std::string vname = CHECK_NAME(get_identifier());
        ST::Expr type = nullptr;
        next_token();

        // type declaration
        if (token().type() == ':') {
            next_token();
            type = parse_type(depth + 1);
        }

        if (token().type() == ',') {
            next_token();
        }

        // Add parameter
        list.push_back(AST::Parameter(vname, type));
    }
    EXPECT(')', "Expected end of parameter list"); EAT(')');
    TRACE_END();
    return list;
}

ST::Expr Parser::parse_type(size_t depth){
    TRACE_START();

    auto name = new AST::Ref();
    name->name() = "<typename>";

    WITH_EXPECT(tok_identifier, "expect type identifier"){
        name->name() = token().identifier();
    }; EAT(tok_identifier);

    TRACE_END();
    return ST::Expr(name);
}

ST::Expr Parser::parse_function(std::size_t depth){
    TRACE_START();

    EXPECT(tok_def, "Expected function to start by `def`"); EAT(tok_def);

    std::string function_name = "<identifier>";

    WITH_EXPECT(tok_identifier, "Expected an identifier"){
        function_name = token().identifier();
    }; EAT(tok_identifier);

    auto parameters = parse_parameter_list(depth + 1);
    auto fun = new AST::Function(function_name);

    fun->args() = parameters;

    WITH_EXPECT(tok_arrow, "Expected -> before return type"){
       EAT(tok_arrow);
       fun->return_type() = parse_type(depth + 1);
    };

    EXPECT(':'        , "Expected function to end with a `:`")                ; EAT(':');
    EXPECT(tok_newline, "Expected function to end with a new line")           ; EAT(tok_newline);
    EXPECT(tok_indent , "Expected function body to start with an indentation"); EAT(tok_indent);

    if (token().type() == tok_docstring){
        fun->docstring() = token().identifier();
        EAT(tok_docstring);
        EXPECT(tok_newline, "new line was expected"); EAT(tok_newline);
    }

    fun->body() = parse_compound_statement(depth + 1);
    TRACE_END();
    return ST::Expr(fun);
}

Token Parser::ignore_newlines(){
    Token tok = token();
    while(tok.type() == tok_newline){
        tok = next_token();
    }
    return tok;
}

ST::Expr Parser::parse_compound_statement(std::size_t depth){
    TRACE_START();
    // if a docstring is present the indent token was already eaten
    // EXPECT(tok_indent, "Expected start of compound statement"); EAT(tok_indent);

    auto* block = new AST::SeqBlock();
    Token tok = token();

    while (tok.type() != tok_desindent && tok.type() != tok_eof){
        auto expr = parse_expression(depth + 1);

        block->blocks().push_back(expr);

        tok = ignore_newlines();
    }

    if (token().type() == tok_eof){
        return ST::Expr(block);
    }

    EXPECT(tok_desindent, "Expected end of compound statement"); EAT(tok_desindent);
    TRACE_END();
    return ST::Expr(block);
}
}
