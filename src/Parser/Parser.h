#ifndef PARSER_H
#define PARSER_H

#include "../Lexer/Lexer.h"
#include "../Logging/logging.h"

#include "Module.h"
#include <iostream>

/*
 *  TODO:
 *      Handle tok_incorrect so the parser does not stop
 *      I need a more generic way to parse tokens since
 *      I will need to parse vector<Token> too
 *
 *      Some statistic about compiler memory usage would be nice
 *
 *  if definition 1 is incorrect
 *  The parser should be able to parse definition 2 (which is correct)
 *  correctly
 *
 * if a body is incorrect it has not impact if the function is not used
 *
 *  I can't have an incorrect identifier. Only Numbers can be incorrect
 *
 */

#define EAT(tok)                                                               \
    if (token().type() == tok) {                                               \
        next_token();                                                          \
    }
#define EXPECT(tok, msg) ASSERT(token().type() == tok, msg);
// assert(token().type() == tok && msg)
#define TRACE(out) out << "function " << std::endl;
#define CHECK_TYPE(type) type
#define CHECK_NAME(name) name
#define PARSE_ERROR(msg) std::cout << msg;

namespace lython {

class Parser {
  public:
    Parser(AbstractBuffer &buffer) : _lex(buffer) {}

    // Shortcut
    Token next_token() { return _lex.next_token(); }
    Token token() { return _lex.token(); }

    ST::Expr get_unparsed_block() {}

    // currently type is a string
    // nevertheless we want to support advanced typing
    // which means type will be an expression too
    Type parse_type() {}

    std::string &get_identifier() {
        if (token().type() == tok_identifier)
            return token().identifier();

        ASSERT(true, "An Identifier was expected");
    }

    // Parsing routines
    ST::Expr parse_function() {
        info("");

        EAT(tok_def);

        // Get Name
        AST::Function *fun = new AST::Function(get_identifier());
        std::string ret_type = "unknown";
        next_token();

        // Parse Args
        EXPECT('(', "( was expected");
        EAT('(');
        while (token().type() != ')' && token()) {

            std::string vname = CHECK_NAME(get_identifier());
            std::string type = "unknown";
            next_token();

            // type declaration
            if (token().type() == ':') {
                next_token();
                type = CHECK_TYPE(get_identifier());
                next_token();
            }

            if (token().type() == ',') {
                next_token();
            }

            // Add parameter
            fun->args().push_back(AST::Placeholder(vname, type));
        }
        EAT(')');
        EXPECT(':', ": was expected");
        EAT(':');

        // Read return type if any
        if (token().type() == tok_arrow) {
            EAT(tok_arrow);
            ret_type = CHECK_TYPE(get_identifier());
            next_token();
        }
        fun->return_type() = make_type(ret_type);

        EXPECT(tok_newline, "new line was expected");
        EAT(tok_newline);
        EXPECT(tok_indent, "indent was expected"); // EAT(tok_indent);
                                                   // EAT(tok_docstring);

        /*  Dont think too much about what the function does */
        AST::UnparsedBlock *body = new AST::UnparsedBlock();

        while (token().type() != tok_desindent && token()) {
            body->tokens().push_back(next_token());
        }

        fun->body() = ST::Expr(body);
        return ST::Expr(fun);
    }

    // return One Top level Expression (Functions)
    ST::Expr parse_one() {
        Token tok = next_token();

        tok.print(std::cout) << "\n";

        info(tok_to_string(token().type()));

        switch (tok.type()) {

        case tok_def:
            return parse_function();
        default:
            assert("Unknown Token");
        }
    }

    //
    BaseScope parse_all() {
        info("");

        // first token is tok_incorrect
        while (token()) {
            _scope.insert(parse_one());
        }
    }

  private:
    Lexer _lex;
    BaseScope _scope;
};

} // namespace lython

#endif // PARSER_H
