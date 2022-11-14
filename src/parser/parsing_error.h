#ifndef LYTHON_PARSER_ERROR_H
#define LYTHON_PARSER_ERROR_H

#include "../dtypes.h"
#include "ast/nodes.h"
#include "lexer/token.h"

namespace lython {

struct ParsingError {
    Array<int>   expected_tokens;
    Token        received_token;
    StmtNode*    stmt = nullptr;
    ExprNode*    expr = nullptr;
    Pattern*     pat  = nullptr;
    String       error_kind;
    String       message;
    CodeLocation loc;
    Array<Token> remaining;  // Remaining token we have eaten to recover
                             // in practice we just eat all tokens until next line

    Array<Token> line;  // Line as a stream of tokens
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
    ParsingErrorPrinter(std::ostream& out, class AbstractLexer* lexer = nullptr):
        out(out), lexer(lexer) {}

    void print(ParsingError const& err);
    void print_ast(ParsingError const& error, Node* node, CommonAttributes* srcloc);
    void print_tok(ParsingError const& error, CommonAttributes* srcloc);

    bool          with_compiler_code_loc = false;
    std::ostream& out;
    int           indent = 0;

    void underline(Token const& tok);
    void underline(CommonAttributes const& attr);

    String        indentation() { return String(indent * 2, ' '); }
    std::ostream& firstline() { return out; }
    std::ostream& newline() { return out << std::endl << indentation(); }
    std::ostream& errorline() { return out << std::endl; }

    std::ostream& codeline() { return out << std::endl << indentation() << indentation() << "|"; }
    void          end() { out << std::endl; }

    class AbstractLexer* lexer;
};

class SyntaxError: public ParsingException {
    public:
    SyntaxError(String const& message = ""): msg(message) {}

    virtual const char* what() const NOTHROW { return msg.c_str(); }

    String msg;
};

String      shortprint(Node const* node);
Node const* get_parent(Node const* parent);

void add_wip_expr(ParsingError& err, StmtNode* stmt);
void add_wip_expr(ParsingError& err, ExprNode* expr);
void add_wip_expr(ParsingError& err, Node* expr);

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

}  // namespace lython

#endif
