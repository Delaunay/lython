#pragma once

#include "ast/nodes.h"
#include "ast/visitor.h"

namespace lython {

/*
 * Simply walk the AST
 */
template <typename Implementation, bool isConst, typename VisitorTrait, typename... Args>
struct TreeWalk: public BaseVisitor<TreeWalk<Implementation, isConst, VisitorTrait, Args...>,
                                    isConst,
                                    VisitorTrait,
                                    Args...> {
    using Super   = BaseVisitor<TreeWalk<Implementation, isConst, VisitorTrait, Args...>,
                              isConst,
                              VisitorTrait,
                              Args...>;
    using StmtRet = typename Super::StmtRet;
    using ExprRet = typename Super::ExprRet;
    using ModRet  = typename Super::ModRet;
    using PatRet  = typename Super::PatRet;

    Array<GCObject*>         parents;
    Array<Array<StmtNode*>*> body_stack;

    template <typename T>
    struct ScopedOwner {
        ScopedOwner(T* owner, TreeWalk* self = nullptr): self(self), owner(owner) {
            if (self) {
                self->parents.push_back(owner);
            }
        }

        ~ScopedOwner() {
            if (self) {
                self->parents.pop_back();
            }
        }

        T* operator->() {
            kwassert(owner != nullptr, "Should be not null");
            return owner;
        }

        operator T*() { return owner; }

        TreeWalk* self;
        T*        owner;
    };

#define TYPE_GEN(rtype, _) \
    using rtype##_t = typename Super::rtype##_t; \
    using rtype##_st = ScopedOwner<typename Super::rtype##_t>;

    KW_FOREACH_ALL(TYPE_GEN)

#undef TYPE_GEN

#define KW_REPLACE(node, member, depth, ...) replace(node, member, depth, __VA_ARGS__)

#define KW_COPY(dest, source, ...) copy(dest, source, depth, args...)

    template <typename T, typename A>
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

    GCObject* get_arena() { return (*parents.rbegin()); }

    template <typename A>
    ScopedOwner<A> new_from(A* original, int depth, Args... args) {
        if (parents.size() == 0) {
            return ScopedOwner<A>(new A(args...), this);    
        }
        A* obj = get_arena()->template new_object<A>();
        return ScopedOwner<A>(obj, this);
    }

    template <typename A>
    ScopedOwner<A> new_object() {
        A* obj = get_arena()->template new_object<A>();
        return ScopedOwner<A>(obj, this);
    }

    template <typename T>
    ScopedOwner<T> copy_node(T* original, int depth, Args... args) {
        static bool deep_copy = true;
        if (deep_copy) {
            auto* cpy = Super::exec(original, depth, args...);

            if (original != nullptr) {
                kwassert(cpy != nullptr, "Should have copied");
            }

            if (cpy) { 
                if (cpy->kind == original->kind) {
                    return ScopedOwner<T>((T*)(cpy), this);    
                }
            }
            return ScopedOwner<T>(nullptr, this);
        }
        return ScopedOwner<T>(original, nullptr);
    }

    template <typename T>
    void copy(T*& dest, T*& source, int depth, Args... args) {
        T* cpy = copy_node(source, depth, args...);
        dest = cpy;
    }

    template <typename T>
    void copy(Optional<T>& dest, Optional<T>& source, int depth, Args... args) {
        if (source.has_value()) {
            copy(dest.storage(), source.value(), depth, args...);
        }
    }

    // Arguments copy(Arguments& src, int depth, Args... args) { return src; }
    void copy(Arguments& dest, Arguments& src, int depth, Args... args) {
        KW_COPY(dest.posonlyargs, src.posonlyargs);
        KW_COPY(dest.args, src.args);
        KW_COPY(dest.vararg, src.vararg);
        KW_COPY(dest.kwonlyargs, src.kwonlyargs);
        KW_COPY(dest.kw_defaults, src.kw_defaults);
        KW_COPY(dest.kwarg, src.kwarg);
        KW_COPY(dest.defaults, src.defaults);
        return;
    }

    void copy(CmpOperator& dest, CmpOperator& src, int depth, Args... args) { dest = src; }
    void copy(StringRef& dest, StringRef src, int depth, Args... args) { 
        dest = src; 
    }
    void copy(int& dest, int& src, int depth, Args... args) { 
        dest = src; 
    }
    void copy(Arg& dest, Arg& src, int depth, Args... args) {
        KW_COPY(dest.arg, src.arg);
        KW_COPY(dest.annotation, src.annotation);
    }
    void copy(Comprehension& dest, Comprehension& src, int depth, Args... args) {
        KW_COPY(dest.target, src.target);
        KW_COPY(dest.iter, src.iter);
        KW_COPY(dest.ifs, src.ifs);
        dest.is_async = src.is_async;
    }
    void copy(Keyword& dest, Keyword& src, int depth, Args... args) {
        KW_COPY(dest.arg, src.arg);
        KW_COPY(dest.value, src.value);
    }
    void copy(Decorator& dest, Decorator& src, int depth, Args... args) {
        KW_COPY(dest.expr, src.expr);
    }
    void copy(WithItem& dest, WithItem& src, int depth, Args... args) {
        KW_COPY(dest.context_expr, src.context_expr);
        KW_COPY(dest.optional_vars, src.optional_vars);
    }
    void copy(ExceptHandler& dest, ExceptHandler& src, int depth, Args... args) {
        KW_COPY(dest.type, src.type);
        KW_COPY(dest.name, src.name);
        KW_COPY(dest.body, src.body);
    }
    void copy(Alias& dest, Alias& src, int depth, Args... args) {
        KW_COPY(dest.name, src.name);
        KW_COPY(dest.asname, src.asname);
    }
    void copy(MatchCase& dest, MatchCase& src, int depth, Args... args) {
        KW_COPY(dest.pattern, src.pattern);
        KW_COPY(dest.guard, src.guard);
        KW_COPY(dest.body, src.body);
    }

    void body_append(StmtNode* stmt) { (*body_stack.rbegin())->push_back(stmt); }

    template <typename T>
    void copy_item(Array<T*>& dest, int j, Array<T*>& source, int depth, Args... args) {
        T* tmp = copy_node(source[j], depth, args...);

        // create space AFTER the copy
        int i = int(dest.size());
        dest.push_back(nullptr);
        dest[i] = tmp;
    }

    template <typename T>
    void copy_item(Array<T>& dest, int j, Array<T>& source, int depth, Args... args) {
        T tmp;
        copy(tmp, source[j], depth, args...);

        // create space AFTER the copy
        int i = int(dest.size());
        dest.push_back(T());
        dest[i] = tmp;
    }

    template <typename T>
    void copy(Array<T>& dest, Array<T>& source, int depth, Args... args) {
        if constexpr (std::is_same_v<T, StmtNode*>) {
            body_stack.push_back(&dest);
        }
        // std::cout << (void*)(&dest) << " " << (void*)(&source) << "\n";
        // std::cout << dest.size() << " " << source.size() << std::endl;
        // This is UB
        if ((void*)(&dest) != (void*)(&source)) {
            // Copy array
            // this cannot work because SSA will insert more stuff in it
            dest.reserve(source.size());
            
            for (int i = 0; i < source.size(); i++) {
                copy_item(dest, i, source, depth, args...);
            }
        } else {
            // modify array
            for (int i = 0; i < source.size(); i++) {
                copy_item(dest, i, source, depth, args...);
            }
        }
        if constexpr (std::is_same_v<T, StmtNode*>) {
            body_stack.pop_back();
        }
    }

    // Literal
    virtual ExprRet dictexpr(DictExpr_t* n, int depth, Args... args) {
        DictExpr_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->keys, n->keys);
        KW_COPY(cpy->values, n->values);
        return cpy;
    }
    virtual ExprRet setexpr(SetExpr_t* n, int depth, Args... args) {
        SetExpr_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->elts, n->elts);
        return cpy;
    }
    virtual ExprRet listexpr(ListExpr_t* n, int depth, Args... args) {
        ListExpr_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->elts, n->elts);
        return cpy;
    }
    virtual ExprRet tupleexpr(TupleExpr_t* n, int depth, Args... args) {
        TupleExpr_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->elts, n->elts);
        return cpy;
    }
    virtual ExprRet constant(Constant_t* n, int depth, Args... args) { return n; }

    // Comprehension
    virtual ExprRet generateexpr(GeneratorExp_t* n, int depth, Args... args) {
        GeneratorExp_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->generators, n->generators);
        KW_COPY(cpy->elt, n->elt);
        return cpy;
    }
    virtual ExprRet listcomp(ListComp_t* n, int depth, Args... args) {
        ListComp_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->generators, n->generators);
        KW_COPY(cpy->elt, n->elt);
        return cpy;
    }
    virtual ExprRet setcomp(SetComp_t* n, int depth, Args... args) {
        SetComp_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->generators, n->generators);
        KW_COPY(cpy->elt, n->elt);
        return cpy;
    }
    virtual ExprRet dictcomp(DictComp_t* n, int depth, Args... args) {
        DictComp_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->generators, n->generators);
        KW_COPY(cpy->key, n->key);
        KW_COPY(cpy->value, n->value);
        return cpy;
    }

    // Expression
    virtual ExprRet call(Call_t* n, int depth, Args... args) {
        Call_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->func, n->func);
        KW_COPY(cpy->args, n->args);
        KW_COPY(cpy->varargs, n->varargs);
        KW_COPY(cpy->keywords, n->keywords);
        return cpy;
    }

    virtual ExprRet namedexpr(NamedExpr_t* n, int depth, Args... args) {
        NamedExpr_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->target, n->target);
        KW_COPY(cpy->value, n->value);
        return cpy;
    }
    virtual ExprRet boolop(BoolOp_t* n, int depth, Args... args) {
        BoolOp_st cpy = new_from(n, depth, args...);
        cpy->op       = n->op;
        KW_COPY(cpy->values, n->values);
        return cpy;
    }

    virtual ExprRet compare(Compare_t* n, int depth, Args... args) {
        Compare_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->left, n->left);
        KW_COPY(cpy->ops, n->ops);
        KW_COPY(cpy->comparators, n->comparators);
        return cpy;
    }
    virtual ExprRet binop(BinOp_t* n, int depth, Args... args) {
        BinOp_st cpy = new_from(n, depth, args...);
        cpy->op = n->op;
        KW_COPY(cpy->left, n->left);
        KW_COPY(cpy->right, n->right);
        return cpy;
    }
    virtual ExprRet unaryop(UnaryOp_t* n, int depth, Args... args) {
        UnaryOp_st cpy = new_from(n, depth, args...);
        cpy->op = n->op;
        KW_COPY(cpy->operand, n->operand);
        return cpy;
    }
    virtual ExprRet lambda(Lambda_t* n, int depth, Args... args) {
        Lambda_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->body, n->body);
        return cpy;
    }
    virtual ExprRet ifexp(IfExp_t* n, int depth, Args... args) {
        IfExp_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->test, n->test);
        KW_COPY(cpy->body, n->body);
        KW_COPY(cpy->orelse, n->orelse);
        return cpy;
    }

    //
    virtual ExprRet await(Await_t* n, int depth, Args... args) {
        Await_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->value, n->value);
        return cpy;
    }
    virtual ExprRet yield(Yield_t* n, int depth, Args... args) {
        Yield_st cpy = new_from(n, depth, args...);
        KW_COPY(n->value, n->value);
        return cpy;
    }
    virtual ExprRet yieldfrom(YieldFrom_t* n, int depth, Args... args) {
        YieldFrom_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->value, n->value);
        return cpy;
    }
    virtual ExprRet joinedstr(JoinedStr_t* n, int depth, Args... args) {
        JoinedStr_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->values, n->values);
        return cpy;
    }
    virtual ExprRet formattedvalue(FormattedValue_t* n, int depth, Args... args) {
        FormattedValue_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->value, n->value);
        KW_COPY(cpy->format_spec, n->format_spec);
        return cpy;
    }
    virtual ExprRet attribute(Attribute_t* n, int depth, Args... args) {
        Attribute_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->value, n->value);
        return cpy;
    }
    virtual ExprRet subscript(Subscript_t* n, int depth, Args... args) {
        Subscript_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->value, n->value);
        KW_COPY(cpy->slice, n->slice);
        return cpy;
    }
    virtual ExprRet starred(Starred_t* n, int depth, Args... args) {
        Starred_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->value, n->value);
        return cpy;
    }
    virtual ExprRet slice(Slice_t* n, int depth, Args... args) {
        Slice_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->lower, n->lower);
        KW_COPY(cpy->upper, n->upper);
        KW_COPY(cpy->step, n->step);
        return cpy;
    }

    virtual ExprRet name(Name_t* n, int depth, Args... args) {
        Name_st cpy = new_from(n, depth, args...);
        cpy->id     = n->id;
        cpy->ctx    = n->ctx;
        KW_COPY(cpy->type, n->type);
        return cpy;
    }

    // Types
    virtual ExprRet dicttype(DictType_t* n, int depth, Args... args) {
        DictType_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->key, n->key);
        KW_COPY(cpy->value, n->value);
        return cpy;
    }
    virtual ExprRet arraytype(ArrayType_t* n, int depth, Args... args) {
        ArrayType_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->value, n->value);
        return cpy;
    }
    virtual ExprRet arrow(Arrow_t* n, int depth, Args... args) {
        Arrow_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->args, n->args);
        KW_COPY(cpy->returns, n->returns);
        return cpy;
    }
    virtual ExprRet builtintype(BuiltinType_t* n, int depth, Args... args) {
        BuiltinType_st cpy = new_from(n, depth, args...);
        cpy->name          = n->name;
        return cpy;
    }
    virtual ExprRet tupletype(TupleType_t* n, int depth, Args... args) {
        TupleType_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->types, n->types);
        return cpy;
    }
    virtual ExprRet settype(SetType_t* n, int depth, Args... args) {
        SetType_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->value, n->value);
        return cpy;
    }
    virtual ExprRet classtype(ClassType_t* n, int depth, Args... args) {
        ClassType_st cpy = copy_node(n, depth, args...);
        return cpy;
    }
    virtual ExprRet comment(Comment_t* n, int depth, Args... args) {
        Comment_st cpy = new_from(n, depth, args...);
        return cpy;
    }

    // JUMP

    // Leaves
    virtual StmtRet invalidstmt(InvalidStatement_t* n, int depth, Args... args) {
        kwerror(outlog(), "Invalid statement");
        return n;
    }
    virtual StmtRet returnstmt(Return_t* n, int depth, Args... args) {
        Return_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->value, n->value);
        return cpy;
    }
    virtual StmtRet deletestmt(Delete_t* n, int depth, Args... args) {
        Delete_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->targets, n->targets);
        return cpy;
    }
    virtual StmtRet assign(Assign_t* n, int depth, Args... args) {
        Assign_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->targets, n->targets);
        KW_COPY(cpy->value, n->value);
        return cpy;
    }
    virtual StmtRet augassign(AugAssign_t* n, int depth, Args... args) {
        AugAssign_st cpy = new_from(n, depth, args...);
        cpy->op = n->op;
        KW_COPY(cpy->target, n->target);
        KW_COPY(cpy->value, n->value);
        return cpy;
    }
    virtual StmtRet annassign(AnnAssign_t* n, int depth, Args... args) {
        AnnAssign_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->target, n->target);
        KW_COPY(cpy->annotation, n->annotation);
        KW_COPY(cpy->value, n->value);
        return cpy;
    }
    virtual StmtRet exprstmt(Expr_t* n, int depth, Args... args) {
        Expr_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->value, n->value);
        return cpy;
    }
    virtual StmtRet pass(Pass_t* n, int depth, Args... args) {
        Pass_st cpy = new_from(n, depth, args...);
        return cpy;
    }
    virtual StmtRet breakstmt(Break_t* n, int depth, Args... args) {
        Break_st cpy = new_from(n, depth, args...);
        return cpy;
    }
    virtual StmtRet continuestmt(Continue_t* n, int depth, Args... args) {
        Continue_st cpy = new_from(n, depth, args...);
        return cpy;
    }
    virtual StmtRet assertstmt(Assert_t* n, int depth, Args... args) {
        Assert_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->test, n->test);
        KW_COPY(cpy->msg, n->msg);
        return cpy;
    }
    virtual StmtRet raise(Raise_t* n, int depth, Args... args) {
        Raise_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->exc, n->exc);
        KW_COPY(cpy->cause, n->cause);
        return cpy;
    }
    virtual StmtRet global(Global_t* n, int depth, Args... args) {
        Global_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->names, n->names);
        return cpy;
    }
    virtual StmtRet nonlocal(Nonlocal_t* n, int depth, Args... args) {
        Nonlocal_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->names, n->names);
        return cpy;
    }
    virtual StmtRet import(Import_t* n, int depth, Args... args) {
        Import_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->names, n->names);
        return cpy;
    }
    virtual StmtRet importfrom(ImportFrom_t* n, int depth, Args... args) {
        ImportFrom_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->module, n->module);
        KW_COPY(cpy->names, n->names);
        KW_COPY(cpy->level, n->level);
        return cpy;
    }

    virtual StmtRet inlinestmt(Inline_t* n, int depth, Args... args) {
        Inline_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->body, n->body);
        return cpy;
    }
    virtual StmtRet functiondef(FunctionDef_t* n, int depth, Args... args) {
        FunctionDef_st cpy = new_from(n, depth, args...);
        cpy->name = n->name;
        KW_COPY(cpy->decorator_list, n->decorator_list);
        KW_COPY(cpy->args, n->args);
        KW_COPY(cpy->returns, n->returns);
        KW_COPY(cpy->body, n->body);
        return cpy;
    }
    virtual StmtRet classdef(ClassDef_t* n, int depth, Args... args) {
        ClassDef_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->decorator_list, n->decorator_list);
        KW_COPY(cpy->bases, n->bases);
        KW_COPY(cpy->body, n->body);
        return cpy;
    }

    virtual StmtRet forstmt(For_t* n, int depth, Args... args) {
        For_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->target, n->target);
        KW_COPY(cpy->iter, n->iter);
        KW_COPY(cpy->body, n->body);
        KW_COPY(cpy->orelse, n->orelse);
        return cpy;
    }

    virtual StmtRet whilestmt(While_t* n, int depth, Args... args) {
        While_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->test, n->test);
        KW_COPY(cpy->body, n->body);
        KW_COPY(cpy->orelse, n->orelse);
        return cpy;
    }

    virtual StmtRet ifstmt(If_t* n, int depth, Args... args) {
        If_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->test, n->test);
        KW_COPY(cpy->body, n->body);
        KW_COPY(cpy->orelse, n->orelse);
        return cpy;
    }

    virtual StmtRet with(With_t* n, int depth, Args... args) {
        With_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->items, n->items);
        KW_COPY(cpy->body, n->body);
        return cpy;
    }
    virtual StmtRet trystmt(Try_t* n, int depth, Args... args) {
        Try_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->body, n->body);
        KW_COPY(cpy->handlers, n->handlers);
        KW_COPY(cpy->orelse, n->orelse);
        KW_COPY(cpy->finalbody, n->finalbody);
        return cpy;
    }

    virtual StmtRet match(Match_t* n, int depth, Args... args) {
        Match_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->subject, n->subject);
        KW_COPY(cpy->cases, n->cases);
        return cpy;
    }

    virtual PatRet matchvalue(MatchValue_t* n, int depth, Args... args) {
        MatchValue_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->value, n->value);
        return cpy;
    }
    virtual PatRet matchsingleton(MatchSingleton_t* n, int depth, Args... args) {
        MatchSingleton_st cpy = new_from(n, depth, args...);
        return cpy;
    }
    virtual PatRet matchsequence(MatchSequence_t* n, int depth, Args... args) {
        MatchSequence_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->patterns, n->patterns);
        return cpy;
    }
    virtual PatRet matchmapping(MatchMapping_t* n, int depth, Args... args) {
        MatchMapping_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->keys, n->keys);
        KW_COPY(cpy->patterns, n->patterns);
        return cpy;
    }
    virtual PatRet matchclass(MatchClass_t* n, int depth, Args... args) {
        MatchClass_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->cls, n->cls);
        KW_COPY(cpy->patterns, n->patterns);
        KW_COPY(cpy->kwd_patterns, n->kwd_patterns);
        return cpy;
    }
    virtual PatRet matchstar(MatchStar_t* n, int depth, Args... args) {
        MatchStar_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->name, n->name);
        return cpy;
    }
    virtual PatRet matchas(MatchAs_t* n, int depth, Args... args) {
        MatchAs_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->pattern, n->pattern);
        KW_COPY(cpy->name, n->name);
        return cpy;
    }
    virtual PatRet matchor(MatchOr_t* n, int depth, Args... args) {
        MatchOr_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->patterns, n->patterns);
        return cpy;
    }

    virtual ModRet module(Module_t* n, int depth, Args... args) {
        Module_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->body, n->body);
        return cpy;
    };
    virtual ModRet interactive(Interactive_t* n, int depth, Args... args) {
        Interactive_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->body, n->body);
        return cpy;
    }
    virtual ModRet functiontype(FunctionType_t* n, int depth, Args... args) {
        FunctionType_st cpy = new_from(n, depth, args...);
        return cpy;
    }

    virtual ModRet expression(Expression_t* n, int depth, Args... args) {
        Expression_st cpy = new_from(n, depth, args...);
        KW_COPY(cpy->body, n->body);
        return cpy;
    }
};

}  // namespace lython