#ifndef LYTHON_PARSER_ERROR_H
#define LYTHON_PARSER_ERROR_H

#include "../dtypes.h"
#include "ast/nodes.h"
#include "lexer/token.h"

namespace lython {

struct ParsingError {
    Array<int>   expected_tokens;
    Token        received_token;
    StmtNode*    stmt;
    ExprNode*    expr;
    Pattern*     pat;
    String       error_kind;
    String       message;
    CodeLocation loc;
    Array<Token> remaining;  // Remaining token we have eaten to recover
                             // in practice we just eat all tokens until next line

    ParsingError(): received_token(dummy()), loc(LOC) {}

    ParsingError(Array<int> expected, Token token, CodeLocation loc_):
        expected_tokens(expected), received_token(token), loc(loc_) {}

    ParsingError(Array<int> expected, Token token, Node* obj, CodeLocation loc);
};

class ParsingException: public LythonException {
    public:
};

class EndOfFileError: public ParsingException {};

class ParsingErrorPrinter {
    public:
    ParsingErrorPrinter(std::ostream& out): out(out) {}

    void print(ParsingError const& err);

    bool          with_compiler_code_loc = true;
    std::ostream& out;
    int           indent = 0;

    String indentation() { return String(indent * 2, ' '); }

    std::ostream& firstline() { return out; }
    std::ostream& newline() { return out << std::endl << indentation(); }
};

class SyntaxError: public ParsingException {
    public:
    SyntaxError(String const& message = ""): msg(message) {}

    virtual const char* what() const NOTHROW { return msg.c_str(); }

    String msg;
};

void add_wip_expr(ParsingError& err, StmtNode* stmt);
void add_wip_expr(ParsingError& err, ExprNode* expr);
void add_wip_expr(ParsingError& err, Node* expr);

}  // namespace lython

#endif
