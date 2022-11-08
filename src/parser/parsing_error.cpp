#include "parsing_error.h"

#include "utilities/strings.h"

namespace lython {

ParsingError::ParsingError(Array<int> expected, Token token, Node* obj, CodeLocation loc):
    ParsingError(expected, token, loc) {
    switch (obj->family()) {
    case NodeFamily::Expression: expr = (ExprNode*)obj;
    case NodeFamily::Pattern: pat = (Pattern*)obj;
    case NodeFamily::Statement: stmt = (StmtNode*)obj;
    }
}

void add_wip_expr(ParsingError& err, StmtNode* stmt) { err.stmt = stmt; }

void add_wip_expr(ParsingError& err, ExprNode* expr) { err.expr = expr; }

void add_wip_expr(ParsingError& err, Node* expr) {
    if (expr->family() == NodeFamily::Expression) {
        add_wip_expr(err, (ExprNode*)expr);
    }

    if (expr->family() == NodeFamily::Statement) {
        add_wip_expr(err, (StmtNode*)expr);
    }
}

void ParsingErrorPrinter::print(ParsingError const& error) {
    Array<String> toks;
    std::transform(std::begin(error.expected_tokens),
                   std::end(error.expected_tokens),
                   std::back_inserter(toks),
                   [](int tok) -> String { return to_human_name(tok); });

    String expected = join("|", toks);

    if (error.expected_tokens.size() > 0) {
        firstline() << "Expected: '" << expected << "' token but got "
                    << to_human_name(error.received_token);
    } else {
        firstline() << error.message;
    }

    if (with_compiler_code_loc) {
        newline() << error.loc.repr();
    }

    out << std::endl;
}

}  // namespace lython
