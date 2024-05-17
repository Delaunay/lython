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

    // Literal
    ExprRet dictexpr(DictExpr_t* n, int depth, Args... args) {
        for (int i = 0; i < (n->keys).size(); i++) {
            exec(n->keys[i], depth, args...);
            exec(n->values[i], depth, args...);
        }
        return ExprRet();
    }
    ExprRet setexpr(SetExpr_t* n, int depth, Args... args) {
        for (int i = 0; i < (n->elts).size(); i++) {
            exec(n->elts[i], depth, args...);
        }
        return ExprRet();
    }
    ExprRet listexpr(ListExpr_t* n, int depth, Args... args) {
        for (int i = 0; i < (n->elts).size(); i++) {
            exec(n->elts[i], depth, args...);
        }
        return ExprRet();
    }
    ExprRet tupleexpr(TupleExpr_t* n, int depth, Args... args) {
        for (int i = 0; i < n->elts.size(); i++) {
            exec(n->elts[i], depth, args...);
        }
        return ExprRet();
    }
    ExprRet constant(Constant_t* n, int depth, Args... args) { return ExprRet(); }

    // Comprehension
    ExprRet generateexpr(GeneratorExp_t* n, int depth, Args... args) {
        for (Comprehension& comp: n->generators) {
            exec(comp.iter, depth, args...);
            exec(comp.target, depth, args...);

            for (ExprNode* cond: comp.ifs) {
                exec(cond, depth, args...);
            }
        }
        exec(n->elt, depth, args...);
        return ExprRet();
    }
    ExprRet listcomp(ListComp_t* n, int depth, Args... args) {
        for (Comprehension& comp: n->generators) {
            exec(comp.iter, depth, args...);
            exec(comp.target, depth, args...);

            for (ExprNode* cond: comp.ifs) {
                exec(cond, depth, args...);
            }
        }
        exec(n->elt, depth, args...);
        return ExprRet();
    }
    ExprRet setcomp(SetComp_t* n, int depth, Args... args) {
        for (Comprehension& comp: n->generators) {
            exec(comp.iter, depth, args...);
            exec(comp.target, depth, args...);

            for (ExprNode* cond: comp.ifs) {
                exec(cond, depth, args...);
            }
        }
        exec(n->elt, depth, args...);
        return ExprRet();
    }
    ExprRet dictcomp(DictComp_t* n, int depth, Args... args) {
        for (Comprehension& comp: n->generators) {
            exec(comp.iter, depth, args...);
            exec(comp.target, depth, args...);

            for (ExprNode* cond: comp.ifs) {
                exec(cond, depth, args...);
            }
        }
        exec(n->key, depth, args...);
        exec(n->value, depth, args...);
        return ExprRet();
    }

    // Expression
    ExprRet call(Call_t* n, int depth, Args... args) {
        exec(n->func, depth, args...);
        for (auto* arg: n->args) {
            exec(arg, depth, args...);
        }
        return ExprRet();
    }

    ExprRet namedexpr(NamedExpr_t* n, int depth, Args... args) {
        exec(n->target, depth, args...);
        exec(n->value, depth, args...);
        return ExprRet();
    }
    ExprRet boolop(BoolOp_t* n, int depth, Args... args) {
        for (auto* expr: n->values) {
            exec(expr, depth, args...);
        }
        return ExprRet();
    }

    ExprRet compare(Compare_t* n, int depth, Args... args) {
        exec(n->left, depth, args...);
        for (auto* expr: n->comparators) {
            exec(expr, depth, args...);
        }
        return ExprRet();
    }
    ExprRet binop(BinOp_t* n, int depth, Args... args) {
        exec(n->left, depth, args...);
        exec(n->right, depth, args...);
        return ExprRet();
    }
    ExprRet unaryop(UnaryOp_t* n, int depth, Args... args) {
        exec(n->operand, depth, args...);
        return ExprRet();
    }
    ExprRet lambda(Lambda_t* n, int depth, Args... args) {
        exec(n->body, depth, args...);
        return ExprRet();
    }
    ExprRet ifexp(IfExp_t* n, int depth, Args... args) {
        exec(n->test, depth, args...);
        exec(n->body, depth, args...);
        exec(n->orelse, depth, args...);
        return ExprRet();
    }

    //
    ExprRet await(Await_t* n, int depth, Args... args) {
        exec(n->value, depth, args...);
        return ExprRet();
    }
    ExprRet yield(Yield_t* n, int depth, Args... args) {
        if (n->value.has_value()) {
            exec(n->value.value(), depth, args...);
        }
        return ExprRet();
    }
    ExprRet yieldfrom(YieldFrom_t* n, int depth, Args... args) {
        exec(n->value, depth, args...);
        return ExprRet();
    }
    ExprRet joinedstr(JoinedStr_t* n, int depth, Args... args) {
        for (ExprNode* expr: n->values) {
            exec(expr, depth, args...);
        }
        return ExprRet();
    }
    ExprRet formattedvalue(FormattedValue_t* n, int depth, Args... args) {
        exec(n->value, depth, args...);
        exec(n->format_spec, depth, args...);
        return ExprRet();
    }
    ExprRet attribute(Attribute_t* n, int depth, Args... args) {
        exec(n->value, depth, args...);
        return ExprRet();
    }
    ExprRet subscript(Subscript_t* n, int depth, Args... args) {
        exec(n->value, depth, args...);
        exec(n->slice, depth, args...);
        return ExprRet();
    }
    ExprRet starred(Starred_t* n, int depth, Args... args) {
        exec(n->value, depth, args...);
        return ExprRet();
    }
    ExprRet slice(Slice_t* n, int depth, Args... args) {
        if (n->lower.has_value()) {
            exec(n->lower.value(), depth, args...);
        }

        if (n->upper.has_value()) {
            exec(n->upper.value(), depth, args...);
        }

        if (n->step.has_value()) {
            exec(n->step.value(), depth, args...);
        }

        return ExprRet();
    }

    ExprRet name(Name_t* n, int depth, Args... args) { 
        return ExprRet(); 
    }

    // Types
    ExprRet dicttype(DictType_t* n, int depth, Args... args) {
        exec(n->key, depth, args...);
        exec(n->value, depth, args...);
        return ExprRet();
    }
    ExprRet arraytype(ArrayType_t* n, int depth, Args... args) {
        exec(n->value, depth, args...);
        return ExprRet();
    }
    ExprRet arrow(Arrow_t* n, int depth, Args... args) {
        for (ExprNode* arg: n->args) {
            exec(arg, depth, args...);
        }
        exec(n->returns, depth, args...);
        return ExprRet();
    }
    ExprRet builtintype(BuiltinType_t* n, int depth, Args... args) { return ExprRet(); }
    ExprRet tupletype(TupleType_t* n, int depth, Args... args) {
        for (ExprNode* type: n->types) {
            exec(type, depth, args...);
        }
        return ExprRet();
    }
    ExprRet settype(SetType_t* n, int depth, Args... args) {
        exec(n->value, depth, args...);
        return ExprRet();
    }
    ExprRet classtype(ClassType_t* n, int depth, Args... args) { return ExprRet(); }
    ExprRet comment(Comment_t* n, int depth, Args... args) { return ExprRet(); }

    // JUMP

    // Leaves
    StmtRet invalidstmt(InvalidStatement_t* n, int depth, Args... args) {
        kwerror(outlog(), "Invalid statement");
        return StmtRet();
    }
    StmtRet returnstmt(Return_t* n, int depth, Args... args) {
        if (n->value.has_value())
            exec(n->value.value(), depth, args...);
        return StmtRet();
    }
    StmtRet deletestmt(Delete_t* n, int depth, Args... args) {
        for (ExprNode* target: n->targets) {
            exec(target, depth, args...);
        }
        return StmtRet();
    }
    StmtRet assign(Assign_t* n, int depth, Args... args) {
        for (ExprNode* target: n->targets) {
            exec(target, depth, args...);
        }
        exec(n->value, depth, args...);
        return StmtRet();
    }
    StmtRet augassign(AugAssign_t* n, int depth, Args... args) {
        exec(n->target, depth, args...);
        exec(n->value, depth, args...);
        return StmtRet();
    }
    StmtRet annassign(AnnAssign_t* n, int depth, Args... args) {
        exec(n->target, depth, args...);
        exec(n->annotation, depth, args...);
        if (n->value.has_value()) {
            exec(n->value.value(), depth, args...);
        }
        return StmtRet();
    }
    StmtRet exprstmt(Expr_t* n, int depth, Args... args) {
        exec(n->value, depth, args...);
        return StmtRet();
    }
    StmtRet pass(Pass_t* n, int depth, Args... args) { return StmtRet(); }
    StmtRet breakstmt(Break_t* n, int depth, Args... args) { return StmtRet(); }
    StmtRet continuestmt(Continue_t* n, int depth, Args... args) { return StmtRet(); }
    StmtRet assertstmt(Assert_t* n, int depth, Args... args) {
        exec(n->test, depth, args...);
        if (n->msg.has_value()) {
            exec(n->msg.value(), depth, args...);
        }
        return StmtRet();
    }
    StmtRet raise(Raise_t* n, int depth, Args... args) {
        if (n->exc.has_value()) {
            exec(n->exc.value(), depth, args...);
        }
        if (n->cause.has_value()) {
            exec(n->cause.value(), depth, args...);
        }
        return StmtRet();
    }
    StmtRet global(Global_t* n, int depth, Args... args) { return StmtRet(); }
    StmtRet nonlocal(Nonlocal_t* n, int depth, Args... args) { return StmtRet(); }

    StmtRet import(Import_t* n, int depth, Args... args) { return StmtRet(); }
    StmtRet importfrom(ImportFrom_t* n, int depth, Args... args) { return StmtRet(); }

    StmtRet inlinestmt(Inline_t* n, int depth, Args... args) {
        for (StmtNode* stmt: n->body) {
            exec(stmt, depth, args...);
        }
        return StmtRet();
    }
    StmtRet functiondef(FunctionDef_t* n, int depth, Args... args) {
        for (Decorator& decorator: n->decorator_list) {
            exec(decorator.expr, depth, args...);
        }
        for (ExprNode* defaultarg: n->args.defaults) {
            exec(defaultarg, depth, args...);
        }
        for (ExprNode* defaultarg: n->args.kw_defaults) {
            exec(defaultarg, depth, args...);
        }
        if (n->returns.has_value()) {
            exec(n->returns.value(), depth, args...);
        }
        for (StmtNode* stmt: n->body) {
            exec(stmt, depth, args...);
        }
        return StmtRet();
    }
    StmtRet classdef(ClassDef_t* n, int depth, Args... args) {
        for (Decorator& decorator: n->decorator_list) {
            exec(decorator.expr, depth, args...);
        }

        for (ExprNode* base: n->bases) {
            exec(base, depth, args...);
        }

        for (StmtNode* stmt: n->body) {
            exec(stmt, depth, args...);
        }
        return StmtRet();
    }

    StmtRet forstmt(For_t* n, int depth, Args... args) {
        exec(n->target, depth, args...);
        exec(n->iter, depth, args...);
        for (StmtNode* stmt: n->body) {
            exec(stmt, depth, args...);
        }
        for (StmtNode* stmt: n->orelse) {
            exec(stmt, depth, args...);
        }
        return StmtRet();
    }

    StmtRet whilestmt(While_t* n, int depth, Args... args) {
        exec(n->test, depth, args...);

        for (StmtNode* stmt: n->body) {
            exec(stmt, depth, args...);
        }
        for (StmtNode* stmt: n->orelse) {
            exec(stmt, depth, args...);
        }
        return StmtRet();
    }

    StmtRet ifstmt(If_t* n, int depth, Args... args) {
        exec(n->test, depth, args...);

        for (StmtNode* stmt: n->body) {
            exec(stmt, depth, args...);
        }
        for (StmtNode* stmt: n->orelse) {
            exec(stmt, depth, args...);
        }
        return StmtRet();
    }

    StmtRet with(With_t* n, int depth, Args... args) {
        for (WithItem& item: n->items) {
            exec(item.context_expr, depth, args...);
            if (item.optional_vars.has_value())
                exec(item.optional_vars.value(), depth, args...);
        }

        for (StmtNode* stmt: n->body) {
            exec(stmt, depth, args...);
        }
        return StmtRet();
    }
    StmtRet trystmt(Try_t* n, int depth, Args... args) {
        for (StmtNode* stmt: n->body) {
            exec(stmt, depth, args...);
        }
        for (ExceptHandler& handler: n->handlers) {
            if (handler.type.has_value())
                exec(handler.type.value(), depth, args...);

            for (StmtNode* stmt: handler.body) {
                exec(stmt, depth, args...);
            }
        }
        for (StmtNode* stmt: n->orelse) {
            exec(stmt, depth, args...);
        }
        for (StmtNode* stmt: n->finalbody) {
            exec(stmt, depth, args...);
        }
        return StmtRet();
    }

    StmtRet match(Match_t* n, int depth, Args... args) {
        exec(n->subject, depth, args...);

        for (MatchCase& branch: n->cases) {
            exec(branch.pattern, depth, args...);
            if (branch.guard.has_value()) {
                exec(branch.guard.value(), depth, args...);
            }
            for (StmtNode* stmt: branch.body) {
                exec(stmt, depth, args...);
            }
        }
        return StmtRet();
    }

    PatRet matchvalue(MatchValue_t* n, int depth, Args... args) {
        exec(n->value, depth, args...);
        return PatRet();
    }
    PatRet matchsingleton(MatchSingleton_t* n, int depth, Args... args) { return PatRet(); }
    PatRet matchsequence(MatchSequence_t* n, int depth, Args... args) {
        for (Pattern* pat: n->patterns) {
            exec(pat, depth, args...);
        }
        return PatRet();
    }
    PatRet matchmapping(MatchMapping_t* n, int depth, Args... args) {
        for (ExprNode* key: n->keys) {
            exec(key, depth, args...);
        }
        for (Pattern* pat: n->patterns) {
            exec(pat, depth, args...);
        }
        return PatRet();
    }
    PatRet matchclass(MatchClass_t* n, int depth, Args... args) {
        exec(n->cls, depth, args...);
        for (Pattern* key: n->patterns) {
            exec(key, depth, args...);
        }
        for (Pattern* key: n->kwd_patterns) {
            exec(key, depth, args...);
        }
        return PatRet();
    }
    PatRet matchstar(MatchStar_t* n, int depth, Args... args) { return PatRet(); }
    PatRet matchas(MatchAs_t* n, int depth, Args... args) {
        if (n->pattern.has_value())
            exec(n->pattern.value(), depth, args...);
        return PatRet();
    }
    PatRet matchor(MatchOr_t* n, int depth, Args... args) {
        for (Pattern* key: n->patterns) {
            exec(key, depth, args...);
        }
        return PatRet();
    }

    ModRet module(Module_t* n, int depth, Args... args) {
        for (StmtNode* stmt: n->body) {
            exec(stmt, depth, args...);
        }
        return ModRet();
    };
    ModRet interactive(Interactive_t* n, int depth, Args... args) {
        for (StmtNode* stmt: n->body) {
            exec(stmt, depth, args...);
        }
        return ModRet();
    }
    ModRet functiontype(FunctionType_t* n, int depth, Args... args) { return ModRet(); }

    ModRet expression(Expression_t* n, int depth, Args... args) { return ModRet(); }
};

}  // namespace lython