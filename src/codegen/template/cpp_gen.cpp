#include "codegen/cpp/cpp_gen.h"
#include "ast/magic.h"
#include "builtin/operators.h"
#include "utilities/guard.h"
#include "utilities/strings.h"

namespace lython {

using StmtRet = CPPGen::Trait::StmtRet;
using ExprRet = CPPGen::Trait::ExprRet;
using ModRet  = CPPGen::Trait::ModRet;
using PatRet  = CPPGen::Trait::PatRet;

ExprRet CPPGen::boolop(BoolOp* n, int depth) {}
ExprRet CPPGen::namedexpr(NamedExpr* n, int depth) {}
ExprRet CPPGen::compare(Compare* n, int depth) {}
ExprRet CPPGen::binop(BinOp* n, int depth) {}
ExprRet CPPGen::unaryop(UnaryOp* n, int depth) {}
ExprRet CPPGen::lambda(Lambda* n, int depth) {}
ExprRet CPPGen::ifexp(IfExp* n, int depth) {}
ExprRet CPPGen::dictexpr(DictExpr* n, int depth) {}
ExprRet CPPGen::setexpr(SetExpr* n, int depth) {}
ExprRet CPPGen::listcomp(ListComp* n, int depth) {}
ExprRet CPPGen::generateexpr(GeneratorExp* n, int depth) {}
ExprRet CPPGen::setcomp(SetComp* n, int depth) {}
ExprRet CPPGen::dictcomp(DictComp* n, int depth) {}
ExprRet CPPGen::await(Await* n, int depth) {}
ExprRet CPPGen::yield(Yield* n, int depth) {}
ExprRet CPPGen::yieldfrom(YieldFrom* n, int depth) {}
ExprRet CPPGen::call(Call* n, int depth) {}
ExprRet CPPGen::joinedstr(JoinedStr* n, int depth) {}
ExprRet CPPGen::formattedvalue(FormattedValue* n, int depth) {}
ExprRet CPPGen::constant(Constant* n, int depth) {}
ExprRet CPPGen::attribute(Attribute* n, int depth) {}
ExprRet CPPGen::subscript(Subscript* n, int depth) {}
ExprRet CPPGen::starred(Starred* n, int depth) {}
ExprRet CPPGen::name(Name* n, int depth) {}
ExprRet CPPGen::listexpr(ListExpr* n, int depth) {}
ExprRet CPPGen::tupleexpr(TupleExpr* n, int depth) {}
ExprRet CPPGen::slice(Slice* n, int depth) {}

StmtRet CPPGen::comment(Comment* n, int depth) {}
StmtRet CPPGen::functiondef(FunctionDef* n, int depth) {}
StmtRet CPPGen::classdef(ClassDef* n, int depth) {}
StmtRet CPPGen::invalidstmt(InvalidStatement_t* n, int depth) {}
StmtRet CPPGen::returnstmt(Return* n, int depth) {}
StmtRet CPPGen::deletestmt(Delete* n, int depth) {}
StmtRet CPPGen::assign(Assign* n, int depth) {}
StmtRet CPPGen::augassign(AugAssign* n, int depth) {}
StmtRet CPPGen::annassign(AnnAssign* n, int depth) {}
StmtRet CPPGen::forstmt(For* n, int depth) {}
StmtRet CPPGen::whilestmt(While* n, int depth) {}
StmtRet CPPGen::ifstmt(If* n, int depth) {}
StmtRet CPPGen::with(With* n, int depth) {}
StmtRet CPPGen::raise(Raise* n, int depth) {}
StmtRet CPPGen::trystmt(Try* n, int depth) {}
StmtRet CPPGen::assertstmt(Assert* n, int depth) {}
StmtRet CPPGen::global(Global* n, int depth) {}
StmtRet CPPGen::nonlocal(Nonlocal* n, int depth) {}
StmtRet CPPGen::exprstmt(Expr* n, int depth) {}
StmtRet CPPGen::pass(Pass* n, int depth) {}
StmtRet CPPGen::breakstmt(Break* n, int depth) {}
StmtRet CPPGen::continuestmt(Continue* n, int depth) {}
StmtRet CPPGen::match(Match* n, int depth) {}
StmtRet CPPGen::inlinestmt(Inline* n, int depth) {}

PatRet CPPGen::matchvalue(MatchValue* n, int depth) {}
PatRet CPPGen::matchsingleton(MatchSingleton* n, int depth) {}
PatRet CPPGen::matchsequence(MatchSequence* n, int depth) {}
PatRet CPPGen::matchmapping(MatchMapping* n, int depth) {}
PatRet CPPGen::matchclass(MatchClass* n, int depth) {}
PatRet CPPGen::matchstar(MatchStar* n, int depth) {}
PatRet CPPGen::matchas(MatchAs* n, int depth) {}
PatRet CPPGen::matchor(MatchOr* n, int depth) {}

StmtRet CPPGen::dicttype(DictType* n, int depth) {}
StmtRet CPPGen::arraytype(ArrayType* n, int depth) {}
StmtRet CPPGen::arrow(Arrow* n, int depth) {}
StmtRet CPPGen::builtintype(BuiltinType* n, int depth) {}
StmtRet CPPGen::tupletype(TupleType* n, int depth) {}
StmtRet CPPGen::settype(SetType* n, int depth) {}
StmtRet CPPGen::classtype(ClassType* n, int depth) {}

ModRet CPPGen::module(Module* stmt, int depth){};
ModRet CPPGen::interactive(Interactive* n, int depth) {}

ExprRet CPPGen::functiontype(FunctionType* n, int depth) {}
ExprRet CPPGen::expression(Expression* n, int depth) {}

}  // namespace lython
