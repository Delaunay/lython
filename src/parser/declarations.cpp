#include "parser.h"

namespace lython {

/*
<function-definition> ::= {<declaration-specifier>}* <declarator>
{<declaration>}* <compound-statement>

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

AST::ParameterList Parser::parse_parameter_list(Module& m, std::size_t depth) {
    TRACE_START();
    EXPECT('(', "Expected start of parameter list");
    EAT('(');

    AST::ParameterList list;
    while (token().type() != ')' && token()) {

        String vname = CHECK_NAME(get_identifier());
        ST::Expr type = nullptr;
        next_token();

        // type declaration
        if (token().type() == ':') {
            next_token();
            type = parse_type(m, depth + 1);
        }

        if (token().type() == ',') {
            next_token();
        }

        // Add parameter
        list.push_back(AST::Parameter(vname, type));
    }
    EXPECT(')', "Expected end of parameter list");
    EAT(')');
    TRACE_END();
    return list;
}

ST::Expr Parser::parse_type(Module& m, size_t depth) {
    TRACE_START();

    String name = "<typename>";
    int loc = -1;

    WITH_EXPECT(tok_identifier, "expect type identifier") {
        name = token().identifier();
        loc = m.find_index(name);
    };

    if (loc < 0){
        warn("Undefined type \"%s\"", name.c_str());
    }
    auto type = new AST::Ref(name, loc);
    EAT(tok_identifier);

    TRACE_END();
    return ST::Expr(type);
}

ST::Expr Parser::parse_function(Module& m, std::size_t depth) {
    TRACE_START();

    EXPECT(tok_def, "Expected function to start by `def`");
    EAT(tok_def);

    String function_name = "<identifier>";

    WITH_EXPECT(tok_identifier, "Expected an identifier") {
        function_name = token().identifier();
    };
    EAT(tok_identifier);

    // Creating a new module
    AccessTracker tracker;
    Module module = m.enter(&tracker);
    auto parameters = parse_parameter_list(m, depth + 1);
    auto fun = new AST::Function(function_name);
    ST::Expr fun_ptr = ST::Expr(fun);

    // Insert function for recursive calls
    // debug("Insert Function");
    module.insert(function_name, fun_ptr);

    fun->args() = parameters;

    // Insert the parameters into the Scope
    for(AST::Parameter& param: parameters){
        // debug("Insert Parameter");
        module.insert(param.name(), param.type());
    }

    WITH_EXPECT(tok_arrow, "Expected -> before return type") {
        EAT(tok_arrow);
        fun->return_type() = parse_type(m, depth + 1);
    };

    EXPECT(':', "Expected function to end with a `:`");
    EAT(':');
    EXPECT(tok_newline, "Expected function to end with a new line");
    EAT(tok_newline);
    EXPECT(tok_indent, "Expected function body to start with an indentation");
    EAT(tok_indent);

    if (token().type() == tok_docstring) {
        fun->docstring() = token().identifier();
        EAT(tok_docstring);
        EXPECT(tok_newline, "new line was expected");
        EAT(tok_newline);
    }

    fun->body() = parse_compound_statement(module, depth + 1);
    TRACE_END();

    m.insert(function_name, fun_ptr);
    fun->frame = tracker.access;
    return fun_ptr;
}

Token Parser::ignore_newlines() {
    Token tok = token();
    while (tok.type() == tok_newline) {
        tok = next_token();
    }
    return tok;
}

ST::Expr Parser::parse_compound_statement(Module& m, std::size_t depth) {
    TRACE_START();
    // if a docstring is present the indent token was already eaten
    // EXPECT(tok_indent, "Expected start of compound statement");
    // EAT(tok_indent);

    auto *block = new AST::SeqBlock();
    Token tok = token();

    while (tok.type() != tok_desindent && tok.type() != tok_eof) {
        auto expr = parse_top_expression(m, depth + 1);

        block->blocks().push_back(expr);

        tok = ignore_newlines();
    }

    if (token().type() == tok_eof) {
        return ST::Expr(block);
    }

    EXPECT(tok_desindent, "Expected end of compound statement");
    EAT(tok_desindent);
    TRACE_END();
    return ST::Expr(block);
}

/* <statement> ::= <labeled-statement>  <labeled-statement>     ::= <identifier>
   : <statement> | case <constant-expression> : <statement>  | default :
   <statement>
              | <selection-statement>   <selection-statement>   ::= if (
   <expression> ) <statement>    | if ( <expression> ) <statement> else
   <statement> | switch ( <expression> ) <statement>
              | <iteration-statement>   <iteration-statement>   ::= while (
   <expression> ) <statement> | do <statement> while ( <expression> ) ;  | for (
   {<expression>}? ; {<expression>}? ; {<expression>}? ) <statement>
              | <jump-statement>        <jump-statement>        ::= goto
   <identifier> ;   | continue ; | break ;   | return {<expression>}? ;
              | <expression-statement>  <expression-statement>  ::=
   {<expression>}? ;
              | <compound-statement>    <compound-statement>    ::= {
   {<declaration>}* {<statement>}* }
 */
/*
ST::Expr Parser::parse_statement(int8 statement, std::size_t depth){
    // labeled statement
    if (token().type() == tok_identifier){
        String label = token().identifier();

        // <selection-statement>
        if (label == "if") {}
        if (label == "switch") {}

        // <iteration-statement>
        if (label == "while") {}
        if (label == "do") {}
        if (label == "for") {}

        // <jump-statement>
        if (label == "goto") {}
        if (label == "continue") {}
        if (label == "return") {}
    }
}*/
}
