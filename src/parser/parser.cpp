#include "parser.h"

namespace lython {


StmtNode* not_implemented_stmt(GCObject* parent) {
    return parent->new_object<NotImplementedStmt>();
}

ExprNode* not_implemented_expr(GCObject* parent) {
    return parent->new_object<NotImplementedExpr>();
}

void Parser::start_code_loc(CommonAttributes* target, Token tok) {}
void Parser::end_code_loc(CommonAttributes* target, Token tok) {}

// Statement_1
StmtNode* Parser::parse_function_def(GCObject* parent, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_async_function_def(GCObject* parent, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_class_def(GCObject* parent, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_for(GCObject* parent, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_while(GCObject* parent, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_if(GCObject* parent, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_match(GCObject* parent, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_with(GCObject* parent, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_raise(GCObject* parent, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_try(GCObject* parent, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_assert(GCObject* parent, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_import(GCObject* parent, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_import_from(GCObject* parent, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_global(GCObject* parent, int depth) { return not_implemented_stmt(parent); }

StmtNode* Parser::parse_return(GCObject* parent, int depth) {
     auto stmt = parent->new_object<Return>();
     start_code_loc(stmt, token());
     next_token();

     if (token().type() != tok_newline)  {
        stmt->value = parse_expression(stmt, depth + 1);
        end_code_loc(stmt, token());
     } else {
         end_code_loc(stmt, token());
         next_token();
     }

     return stmt;
}

StmtNode* Parser::parse_del(GCObject* parent, int depth) {
    auto stmt = parent->new_object<Delete>();
    start_code_loc(stmt, token());
    next_token();

    while (token().type() != tok_newline) {
        auto expr = parse_expression(stmt, depth + 1);
        stmt->targets.push_back(expr);
    }

    end_code_loc(stmt, token());
    next_token();
    return stmt;
}

StmtNode* Parser::parse_pass(GCObject* parent, int depth) {
    auto stmt = parent->new_object<Pass>();
    start_code_loc(stmt, token());
    end_code_loc(stmt, token());
    next_token();
    return stmt;
}

StmtNode* Parser::parse_break(GCObject* parent, int depth) {
    auto stmt = parent->new_object<Break>();
    start_code_loc(stmt, token());
    end_code_loc(stmt, token());
    next_token();
    return stmt;
}

StmtNode* Parser::parse_continue(GCObject* parent, int depth) {
    auto stmt = parent->new_object<Continue>();
    start_code_loc(stmt, token());
    end_code_loc(stmt, token());
    next_token();
    return stmt;
}

// Statement_2
StmtNode* Parser::parse_assign(GCObject* parent, ExprNode* expr, int depth) {
    auto stmt = parent->new_object<Assign>();
    stmt->targets.push_back(expr);

    // FIXME: this is the location of '=' not the start of the full expression
    start_code_loc(stmt, token());
    next_token();

    stmt->value = parse_expression(stmt, depth + 1);

    end_code_loc(stmt, token());
    return stmt;
}

StmtNode* Parser::parse_augassign(GCObject* parent, ExprNode* expr, int depth) {
    auto stmt = parent->new_object<AugAssign>();
    stmt->target = expr;

    // FIXME: this is the location of the operator not the start of the full expression
    start_code_loc(stmt, token());
    // stmt->op = get_binary_operator(token());
    next_token();

    stmt->value = parse_expression(stmt, depth + 1);
    end_code_loc(stmt, token());
    return stmt;
}

StmtNode* Parser::parse_annassign(GCObject* parent, ExprNode* expr, int depth) {
    auto stmt = parent->new_object<AnnAssign>();
    stmt->target = expr;

    // FIXME: this is the location of :' not the start of the full expression
    start_code_loc(stmt, token());
    next_token();

    stmt->annotation = parse_expression(stmt, depth + 1);

    // EXPECT_ASSIGN
    next_token();
    stmt->value = parse_expression(stmt, depth + 1);

    end_code_loc(stmt, token());
    return stmt;
}

// parse_expression_1
ExprNode* Parser::parse_name(GCObject* parent, int depth) {
    auto expr = parent->new_object<Name>();
    start_code_loc(expr, token());
    next_token();

    // EXPECT_IDENTIFIER;
    // expr->ctx = ;

    expr->id = token().identifier();
    end_code_loc(expr, token());
    next_token();
    return expr;
}
ExprNode* Parser::parse_constant(GCObject* parent, int depth){
    auto expr = parent->new_object<Constant>();
    start_code_loc(expr, token());

    switch (token().type()){
    case tok_string:{
        expr->value = token().identifier();
    }
    case tok_int:{
         expr->value = token().as_integer();
    }
    case tok_float:{
        expr->value = token().as_float();
    }
    }

    end_code_loc(expr, token());
    next_token();
    return expr;
}

ExprNode* Parser::parse_await(GCObject* parent, int depth) {
    auto expr = parent->new_object<Await>();
    start_code_loc(expr, token());
    next_token();

    expr->value = parse_expression(expr, depth + 1);
    end_code_loc(expr, token());
    return expr;
}

ExprNode* Parser::parse_yield(GCObject* parent, int depth){
    auto expr = parent->new_object<Yield>();
    start_code_loc(expr, token());
    next_token();

    if (token().type() != tok_newline)  {
        expr->value = parse_expression(expr, depth + 1);
        end_code_loc(expr, token());
    } else {
        next_token();
        end_code_loc(expr, token());
    }
    return expr;
}

ExprNode* Parser::parse_yield_from(GCObject* parent, int depth){
    auto expr = parent->new_object<YieldFrom>();
    start_code_loc(expr, token());
    next_token();

    expr->value = parse_expression(expr, depth + 1);
    end_code_loc(expr, token());
    return expr;
}

Arguments Parser::parse_arguments(char kind) {
    return Arguments();
}

ExprNode* Parser::parse_lambda(GCObject* parent, int depth){
    auto expr = parent->new_object<Lambda>();
    start_code_loc(expr, token());
    expr->args = parse_arguments(':');
    expr->body = parse_expression(expr, depth + 1);
    end_code_loc(expr, token());
    return expr;
}

ExprNode* Parser::parse_joined_string(GCObject* parent, int depth){ return not_implemented_expr(parent); }

ExprNode* Parser::parse_ifexp(GCObject* parent, int depth){
    auto expr = parent->new_object<IfExp>();
    start_code_loc(expr, token());

    expr->test = parse_expression(expr, depth + 1);
    // EXPECT_COLON
    next_token();

    expr->body = parse_expression(expr, depth + 1);

    // EXPECT_ELSE;
    next_token();

    expr->orelse = parse_expression(expr, depth + 1);

    end_code_loc(expr, token());
    return expr;
}

ExprNode* Parser::parse_starred(GCObject* parent, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_list(GCObject* parent, int depth){ return not_implemented_expr(parent); }

ExprNode* Parser::parse_tuple_generator(GCObject* parent, int depth){
    // EXPECT_PARENS // (
    // Save the start token to set the code loc when we know if this is a tuple or a generator
    auto start_tok = token();
    next_token();

    // TODO: Warning here the parent is wrong but we need to parse the expression right now
    // we have to implement some reparenting logic
    auto child = parse_expression(parent, depth + 1);

    if (token().type() == tok_for) {
        // This is generator comprehension
        auto expr = parent->new_object<GeneratorExp>();
        start_code_loc(expr, start_tok);

        // parent->remove(child)
        // expr->add_child(child)

        expr->elt = child;
        expr->generators = parse_comprehension(expr, ')', depth);

        end_code_loc(expr, token());
        return expr;
    } else {
        // This is a tuple
        auto expr = parent->new_object<TupleExpr>();
        start_code_loc(expr, start_tok);

        // parent->remove(child)
        // expr->add_child(child)

        expr->elts.push_back(child);

        while (token().type() != ')') {
            expr->elts.push_back(parse_expression(parent, depth + 1));
        }

        end_code_loc(expr, token());
        return expr;
    }
}
ExprNode* Parser::parse_set_dict(GCObject* parent, int depth){ return not_implemented_expr(parent); }

// parse_expression_2
ExprNode* Parser::parse_named_expr(GCObject* parent, ExprNode* primary, int depth){ return not_implemented_expr(parent); }

ExprNode* Parser::parse_bool_operator(GCObject* parent, ExprNode* primary, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_binary_operator(GCObject* parent, ExprNode* primary, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_compare_operator(GCObject* parent, ExprNode* primary, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_unary(GCObject* parent, ExprNode* primary, int depth){ return not_implemented_expr(parent); }

ExprNode* Parser::parse_call(GCObject* parent, ExprNode* primary, int depth){
    auto expr = parent->new_object<Call>();
    start_code_loc(expr, token());
    next_token();

    expr->func = primary;

    // EXPECT_PARENS; // (

    // FIXME: this is the location of '(' not the start of the full expression
    start_code_loc(expr, token());
    next_token();

    bool keyword = false;
    while (token().type() != ')') {
        // if not in keyword mode check if next argument is one
        if (keyword == false) {
            auto lookahead = _lex.peek_token();
            if (token().type() == tok_identifier && lookahead.type() == tok_assign) {
                keyword = true;
            }
        }

        if (!keyword) {
            auto arg = parse_expression(expr, depth + 1);
            expr->args.push_back(arg);
        } else {
            // EXPECT_IDENTIFIER;
            auto kwarg = Keyword();
            kwarg.arg = token().identifier();
            kwarg.value = parse_expression(expr, depth + 1);
            expr->keywords.push_back(kwarg);
        }

        if (token().type() == ',') {
            next_token();
        }
    }

    // EXPECT_END_PARENS; // )
    end_code_loc(expr, token());
    next_token();
    return expr;
}

ExprNode* Parser::parse_attribute(GCObject* parent, ExprNode* primary, int depth) {
    auto expr = parent->new_object<Attribute>();
    expr->value = primary;

    // FIXME: this is the location of '.' not the start of the full expression
    start_code_loc(expr, token());
    next_token();

    // EXPECT_IDENTFIER();
    expr->attr = token().identifier();

    // expr->ctx = ;

    return expr;
}

ExprNode* Parser::parse_subscript(GCObject* parent, ExprNode* primary, int depth){
    auto expr = parent->new_object<Subscript>();
    expr->value = primary;

    // FIXME: this is the location of '[' not the start of the full expression
    start_code_loc(expr, token());
    next_token();

    expr->slice = parse_slice(expr, depth + 1);

    // expr->ctx = ;
    end_code_loc(expr, token());
    return expr;
}

ExprNode* Parser::parse_slice(GCObject* parent, int depth) {
    return not_implemented_expr(parent);
}

}
