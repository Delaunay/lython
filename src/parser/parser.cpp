#include "parser.h"

namespace lython {


StmtNode* not_implemented_stmt(GCObject* parent) {
    return parent->new_object<NotImplementedStmt>();
}

ExprNode* not_implemented_expr(GCObject* parent) {
    return parent->new_object<NotImplementedExpr>();
}

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
StmtNode* Parser::parse_return(GCObject* parent, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_del(GCObject* parent, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_pass(GCObject* parent, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_break(GCObject* parent, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_continue(GCObject* parent, int depth) { return not_implemented_stmt(parent); }

// Statement_2
StmtNode* Parser::parse_assign(GCObject* parent, ExprNode* epxr, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_augassign(GCObject* parent, ExprNode* epxr, int depth) { return not_implemented_stmt(parent); }
StmtNode* Parser::parse_annassign(GCObject* parent, ExprNode* epxr, int depth) { return not_implemented_stmt(parent); }

// parse_expression_1
ExprNode* Parser::parse_await(GCObject* parent, int depth) { return not_implemented_expr(parent); }
ExprNode* Parser::parse_yield(GCObject* parent, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_yield_from(GCObject* parent, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_name(GCObject* parent, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_lambda(GCObject* parent, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_constant(GCObject* parent, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_joined_string(GCObject* parent, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_ifexp(GCObject* parent, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_starred(GCObject* parent, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_list(GCObject* parent, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_tuple_generator(GCObject* parent, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_set_dict(GCObject* parent, int depth){ return not_implemented_expr(parent); }

// parse_expression_2
ExprNode* Parser::parse_named_expr(GCObject* parent, ExprNode* primary, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_bool_operator(GCObject* parent, ExprNode* primary, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_binary_operator(GCObject* parent, ExprNode* primary, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_compare_operator(GCObject* parent, ExprNode* primary, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_unary(GCObject* parent, ExprNode* primary, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_call(GCObject* parent, ExprNode* primary, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_attribute(GCObject* parent, ExprNode* primary, int depth){ return not_implemented_expr(parent); }

ExprNode* Parser::parse_subscript(GCObject* parent, ExprNode* primary, int depth){
    //TODO: parse slicehere
    return not_implemented_expr(parent);
}

}
