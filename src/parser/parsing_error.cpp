#include "parsing_error.h"

#include "ast/magic.h"
#include "ast/ops.h"
#include "lexer/lexer.h"
#include "lexer/unlex.h"
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
    switch (expr->family()) {
    case NodeFamily::Expression: err.expr = (ExprNode*)expr;
    case NodeFamily::Pattern: err.pat = (Pattern*)expr;
    case NodeFamily::Statement: err.stmt = (StmtNode*)expr;
    }
}

// Supresses newlines from a a stream
class NoNewLine: public std::basic_stringbuf<char, std::char_traits<char>, std::allocator<char>> {
    public:
    NoNewLine(std::ostream& out): out(out) {}

    virtual int sync() {
        std::string data = str();
        int         val  = int(data.size());

        // this might be too simplistic
        // what if the new line is in the middle of the expression
        for (char c: data) {
            if (c == '\n')
                continue;

            out << c;
        }
        // clear buffer
        str("");
        return val;
    }

    std::ostream& out;
};
//  python3 main.py
//   File "/home/runner/ShyCalculatingGraph/main.py", line 1
//     2 *
//         ^
// SyntaxError: invalid syntax
// exit status 1

void ParsingErrorPrinter::print(ParsingError const& error) {
    Array<String> toks;
    std::transform(std::begin(error.expected_tokens),
                   std::end(error.expected_tokens),
                   std::back_inserter(toks),
                   [](int tok) -> String { return to_human_name(tok); });

    String expected = join("|", toks);

    String filename = "";
    if (lexer) {
        filename = lexer->file_name();
    }
    int line = error.received_token.begin_line();

    firstline() << "File \"" << filename << "\", line " << line;

    CommonAttributes* val        = nullptr;
    auto              noline_val = NoNewLine(out);
    std::ostream      noline(&noline_val);

    if (error.stmt) {
        newline();
        noline << str(error.stmt);
        val = error.stmt;
    } else if (error.expr) {
        newline();
        noline << str(error.expr);
        val = error.expr;
    } else if (error.pat) {
        newline();
        noline << str(error.pat);
        val = error.pat;
    }

    noline.flush();

    Unlex unlex;
    unlex.format(out, error.remaining);

    if (val) {
        underline(*val);
    } else {
        // this does not work that well
        underline(error.received_token);
    }

    newline() << error.error_kind << ": ";

    if (error.expected_tokens.size() > 0) {
        out << "Expected: " << expected << " token but got " << to_human_name(error.received_token);
    } else {
        out << error.message;
    }

    if (with_compiler_code_loc) {
        newline() << error.loc.repr();
    }

    end();
}

void ParsingErrorPrinter::underline(CommonAttributes const& attr) {

    int32 size = 1;

    if (attr.end_col_offset.has_value()) {
        size = attr.end_col_offset.value() - attr.col_offset;
    }

    int32 start = attr.col_offset;
    newline() << String(start, ' ') << String(size, '^');
}

void ParsingErrorPrinter::underline(Token const& tok) {
    int32 tok_size  = std::max(1, tok.end_col() - tok.begin_col());
    int32 tok_start = tok.begin_col();
    newline() << String(tok_start, ' ') << String(tok_size, '^');
}

}  // namespace lython
