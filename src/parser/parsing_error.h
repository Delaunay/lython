#ifndef LYTHON_PARSER_ERROR_H
#define LYTHON_PARSER_ERROR_H

#include "../dtypes.h"
#include "ast/sexpression.h"
#include "lexer/token.h"

namespace lython {

struct ParsingError {
    Array<int>   expected_tokens;
    Token        received_token;
    StmtNode *   stmt;
    ExprNode *   expr;
    Pattern *    pat;
    String       message;
    CodeLocation loc;

    ParsingError(): received_token(dummy()), loc(LOC) {}

    ParsingError(Array<int> expected, Token token, CodeLocation loc_):
        expected_tokens(expected), received_token(token), loc(loc_) {}

    ParsingError(Array<int> expected, Token token, Node *obj, CodeLocation loc);

    static ParsingError syntax_error(String const &message);

    void print(std::ostream &out);
};

void add_wip_expr(ParsingError *err, StmtNode *stmt);
void add_wip_expr(ParsingError *err, ExprNode *expr);

} // namespace lython

#endif
