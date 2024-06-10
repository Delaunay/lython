#pragma once

#include "ast/nodes.h"
#include "ast/visitor.h"

namespace lython {

/*
 * Simply walk the AST
 */
template <typename Implementation, bool isConst, typename VisitorTrait, typename... Args>
struct TreeWalk: public BaseVisitor<TreeWalk<Implementation, isConst, VisitorTrait, Args...>, isConst, VisitorTrait, Args...> {
    using Super = BaseVisitor<TreeWalk<Implementation, isConst, VisitorTrait, Args...>, isConst, VisitorTrait, Args...>;

    using StmtRet = typename Super::StmtRet;
    using ExprRet = typename Super::ExprRet;
    using ModRet  = typename Super::ModRet;
    using PatRet  = typename Super::PatRet;


#define TYPE_GEN(rtype, _) \
    using rtype##_t = typename Super::rtype##_t;

    KW_FOREACH_ALL(TYPE_GEN)

#undef TYPE_GEN

    #define KW_REPLACE(node, member, depth, ...)     \
        replace(node, &member, depth, __VA_ARGS__)

    #define KW_COPY(dest, source, ...)\
        copy(&dest, &source, depth, args...)

    template<typename T, typename A>
    bool replace(T* node, A** original, int depth) {
        A* newer = exec(original[0], depth);

        if (original[0] == newer) {
            return false;
        }

        node->remove_child_if_parent(original[0], false);
        original[0] = newer;
        node->add_child(newer);
        return true;
    }

    template<typename T>
    T* copy(T* original, int depth, Args... args) {
        static bool deep_copy = true;
        if (deep_copy) {
            return original->new_object<T>();
        }
        return original;
    }

    template<typename T>
    void copy(T** dest, T const** source, int depth, Args... args) {
        dest[0] = exec(source[0], depth, args...);
    }

    template<typename T>
    void copy(Array<T>* dest, Array<T> const* source, int depth, Args... args) {
        if (dest != source) {
            // Copy array
            dest->reserve(source->size());
            for (int i = 0; i < source->size(); i++) {
                dest->emplace_back(copy((*source)[i], depth, args...));
            }
        } else {
            // modify array
            for (int i = 0; i < source->size(); i++) {
                (*dest)[i] = (copy((*source)[i], depth, args...));
            }
        }
    }

    // Literal
    ExprRet dictexpr(DictExpr_t* n, int depth, Args... args) {
        auto* cpy = copy(n, depth, args...);
        KW_COPY(cpy->keys, n->keys  , depth, args...);
        KW_COPY(cpy->values, n->values, depth, args...);
        return cpy;
    }
    ExprRet setexpr(SetExpr_t* n, int depth, Args... args) {
        auto* cpy = copy(n, depth, args...);
        KW_COPY(cpy->elts, n->elts  , depth, args...);
        return cpy;
    }
    ExprRet listexpr(ListExpr_t* n, int depth, Args... args) {
        auto* cpy = copy(n, depth, args...);
        KW_COPY(cpy->elts, n->elts, depth, args...);
        return cpy;
    }
    ExprRet tupleexpr(TupleExpr_t* n, int depth, Args... args) {
        auto* cpy = copy(n, depth, args...);
        KW_COPY(cpy->elts, n->elts, depth, args...);
        return cpy;
    }
    ExprRet constant(Constant_t* n, int depth, Args... args) { 
        return n; 
    }

