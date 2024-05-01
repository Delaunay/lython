#include "ast/visitor.h"

namespace lython {

struct void_t {};

struct TraverseTrait {
    using StmtRet = void_t;
    using ExprRet = void_t;
    using ModRet  = void_t;
    using PatRet  = void_t;
    using Trace   = std::false_type;

    enum
    { MaxRecursionDepth = LY_MAX_VISITOR_RECURSION_DEPTH };
};

// Generic visitor for simple tree operation
struct Traverse: public BaseVisitor<Traverse, false, TraverseTrait> {
    using Super = BaseVisitor<Traverse, false, TraverseTrait>;

    virtual void_t boolop(BoolOp* n, int depth) { return void_t(); }
    virtual void_t exported(Exported* n, int depth) { return void_t(); }
    virtual void_t namedexpr(NamedExpr* n, int depth) { return void_t(); }
    virtual void_t binop(BinOp* n, int depth) { return void_t(); }
    virtual void_t unaryop(UnaryOp* n, int depth) { return void_t(); }
    virtual void_t lambda(Lambda* n, int depth) { return void_t(); }
    virtual void_t ifexp(IfExp* n, int depth) { return void_t(); }
    virtual void_t dictexpr(DictExpr* n, int depth) { return void_t(); }
    virtual void_t setexpr(SetExpr* n, int depth) { return void_t(); }
    virtual void_t listcomp(ListComp* n, int depth) { return void_t(); }
    virtual void_t generateexpr(GeneratorExp* n, int depth) { return void_t(); }
    virtual void_t setcomp(SetComp* n, int depth) { return void_t(); }
    virtual void_t dictcomp(DictComp* n, int depth) { return void_t(); }
    virtual void_t await(Await* n, int depth) { return void_t(); }
    virtual void_t yield(Yield* n, int depth) { return void_t(); }
    virtual void_t yieldfrom(YieldFrom* n, int depth) { return void_t(); }
    virtual void_t compare(Compare* n, int depth) { return void_t(); }
    virtual void_t call(Call* n, int depth) { return void_t(); }
    virtual void_t joinedstr(JoinedStr* n, int depth) { return void_t(); }
    virtual void_t formattedvalue(FormattedValue* n, int depth) { return void_t(); }
    virtual void_t constant(Constant* n, int depth) { return void_t(); }
    virtual void_t placeholder(Placeholder_t* n, int depth) {return void_t(); } 
    virtual void_t attribute(Attribute* n, int depth) { return void_t(); }
    virtual void_t subscript(Subscript* n, int depth) { return void_t(); }
    virtual void_t starred(Starred* n, int depth) { return void_t(); }
    virtual void_t name(Name* n, int depth) { return void_t(); }
    virtual void_t listexpr(ListExpr* n, int depth) {
        for (auto* i: n->elts) {
            Super::exec(i, depth);
        }
        return void_t();
    }
    virtual void_t tupleexpr(TupleExpr* n, int depth) {
        for (auto* i: n->elts) {
            Super::exec(i, depth);
        }
        return void_t();
    }
    virtual void_t slice(Slice* n, int depth) { return void_t(); }
    virtual void_t dicttype(DictType* n, int depth) { return void_t(); }
    virtual void_t arraytype(ArrayType* n, int depth) { return void_t(); }
    virtual void_t tupletype(TupleType* n, int depth) { return void_t(); }
    virtual void_t arrow(Arrow* n, int depth) { return void_t(); }
    virtual void_t classtype(ClassType* n, int depth) { return void_t(); }
    virtual void_t settype(SetType* n, int depth) { return void_t(); }
    virtual void_t builtintype(BuiltinType* n, int depth) { return void_t(); }
    virtual void_t functiondef(FunctionDef* n, int depth) { return void_t(); }
    virtual void_t classdef(ClassDef* n, int depth) { return void_t(); }
    virtual void_t returnstmt(Return* n, int depth) { return void_t(); }
    virtual void_t deletestmt(Delete* n, int depth) { return void_t(); }
    virtual void_t assign(Assign* n, int depth) { return void_t(); }
    virtual void_t augassign(AugAssign* n, int depth) { return void_t(); }
    virtual void_t annassign(AnnAssign* n, int depth) { return void_t(); }
    virtual void_t forstmt(For* n, int depth) { return void_t(); }
    virtual void_t whilestmt(While* n, int depth) { return void_t(); }
    virtual void_t ifstmt(If* n, int depth) { return void_t(); }
    virtual void_t with(With* n, int depth) { return void_t(); }
    virtual void_t raise(Raise* n, int depth) { return void_t(); }
    virtual void_t trystmt(Try* n, int depth) { return void_t(); }
    virtual void_t assertstmt(Assert* n, int depth) { return void_t(); }
    virtual void_t import(Import* n, int depth) { return void_t(); }
    virtual void_t importfrom(ImportFrom* n, int depth) { return void_t(); }
    virtual void_t global(Global* n, int depth) { return void_t(); }
    virtual void_t nonlocal(Nonlocal* n, int depth) { return void_t(); }
    virtual void_t exprstmt(Expr* n, int depth) { return void_t(); }
    virtual void_t pass(Pass* n, int depth) { return void_t(); }
    virtual void_t breakstmt(Break* n, int depth) { return void_t(); }
    virtual void_t continuestmt(Continue* n, int depth) { return void_t(); }
    virtual void_t match(Match* n, int depth) { return void_t(); }
    virtual void_t inlinestmt(Inline* n, int depth) { return void_t(); }
    virtual void_t matchvalue(MatchValue* n, int depth) { return void_t(); }
    virtual void_t matchsingleton(MatchSingleton* n, int depth) { return void_t(); }
    virtual void_t matchsequence(MatchSequence* n, int depth) { return void_t(); }
    virtual void_t matchmapping(MatchMapping* n, int depth) { return void_t(); }
    virtual void_t matchclass(MatchClass* n, int depth) { return void_t(); }
    virtual void_t matchstar(MatchStar* n, int depth) { return void_t(); }
    virtual void_t matchas(MatchAs* n, int depth) { return void_t(); }
    virtual void_t matchor(MatchOr* n, int depth) { return void_t(); }

