// extension to python AST
#include "parser/parser.h"

#define TRACE_START2(tok) \
    kwtrace_start(outlog(), depth, "{}: {} - `{}`", to_string(tok.type()).c_str(), tok.type(), tok.identifier())

#define TRACE_START() TRACE_START2(token())

namespace lython {

ExprNode* Parser::parse_ifexp_ext(Node* parent, int depth) {
    TRACE_START();

    auto expr = parent->new_object<IfExp>();
    start_code_loc(expr, token());
    next_token();

    expr->test = parse_expression(expr, depth + 1);
    expect_token(':', true, expr, LOC);

    expr->body = parse_expression(expr, depth + 1);

    expect_token(tok_else, true, expr, LOC);

    expr->orelse = parse_expression(expr, depth + 1);

    end_code_loc(expr, token());
    return expr;
}
}  // namespace lython