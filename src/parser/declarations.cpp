#include "parser.h"

namespace lython {

Expression Parser::parse_type(Module& m, size_t depth) {
    TRACE_START();
    String name = "<typename>";
    WITH_EXPECT(tok_identifier, "expect type identifier") {
        name = token().identifier();
    }

    auto type = m.reference(name);
    type.start() = token();
    EAT(tok_identifier);

    TRACE_END();
    return type;
}

AST::ParameterList Parser::parse_parameter_list(Module& m, std::size_t depth) {
    TRACE_START();
    EXPECT('(', "Expected start of parameter list");
    EAT('(');

    AST::ParameterList list;
    while (token().type() != ')' && token()) {
        String vname = CHECK_NAME(get_identifier());
        Expression type;
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

Expression Parser::parse_function_body(Expression expr, Module& m, std::size_t depth)
{
    // prepare the parsing context to parse the body
    auto fun = expr.ref<AST::Function>();
    Module module = m.enter();

    for(AST::Parameter& param: fun->args){
        auto ref = Expression::make<AST::Parameter>(param.name, param.type);
        module.insert(param.name.str(), ref);
    }

    return parse_compound_statement(module, depth + 1);
}

Expression Parser::parse_function(Module& m, std::size_t depth) {
    TRACE_START();
    Token start = token();
    EXPECT(tok_def, "Expected function to start by `def`");
    EAT(tok_def);

    String function_name = "<identifier>";

    WITH_EXPECT(tok_identifier, "Expected an identifier") {
        function_name = token().identifier();
    }
    EAT(tok_identifier);

    auto expr = Expression::make<AST::Function>(function_name);
    m.insert(function_name, expr);

    expr.start() = start;
    auto fun = expr.ref<AST::Function>();

    {
        // Creating a new module
        Module module = m.enter();
        fun->args = parse_parameter_list(m, depth + 1);

        WITH_EXPECT(tok_arrow, "Expected -> before return type") {
            EAT(tok_arrow);
            fun->return_type = parse_type(m, depth + 1);
        }

        EXPECT(':', "Expected function to end with a `:`");
        EAT(':');
        EXPECT(tok_newline, "Expected function to end with a new line");
        EAT(tok_newline);
        EXPECT(tok_indent, "Expected function body to start with an indentation");
        EAT(tok_indent);

        if (token().type() == tok_docstring) {
            fun->docstring = token().identifier();
            EAT(tok_docstring);
            EXPECT(tok_newline, "new line was expected");
            EAT(tok_newline);
        }

        auto tokens = consume_block(depth);
        fun->body = Expression::make<AST::UnparsedBlock>(tokens);

        expr.end() = fun->body.end();
        TRACE_END();
    }
    return expr;
}

Token Parser::ignore_newlines() {
    Token tok = token();
    while (tok.type() == tok_newline) {
        tok = next_token();
    }
    return tok;
}

Expression Parser::parse_compound_statement(Module& m, std::size_t depth) {
    TRACE_START();
    // if a docstring is present the indent token was already eaten
    // EXPECT(tok_indent, "Expected start of compound statement");
    // EAT(tok_indent);

    auto expr = Expression::make<AST::SeqBlock>();
    auto *block = expr.ref<AST::SeqBlock>();
    Token tok = token();

    while (tok.type() != tok_desindent && tok.type() != tok_eof) {
        auto expr = parse_top_expression(m, depth + 1);

        block->blocks.push_back(expr);

        tok = ignore_newlines();
    }

    if (token().type() == tok_eof) {
        TRACE_END();
        return expr;
    }

    EXPECT(tok_desindent, "Expected end of compound statement");
    EAT(tok_desindent);
    TRACE_END();
    return expr;
}

}