    virtual void_t module(Module* n, int depth) { return void_t(); }
    virtual void_t interactive(Interactive* n, int depth) { return void_t(); }
    virtual void_t functiontype(FunctionType* n, int depth) { return void_t(); }
    virtual void_t comment(Comment* n, int depth) { return void_t(); }
    virtual void_t invalidstmt(InvalidStatement* n, int depth) { return void_t(); }
    virtual void_t expression(Expression* n, int depth) { return void_t(); }
    // virtual void_t condjump(CondJump_t* n, int depth) { return void_t(); }
};

struct SetContext: public Traverse {
    using Super = Traverse;

    ExprContext ctx;

    virtual void_t attribute(Attribute* n, int depth) override {
        n->ctx = ctx;
        return Super::attribute(n, depth);
    }

    virtual void_t subscript(Subscript* n, int depth) override {
        n->ctx = ctx;
        return Super::subscript(n, depth);
    }

    virtual void_t starred(Starred* n, int depth) override {
        n->ctx = ctx;
        return Super::starred(n, depth);
    }

    virtual void_t name(Name* n, int depth) override {
        n->ctx = ctx;
        return Super::name(n, depth);
    }

    virtual void_t listexpr(ListExpr* n, int depth) override {
        n->ctx = ctx;
        return Super::listexpr(n, depth);
    }

    virtual void_t tupleexpr(TupleExpr* n, int depth) override {
        n->ctx = ctx;
        return Super::tupleexpr(n, depth);
    }
};

void set_context(Node* n, ExprContext ctx) {
    SetContext ctx_visitor;
    ctx_visitor.ctx = ctx;
    ctx_visitor.exec<void_t>(n, 0);
}

}  // namespace lython
