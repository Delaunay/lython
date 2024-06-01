#include "ast/ops.h"
#include "ast/visitor.h"
#include "logging/logging.h"

#include "dependencies/formatter.h"

namespace lython {

// Equality Visitor
struct Equality {
    using Return_t = bool;

    template <typename T>
    bool exec(T const& a, T const& b, int depth) {
        return a == b;
    }

    bool exec(Exported const& a, Exported const& b) { return exec(a.node, b.node); }

    bool exec(MatchCase const& a, MatchCase const& b, int depth) {
        kwtrace(outlog(), depth, "MatchCase");
        return exec(a.pattern, b.pattern, depth) && exec(a.guard, b.guard, depth) &&
               exec(a.body, b.body, depth);
    }

    bool exec(ExceptHandler const& a, ExceptHandler const& b, int depth) {
        return exec(a.type, b.type, depth) && exec(a.name, b.name, depth) &&
               exec(a.body, b.body, depth);
    }

    bool exec(JoinedStr const& a, JoinedStr const& b, int depth) {
        return exec(a.values, b.values, depth);
    }

    bool exec(WithItem const& a, WithItem const& b, int depth) {
        return exec(a.context_expr, b.context_expr, depth) &&
               exec(a.optional_vars, b.optional_vars, depth);
    }

    bool exec(Comprehension const& a, Comprehension const& b, int depth) {
        return exec(a.target, b.target, depth) && exec(a.iter, b.iter, depth) &&
               exec(a.ifs, b.ifs, depth);
    }

    bool exec(Alias const& a, Alias const& b, int depth) {
        return a.asname == b.asname && a.name == b.name;
    }

    bool exec(Arg const& a, Arg const& b, int depth) {
        return a.arg == b.arg && exec(a.annotation, b.annotation, depth);
    }

    bool exec(Keyword const& a, Keyword const& b, int depth) {
        return a.arg == b.arg && exec(a.value, b.value, depth);
    }

    bool exec(Arguments const& a, Arguments const& b, int depth) {
        return exec(a.posonlyargs, b.posonlyargs, depth) && exec(a.args, b.args, depth) &&
               exec(a.vararg, b.vararg, depth) && exec(a.kwonlyargs, b.kwonlyargs, depth) &&
               exec(a.kw_defaults, b.kw_defaults, depth) && exec(a.kwarg, b.kwarg, depth) &&
               exec(a.defaults, b.defaults, depth);
    }

    template <typename T>
    bool exec(Optional<T> const& a, Optional<T> const& b, int depth) {
        if (a.has_value() == b.has_value()) {
            if (a.has_value()) {
                return exec(a.value(), b.value(), depth);
            }
            return true;
        }

        return false;
    }

    template <typename T>
    bool exec(Array<T> const& a, Array<T> const& b, int depth) {
        if (a.size() != b.size()) {
            return false;
        }
        for (int i = 0; i < a.size(); i++) {
            if (!exec(a[i], b[i], depth)) {
                return false;
            }
        }
        return true;
    }

    bool exec(Node* a, Node* b) {
        if (a->kind != b->kind) {
            return false;
        }
        // clang-format off
        switch (a->family()) {
        case NodeFamily::Module:
            return exec(reinterpret_cast<ModNode *>(a), reinterpret_cast<ModNode *>(b), 0);
        case NodeFamily::Statement:
            return exec(reinterpret_cast<StmtNode *>(a), reinterpret_cast<StmtNode *>(b), 0);
        case NodeFamily::Expression:
            return exec(reinterpret_cast<ExprNode *>(a), reinterpret_cast<ExprNode *>(b), 0);
        case NodeFamily::Pattern:
            return exec(reinterpret_cast<Pattern *>(a), reinterpret_cast<Pattern *>(b), 0);
        case NodeFamily::VM:
            return false;
        }
        // clang-format on
        return false;
    }

    bool exec(ModNode* a, ModNode* b, int depth) {
        kwtrace(outlog(), depth, "{}", str(a->kind));

        if (a->kind != b->kind) {
            return false;
        }
        // clang-format off
        switch (a->kind) {
            #define X(name, _)
            #define PASS(_a, _b)
            #define SECTION(_)
            #define MOD(name, fun)\
                case NodeKind::name: {\
                    name* aa = reinterpret_cast<name*>(a);\
                    name* bb = reinterpret_cast<name*>(b);\
                    return fun(aa, bb, depth + 1);\
                }

            NODEKIND_ENUM(X, SECTION, PASS, PASS, MOD, PASS, PASS)

            #undef X
            #undef PASS
            #undef SECTION
            #undef MOD

            default:
                return false;

        }
        // clang-format on
        return false;
    }

