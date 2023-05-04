#include "tide/convert/to_graph.h"
#include "ast/magic.h"
#include "builtin/operators.h"
#include "utilities/guard.h"
#include "utilities/strings.h"

namespace lython {

using StmtRet = ToGraph::StmtRet;
using ExprRet = ToGraph::ExprRet;
using ModRet  = ToGraph::ModRet;
using PatRet  = ToGraph::PatRet;

ExprRet ToGraph::boolop(BoolOp_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::namedexpr(NamedExpr_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::compare(Compare_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::binop(BinOp_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::unaryop(UnaryOp_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::lambda(Lambda_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::ifexp(IfExp_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::dictexpr(DictExpr_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::setexpr(SetExpr_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::listcomp(ListComp_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::generateexpr(GeneratorExp_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::setcomp(SetComp_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::dictcomp(DictComp_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::await(Await_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::yield(Yield_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::yieldfrom(YieldFrom_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::call(Call_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::joinedstr(JoinedStr_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::formattedvalue(FormattedValue_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::constant(Constant_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::attribute(Attribute_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::subscript(Subscript_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::starred(Starred_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::name(Name_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::listexpr(ListExpr_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::tupleexpr(TupleExpr_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::slice(Slice_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::dicttype(DictType_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::arraytype(ArrayType_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::arrow(Arrow_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::builtintype(BuiltinType_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::tupletype(TupleType_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::settype(SetType_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::classtype(ClassType_t* n, int depth) { return ExprRet(); }
ExprRet ToGraph::comment(Comment_t* n, int depth) { return ExprRet(); }

StmtRet ToGraph::functiondef(FunctionDef_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::classdef(ClassDef_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::invalidstmt(InvalidStatement_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::returnstmt(Return_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::deletestmt(Delete_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::assign(Assign_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::augassign(AugAssign_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::annassign(AnnAssign_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::forstmt(For_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::whilestmt(While_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::ifstmt(If_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::with(With_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::raise(Raise_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::trystmt(Try_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::assertstmt(Assert_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::global(Global_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::nonlocal(Nonlocal_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::exprstmt(Expr_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::pass(Pass_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::breakstmt(Break_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::continuestmt(Continue_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::match(Match_t* n, int depth) { return StmtRet(); }
StmtRet ToGraph::inlinestmt(Inline_t* n, int depth) { return StmtRet(); }

PatRet ToGraph::matchvalue(MatchValue_t* n, int depth) { return PatRet(); }
PatRet ToGraph::matchsingleton(MatchSingleton_t* n, int depth) { return PatRet(); }
PatRet ToGraph::matchsequence(MatchSequence_t* n, int depth) { return PatRet(); }
PatRet ToGraph::matchmapping(MatchMapping_t* n, int depth) { return PatRet(); }
PatRet ToGraph::matchclass(MatchClass_t* n, int depth) { return PatRet(); }
PatRet ToGraph::matchstar(MatchStar_t* n, int depth) { return PatRet(); }
PatRet ToGraph::matchas(MatchAs_t* n, int depth) { return PatRet(); }
PatRet ToGraph::matchor(MatchOr_t* n, int depth) { return PatRet(); }

ModRet ToGraph::module(Module_t* stmt, int depth) { return ModRet(); };
ModRet ToGraph::interactive(Interactive_t* n, int depth) { return ModRet(); }
ModRet ToGraph::functiontype(FunctionType_t* n, int depth) { return ModRet(); }
ModRet ToGraph::expression(Expression_t* n, int depth) { return ModRet(); }

}  // namespace lython
