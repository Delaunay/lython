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
}