    // Comprehension
    ExprRet generateexpr(GeneratorExp_t* n, int depth, Args... args) {
        auto* cpy = copy(n, depth, args...);
        KW_COPY(cpy->generators, n->generators);
        KW_COPY(cpy->elt, n->elt);
        return cpy;
    }
    ExprRet listcomp(ListComp_t* n, int depth, Args... args) {
        auto* cpy = copy(n, depth, args...);
        KW_COPY(cpy->generators, n->generators);
        KW_COPY(cpy->elt, n->elt);
        return cpy;
    }
    ExprRet setcomp(SetComp_t* n, int depth, Args... args) {
        auto* cpy = copy(n, depth, args...);
        KW_COPY(cpy->generators, n->generators);
        KW_COPY(cpy->elt, n->elt);
        return cpy;
    }
    ExprRet dictcomp(DictComp_t* n, int depth, Args... args) {
        auto* cpy = copy(n, depth, args...);
        KW_COPY(cpy->generators, n->generators);
        KW_COPY(cpy->key, n->key);
        KW_COPY(cpy->value, n->value);
        return cpy;
    }

    // Expression
    ExprRet call(Call_t* n, int depth, Args... args) {
        auto* cpy = copy(n, depth, args...);
        KW_COPY(cpy->func, n->func);
        KW_COPY(cpy->args, n->args);
        KW_COPY(cpy->varargs, n->varargs);
        KW_COPY(cpy->keywords, n->keywords);
        return cpy;
    }

    ExprRet namedexpr(NamedExpr_t* n, int depth, Args... args) {
        auto* cpy = copy(n, depth, args...);
        KW_COPY(cpy->target, n->target);
        KW_COPY(cpy->value, n->value);
        return n;
    }
    ExprRet boolop(BoolOp_t* n, int depth, Args... args) {
        for (int i = 0; i < n->values.size(); i++) {
            KW_REPLACE(n, n->values[i], depth, args...);
        }
        return n;
    }

    ExprRet compare(Compare_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->left, depth, args...);
        for (int i = 0; i < n->comparators.size(); i++) {
            KW_REPLACE(n, n->comparators[i], depth, args...);
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
        for (int i = 0; i < n->values.size(); i++) {
            KW_REPLACE(n, n->values[i], depth, args...);
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
        for (int i = 0; i < n->args.size(); i++) {
            KW_REPLACE(n, n->args[i], depth, args...);
        }
        KW_REPLACE(n, n->returns, depth, args...);
        return n;
    }
    ExprRet builtintype(BuiltinType_t* n, int depth, Args... args) { 
        return n; 
    }
    ExprRet tupletype(TupleType_t* n, int depth, Args... args) {
        for (int i = 0; i < n->types.size(); i++) {
            KW_REPLACE(n, n->types[i], depth, args...);
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
        for (int i = 0; i < n->targets.size(); i++) {
            KW_REPLACE(n, n->targets[0], depth, args...);
        }
        return n;
    }
    StmtRet assign(Assign_t* n, int depth, Args... args) {
        for (int i = 0; i < n->targets.size(); i++) {
            KW_REPLACE(n, n->targets[i], depth, args...);
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
        exec_body(n, n->body, depth, args...);
        return n;
    }
    StmtRet functiondef(FunctionDef_t* n, int depth, Args... args) {
        for (int i = 0; i < n->decorator_list.size(); i++) {
            KW_REPLACE(n, n->decorator_list[i].expr, depth, args...);
        }
        for (int i = 0; i < n->args.defaults.size(); i++) {
            KW_REPLACE(n,  n->args.defaults[i], depth, args...);
        }
        for (int i = 0; i < n->args.kw_defaults.size(); i++) {
            KW_REPLACE(n, n->args.kw_defaults[i], depth, args...);
        }
        if (n->returns.has_value()) {
            KW_REPLACE(n, n->returns.value(), depth, args...);
        }
        exec_body(n, n->body, depth, args...);
        return n;
    }
    StmtRet classdef(ClassDef_t* n, int depth, Args... args) {
        for (int i = 0; i < n->decorator_list.size(); i++) {
            KW_REPLACE(n, n->decorator_list[i].expr, depth, args...);
        }

        for (int i = 0; i < n->bases.size(); i++) {
            KW_REPLACE(n, n->bases[i], depth, args...);
        }

        exec_body(n, n->body, depth, args...);
        return n;
    }

    StmtRet forstmt(For_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->target, depth, args...);
        KW_REPLACE(n, n->iter, depth, args...);

        exec_body(n, n->body, depth, args...);
        exec_body(n, n->orelse, depth, args...);

        return n;
    }

    StmtRet whilestmt(While_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->test, depth, args...);
        exec_body(n, n->body, depth, args...);
        exec_body(n, n->orelse, depth, args...);
        return n;
    }

    StmtRet ifstmt(If_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->test, depth, args...);
        exec_body(n, n->body, depth, args...);
        exec_body(n, n->orelse, depth, args...);
        return n;
    }

