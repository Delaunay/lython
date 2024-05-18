#pragma once

#include "ast/nodes.h"
#include "ast/visitor.h"

namespace lython {

/*
 * Simply walk the AST
 */
template <typename Implementation, bool isConst, typename VisitorTrait, typename... Args>
struct TreeWalk: public BaseVisitor<TreeWalk<Implementation>, isConst, VisitorTrait, Args...> {
    using Super = BaseVisitor<TreeWalk<Implementation>, isConst, VisitorTrait, Args...>;

    using StmtRet = Super::StmtRet;
    using ExprRet = Super::ExprRet;
    using ModRet  = Super::ModRet;
    using PatRet  = Super::PatRet;


    #define KW_REPLACE(node, member, depth, ...)     \
        relplace(node, &node->member, depth, __VA_ARGS__)

    template<typename T, typename A>
    bool replace(T* node, A** original, int depth) {
        A* newer = KW_REPLACE(n, original[0], depth);

        if (original[0] == newer) {
            return false;
        }

        node->remove_child_if_parent(original[0], dofree);
        original[0] = newer;
        node->add_child(newer);
        return true;
    }

    // Literal
    ExprRet dictexpr(DictExpr_t* n, int depth, Args... args) {
        for (int i = 0; i < (n->keys).size(); i++) {
            KW_REPLACE(n, n->keys[i], depth, args...);
            KW_REPLACE(n, n->values[i], depth, args...);

            // KW_REPLACE(n, n->keys[i], depth, args...);
            // KW_REPLACE(n, n->values[i], depth, args...);
        }
        return n;
    }
    ExprRet setexpr(SetExpr_t* n, int depth, Args... args) {
        for (int i = 0; i < (n->elts).size(); i++) {
            KW_REPLACE(n, n->elts[i], depth, args...);
        }
        return n;
    }
    ExprRet listexpr(ListExpr_t* n, int depth, Args... args) {
        for (int i = 0; i < (n->elts).size(); i++) {
            KW_REPLACE(n, n->elts[i], depth, args...);
        }
        return n;
    }
    ExprRet tupleexpr(TupleExpr_t* n, int depth, Args... args) {
        for (int i = 0; i < n->elts.size(); i++) {
            KW_REPLACE(n, n->elts[i], depth, args...);
        }
        return n;
    }
    ExprRet constant(Constant_t* n, int depth, Args... args) { 
        return n; 
    }

    // Comprehension
    ExprRet generateexpr(GeneratorExp_t* n, int depth, Args... args) {
        for (Comprehension& comp: n->generators) {
            KW_REPLACE(n, comp.iter, depth, args...);
            KW_REPLACE(n, comp.target, depth, args...);

            for (ExprNode* cond: comp.ifs) {
                KW_REPLACE(n, cond, depth, args...);
            }
        }
        KW_REPLACE(n, n->elt, depth, args...);
        return n;
    }
    ExprRet listcomp(ListComp_t* n, int depth, Args... args) {
        for (Comprehension& comp: n->generators) {
            KW_REPLACE(n, comp.iter, depth, args...);
            KW_REPLACE(n, comp.target, depth, args...);

            for (ExprNode* cond: comp.ifs) {
                KW_REPLACE(n, cond, depth, args...);
            }
        }
        KW_REPLACE(n, n->elt, depth, args...);
        return n;
    }
    ExprRet setcomp(SetComp_t* n, int depth, Args... args) {
        for (Comprehension& comp: n->generators) {
            KW_REPLACE(n, comp.iter, depth, args...);
            KW_REPLACE(n, comp.target, depth, args...);

            for (ExprNode* cond: comp.ifs) {
                KW_REPLACE(n, cond, depth, args...);
            }
        }
        KW_REPLACE(n, n->elt, depth, args...);
        return n;
    }
    ExprRet dictcomp(DictComp_t* n, int depth, Args... args) {
        for (Comprehension& comp: n->generators) {
            KW_REPLACE(n, comp.iter, depth, args...);
            KW_REPLACE(n, comp.target, depth, args...);

            for (ExprNode* cond: comp.ifs) {
                KW_REPLACE(n, cond, depth, args...);
            }
        }
        KW_REPLACE(n, n->key, depth, args...);
        KW_REPLACE(n, n->value, depth, args...);
        return n;
    }

