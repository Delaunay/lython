#include "parser.h"


namespace lython {
// Consume a tokens without parsing them
Expression Parser::consume_block(std::size_t depth){
    TRACE_START();
    // a block starts with indent and finished disindent
    Array<Token> tokens;
    Token tok = token();

    EXPECT(tok_indent, "Indentation is expected");

    auto level = 1;
    while (level != 0) {
        tok = next_token();

        if (tok.type() == tok_desindent)
            level -= 1;
        else if (tok.type() == tok_indent)
            level += 1;

        tokens.push_back(tok);
    }

    return Expression::make<AST::UnparsedBlock>(tokens);
}
}