    bool exec(Pattern* a, Pattern* b, int depth) {
        kwtrace(outlog(), depth, "{}", str(a->kind));

        if (a->kind != b->kind) {
            return false;
        }
        // clang-format off
        switch (a->kind) {
            #define X(name, _)
            #define PASS(_a, _b)
            #define SECTION(_)
            #define MATCH(name, fun)\
                case NodeKind::name: {\
                    name* aa = reinterpret_cast<name*>(a);\
                    name* bb = reinterpret_cast<name*>(b);\
                    return fun(aa, bb, depth + 1);\
                }

            NODEKIND_ENUM(X, SECTION, PASS, PASS, PASS, MATCH, PASS)

            #undef X
            #undef PASS
            #undef SECTION
            #undef MATCH

            default:
                return false;
        }
        // clang-format on
        return false;
    }

    bool exec(ExprNode* a, ExprNode* b, int depth) {
        if (a == nullptr && b == nullptr) {
            return true;
        }
        if (a == nullptr || b == nullptr) {
            return false;
        }
        kwtrace(outlog(), depth, "{}", str(a->kind));

        if (a->kind != b->kind) {
            return false;
        }
        // clang-format off
        switch (a->kind) {
            #define X(name, _)
            #define PASS(_a, _b)
            #define SECTION(_)
            #define EXPR(name, fun)\
                case NodeKind::name: {\
                    name* aa = reinterpret_cast<name*>(a);\
                    name* bb = reinterpret_cast<name*>(b);\
                    return fun(aa, bb, depth + 1);\
                }

            NODEKIND_ENUM(X, SECTION, EXPR, PASS, PASS, PASS, PASS)

            #undef X
            #undef PASS
            #undef SECTION
            #undef EXPR

            default:
                return false;
        }
        // clang-format on
        return false;
    }

    bool exec(StmtNode* a, StmtNode* b, int depth) {
        kwtrace(outlog(), depth, "{}", str(a->kind));

        if (a->kind != b->kind) {
            return false;
        }
        // clang-format off
        switch (a->kind) {
            #define X(name, _)
            #define PASS(_a, _b)
            #define SECTION(_)
            #define STMT(name, fun)\
                case NodeKind::name: {\
                    name* aa = reinterpret_cast<name*>(a);\
                    name* bb = reinterpret_cast<name*>(b);\
                    return this->fun(aa, bb, depth + 1);\
                }

            NODEKIND_ENUM(X, SECTION, PASS, STMT, PASS, PASS, PASS)

            #undef X
            #undef PASS
            #undef SECTION
            #undef STMT

            default:
                return false;
        }
        // clang-format on
        return false;
    }

    // >>>>>>>>>>>> Types
    bool functiontype(FunctionType* a, FunctionType* b, int depth) {
        if (a->argtypes.size() != b->argtypes.size()) {
            return false;
        }

        for (int i = 0; i < a->argtypes.size(); i++) {
            auto a_arg = a->argtypes[i];
            auto b_arg = b->argtypes[i];

            if (!exec(a_arg, b_arg, depth)) {
                return false;
            }
        }

        return exec(a->returns, b->returns, depth);
    }

    bool comment(Comment* a, Comment* b, int depth) { return true; }

    bool dicttype(DictType* a, DictType* b, int depth) {
        return exec(a->key, b->key, depth) && exec(a->value, b->value, depth);
    }
    bool arraytype(ArrayType* a, ArrayType* b, int depth) {
        return exec(a->value, b->value, depth);
    }
    bool settype(SetType* a, SetType* b, int depth) { return exec(a->value, b->value, depth); }
    bool tupletype(TupleType* a, TupleType* b, int depth) {
        return exec(a->types, b->types, depth);
    }

    bool arrow(Arrow* a, Arrow* b, int depth) {
        return exec(a->args, b->args, depth) && exec(a->returns, b->returns, depth);
    }

