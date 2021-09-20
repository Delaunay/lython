#ifndef LYTHON_PARSER_H
#define LYTHON_PARSER_H

#include "ast/magic.h"
#include "ast/sexpression.h"
#include "lexer/lexer.h"
#include "logging/logging.h"
#include "parser/parsing_error.h"
#include "utilities/metadata.h"

#include <iostream>
#include <numeric>

#define TRACE_START()                                                                      \
    trace_start(depth, "{}: {} - `{}`", to_string(token().type()).c_str(), token().type(), \
                token().identifier())

#define TRACE_END() trace_end(depth, "{}: {}", to_string(token().type()).c_str(), token().type())

namespace lython {

class Parser {
    public:
    Parser(AbstractLexer &lexer): _lex(lexer) { metadata_init_names(); }

    Module *parse_module() {
        if (token().type() == tok_incorrect) {
            next_token();
        }

        // lookup the module
        Module *module = new Module();
        parse_body(module, module->body, 0);
        return module;
    }

    Token  parse_body(Node *parent, Array<StmtNode *> &out, int depth);
    Token  parse_except_handler(Node *parent, Array<ExceptHandler> &out, int depth);
    void   parse_alias(Node *parent, Array<Alias> &out, int depth);
    Token  parse_match_case(Node *parent, Array<MatchCase> &out, int depth);
    String parse_module_path(Node *parent, int &level, int depth);

    Pattern *parse_pattern(Node *parent, int depth);
    Pattern *parse_pattern_1(Node *parent, int depth);

    Pattern *parse_match_sequence(Node *parent, int depth);
    Pattern *parse_match_star(Node *parent, int depth);
    Pattern *parse_match_mapping(Node *parent, int depth);

    Pattern *parse_match_or(Node *parent, Pattern *primary, int depth);
    Pattern *parse_match_as(Node *parent, Pattern *primary, int depth);
    Pattern *parse_match_class(Node *parent, ExprNode *cls, int depth);

    // Statement_1
    StmtNode *parse_function_def(Node *parent, bool async, int depth);
    StmtNode *parse_class_def(Node *parent, int depth);
    StmtNode *parse_for(Node *parent, int depth);
    StmtNode *parse_while(Node *parent, int depth);
    StmtNode *parse_if(Node *parent, int depth);
    StmtNode *parse_match(Node *parent, int depth);
    StmtNode *parse_with(Node *parent, int depth);
    StmtNode *parse_raise(Node *parent, int depth);
    StmtNode *parse_try(Node *parent, int depth);
    StmtNode *parse_assert(Node *parent, int depth);
    StmtNode *parse_import(Node *parent, int depth);
    StmtNode *parse_import_from(Node *parent, int depth);
    StmtNode *parse_global(Node *parent, int depth);
    StmtNode *parse_nonlocal(Node *parent, int depth);
    StmtNode *parse_return(Node *parent, int depth);
    StmtNode *parse_del(Node *parent, int depth);
    StmtNode *parse_pass(Node *parent, int depth);
    StmtNode *parse_break(Node *parent, int depth);
    StmtNode *parse_continue(Node *parent, int depth);

    // Statement_2
    StmtNode *parse_assign(Node *parent, ExprNode *epxr, int depth);
    StmtNode *parse_augassign(Node *parent, ExprNode *epxr, int depth);
    StmtNode *parse_annassign(Node *parent, ExprNode *epxr, int depth);

    Token previous = dummy();

    StmtNode *parse_statement(Node *parent, int depth) {
        TRACE_START();

        if (previous == token()) {
            error("Unhandled token {} previous tok was {}", str(token()), str(previous));
            return nullptr;
        } else {
            previous = token();
        }

        // Statement we can guess rightaway from the current token we are seeing
        switch (token().type()) {
        // def <name>(...
        case tok_def:
            return parse_function_def(parent, false, depth);

        // async def <name>(...
        case tok_async:
            return parse_function_def(parent, true, depth);

        case tok_class:
            return parse_class_def(parent, depth);

        // Async for: only valid inside async function
        case tok_for:
            return parse_for(parent, depth);

        case tok_while:
            return parse_while(parent, depth);
        case tok_if:
            return parse_if(parent, depth);

        case tok_match:
            return parse_match(parent, depth);
        case tok_with:
            return parse_with(parent, depth);
            // Async with: only valid inside async function

        case tok_raise:
            return parse_raise(parent, depth);
        case tok_try:
            return parse_try(parent, depth);
        case tok_assert:
            return parse_assert(parent, depth);

        case tok_import:
            return parse_import(parent, depth);
        case tok_from:
            return parse_import_from(parent, depth);

        case tok_global:
            return parse_global(parent, depth);
        case tok_nonlocal:
            return parse_nonlocal(parent, depth);

        case tok_return:
            return parse_return(parent, depth);
        case tok_del:
            return parse_del(parent, depth);
        case tok_pass:
            return parse_pass(parent, depth);
        case tok_break:
            return parse_break(parent, depth);
        case tok_continue:
            return parse_continue(parent, depth);
        }

        auto expr = parse_expression(parent, depth);

        switch (token().type()) {
        // <expr> = <>
        case tok_assign:
            return parse_assign(parent, expr, depth);
        // <expr> += <>
        case tok_augassign:
            return parse_augassign(parent, expr, depth);
        // <expr>: type = <>
        case tok_annassign:
            return parse_annassign(parent, expr, depth);
        }

        auto stmt_expr   = parent->new_object<Expr>();
        stmt_expr->value = expr;
        return stmt_expr;
    }

