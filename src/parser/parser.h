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

#define TRACE()\
    TRACE_START();\
    auto _ = guard([&](){\
        TRACE_END();\
    });

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

    StmtNode* parse_statement(GCObject* parent) {
        TRACE();

        // Statement we can guess rightaway from the current token we are seeing
        switch (token().type()) {
            // def <name>(...
            case tok_def:       return parse_function_def(parent);

            // async def <name>(...
            case tok_async:     return parse_async_function_def(parent);

            case tok_class:     return parse_class_def(parent);

            // Async for ?
            case tok_for:       return parse_for(parent);

            case tok_while:     return parse_while(parent);
            case tok_if:        return parse_if(parent);

            case tok_with:      return parse_with(parent);
            // Async with ?

            case tok_raise:     return parse_raise(parent);
            case tok_try:       return parse_try(parent);
            case tok_assert:    return parse_assert(parent);

            case tok_import:    return parse_import(parent);
            case tok_from:      return parse_import_from(parent);

            case tok_global:    return parse_global(parent);
            // Nonlocal(identifier* names)

            case tok_return:    return parse_return(parent);
            case tok_del:       return parse_del(parent);
            case tok_pass:      return parse_pass(parent);
            case tok_break:     return parse_break(parent);
            case tok_continue:  return parse_continue(parent):
        }

        auto expr = parse_expression(parent);

        switch (token().type()) {
            // <expr> = <>
            case tok_assign:    return parse_assign(parent, expr);
            // <expr> += <>
            case tok_augassign: return parse_augassign(parent, expr);
            // <expr>: type = <>
            case tok_annassign: return parse_annassign(parent, expr);
        }

        auto stmt_expr = parent->new_object<Expr>();
        stmt_expr->expr = expr;
        return stmt_expr;
    }

    ExprNode* parse_expression(GCObject* parent) {
        auto primary = parse_expression_1(parent);

        switch (token().type()) {
        // <expr> := <expr>
        case tok_walrus: parse_named_expr(parent, primary);

        // <expr> boolop <expr>
        case tok_boolop:
        case tok_binaryop:
        case tok_compareop:

        case tok_unaryop: parse_unary(parent, primary)

        // <expr>(args...)
        case tok_parens: parse_call(parent, primary);
        // <expr>.<identifier>
        case tok_dot: parse_attribute(parent, primary);
        // <expr>[
        case tok_square: parse_subscript(parent, primary);

        }

        return primary;
    }

    ExprNode* parse_subscript(GCObject* parent, ExprNode* primary) {
        //TODO: parse slice
    }

    ExprNode* parse_expression_1(GCObject* parent) {
        // Statement we can guess rightaway from the current token we are seeing
        switch (token().type()) {
        // await <expr>
        case tok_await:      return parse_await(parent);
        // yield <expr>
        case tok_yield:      return parse_yield(parent);
        // yield from <expr>
        case tok_yield_from: return parse_yield(parent);
        // <identifier>
        case tok_identifier: return parse_name(parent);
        // lambda <name>:
        case tok_lambda:     return parse_lambda(parent);
        case tok_int:        return parse_constant(parent);
        // "
        case tok_string:     return parse_constant(parent);
        // f"
        case tok_fstring:    return parse_joined_string(parent);
        // if <expr> else <expr>
        case tok_if:         return parse_ifexp(parent);
        // *<expr>
        case tok_star:       return parse_starred(parent);

        // List: [a, b]
        // Comprehension [a for a in b]
        case tok_square: return parse_list(parent):

        // Tuple: (a, b)
        // Generator Comprehension: (a for a in b)
        case tok_parens: return parse_tuple_generator(parent);

        // Set: {a, b}
        // Comprehension {a for a in b}
        //      OR
        // Dict: {a : b}
        // Comprehension {a: b for a, b in c}
        case tok_curly: return parse_set_dict(parent);
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