    bool builtintype(BuiltinType* a, BuiltinType* b, int depth) {
        kwdebug(outlog(), "{} {} {}", a->name, b->name, exec(a->name, b->name, depth));
        return exec(a->name, b->name, depth);
    }
    bool functiondef(FunctionDef* a, FunctionDef* b, int depth) {
        // TODO check full module path
        return exec(a->name, b->name, depth);
    }
    bool classdef(ClassDef* a, ClassDef* b, int depth) {
        // TODO check full module path
        return exec(a->name, b->name, depth);
    }

    bool classtype(ClassType* a, ClassType* b, int depth) { return bool(); }
    // Types <<<<<<<<<<<<<<<<<<<<

    bool module(Module* a, Module* b, int depth) { return exec(a->body, b->body, depth); }
    bool interactive(Interactive* a, Interactive* b, int depth) {
        return exec(a->body, b->body, depth);
    }
    bool expression(Expression* a, Expression* b, int depth) {
        return exec(a->body, b->body, depth);
    }

    bool pass(Pass* a, Pass* b, int depth) { return true; }
    bool breakstmt(Break* a, Break* b, int depth) { return true; }
    bool continuestmt(Continue* a, Continue* b, int depth) { return true; }

    bool invalidstmt(InvalidStatement* a, InvalidStatement* b, int depth) { return false; }

    bool constant(Constant* a, Constant* b, int depth) {
        static int string_tid = meta::type_id<String>();

        if (a->value.tag == b->value.tag) {
            if (a->value.tag < int(meta::ValueTypes::Max)) {
                return a->value == b->value; 
            }

            // FIXME: implement this in the value operator==
            if (a->value.tag == string_tid) {
                String* stra = a->value.as<String*>();
                String* strb = b->value.as<String*>();

                return (*stra) == (*strb);
            }
        }

        return false;
    }

    bool exprstmt(Expr* a, Expr* b, int depth) { return exec(a->value, b->value, depth); }
    bool returnstmt(Return* a, Return* b, int depth) { return exec(a->value, b->value, depth); }
    bool await(Await* a, Await* b, int depth) { return exec(a->value, b->value, depth); }
    bool yield(Yield* a, Yield* b, int depth) { return exec(a->value, b->value, depth); }
    bool yieldfrom(YieldFrom* a, YieldFrom* b, int depth) {
        return exec(a->value, b->value, depth);
    }

    bool exported(Exported* a, Exported* b, int depth) { return exec(a->node, b->node, depth); }

    bool condjump(CondJump* a, CondJump* b, int depth) { return exec(a->condition, b->condition, depth); }

    bool boolop(BoolOp* a, BoolOp* b, int depth) {
        return a->op == b->op && exec(a->values, b->values, depth);
    }
    bool namedexpr(NamedExpr* a, NamedExpr* b, int depth) {
        return exec(a->target, b->target, depth) && exec(a->value, b->value, depth);
    }
    bool binop(BinOp* a, BinOp* b, int depth) {
        return a->op == b->op && exec(a->left, b->left, depth) && exec(a->right, b->right, depth);
    }
    bool unaryop(UnaryOp* a, UnaryOp* b, int depth) {
        return a->op == b->op && exec(a->operand, b->operand, depth);
    }
    bool lambda(Lambda* a, Lambda* b, int depth) {
        return exec(a->args, b->args, depth) && exec(a->body, b->body, depth);
    }
    bool ifexp(IfExp* a, IfExp* b, int depth) {
        return exec(a->test, b->test, depth) && exec(a->body, b->body, depth) &&
               exec(a->orelse, b->orelse, depth);
    }
    bool dictexpr(DictExpr* a, DictExpr* b, int depth) {
        return exec(a->keys, b->keys, depth) && exec(a->values, b->values, depth);
    }
    bool setexpr(SetExpr* a, SetExpr* b, int depth) { return exec(a->elts, b->elts, depth); }
    bool listcomp(ListComp* a, ListComp* b, int depth) {
        return exec(a->elt, b->elt, depth) && exec(a->generators, b->generators, depth);
    }
    bool generateexpr(GeneratorExp* a, GeneratorExp* b, int depth) {
        return exec(a->elt, b->elt, depth) && exec(a->generators, b->generators, depth);
    }
    bool setcomp(SetComp* a, SetComp* b, int depth) {
        return exec(a->elt, b->elt, depth) && exec(a->generators, b->generators, depth);
    }
    bool dictcomp(DictComp* a, DictComp* b, int depth) {
        return exec(a->key, b->key, depth) && exec(a->value, b->value, depth) &&
               exec(a->generators, b->generators, depth);
    }

