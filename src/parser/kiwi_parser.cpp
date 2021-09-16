#include "parser.h"


namespace lython {
// Consume a tokens without parsing them
Array<Token> Parser::consume_block(std::size_t depth){
    TRACE_START();
    // a block starts with indent and finished disindent
    Array<Token> tokens;
    Token tok = token();

    EAT(tok_indent);
    tok = token();
    auto level = 1;

    // Improve that loop
    while (level != 0 && tok.type() != tok_eof) {
        if (tok.type() == tok_desindent) {
            level -= 1;
        }

        if (tok.type() == tok_indent) {
            level += 1;
        }

        tokens.push_back(tok);
        tok = next_token();
    }

    // Generate a tok_desindent if EOF was reached
    if (tok.type() == tok_eof){
        tokens.push_back(Token(tok_desindent, tok.line(), tok.col()));
    }

    EAT(tok_desindent);
    TRACE_END();
    return tokens;
}


void unparse_function(Expression funexpr, Module& module){
    assert(funexpr.kind() == AST::NodeKind::KFunction, "Can only unparse function expression");
    auto fun = funexpr.ref<AST::Function>();

    if (fun->body.kind() == AST::NodeKind::KUnparsedBlock){
        auto unparsed = fun->body.ref<AST::UnparsedBlock>();
        ReplayLexer lexer(unparsed->tokens);
        Parser parser(lexer, &module);

        trace(0, "Unparsing function");
        fun->body = parser.parse_function_body(funexpr, module, 0);
    }
}

void unparse_struct(Expression struct_, Module& module){
    assert(struct_.kind() == AST::NodeKind::KStruct, "Can only unparse function expression");
    auto structt = struct_.ref<AST::Struct>();

    if (structt->unparsed_tokens.size() > 0){
        ReplayLexer lexer(structt->unparsed_tokens);
        Parser parser(lexer, &module);

        trace(0, "Unparsing struct");
        parser.parse_struct_fields(module, struct_, 0);
        structt->unparsed_tokens = Array<Token>();
    }
}


void parse_sublocks(Expression expr, Module& module){
    assert(expr, "Expression should not be null");

    if (expr.kind() == AST::NodeKind::KFunction){
        unparse_function(expr, module);
    }

    if (expr.kind() == AST::NodeKind::KStruct){
        unparse_struct(expr, module);
    }
}

void parse(AbstractLexer& lexer, Module& module){
    Parser par(lexer, &module);

    {
        Expression expr;

        // First pass, insert top level expression
        // to the context
        do {
            expr = par.parse_one(module);
        } while(expr);
    }

    // Second pass
    // Parse top level expression
    for (auto pair: module){
        Expression expr = pair.second;
        parse_sublocks(expr, module);
    }
};

}
