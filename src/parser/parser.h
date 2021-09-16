#ifndef LYTHON_PARSER_H
#define LYTHON_PARSER_H

#include "lexer/lexer.h"
#include "logging/logging.h"

#include "utilities/optional.h"
#include "utilities/stack.h"
#include "utilities/trie.h"
#include "utilities/metadata.h"
#include "utilities/guard.h"

#include "ast/sexpression.h"
#include "parser/module.h"

#include <iostream>
#include <numeric>

#define TRACE_START()                                                          \
    trace_start(depth, "({}: {}, {})", to_string(token().type()).c_str(),      \
                token().type(), token().identifier())
#define TRACE_END()                                                            \
    trace_end(depth, "({}: {})", to_string(token().type()).c_str(),            \
              token().type())

#define TRACE() {\
    TRACE_START();\
    auto _ = guard([&](){\
        TRACE_END();\
    });}

namespace lython {
class Parser {
  public:
    Parser(AbstractLexer &lexer): _lex(lexer) {
        metadata_init_names();
    }

    static ModNode* parse_module(String name) {
        // lookup the module

        Module* module = new Module();


        return (ModNode*)module;

        // is this just for repl ?
        // Interactive;
        // Expression;
        //?
        // FunctionType;
    }

    // Statement_1
    StmtNode* parse_function_def(GCObject* parent, int depth);
    StmtNode* parse_async_function_def(GCObject* parent, int depth);
    StmtNode* parse_class_def(GCObject* parent, int depth);
    StmtNode* parse_for(GCObject* parent, int depth);
    StmtNode* parse_while(GCObject* parent, int depth);
    StmtNode* parse_if(GCObject* parent, int depth);
    StmtNode* parse_match(GCObject* parent, int depth);
    StmtNode* parse_with(GCObject* parent, int depth);
    StmtNode* parse_raise(GCObject* parent, int depth);
    StmtNode* parse_try(GCObject* parent, int depth);
    StmtNode* parse_assert(GCObject* parent, int depth);
    StmtNode* parse_import(GCObject* parent, int depth);
    StmtNode* parse_import_from(GCObject* parent, int depth);
    StmtNode* parse_global(GCObject* parent, int depth);
    StmtNode* parse_return(GCObject* parent, int depth);
    StmtNode* parse_del(GCObject* parent, int depth);
    StmtNode* parse_pass(GCObject* parent, int depth);
    StmtNode* parse_break(GCObject* parent, int depth);
    StmtNode* parse_continue(GCObject* parent, int depth);

    // Statement_2
    StmtNode* parse_assign(GCObject* parent, ExprNode* epxr, int depth);
    StmtNode* parse_augassign(GCObject* parent, ExprNode* epxr, int depth);
    StmtNode* parse_annassign(GCObject* parent, ExprNode* epxr, int depth);

    StmtNode* parse_statement(GCObject* parent, int depth) {
        TRACE();

        // Statement we can guess rightaway from the current token we are seeing
        switch (token().type()) {
            // def <name>(...
            case tok_def:       return parse_function_def(parent, depth);

            // async def <name>(...
            case tok_async:     return parse_async_function_def(parent, depth);

            case tok_class:     return parse_class_def(parent, depth);

            // Async for ?
            case tok_for:       return parse_for(parent, depth);

            case tok_while:     return parse_while(parent, depth);
            case tok_if:        return parse_if(parent, depth);

            case tok_match:     return parse_match(parent, depth);
            case tok_with:      return parse_with(parent, depth);
            // Async with ?

            case tok_raise:     return parse_raise(parent, depth);
            case tok_try:       return parse_try(parent, depth);
            case tok_assert:    return parse_assert(parent, depth);

            case tok_import:    return parse_import(parent, depth);
            case tok_from:      return parse_import_from(parent, depth);

            case tok_global:    return parse_global(parent, depth);
            // Nonlocal(identifier* names)

            case tok_return:    return parse_return(parent, depth);
            case tok_del:       return parse_del(parent, depth);
            case tok_pass:      return parse_pass(parent, depth);
            case tok_break:     return parse_break(parent, depth);
            case tok_continue:  return parse_continue(parent, depth);
        }

        auto expr = parse_expression(parent, depth);

        switch (token().type()) {
            // <expr> = <>
            case tok_assign:    return parse_assign(parent, expr, depth);
            // <expr> += <>
            case tok_augassign: return parse_augassign(parent, expr, depth);
            // <expr>: type = <>
            case tok_annassign: return parse_annassign(parent, expr, depth);
        }

        auto stmt_expr = parent->new_object<Expr>();
        stmt_expr->value = expr;
        return stmt_expr;
    }