    StmtRet with(With_t* n, int depth, Args... args) {
        for (WithItem& item: n->items) {
            KW_REPLACE(n, item.context_expr, depth, args...);
            if (item.optional_vars.has_value())
                KW_REPLACE(n, item.optional_vars.value(), depth, args...);
        }

        exec_body(n, n->body, depth, args...);
        return n;
    }
    StmtRet trystmt(Try_t* n, int depth, Args... args) {
        exec_body(n, n->body, depth, args...);

        for (ExceptHandler& handler: n->handlers) {
            if (handler.type.has_value())
                KW_REPLACE(n, handler.type.value(), depth, args...);

            exec_body(n, handler.body[i], depth, args...);
        }

        exec_body(n, n->orelse, depth, args...);
        exec_body(n, n->finalbody, depth, args...);

        return n;
    }

    StmtRet match(Match_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->subject, depth, args...);


        for (int k = 0; k < n->cases.size(); k++) {
            KW_REPLACE(n, n->vases[k].pattern, depth, args...);
            if (n->vases[k].guard.has_value()) {
                KW_REPLACE(n, n->vases[k].guard.value(), depth, args...);
            }
            for (int i =0; i < n->vases[k].body.size(); i++) {
                KW_REPLACE(n, n->vases[k].body[i], depth, args...);
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
        for (int i = 0; i < n->patterns.size(); i++) {
            KW_REPLACE(n, n->patterns[i], depth, args...);
        }
        return n;
    }
    PatRet matchmapping(MatchMapping_t* n, int depth, Args... args) {
        for (int i = 0; i < n->keys.size(); i++) {
            KW_REPLACE(n, n->keys[i], depth, args...);
        }
        for (int i = 0; i < n->patterns.size(); i++) {
            KW_REPLACE(n, n->patterns[i], depth, args...);
        }
        return n;
    }
    PatRet matchclass(MatchClass_t* n, int depth, Args... args) {
        KW_REPLACE(n, n->cls, depth, args...);

        for (int i = 0; i < n->patterns.size(); i++) {
            KW_REPLACE(n, n->patterns[i], depth, args...);
        }

        for (int i = 0; i < n->kwd_patterns.size(); i++) {
            KW_REPLACE(n, n->kwd_patterns[i], depth, args...);
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
        for (int i = 0; i < n->patterns.size(); i++) {
            KW_REPLACE(n, n->patterns[i], depth, args...);
        }
        return n;
    }

    ModRet module(Module_t* n, int depth, Args... args) {
        exec_body(n, n->body, depth, args...);
        return n;
    };
    ModRet interactive(Interactive_t* n, int depth, Args... args) {
        exec_body(n, n->body, depth, args...);
        return n;
    }
    ModRet functiontype(FunctionType_t* n, int depth, Args... args) { 
        return n; 
    }

    ModRet expression(Expression_t* n, int depth, Args... args) { 
        return n; 
    }


    template<typename Owner>
    void exec_body(Owner* n, Array<StmtNode*>& body, int depth, Args... args) {
        body_stack.push_back(body);
        for (int i = 0; i < body.size(); i++) {
            KW_REPLACE(n, body[i], depth, args...);
        }
        body_stack.pop();
    }

    Array<Array<StmtNode*>&> body_stack;
};

}  // namespace lython