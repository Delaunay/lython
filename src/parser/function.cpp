#include "parser/parser.h"

namespace lython {
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
}
