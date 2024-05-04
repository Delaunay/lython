#include "codegen/cpp/cpp_gen.h"
#include "utilities/printing.h"
#include "builtin/operators.h"
#include "utilities/guard.h"
#include "utilities/strings.h"

namespace lython {

using StmtRet = CPPGen::StmtRet;
using ExprRet = CPPGen::ExprRet;
using ModRet  = CPPGen::ModRet;
using PatRet  = CPPGen::PatRet;

ExprRet CPPGen::boolop(BoolOp_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::namedexpr(NamedExpr_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::compare(Compare_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::binop(BinOp_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::unaryop(UnaryOp_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::lambda(Lambda_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::ifexp(IfExp_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::dictexpr(DictExpr_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::setexpr(SetExpr_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::listcomp(ListComp_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::generateexpr(GeneratorExp_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::setcomp(SetComp_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::dictcomp(DictComp_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::await(Await_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::yield(Yield_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::yieldfrom(YieldFrom_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::call(Call_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::joinedstr(JoinedStr_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::formattedvalue(FormattedValue_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::constant(Constant_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::attribute(Attribute_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::subscript(Subscript_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::starred(Starred_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::name(Name_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::listexpr(ListExpr_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::tupleexpr(TupleExpr_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::slice(Slice_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::dicttype(DictType_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::arraytype(ArrayType_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::arrow(Arrow_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::builtintype(BuiltinType_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::tupletype(TupleType_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::settype(SetType_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::classtype(ClassType_t* n, int depth) { return ExprRet(); }
ExprRet CPPGen::comment(Comment_t* n, int depth) { return ExprRet(); }

StmtRet CPPGen::functiondef(FunctionDef_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::classdef(ClassDef_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::invalidstmt(InvalidStatement_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::returnstmt(Return_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::deletestmt(Delete_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::assign(Assign_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::augassign(AugAssign_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::annassign(AnnAssign_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::forstmt(For_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::whilestmt(While_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::ifstmt(If_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::with(With_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::raise(Raise_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::trystmt(Try_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::assertstmt(Assert_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::global(Global_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::nonlocal(Nonlocal_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::exprstmt(Expr_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::pass(Pass_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::breakstmt(Break_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::continuestmt(Continue_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::match(Match_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::inlinestmt(Inline_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::import(Import_t* n, int depth) { return StmtRet(); }
StmtRet CPPGen::importfrom(ImportFrom_t* n, int depth) { return StmtRet(); }

PatRet CPPGen::matchvalue(MatchValue_t* n, int depth) { return PatRet(); }
PatRet CPPGen::matchsingleton(MatchSingleton_t* n, int depth) { return PatRet(); }
PatRet CPPGen::matchsequence(MatchSequence_t* n, int depth) { return PatRet(); }
PatRet CPPGen::matchmapping(MatchMapping_t* n, int depth) { return PatRet(); }
PatRet CPPGen::matchclass(MatchClass_t* n, int depth) { return PatRet(); }
PatRet CPPGen::matchstar(MatchStar_t* n, int depth) { return PatRet(); }
PatRet CPPGen::matchas(MatchAs_t* n, int depth) { return PatRet(); }
PatRet CPPGen::matchor(MatchOr_t* n, int depth) { return PatRet(); }

ModRet CPPGen::module(Module_t* stmt, int depth) { return ModRet(); };
ModRet CPPGen::interactive(Interactive_t* n, int depth) { return ModRet(); }
ModRet CPPGen::functiontype(FunctionType_t* n, int depth) { return ModRet(); }
ModRet CPPGen::expression(Expression_t* n, int depth) { return ModRet(); }

}  // namespace lython
