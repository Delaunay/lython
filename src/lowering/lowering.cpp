#include "lowering.h"


namespace lython {

using StmtRet = Lowering::StmtRet;
using ExprRet = Lowering::ExprRet;
using ModRet  = Lowering::ModRet;
using PatRet  = Lowering::PatRet;



ExprRet Lowering::namedexpr(NamedExpr_t* n, int depth) { 
    exec(n->target, depth);
    exec(n->value, depth);
    return ExprRet(); 
}
ExprRet Lowering::boolop(BoolOp_t* n, int depth) { 
    for(auto* expr: n->values) {
        exec(expr, depth);
    }
    return ExprRet(); }
ExprRet Lowering::compare(Compare_t* n, int depth) { 
    exec(n->left, depth);
    for(auto* expr: n->comparators) {
        exec(expr, depth);
    }
    return ExprRet(); 
}
ExprRet Lowering::binop(BinOp_t* n, int depth) { 
    exec(n->left, depth);
    exec(n->right, depth);
    return ExprRet(); 
}
ExprRet Lowering::unaryop(UnaryOp_t* n, int depth) {
    exec(n->operand, depth);
    return ExprRet(); 
}
ExprRet Lowering::lambda(Lambda_t* n, int depth) { 
    exec(n->body, depth);
    return ExprRet(); 
}
ExprRet Lowering::ifexp(IfExp_t* n, int depth) { 
    exec(n->test, depth);
    exec(n->body, depth);
    exec(n->orelse, depth);
    return ExprRet(); 
}
ExprRet Lowering::dictexpr(DictExpr_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::setexpr(SetExpr_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::generateexpr(GeneratorExp_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::listexpr(ListExpr_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::tupleexpr(TupleExpr_t* n, int depth) { return ExprRet(); }

ExprRet Lowering::listcomp(ListComp_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::setcomp(SetComp_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::dictcomp(DictComp_t* n, int depth) { return ExprRet(); }

ExprRet Lowering::await(Await_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::yield(Yield_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::yieldfrom(YieldFrom_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::joinedstr(JoinedStr_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::formattedvalue(FormattedValue_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::constant(Constant_t* n, int depth) { 
    return ExprRet();
}
ExprRet Lowering::attribute(Attribute_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::subscript(Subscript_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::starred(Starred_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::slice(Slice_t* n, int depth) { return ExprRet(); }

ExprRet Lowering::dicttype(DictType_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::arraytype(ArrayType_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::arrow(Arrow_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::builtintype(BuiltinType_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::tupletype(TupleType_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::settype(SetType_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::classtype(ClassType_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::comment(Comment_t* n, int depth) { return ExprRet(); }
ExprRet Lowering::name(Name_t* n, int depth) { return ExprRet(); }

// JUMP
ExprRet Lowering::call(Call_t* n, int depth) { 
    
    return ExprRet(); 
}

// Leaves
StmtRet Lowering::invalidstmt(InvalidStatement_t* n, int depth) {
    kwerror(outlog(), "Invalid statement");
    return StmtRet();
}
StmtRet Lowering::returnstmt(Return_t* n, int depth) {
    
    return StmtRet();
}
StmtRet Lowering::deletestmt(Delete_t* n, int depth) {
    
    return StmtRet();
}
StmtRet Lowering::assign(Assign_t* n, int depth) {
    return StmtRet();
}
StmtRet Lowering::augassign(AugAssign_t* n, int depth) {
    return StmtRet();
}
StmtRet Lowering::annassign(AnnAssign_t* n, int depth) {
    return StmtRet();
}
StmtRet Lowering::exprstmt(Expr_t* n, int depth) {
    return StmtRet();
}
StmtRet Lowering::pass(Pass_t* n, int depth) { return StmtRet(); }
StmtRet Lowering::breakstmt(Break_t* n, int depth) {
    return StmtRet();
}
StmtRet Lowering::continuestmt(Continue_t* n, int depth) {
    return StmtRet();
}
StmtRet Lowering::assertstmt(Assert_t* n, int depth) {
    return StmtRet();
}
StmtRet Lowering::raise(Raise_t* n, int depth) {
    return StmtRet();
}
StmtRet Lowering::global(Global_t* n, int depth) {
    return StmtRet();
}
StmtRet Lowering::nonlocal(Nonlocal_t* n, int depth) {
    return StmtRet();
}

// StmtRet Lowering::condjump(CondJump_t* n, int depth) {
//     return StmtRet();
// }

StmtRet Lowering::import(Import_t* n, int depth) { return StmtRet(); }
StmtRet Lowering::importfrom(ImportFrom_t* n, int depth) { return StmtRet(); }

StmtRet Lowering::inlinestmt(Inline_t* n, int depth) {
    return StmtRet();
}
StmtRet Lowering::functiondef(FunctionDef_t* n, int depth) {
    return StmtRet();
}
StmtRet Lowering::classdef(ClassDef_t* n, int depth) {
    return StmtRet();
}

StmtRet Lowering::forstmt(For_t* n, int depth) {
    return StmtRet();
}

StmtRet Lowering::whilestmt(While_t* n, int depth) {
    return StmtRet();
}

StmtRet Lowering::ifstmt(If_t* n, int depth) {
    return StmtRet();
}

StmtRet Lowering::with(With_t* n, int depth) {
    return StmtRet();
}
StmtRet Lowering::trystmt(Try_t* n, int depth) {
    return StmtRet();
}

StmtRet Lowering::match(Match_t* n, int depth) { return StmtRet(); }

PatRet Lowering::matchvalue(MatchValue_t* n, int depth) { return PatRet(); }
PatRet Lowering::matchsingleton(MatchSingleton_t* n, int depth) { return PatRet(); }
PatRet Lowering::matchsequence(MatchSequence_t* n, int depth) { return PatRet(); }
PatRet Lowering::matchmapping(MatchMapping_t* n, int depth) { return PatRet(); }
PatRet Lowering::matchclass(MatchClass_t* n, int depth) { return PatRet(); }
PatRet Lowering::matchstar(MatchStar_t* n, int depth) { return PatRet(); }
PatRet Lowering::matchas(MatchAs_t* n, int depth) { return PatRet(); }
PatRet Lowering::matchor(MatchOr_t* n, int depth) { return PatRet(); }

ModRet Lowering::module(Module_t* n, int depth) {
    return ModRet();
};
ModRet Lowering::interactive(Interactive_t* n, int depth) { return ModRet(); }
ModRet Lowering::functiontype(FunctionType_t* n, int depth) { return ModRet(); }
ModRet Lowering::expression(Expression_t* n, int depth) { return ModRet(); }

/*
ModRet Lowering::exported(Exported_t* n, int depth) { return ModRet(); }
ModRet Lowering::placeholder(Placeholder_t* n, int depth) { return ModRet(); }
*/

}