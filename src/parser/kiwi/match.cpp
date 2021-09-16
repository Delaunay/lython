#include "parser/parser.h"

namespace lython {


void parse_branch(){

}

Expression Parser::parse_match(Module& m, std::size_t depth){
    EAT(tok_match);

    Expression match = Expression::make<AST::Match>();
    AST::Match* mat = match.ref<AST::Match>();

    // match <target>:
    Expression target = parse_expression(m, depth + 1);

    EXPECT(':', "Expect `:` after match <target>:");
    EAT(tok_newline);

    EXPECT(tok_indent, "Indentation expected after `:`");

    while (token().type() != tok_desindent){
        EXPECT(tok_case, "Expect `case <cond>:` after match");
        EAT(tok_case);

        auto cond = parse_expression(m, depth + 1);

        // EAT(tok_when)
        EXPECT(':', "Expect `:` after target");
        EXPECT(tok_indent, "");

        auto result = parse_expression(m, depth + 1);
        mat->branches.emplace_back(cond, result);
    }

    return match;
}

}