    void start_code_loc(CommonAttributes *target, Token tok);
    void end_code_loc(CommonAttributes *target, Token tok);

    ConstantValue get_value();

    // Primary expression
    // parse_expression_1
    ExprNode *parse_await(Node *parent, int depth);
    ExprNode *parse_yield(Node *parent, int depth);
    ExprNode *parse_yield_from(Node *parent, int depth);
    ExprNode *parse_name(Node *parent, int depth);
    ExprNode *parse_lambda(Node *parent, int depth);
    ExprNode *parse_constant(Node *parent, int depth);
    ExprNode *parse_joined_string(Node *parent, int depth);
    ExprNode *parse_ifexp(Node *parent, int depth);
    ExprNode *parse_starred(Node *parent, int depth);
    ExprNode *parse_list(Node *parent, int depth);
    ExprNode *parse_tuple_generator(Node *parent, int depth);
    ExprNode *parse_set_dict(Node *parent, int depth);
    ExprNode *parse_prefix_unary(Node *parent, int depth);

    void      parse_comprehension(Node *parent, Array<Comprehension> &out, char kind, int depth);
    Arguments parse_arguments(Node *parent, char kind, int depth);
    Token     parse_call_args(Node *parent, Array<ExprNode *> &args, Array<Keyword> &keywords,
                              int depth);
    void      parse_withitem(Node *parent, Array<WithItem> &out, int depth);
    ExprNode *parse_star_targets(Node *parent, int depth);

    // parse_expression_2
    // TODO: primary has the parent has GC not the expression it belongs to
    ExprNode *parse_named_expr(Node *parent, ExprNode *primary, int depth);
    ExprNode *parse_bool_operator(Node *parent, ExprNode *primary, int depth);
    ExprNode *parse_binary_operator(Node *parent, ExprNode *primary, int depth);
    ExprNode *parse_compare_operator(Node *parent, ExprNode *primary, int depth);
    ExprNode *parse_suffix_unary(Node *parent, ExprNode *primary, int depth);
    ExprNode *parse_call(Node *parent, ExprNode *primary, int depth);
    ExprNode *parse_attribute(Node *parent, ExprNode *primary, int depth);
    ExprNode *parse_subscript(Node *parent, ExprNode *primary, int depth);
    ExprNode *parse_slice(Node *parent, ExprNode *primary, int depth);

    ExprNode *parse_expression(Node *parent, int depth) {
        // parse primary
        auto primary = parse_expression_primary(parent, depth);

        switch (token().type()) {
        // <expr>(args...)
        case tok_parens:
            primary = parse_call(parent, primary, depth);
        }

        primary = parse_expression_1(parent, primary, 0, depth);

        return primary;
    }

    // check if this is a composed expression
    ExprNode *parse_expression_1(Node *parent, ExprNode *primary, int min_precedence, int depth) {
        //
        switch (token().type()) {
        // <expr> := <expr>
        // assign expression instead of the usual assign statement
        case tok_walrus:
            return parse_named_expr(parent, primary, depth);

        // <expr> boolop <expr>
        /*
        case tok_boolop:
            return parse_bool_operator(parent, primary, depth);
        case tok_binaryop:
            return parse_binary_operator(parent, primary, depth);
        case tok_compareop:
            return parse_compare_operator(parent, primary, depth);
        */
        case tok_unaryop:
            return parse_suffix_unary(parent, primary, depth);

        case tok_in:
        case tok_operator:
            return parse_operators(parent, primary, min_precedence, depth);

        // <expr>.<identifier>
        case tok_dot:
            return parse_attribute(parent, primary, depth);
        // <expr>[
        case tok_square:
            return parse_subscript(parent, primary, depth);

        // ':' is only valid inside a subscript
        case ':': {
            if (allow_slice())
                return parse_slice(parent, primary, depth);
        }
        }

        return primary;
    }

    OpConfig get_operator_config(Token const &tok) const {
        Dict<String, OpConfig> const &confs = default_precedence();

        auto result = confs.find(tok.operator_name());
        if (result == confs.end()) {
            error("Could not find operator settings for {}", str(tok));
            return OpConfig();
        }
        return result->second;
    }

    bool is_binary_operator_family(OpConfig const &conf) {
        return conf.binarykind != BinaryOperator::None || conf.cmpkind != CmpOperator::None ||
               conf.boolkind != BoolOperator::None;
    }