    bool compare(Compare* a, Compare* b, int depth) {
        return exec(a->left, b->left, depth) && exec(a->ops, b->ops, depth) &&
               exec(a->comparators, b->comparators, depth);
    }
    bool call(Call* a, Call* b, int depth) {
        return exec(a->func, b->func, depth) && exec(a->args, b->args, depth) &&
               exec(a->keywords, b->keywords, depth);
    }
    bool joinedstr(JoinedStr* a, JoinedStr* b, int depth) {
        return exec(a->values, b->values, depth);
    }
    bool formattedvalue(FormattedValue* a, FormattedValue* b, int depth) {
        return true                                                          //
            && exec(a->value, b->value, depth)                               //
            && exec(a->conversion, b->conversion, depth)                     //
            && exec(a->format_spec->values, b->format_spec->values, depth);  //
    }
    bool attribute(Attribute* a, Attribute* b, int depth) {
        return exec(a->value, b->value, depth) && exec(a->attr, b->attr, depth) /* &&
               exec(a->ctx, b->ctx, depth) */
            ;
    }
    bool subscript(Subscript* a, Subscript* b, int depth) {

        return exec(a->value, b->value, depth) && exec(a->slice, b->slice, depth); /*&&
               exec(a->ctx, b->ctx, depth) */
        ;
    }
    bool starred(Starred* a, Starred* b, int depth) {
        return exec(a->value, b->value, depth) /*&& exec(a->ctx, b->ctx, depth) */;
    }
    bool name(Name* a, Name* b, int depth) { return exec(a->id, b->id, depth); }
    bool listexpr(ListExpr* a, ListExpr* b, int depth) { return exec(a->elts, b->elts, depth); }
    bool tupleexpr(TupleExpr* a, TupleExpr* b, int depth) { return exec(a->elts, b->elts, depth); }
    bool slice(Slice* a, Slice* b, int depth) {
        return exec(a->lower, b->lower, depth) && exec(a->upper, b->upper, depth) &&
               exec(a->step, b->step, depth);
    }
    bool deletestmt(Delete* a, Delete* b, int depth) { return exec(a->targets, b->targets, depth); }
    bool assign(Assign* a, Assign* b, int depth) {
        return exec(a->targets, b->targets, depth) && exec(a->value, b->value, depth);
    }
    bool augassign(AugAssign* a, AugAssign* b, int depth) {
        return exec(a->target, b->target, depth) && exec(a->value, b->value, depth);
    }
    bool annassign(AnnAssign* a, AnnAssign* b, int depth) {
        return exec(a->target, b->target, depth) && exec(a->value, b->value, depth) &&
               exec(a->annotation, b->annotation, depth);
    }
    bool forstmt(For* a, For* b, int depth) {
        return exec(a->target, b->target, depth) && exec(a->iter, b->iter, depth) &&
               exec(a->body, b->body, depth) && exec(a->orelse, b->orelse, depth);
    }
    bool whilestmt(While* a, While* b, int depth) {
        return exec(a->test, b->test, depth) && exec(a->body, b->body, depth) &&
               exec(a->orelse, b->orelse, depth);
    }
    bool ifstmt(If* a, If* b, int depth) {
        return exec(a->test, b->test, depth) && exec(a->body, b->body, depth) &&
               exec(a->orelse, b->orelse, depth) && exec(a->tests, b->tests, depth) &&
               exec(a->bodies, b->bodies, depth);
    }
    bool with(With* a, With* b, int depth) {
        return exec(a->items, b->items, depth) && exec(a->body, b->body, depth) &&
               exec(a->async, b->async, depth);
    }
    bool raise(Raise* a, Raise* b, int depth) {
        return exec(a->exc, b->exc, depth) && exec(a->cause, b->cause, depth);
    }
    bool trystmt(Try* a, Try* b, int depth) {
        return exec(a->body, b->body, depth) && exec(a->handlers, b->handlers, depth) &&
               exec(a->orelse, b->orelse, depth) && exec(a->finalbody, b->finalbody, depth);
    }
    bool assertstmt(Assert* a, Assert* b, int depth) {
        return exec(a->test, b->test, depth) && exec(a->msg, b->msg, depth);
    }
    bool import(Import* a, Import* b, int depth) { return exec(a->names, b->names, depth); }
    bool importfrom(ImportFrom* a, ImportFrom* b, int depth) {
        return exec(a->module, b->module, depth) && exec(a->names, b->names, depth) &&
               exec(a->level, b->level, depth);
    }