    // Expression
    ExprRet call(Call_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->func, depth, args...);
        for (auto* arg: n->args) {
            KW_REPLACE(n, arg, depth, args...);
        }
        return n;
    }

    ExprRet namedexpr(NamedExpr_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->target, depth, args...);
        KW_REPLACE(n, n->value, depth, args...);
        return n;
    }
    ExprRet boolop(BoolOp_t* n, int depth, Args... args) {
        for (auto* expr: n->values) {
            KW_REPLACE(n, expr, depth, args...);
        }
        return n;
    }

    ExprRet compare(Compare_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->left, depth, args...);
        for (auto* expr: n->comparators) {
            KW_REPLACE(n, expr, depth, args...);
        }
        return n;
    }
    ExprRet binop(BinOp_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->left, depth, args...);
        KW_REPLACE(n, n->right, depth, args...);
        return n;
    }
    ExprRet unaryop(UnaryOp_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->operand, depth, args...);
        return n;
    }
    ExprRet lambda(Lambda_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->body, depth, args...);
        return n;
    }
    ExprRet ifexp(IfExp_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->test, depth, args...);
        KW_REPLACE(n, n->body, depth, args...);
        KW_REPLACE(n, n->orelse, depth, args...);
        return n;
    }

    //
    ExprRet await(Await_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->value, depth, args...);
        return n;
    }
    ExprRet yield(Yield_t* n, int depth, Args... args) {
        if (n->value.has_value()) {
            KW_REPLACE(n, n->value.value(), depth, args...);
        }
        return n;
    }
    ExprRet yieldfrom(YieldFrom_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->value, depth, args...);
        return n;
    }
    ExprRet joinedstr(JoinedStr_t* n, int depth, Args... args) {
        for (ExprNode* expr: n->values) {
            KW_REPLACE(n, expr, depth, args...);
        }
        return n;
    }
    ExprRet formattedvalue(FormattedValue_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->value, depth, args...);
        KW_REPLACE(n, n->format_spec, depth, args...);
        return n;
    }
    ExprRet attribute(Attribute_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->value, depth, args...);
        return n;
    }
    ExprRet subscript(Subscript_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->value, depth, args...);
        KW_REPLACE(n, n->slice, depth, args...);
        return n;
    }
    ExprRet starred(Starred_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->value, depth, args...);
        return n;
    }
    ExprRet slice(Slice_t* n, int depth, Args... args) {
        if (n->lower.has_value()) {
            KW_REPLACE(n, n->lower.value(), depth, args...);
        }

        if (n->upper.has_value()) {
            KW_REPLACE(n, n->upper.value(), depth, args...);
        }

        if (n->step.has_value()) {
            KW_REPLACE(n, n->step.value(), depth, args...);
        }

        return n;
    }

    ExprRet name(Name_t* n, int depth, Args... args) { 
        return n; 
    }

    // Types
    ExprRet dicttype(DictType_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->key, depth, args...);
        KW_REPLACE(n, n->value, depth, args...);
        return n;
    }
    ExprRet arraytype(ArrayType_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->value, depth, args...);
        return n;
    }
    ExprRet arrow(Arrow_t* n, int depth, Args... args) {
        for (ExprNode* arg: n->args) {
            KW_REPLACE(n, arg, depth, args...);
        }
        KW_REPLACE(n, n->returns, depth, args...);
        return n;
    }
    ExprRet builtintype(BuiltinType_t* n, int depth, Args... args) { 
        return n; 
    }
    ExprRet tupletype(TupleType_t* n, int depth, Args... args) {
        for (ExprNode* type: n->types) {
            KW_REPLACE(n, type, depth, args...);
        }
        return n;
    }
    ExprRet settype(SetType_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->value, depth, args...);
        return n;
    }
    ExprRet classtype(ClassType_t* n, int depth, Args... args) { 
        return n; 
    }
    ExprRet comment(Comment_t* n, int depth, Args... args) { 
        return n; 
    }

    // JUMP

    // Leaves
    StmtRet invalidstmt(InvalidStatement_t* n, int depth, Args... args) {
        kwerror(outlog(), "Invalid statement");
        return n;
    }
    StmtRet returnstmt(Return_t* n, int depth, Args... args) {
        if (n->value.has_value())
            KW_REPLACE(n, n->value.value(), depth, args...);
        return n;
    }
    StmtRet deletestmt(Delete_t* n, int depth, Args... args) {
        for (ExprNode* target: n->targets) {
            KW_REPLACE(n, target, depth, args...);
        }
        return n;
    }
    StmtRet assign(Assign_t* n, int depth, Args... args) {
        for (ExprNode* target: n->targets) {
            KW_REPLACE(n, target, depth, args...);
        }
        KW_REPLACE(n, n->value, depth, args...);
        return n;
    }
    StmtRet augassign(AugAssign_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->target, depth, args...);
        KW_REPLACE(n, n->value, depth, args...);
        return n;
    }
    StmtRet annassign(AnnAssign_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->target, depth, args...);
        KW_REPLACE(n, n->annotation, depth, args...);
        if (n->value.has_value()) {
            KW_REPLACE(n, n->value.value(), depth, args...);
        }
        return n;
    }
    StmtRet exprstmt(Expr_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->value, depth, args...);
        return n;
    }
    StmtRet pass(Pass_t* n, int depth, Args... args) { 
        return n; 
    }
    StmtRet breakstmt(Break_t* n, int depth, Args... args) { 
        return n; 
    }
    StmtRet continuestmt(Continue_t* n, int depth, Args... args) { 
        return n; 
    }
    StmtRet assertstmt(Assert_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->test, depth, args...);
        if (n->msg.has_value()) {
            KW_REPLACE(n, n->msg.value(), depth, args...);
        }
        return n;
    }
    StmtRet raise(Raise_t* n, int depth, Args... args) {
        if (n->exc.has_value()) {
            KW_REPLACE(n, n->exc.value(), depth, args...);
        }
        if (n->cause.has_value()) {
            KW_REPLACE(n, n->cause.value(), depth, args...);
        }
        return n;
    }
    StmtRet global(Global_t* n, int depth, Args... args) { 
        return n; 
    }
    StmtRet nonlocal(Nonlocal_t* n, int depth, Args... args) { 
        return n; 
    }
    StmtRet import(Import_t* n, int depth, Args... args) { 
        return n; 
    }
    StmtRet importfrom(ImportFrom_t* n, int depth, Args... args) { 
        return n; 
    }

    StmtRet inlinestmt(Inline_t* n, int depth, Args... args) {
        for (StmtNode* stmt: n->body) {
            KW_REPLACE(n, stmt, depth, args...);
        }
        return n;
    }
    StmtRet functiondef(FunctionDef_t* n, int depth, Args... args) {
        for (Decorator& decorator: n->decorator_list) {
            KW_REPLACE(n, decorator.expr, depth, args...);
        }
        for (ExprNode* defaultarg: n->args.defaults) {
            KW_REPLACE(n, defaultarg, depth, args...);
        }
        for (ExprNode* defaultarg: n->args.kw_defaults) {
            KW_REPLACE(n, defaultarg, depth, args...);
        }
        if (n->returns.has_value()) {
            KW_REPLACE(n, n->returns.value(), depth, args...);
        }
        for (StmtNode* stmt: n->body) {
            KW_REPLACE(n, stmt, depth, args...);
        }
        return n;
    }
    StmtRet classdef(ClassDef_t* n, int depth, Args... args) {
        for (Decorator& decorator: n->decorator_list) {
            KW_REPLACE(n, decorator.expr, depth, args...);
        }

        for (ExprNode* base: n->bases) {
            KW_REPLACE(n, base, depth, args...);
        }

        for (StmtNode* stmt: n->body) {
            KW_REPLACE(n, stmt, depth, args...);
        }
        return n;
    }

    StmtRet forstmt(For_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->target, depth, args...);
        KW_REPLACE(n, n->iter, depth, args...);
        for (StmtNode* stmt: n->body) {
            KW_REPLACE(n, stmt, depth, args...);
        }
        for (StmtNode* stmt: n->orelse) {
            KW_REPLACE(n, stmt, depth, args...);
        }
        return n;
    }

    StmtRet whilestmt(While_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->test, depth, args...);

        for (StmtNode* stmt: n->body) {
            KW_REPLACE(n, stmt, depth, args...);
        }
        for (StmtNode* stmt: n->orelse) {
            KW_REPLACE(n, stmt, depth, args...);
        }
        return n;
    }

    StmtRet ifstmt(If_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->test, depth, args...);

        for (StmtNode* stmt: n->body) {
            KW_REPLACE(n, stmt, depth, args...);
        }
        for (StmtNode* stmt: n->orelse) {
            KW_REPLACE(n, stmt, depth, args...);
        }
        return n;
    }

    StmtRet with(With_t* n, int depth, Args... args) {
        for (WithItem& item: n->items) {
            KW_REPLACE(n, item.context_expr, depth, args...);
            if (item.optional_vars.has_value())
                KW_REPLACE(n, item.optional_vars.value(), depth, args...);
        }

        for (StmtNode* stmt: n->body) {
            KW_REPLACE(n, stmt, depth, args...);
        }
        return n;
    }
    StmtRet trystmt(Try_t* n, int depth, Args... args) {
        for (StmtNode* stmt: n->body) {
            KW_REPLACE(n, stmt, depth, args...);
        }
        for (ExceptHandler& handler: n->handlers) {
            if (handler.type.has_value())
                KW_REPLACE(n, handler.type.value(), depth, args...);

            for (StmtNode* stmt: handler.body) {
                KW_REPLACE(n, stmt, depth, args...);
            }
        }
        for (StmtNode* stmt: n->orelse) {
            KW_REPLACE(n, stmt, depth, args...);
        }
        for (StmtNode* stmt: n->finalbody) {
            KW_REPLACE(n, stmt, depth, args...);
        }
        return n;
    }

    StmtRet match(Match_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->subject, depth, args...);

        for (MatchCase& branch: n->cases) {
            KW_REPLACE(n, branch.pattern, depth, args...);
            if (branch.guard.has_value()) {
                KW_REPLACE(n, branch.guard.value(), depth, args...);
            }
            for (StmtNode* stmt: branch.body) {
                KW_REPLACE(n, stmt, depth, args...);
            }
        }
        return n;
    }

    PatRet matchvalue(MatchValue_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->value, depth, args...);
        return n;
    }
    PatRet matchsingleton(MatchSingleton_t* n, int depth, Args... args) { 
        return n; 
    }
    PatRet matchsequence(MatchSequence_t* n, int depth, Args... args) {
        for (Pattern* pat: n->patterns) {
            KW_REPLACE(n, pat, depth, args...);
        }
        return n;
    }
    PatRet matchmapping(MatchMapping_t* n, int depth, Args... args) {
        for (ExprNode* key: n->keys) {
            KW_REPLACE(n, key, depth, args...);
        }
        for (Pattern* pat: n->patterns) {
            KW_REPLACE(n, pat, depth, args...);
        }
        return n;
    }
    PatRet matchclass(MatchClass_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->cls, depth, args...);
        for (Pattern* key: n->patterns) {
            KW_REPLACE(n, key, depth, args...);
        }
        for (Pattern* key: n->kwd_patterns) {
            KW_REPLACE(n, key, depth, args...);
        }
        return n;
    }
    PatRet matchstar(MatchStar_t* n, int depth, Args... args) { 
        return n; 
    }
    PatRet matchas(MatchAs_t* n, int depth, Args... args) {
        if (n->pattern.has_value())
            KW_REPLACE(n, n->pattern.value(), depth, args...);
        return n;
    }
    PatRet matchor(MatchOr_t* n, int depth, Args... args) {
        for (Pattern* key: n->patterns) {
            KW_REPLACE(n, key, depth, args...);
        }
        return n;
    }

    ModRet module(Module_t* n, int depth, Args... args) {
        for (StmtNode* stmt: n->body) {
            KW_REPLACE(n, stmt, depth, args...);
        }
        return n;
    };
    ModRet interactive(Interactive_t* n, int depth, Args... args) {
        for (StmtNode* stmt: n->body) {
            KW_REPLACE(n, stmt, depth, args...);
        }
        return n;
    }
    ModRet functiontype(FunctionType_t* n, int depth, Args... args) { 
        return n; 
    }

    ModRet expression(Expression_t* n, int depth, Args... args) { 
        return n; 
    }
};

}  // namespace lython