    // Precedence climbing method
    // I liked shunting yard algorithm better has it was not recursive
    // but it got issues with some edge cases.
    //
    // https://en.wikipedia.org/wiki/Operator-precedence_parser#:~:text=The%20precedence%20climbing%20method%20is%20a%20compact%2C%20efficient%2C,in%20EBNF%20format%20will%20usually%20look%20like%20this%3A
    ExprNode *parse_operators(Node *parent, ExprNode *lhs, int min_precedence, int depth);

    // Expression we can guess rightaway from the current token we are seeing
    ExprNode *parse_expression_primary(Node *parent, int depth) {
        switch (token().type()) {
        // await <expr>
        case tok_await:
            return parse_await(parent, depth);
        // yield <expr>
        case tok_yield:
            return parse_yield(parent, depth);
        // yield from <expr>
        case tok_yield_from:
            return parse_yield_from(parent, depth);
        // <identifier>
        case tok_identifier:
            return parse_name(parent, depth);
        // lambda <name>:
        case tok_lambda:
            return parse_lambda(parent, depth);

        case tok_float:
            return parse_constant(parent, depth);

        case tok_int:
            return parse_constant(parent, depth);
        // "
        case tok_string:
            return parse_constant(parent, depth);
        // f"
        case tok_fstring:
            return parse_joined_string(parent, depth);
        // if <expr> else <expr>
        case tok_if:
            return parse_ifexp(parent, depth);
        // *<expr>
        case tok_star:
            return parse_starred(parent, depth);

        case tok_operator:
            return parse_prefix_unary(parent, depth);

        // List: [a, b]
        // Comprehension [a for a in b]
        case tok_square:
            return parse_list(parent, depth);

        // Tuple: (a, b)
        // Generator Comprehension: (a for a in b)
        // can be (1 + b)
        case tok_parens:
            return parse_tuple_generator(parent, depth);

        // Set: {a, b}
        // Comprehension {a for a in b}
        //      OR
        // Dict: {a : b}
        // Comprehension {a: b for a, b in c}
        case tok_curly:
            return parse_set_dict(parent, depth);
        }

        // Left Unary operator
        // + <expr> | - <expr> | ! <expr> | ~ <expr>

        // TODO: return a dummy ExprNode
        return nullptr;
    }

    ParsingError *expect_token(int expected, bool eat, Node *wip_expression, CodeLocation loc) {
        return expect_tokens(Array<int>{expected}, eat, wip_expression, loc);
    }

    ParsingError *expect_operator(String const &op, bool eat, Node *wip_expression,
                                  CodeLocation loc) {
        Token tok = token();

        auto err = expect_token(tok_operator, eat, wip_expression, LOC);
        if (err != nullptr) {
            return err;
        }

        if (tok.operator_name() == op) {
            return nullptr;
        }

        err = &errors.emplace_back(ParsingError::syntax_error("Wrong operator"));
        StringStream ss;
        err->print(ss);
        warn("{}", ss.str());
        return err;
    }

    ParsingError *expect_tokens(Array<int> const &expected, bool eat, Node *wip_expression,
                                CodeLocation loc) {
        for (auto &tok: expected) {
            if (token().type() == tok) {
                if (eat) {
                    next_token();
                }

                return nullptr;
            }
        }
        // ----

        return write_error(expected, wip_expression, loc);
    }

    ParsingError *write_error(Array<int> const &expected, Node *wip_expression, CodeLocation loc) {
        // if the token does not match assume we "had it"
        // and record the error
        // so we can try to parse as much as possible
        auto err = &errors.emplace_back(expected, token(), wip_expression, loc);

        StringStream ss;
        err->print(ss);

        warn("{}", ss.str());
        return err;
    }

    // Shortcuts
    Token const &next_token() { return _lex.next_token(); }
    Token const &token() const { return _lex.token(); }
    Token const &peek_token() const { return _lex.peek_token(); }

    String get_identifier() const {
        if (token().type() == tok_identifier) {
            return token().identifier();
        }
        return String("<identifier>");
    }

    bool async() const {
        if (async_mode.size() <= 0) {
            return false;
        }

        return async_mode[int(async_mode.size()) - 1];
    }

    // not 100% sure this is the right way
    ExprContext context() const {
        if (_context.size() <= 0) {
            return ExprContext::Load;
        }

        return _context[int(_context.size()) - 1];
    }

    bool allow_slice() const {
        if (_allow_slice.size() <= 0) {
            return false;
        }

        return _allow_slice[int(_allow_slice.size()) - 1];
    }

    Array<ParsingError> get_errors() { return errors; }

    private:
    std::vector<bool>        _allow_slice;
    std::vector<ExprContext> _context;
    std::vector<bool>        async_mode;
    AbstractLexer &          _lex;
    Array<ParsingError>      errors;
};

} // namespace lython
#endif
