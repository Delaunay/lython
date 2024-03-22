#include "plugin/example.h"


using StmtRet = ExampleVisitor::StmtRet;
using ExprRet = ExampleVisitor::ExprRet;
using ModRet  = ExampleVisitor::ModRet;
using PatRet  = ExampleVisitor::PatRet;

ExprRet ExampleVisitor::boolop(BoolOp_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::namedexpr(NamedExpr_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::compare(Compare_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::binop(BinOp_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::unaryop(UnaryOp_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::lambda(Lambda_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::ifexp(IfExp_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::dictexpr(DictExpr_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::setexpr(SetExpr_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::listcomp(ListComp_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::generateexpr(GeneratorExp_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::setcomp(SetComp_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::dictcomp(DictComp_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::await(Await_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::yield(Yield_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::yieldfrom(YieldFrom_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::call(Call_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::joinedstr(JoinedStr_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::formattedvalue(FormattedValue_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::constant(Constant_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::attribute(Attribute_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::subscript(Subscript_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::starred(Starred_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::name(Name_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::listexpr(ListExpr_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::tupleexpr(TupleExpr_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::slice(Slice_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::dicttype(DictType_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::arraytype(ArrayType_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::arrow(Arrow_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::builtintype(BuiltinType_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::tupletype(TupleType_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::settype(SetType_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::classtype(ClassType_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::comment(Comment_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::exported(Exported_t* n, int depth) { return ExprRet(); }
ExprRet ExampleVisitor::placeholder(Placeholder_t* n, int depth) { return ExprRet(); }

StmtRet ExampleVisitor::functiondef(FunctionDef_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::classdef(ClassDef_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::invalidstmt(InvalidStatement_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::returnstmt(Return_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::deletestmt(Delete_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::assign(Assign_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::augassign(AugAssign_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::annassign(AnnAssign_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::forstmt(For_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::whilestmt(While_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::ifstmt(If_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::with(With_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::raise(Raise_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::trystmt(Try_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::assertstmt(Assert_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::global(Global_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::nonlocal(Nonlocal_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::exprstmt(Expr_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::pass(Pass_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::breakstmt(Break_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::continuestmt(Continue_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::match(Match_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::inlinestmt(Inline_t* n, int depth) { return StmtRet(); }

StmtRet ExampleVisitor::import(Import_t* n, int depth) { return StmtRet(); }
StmtRet ExampleVisitor::importfrom(ImportFrom_t* n, int depth) { return StmtRet(); }

PatRet ExampleVisitor::matchvalue(MatchValue_t* n, int depth) { return PatRet(); }
PatRet ExampleVisitor::matchsingleton(MatchSingleton_t* n, int depth) { return PatRet(); }
PatRet ExampleVisitor::matchsequence(MatchSequence_t* n, int depth) { return PatRet(); }
PatRet ExampleVisitor::matchmapping(MatchMapping_t* n, int depth) { return PatRet(); }
PatRet ExampleVisitor::matchclass(MatchClass_t* n, int depth) { return PatRet(); }
PatRet ExampleVisitor::matchstar(MatchStar_t* n, int depth) { return PatRet(); }
PatRet ExampleVisitor::matchas(MatchAs_t* n, int depth) { return PatRet(); }
PatRet ExampleVisitor::matchor(MatchOr_t* n, int depth) { return PatRet(); }

ModRet ExampleVisitor::module(Module_t* stmt, int depth) { return ModRet(); };
ModRet ExampleVisitor::interactive(Interactive_t* n, int depth) { return ModRet(); }
ModRet ExampleVisitor::functiontype(FunctionType_t* n, int depth) { return ModRet(); }
ModRet ExampleVisitor::expression(Expression_t* n, int depth) { return ModRet(); }

extern "C" {
    VisitorPlugin_C make_plugin() {
        return new ExampleVisitor();
    }
    void free_plugin(VisitorPlugin_C* plugin) {
        return delete ((ExampleVisitor*)(plugin));
    }
}