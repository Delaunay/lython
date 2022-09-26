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

void add_wip_expr(ParsingError* err, StmtNode* stmt) {
    if (err == nullptr)
        return;

    err->stmt = stmt;
}

void add_wip_expr(ParsingError* err, ExprNode* expr) {
    if (err == nullptr)
        return;

    err->expr = expr;
}

void ParsingError::print(std::ostream& out) const {
    out << loc.repr() << std::endl;

    Array<String> toks;
    std::transform(std::begin(expected_tokens),
                   std::end(expected_tokens),
                   std::back_inserter(toks),
                   [](int tok) -> String { return to_string(tok); });

    String expected = join("|", toks);

    if (expected_tokens.size() > 0) {
        out << "    Expected: " << expected << " but got ";
        received_token.debug_print(out);
    } else {
        out << "    " << message;
    }
    out << std::endl << std::endl;
}

ParsingError ParsingError::syntax_error(String const& message) {
    auto p    = ParsingError();
    p.message = message;
    return p;
}

}  // namespace lython
