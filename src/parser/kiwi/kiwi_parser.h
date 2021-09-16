#ifndef PARSER_H
#define PARSER_H

#include "lexer/lexer.h"
#include "logging/logging.h"

#include "utilities/optional.h"
#include "utilities/stack.h"
#include "utilities/trie.h"
#include "utilities/metadata.h"
#include "utilities/guard.h"

#include "ast/nodes.h"
#include "parser/module.h"

#include <iostream>
#include <numeric>

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
 * if a body is incorrect it has no impact if the function is not used
 *
 *  I can't have an incorrect identifier. Only Numbers can be incorrect
 *
 */

// "" at the end of the macro remove a warning about unecessary ;
#define EAT(tok)                                                               \
    if (token().type() == tok) {                                               \
        next_token();                                                          \
    }                                                                          \
    ""

// assert(token().type() == tok && msg)
#define TRACE_START()                                                          \
    trace_start(depth, "({}: {}, {})", to_string(token().type()).c_str(),      \
                token().type(), token().identifier())
#define TRACE_END()                                                            \
    trace_end(depth, "({}: {})", to_string(token().type()).c_str(),        \
              token().type())

#define CHECK_TYPE(type) type
#define CHECK_NAME(name) name
#define PARSE_ERROR(msg) std::cout << msg

#define show_token(tok) debug(tok_to_string(tok.type()))
#define EXPECT(tok, msg)                                                       \
    ASSERT(token().type() == tok, msg);                                        \
    ""


namespace lython {

/**
 * Kiwi parse source code in two passes.
 * The first one insert all the top level expression to the module without parsing the sublocks.
 * A second pass is done to parse the subblocks.
 *
 * - it allows us to get a full picture of the module expressions.
 * Users do not need to care about the order of the declarations.
 * It allows us to make lazy checking (only check what is used)
 * which gives a dynamic feels to the language (error will not happen unless used).
 *
 * - it makes the module size static and we do not need to adjust the debruijn indices
 */
void parse(AbstractLexer& lexer, Module& module);

// Finishing parsing the subblocks
void parse_sublocks(Expression expr, Module& module);

class ParserException : public Exception {
  public:
    template<typename ... Args>
    ParserException(const char* fmt, const Args& ... args) :
          Exception(fmt, "ParserException", args...)
    {}
};

#define WITH_EXPECT(tok, msg)                                                  \
    if (token().type() != tok) {                                               \
        debug("Got (tok: {}, {})", to_string(token().type()).c_str(),      \
              token().type());                                                 \
        throw ParserException(msg);                                            \
    } else


class Parser {
  public:
    Parser(AbstractLexer &lexer, Module *module)
        : module(module), _lex(lexer) {
        metadata_init_names();
    }

    // Shortcut
    Token const& next_token()  { return _lex.next_token(); }
    Token const& token()       { return _lex.token();      }
    Token const& peek_token()  { return _lex.peek_token(); }

    String get_identifier() {
        if (token().type() == tok_identifier) {
            return token().identifier();
        }

        debug("Missing identifier");
        return String("<identifier>");
    }

    // Consume a tokens without parsing them
    Array<Token> consume_block(std::size_t depth);

    Expression parse_function(Module& m, std::size_t depth);

    Expression parse_function_body(Expression expr, Module& m, std::size_t depth);

    Expression parse_compound_statement(Module& m, std::size_t depth);

    Expression parse_expression_1(Module& m, Expression lhs, int precedence, std::size_t depth);

    Token ignore_newlines();

    Expression parse_type(Module& m, std::size_t depth);

    AST::ParameterList parse_parameter_list(Module& m, std::size_t depth);

    Expression make_value(Token tok){
        Expression val;
        int8 type = tok.type();

        switch (type) {
        case tok_string:
            return Expression::make<AST::Value>(tok.identifier(), module->reference("String"));
        case tok_float:
            return Expression::make<AST::Value>(tok.as_float(), module->reference("Float"));
        case tok_int:
            return Expression::make<AST::Value>(tok.as_integer(), module->reference("Int"));
        }

        return Expression();
    }

    // Primary expressions are leaf nodes
    // primary := value         => tok_int/tok_float/tok_string
    //      | reference         => tok_identifier
    //      | unary operator    => tok_identifier
    Expression parse_primary(Module& m, std::size_t depth);

    Expression parse_statement(Module& m, int8 statement, std::size_t depth) {
        TRACE_START();
        EXPECT(statement, ": was expected");
        EAT(statement);

        auto expr = Expression::make<AST::Statement>();
        auto stmt = expr.ref<AST::Statement>();
        stmt->statement = statement;

        stmt->expr = parse_expression(m, depth + 1);
        TRACE_END();
        return expr;
    }

    Expression parse_function_call(Module& m, Expression function, std::size_t depth);

    // Parse a full line of function and stuff
    Expression parse_expression(Module& m, std::size_t depth);

    Expression parse_import(Module& m, std::size_t depth);

    void parse_struct_fields(Module& m, Expression data, std::size_t depth);

    // parse function_name(args...)
    Expression parse_top_expression(Module& m, std::size_t depth) {
        TRACE_START();
        auto _ = guard([&](){
            TRACE_END();
        });

        switch (token().type()) {
        case tok_import:
        case tok_from:
            return parse_import(m, depth + 1);

        case tok_async:
        case tok_yield:
        case tok_return:
            return parse_statement(m, token().type(), depth + 1);

        case tok_def:
            return parse_function(m, depth + 1);

        case tok_struct:
            return parse_struct(m, depth + 1);

        case tok_identifier:
        case tok_string:
        case tok_int:
        case tok_float:
            return parse_expression(m, depth + 1);

        case tok_match:
            return parse_match(m, depth + 1);

        case tok_while:
        case tok_for:
            return parse_loop(m, depth + 1);
        }

        return Expression();
    }

    Expression parse_struct(Module& m, std::size_t depth);

    Expression parse_match(Module& m, std::size_t depth);

    Expression parse_loop(Module& m, std::size_t depth);

    // return One Top level Expression (Functions)
    Expression parse_one(Module& m, std::size_t depth = 0) {
        Token tok = token();
        if (tok.type() == tok_incorrect) {
            tok = next_token();
        }

        while (tok.type() == tok_newline ) {
            tok = next_token();
        }

        info("{}", to_string(tok.type()));

        switch (tok.type()) {
        case tok_def:
            return parse_function(m, depth);

        case tok_struct:
            return parse_struct(m, depth);

        case tok_from:
        case tok_import:
            return parse_import(m, depth);

        default:
            assert(true, "Unknown Token");
        }

        return Expression();
    }

  private:
    // Top Level Module
    Module *module;
    AbstractLexer& _lex;
    // BaseScope _scope;
};

} // namespace lython

#endif // PARSER_H
