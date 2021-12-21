#include "ast/ops.h"
#include "ast/visitor.h"

namespace lython {

// Equality Visitor
struct Equality {
    using Return_t = bool;

    template <typename T>
    bool exec(T const &a, T const &b, int depth) {
        return a == b;
    }

    template <typename T>
    bool exec(Optional<T> const &a, Optional<T> const &b, int depth) {
        if (a.has_value() == b.has_value()) {
            if (a.has_value()) {
                return exec(a.value(), b.value(), depth);
            }
            return true;
        }

        return false;
    }

    template <typename T>
    bool exec(Array<T> const &a, Array<T> const &b, int depth) {
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

    bool exec(Node *a, Node *b) {
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
        }
        // clang-format on
        return false;
    }

    bool exec(ModNode *a, ModNode *b, int depth) {
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

            NODEKIND_ENUM(X, SECTION, PASS, PASS, MOD, PASS)

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

    bool exec(Pattern *a, Pattern *b, int depth) {
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

            NODEKIND_ENUM(X, SECTION, PASS, PASS, PASS, MATCH)

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

    bool exec(ExprNode *a, ExprNode *b, int depth) {
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
                    name* aa = reinterpret_cast<name*>(aa);\
                    name* bb = reinterpret_cast<name*>(bb);\
                    return fun(aa, bb, depth + 1);\
                }

            NODEKIND_ENUM(X, SECTION, EXPR, PASS, PASS, PASS)

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

    bool exec(StmtNode *a, StmtNode *b, int depth) {
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

            NODEKIND_ENUM(X, SECTION, PASS, STMT, PASS, PASS)

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
    bool functiontype(FunctionType *a, FunctionType *b, int depth) {
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
    bool dicttype(DictType *a, DictType *b, int depth) {
        return exec(a->key, b->key, depth) && exec(a->value, b->value, depth);
    }
    bool arraytype(ArrayType *a, ArrayType *b, int depth) {
        return exec(a->value, b->value, depth);
    }
    bool settype(SetType *a, SetType *b, int depth) { return exec(a->value, b->value, depth); }
    bool tupletype(TupleType *a, TupleType *b, int depth) {
        return exec(a->types, b->types, depth);
    }

    bool arrow(Arrow *a, Arrow *b, int depth) {
        return exec(a->args, a->args, depth) && exec(a->returns, b->returns, depth);
    }

    bool builtintype(BuiltinType *a, BuiltinType *b, int depth) { return a->name == b->name; }
    bool functiondef(FunctionDef *a, FunctionDef *b, int depth) {
        // TODO check full module path
        return a->name == b->name;
    }
    bool classdef(ClassDef *a, ClassDef *b, int depth) {
        // TODO check full module path
        return a->name == b->name;
    }

    bool classtype(ClassType *a, ClassType *b, int depth) { return bool(); }
    // Types <<<<<<<<<<<<<<<<<<<<

    bool module(Module *a, Module *b, int depth) { return true; }
    bool interactive(Interactive *a, Interactive *b, int depth) { return true; }
    bool expression(Expression *a, Expression *b, int depth) {
        return exec(a->body, b->body, depth);
    }

    bool pass(Pass *a, Pass *b, int depth) { return true; }
    bool breakstmt(Break *a, Break *b, int depth) { return true; }
    bool continuestmt(Continue *a, Continue *b, int depth) { return true; }

    bool constant(Constant *a, Constant *b, int depth) { return a->value == b->value; }
    bool exprstmt(Expr *a, Expr *b, int depth) { return exec(a->value, b->value, depth); }
    bool returnstmt(Return *a, Return *b, int depth) { return exec(a->value, b->value, depth); }
    bool await(Await *a, Await *b, int depth) { return exec(a->value, b->value, depth); }
    bool yield(Yield *a, Yield *b, int depth) { return exec(a->value, b->value, depth); }
    bool yieldfrom(YieldFrom *a, YieldFrom *b, int depth) {
        return exec(a->value, b->value, depth);
    }

    bool boolop(BoolOp *a, BoolOp *b, int depth) { return bool(); }
    bool namedexpr(NamedExpr *a, NamedExpr *b, int depth) { return bool(); }
    bool binop(BinOp *a, BinOp *b, int depth) { return bool(); }
    bool unaryop(UnaryOp *a, UnaryOp *b, int depth) { return bool(); }
    bool lambda(Lambda *a, Lambda *b, int depth) { return bool(); }
    bool ifexp(IfExp *a, IfExp *b, int depth) { return bool(); }
    bool dictexpr(DictExpr *a, DictExpr *b, int depth) { return bool(); }
    bool setexpr(SetExpr *a, SetExpr *b, int depth) { return bool(); }
    bool listcomp(ListComp *a, ListComp *b, int depth) { return bool(); }
    bool generateexpr(GeneratorExp *a, GeneratorExp *b, int depth) { return bool(); }
    bool setcomp(SetComp *a, SetComp *b, int depth) { return bool(); }
    bool dictcomp(DictComp *a, DictComp *b, int depth) { return bool(); }
    bool compare(Compare *a, Compare *b, int depth) { return bool(); }
    bool call(Call *a, Call *b, int depth) { return bool(); }
    bool joinedstr(JoinedStr *a, JoinedStr *b, int depth) { return bool(); }
    bool formattedvalue(FormattedValue *a, FormattedValue *b, int depth) { return bool(); }
    bool attribute(Attribute *a, Attribute *b, int depth) { return bool(); }
    bool subscript(Subscript *a, Subscript *b, int depth) { return bool(); }
    bool starred(Starred *a, Starred *b, int depth) { return bool(); }
    bool name(Name *a, Name *b, int depth) { return bool(); }
    bool listexpr(ListExpr *a, ListExpr *b, int depth) { return bool(); }
    bool tupleexpr(TupleExpr *a, TupleExpr *b, int depth) { return bool(); }
    bool slice(Slice *a, Slice *b, int depth) { return bool(); }
    bool deletestmt(Delete *a, Delete *b, int depth) { return bool(); }
    bool assign(Assign *a, Assign *b, int depth) { return bool(); }
    bool augassign(AugAssign *a, AugAssign *b, int depth) { return bool(); }
    bool annassign(AnnAssign *a, AnnAssign *b, int depth) { return bool(); }
    bool forstmt(For *a, For *b, int depth) { return bool(); }
    bool whilestmt(While *a, While *b, int depth) { return bool(); }
    bool ifstmt(If *a, If *b, int depth) { return bool(); }
    bool with(With *a, With *b, int depth) { return bool(); }
    bool raise(Raise *a, Raise *b, int depth) { return bool(); }
    bool trystmt(Try *a, Try *b, int depth) { return bool(); }
    bool assertstmt(Assert *a, Assert *b, int depth) {
        return exec(a->test, b->test, depth) && exec(a->msg, b->msg, depth);
    }
    bool import(Import *a, Import *b, int depth) { return bool(); }
    bool importfrom(ImportFrom *a, ImportFrom *b, int depth) { return bool(); }

    bool global(Global *a, Global *b, int depth) { return exec(a->names, b->names, depth); }
    bool nonlocal(Nonlocal *a, Nonlocal *b, int depth) { return exec(a->names, b->names, depth); }
    bool inlinestmt(Inline *a, Inline *b, int depth) { return exec(a->body, b->body, depth); }

    // Patterns
    bool match(Match *a, Match *b, int depth) {
        if (!exec(a->subject, b->subject, depth)) {
            return false;
        }

        if (a->cases.size() != b->cases.size()) {
            return false;
        }

        for (int i = 0; i < a->cases.size(); i++) {
            auto a_case = a->cases[i];
            auto b_case = b->cases[i];

            if (!exec(a_case.pattern, b_case.pattern, depth)) {
                return false;
            }

            if (!exec(a_case.guard, b_case.guard, depth)) {
                return false;
            }

            if (!exec(a_case.body, b_case.body, depth)) {
                return false;
            }
        }

        return true;
    }
    bool matchvalue(MatchValue *a, MatchValue *b, int depth) {
        return exec(a->value, b->value, depth);
    }
    bool matchsingleton(MatchSingleton *a, MatchSingleton *b, int depth) {
        return a->value == b->value;
    }
    bool matchsequence(MatchSequence *a, MatchSequence *b, int depth) {
        return exec(a->patterns, b->patterns, depth);
    }
    bool matchmapping(MatchMapping *a, MatchMapping *b, int depth) {
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
        return a->rest == b->rest;
    }
    bool matchclass(MatchClass *a, MatchClass *b, int depth) {
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
            if (a->kwd_attrs[i] == b->kwd_attrs[i]) {
                return false;
            }
            if (a->kwd_patterns[i] == b->kwd_patterns[i]) {
                return false;
            }
        }

        return true;
    }
    bool matchstar(MatchStar *a, MatchStar *b, int depth) { return a->name == b->name; }
    bool matchas(MatchAs *a, MatchAs *b, int depth) {
        return exec(a->pattern, b->pattern, depth) && a->name == b->name;
    }
    bool matchor(MatchOr *a, MatchOr *b, int depth) {
        return exec(a->patterns, b->patterns, depth);
    }
};

bool equal(Node *a, Node *b) {
    Equality eq;
    return eq.exec(a, b);
}

bool equal(ExprNode *a, ExprNode *b) {
    Equality eq;
    return eq.exec(a, b);
}

bool equal(Pattern *a, Pattern *b) {
    Equality eq;
    return eq.exec(a, b);
}

bool equal(StmtNode *a, StmtNode *b) {
    Equality eq;
    return eq.exec(a, b);
}

bool equal(ModNode *a, ModNode *b) {
    Equality eq;
    return eq.exec(a, b);
}

} // namespace lython
