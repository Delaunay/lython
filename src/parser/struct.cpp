#include "parser.h"

namespace lython {

void Parser::parse_struct_fields(Module& m, AST::Struct* data, std::size_t depth){
    TRACE_START();
    Token tok = token();

    // EXPECT(tok_indent, "Expect indentation");
    EAT(tok_indent);

    while (tok.type() != tok_desindent && tok.type() != tok_eof) {
        String attribute_name = "<attribute>";

        WITH_EXPECT(tok_identifier, "Expected identifier") {
            attribute_name = tok.identifier();
            EAT(tok_identifier);
        }

        EXPECT(':', "Expect :");
        EAT(':');

        data->insert(attribute_name, parse_type(m, depth));
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

    // Temporary implementation
    // Next steps will be to only use the unparsed tokens
    // and parse after the entire file is processed
    {
        // >>> Read the tokens
        auto tokens = consume_block(depth);

        for (auto& t: tokens){
            t.debug_print(std::cout) << "\n";
        }
        // Parse the tokes
        ReplayLexer lexer(tokens);
        Module struct_mod = m.enter();
        Parser par(lexer, &struct_mod);
        par.parse_struct_fields(struct_mod, data, depth + 1);
        // <<<
    }

    struct_.print(std::cout);

    module->insert(struct_name, struct_);
    TRACE_END();
    return struct_;
}
}