    // Primary expression
    // parse_expression_1
    ExprNode* parse_await(GCObject* parent, int depth);
    ExprNode* parse_yield(GCObject* parent, int depth);
    ExprNode* parse_yield_from(GCObject* parent, int depth);
    ExprNode* parse_name(GCObject* parent, int depth);
    ExprNode* parse_lambda(GCObject* parent, int depth);
    ExprNode* parse_constant(GCObject* parent, int depth);
    ExprNode* parse_joined_string(GCObject* parent, int depth);
    ExprNode* parse_ifexp(GCObject* parent, int depth);
    ExprNode* parse_starred(GCObject* parent, int depth);
    ExprNode* parse_list(GCObject* parent, int depth);
    ExprNode* parse_tuple_generator(GCObject* parent, int depth);
    ExprNode* parse_set_dict(GCObject* parent, int depth);

    // parse_expression_2
    ExprNode* parse_named_expr(GCObject* parent, ExprNode* primary, int depth);
    ExprNode* parse_bool_operator(GCObject* parent, ExprNode* primary, int depth);
    ExprNode* parse_binary_operator(GCObject* parent, ExprNode* primary, int depth);
    ExprNode* parse_compare_operator(GCObject* parent, ExprNode* primary, int depth);
    ExprNode* parse_unary(GCObject* parent, ExprNode* primary, int depth);
    ExprNode* parse_call(GCObject* parent, ExprNode* primary, int depth);
    ExprNode* parse_attribute(GCObject* parent, ExprNode* primary, int depth);
    ExprNode* parse_subscript(GCObject* parent, ExprNode* primary, int depth);

    ExprNode* parse_expression(GCObject* parent, int depth) {
        auto primary = parse_expression_1(parent, depth);

        //
        switch (token().type()) {
        // <expr> := <expr>
        case tok_walrus:    return parse_named_expr(parent, primary, depth);

        // <expr> boolop <expr>
        case tok_boolop:    return parse_bool_operator(parent, primary, depth);
        case tok_binaryop:  return parse_binary_operator(parent, primary, depth);
        case tok_compareop: return parse_compare_operator(parent, primary, depth);
        case tok_unaryop:   return parse_unary(parent, primary, depth);

        // <expr>(args...)
        case tok_parens:    return parse_call(parent, primary, depth);
        // <expr>.<identifier>
        case tok_dot:       return parse_attribute(parent, primary, depth);
        // <expr>[
        case tok_square:    return parse_subscript(parent, primary, depth);
        }

        return primary;
    }

    // Expression we can guess rightaway from the current token we are seeing
    ExprNode* parse_expression_1(GCObject* parent, int depth) {
        switch (token().type()) {
        // await <expr>
        case tok_await:      return parse_await(parent, depth);
        // yield <expr>
        case tok_yield:      return parse_yield(parent, depth);
        // yield from <expr>
        case tok_yield_from: return parse_yield_from(parent, depth);
        // <identifier>
        case tok_identifier: return parse_name(parent, depth);
        // lambda <name>:
        case tok_lambda:     return parse_lambda(parent, depth);
        case tok_int:        return parse_constant(parent, depth);
        // "
        case tok_string:     return parse_constant(parent, depth);
        // f"
        case tok_fstring:    return parse_joined_string(parent, depth);
        // if <expr> else <expr>
        case tok_if:         return parse_ifexp(parent, depth);
        // *<expr>
        case tok_star:       return parse_starred(parent, depth);

        // List: [a, b]
        // Comprehension [a for a in b]
        case tok_square: return parse_list(parent, depth);

        // Tuple: (a, b)
        // Generator Comprehension: (a for a in b)
        case tok_parens: return parse_tuple_generator(parent, depth);

        // Set: {a, b}
        // Comprehension {a for a in b}
        //      OR
        // Dict: {a : b}
        // Comprehension {a: b for a, b in c}
        case tok_curly: return parse_set_dict(parent, depth);
        }

        // TODO: return a dummy ExprNode
        return nullptr;
    }

    // Shortcuts
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

private:
  AbstractLexer& _lex;
};

} // namespace lython
#endif
