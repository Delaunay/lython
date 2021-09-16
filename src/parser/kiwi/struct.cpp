#include "parser.h"

namespace lython {

void Parser::parse_struct_fields(Module& m, Expression expr, std::size_t depth){
    TRACE_START();
    Token tok = token();
    auto data = expr.ref<AST::Struct>();
    Module module = m.enter();

    EAT(tok_indent);

    while (tok.type() != tok_desindent && tok.type() != tok_eof) {
        String attribute_name = "<attribute>";

        WITH_EXPECT(tok_identifier, "Expected identifier") {
            attribute_name = tok.identifier();
            EAT(tok_identifier);
        }

        EXPECT(':', "Expect :");
        EAT(':');

        data->insert(attribute_name, parse_type(module, depth));
        tok = token();

        while (tok.type() == tok_newline) {
            tok = next_token();
        }
    }

    EAT(tok_desindent);
    TRACE_END();
}

Expression Parser::parse_struct(Module& m, std::size_t depth) {
    TRACE_START();
    EAT(tok_struct);

    Token tok = token();
    EXPECT(tok_identifier, "Expect an identifier");
    String struct_name = tok.identifier();
    EAT(tok_identifier);

    auto struct_ = Expression::make<AST::Struct>(struct_name);
    // insert now for recursive DS
    m.insert(struct_name, struct_);
    auto *data = struct_.ref<AST::Struct>();

    EXPECT(':', ": was expected");
    EAT(':');
    EXPECT(tok_newline, "newline was expected");
    EAT(tok_newline);

    EXPECT(tok_indent, "indentation was expected");
    EAT(tok_indent);

    tok = token();

    // docstring
    if (tok.type() == tok_docstring) {
        data->docstring = tok.identifier();
        tok = next_token();
    }

    while (tok.type() == tok_newline) {
        tok = next_token();
    }

    // >>> Read the tokens
    auto tokens = consume_block(depth);
    data->unparsed_tokens = tokens;

    // struct_.print(std::cout);
    TRACE_END();
    return struct_;
}
}