    bool global(Global* a, Global* b, int depth) { return exec(a->names, b->names, depth); }
    bool nonlocal(Nonlocal* a, Nonlocal* b, int depth) { return exec(a->names, b->names, depth); }
    bool inlinestmt(Inline* a, Inline* b, int depth) { return exec(a->body, b->body, depth); }

    // Patterns
    bool match(Match* a, Match* b, int depth) {
        return exec(a->subject, b->subject, depth) && exec(a->cases, b->cases, depth);
    }
    bool matchvalue(MatchValue* a, MatchValue* b, int depth) {
        return exec(a->value, b->value, depth);
    }
    bool matchsingleton(MatchSingleton* a, MatchSingleton* b, int depth) {
        return exec(a->value, b->value, depth);
    }
    bool matchsequence(MatchSequence* a, MatchSequence* b, int depth) {
        return exec(a->patterns, b->patterns, depth);
    }
    bool matchmapping(MatchMapping* a, MatchMapping* b, int depth) {
        if (a->patterns.size() != b->patterns.size()) {
            return false;
        }
        for (int i = 0; i < a->patterns.size(); i++) {
            if (!exec(a->patterns[i], b->patterns[i], depth)) {
                return false;
            }
            if (!exec(a->keys[i], b->keys[i], depth)) {
                return false;
            }
        }
        return exec(a->rest, b->rest, depth);
    }
    bool matchclass(MatchClass* a, MatchClass* b, int depth) {
        if (!exec(a->cls, b->cls, depth)) {
            return false;
        }
        // patterns
        if (!exec(a->patterns, b->patterns, depth)) {
            return false;
        }

        // kwd_attrs
        if (a->kwd_attrs.size() != b->kwd_attrs.size()) {
            return false;
        }
        for (int i = 0; i < a->kwd_attrs.size(); i++) {
            if (!exec(a->kwd_attrs[i], b->kwd_attrs[i], depth)) {
                return false;
            }
            if (!exec(a->kwd_patterns[i], b->kwd_patterns[i], depth)) {
                return false;
            }
        }

        return true;
    }
    bool matchstar(MatchStar* a, MatchStar* b, int depth) { return exec(a->name, b->name, depth); }
    bool matchas(MatchAs* a, MatchAs* b, int depth) {
        return exec(a->pattern, b->pattern, depth) && exec(a->name, b->name, depth);
    }
    bool matchor(MatchOr* a, MatchOr* b, int depth) {
        return exec(a->patterns, b->patterns, depth);
    }

    bool placeholder(Placeholder* a, Placeholder* b, int depth) { return false; }
};

bool equal(Node* a, Node* b) {
    int n = (a == nullptr) + (b == nullptr);
    if (n > 0) {
        return n == 2;
    }
    Equality eq;
    return eq.exec(a, b);
}

bool equal(ExprNode* a, ExprNode* b) {
    int n = (a == nullptr) + (b == nullptr);
    if (n > 0) {
        return n == 2;
    }
    Equality eq;
    return eq.exec(a, b);
}

bool equal(Pattern* a, Pattern* b) {
    int n = (a == nullptr) + (b == nullptr);
    if (n > 0) {
        return n == 2;
    }
    Equality eq;
    return eq.exec(a, b);
}

bool equal(StmtNode* a, StmtNode* b) {
    int n = (a == nullptr) + (b == nullptr);
    if (n > 0) {
        return n == 2;
    }
    Equality eq;
    return eq.exec(a, b);
}

bool equal(ModNode* a, ModNode* b) {
    int n = (a == nullptr) + (b == nullptr);
    if (n > 0) {
        return n == 2;
    }
    Equality eq;
    return eq.exec(a, b);
}

}  // namespace lython
