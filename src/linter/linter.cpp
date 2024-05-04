#include "linter/linter.h"
#include "builtin/operators.h"
#include "utilities/guard.h"
#include "utilities/printing.h"
#include "utilities/strings.h"

namespace lython {

void LinterAnalyser::boolop(BoolOp* n, int depth) {}
void LinterAnalyser::namedexpr(NamedExpr* n, int depth) {}
void LinterAnalyser::compare(Compare* n, int depth) {}
void LinterAnalyser::binop(BinOp* n, int depth) {}
void LinterAnalyser::unaryop(UnaryOp* n, int depth) {}
void LinterAnalyser::lambda(Lambda* n, int depth) {}
void LinterAnalyser::ifexp(IfExp* n, int depth) {}
void LinterAnalyser::dictexpr(DictExpr* n, int depth) {}
void LinterAnalyser::setexpr(SetExpr* n, int depth) {}
void LinterAnalyser::listcomp(ListComp* n, int depth) {}
void LinterAnalyser::generateexpr(GeneratorExp* n, int depth) {}
void LinterAnalyser::setcomp(SetComp* n, int depth) {}

void LinterAnalyser::dictcomp(DictComp* n, int depth) {}
void LinterAnalyser::await(Await* n, int depth) {}
void LinterAnalyser::yield(Yield* n, int depth) {}
void LinterAnalyser::yieldfrom(YieldFrom* n, int depth) {}

Node* LinterAnalyser::load_name(Name_t* n) {
    if (n == nullptr) {
        return nullptr;
    }

    Node* result = nullptr;
    int   varid  = 0;

    if (n->dynamic) {
        // Local variables | Arguments
        lyassert(n->offset != -1, "Reference should have a reverse lookup offset");
        varid  = int(bindings.bindings.size()) - n->offset;
        result = bindings.get_value(varid);
    } else {
        // Global variables
        result = bindings.get_value(n->varid);
    }

    return result;
}

void LinterAnalyser::call(Call* n, int depth) {}
void LinterAnalyser::joinedstr(JoinedStr* n, int depth) {}
void LinterAnalyser::formattedvalue(FormattedValue* n, int depth) {}
void LinterAnalyser::constant(Constant* n, int depth) {}

// This is only called when loading
void LinterAnalyser::attribute(Attribute* n, int depth) {}
void LinterAnalyser::attribute_assign(Attribute* n, int depth, void expected) {}

void LinterAnalyser::subscript(Subscript* n, int depth) {}
void LinterAnalyser::starred(Starred* n, int depth) {}
void LinterAnalyser::name(Name* n, int depth) {}
void LinterAnalyser::listexpr(ListExpr* n, int depth) {}
void LinterAnalyser::tupleexpr(TupleExpr* n, int depth) {}
void LinterAnalyser::slice(Slice* n, int depth) {}

void LinterAnalyser::functiondef(FunctionDef* n, int depth) {}

void LinterAnalyser::classdef(ClassDef* n, int depth) {}
void LinterAnalyser::returnstmt(Return* n, int depth) {}
void LinterAnalyser::deletestmt(Delete* n, int depth) {}
void LinterAnalyser::assign(Assign* n, int depth) {}
void LinterAnalyser::augassign(AugAssign* n, int depth) {}

//! Annotation takes priority over the deduced type
//! this enbles users to use annotation to debug
void LinterAnalyser::annassign(AnnAssign* n, int depth) {}
void LinterAnalyser::forstmt(For* n, int depth) {}
void LinterAnalyser::whilestmt(While* n, int depth) {}
void LinterAnalyser::ifstmt(If* n, int depth) {}
void LinterAnalyser::with(With* n, int depth) {}
void LinterAnalyser::raise(Raise* n, int depth) {}
void LinterAnalyser::trystmt(Try* n, int depth) {}
void LinterAnalyser::assertstmt(Assert* n, int depth) {}

// This means the binding lookup for variable should stop before the global scope :/
void LinterAnalyser::global(Global* n, int depth) {}
void LinterAnalyser::nonlocal(Nonlocal* n, int depth) {}
void LinterAnalyser::exprstmt(Expr* n, int depth) {}
void LinterAnalyser::pass(Pass* n, int depth) {}
void LinterAnalyser::breakstmt(Break* n, int depth) {}
void LinterAnalyser::continuestmt(Continue* n, int depth) {}
void LinterAnalyser::match(Match* n, int depth) {}
void LinterAnalyser::inlinestmt(Inline* n, int depth) {}

void LinterAnalyser::matchvalue(MatchValue* n, int depth) {}
void LinterAnalyser::matchsingleton(MatchSingleton* n, int depth) {}
void LinterAnalyser::matchsequence(MatchSequence* n, int depth) {}
void LinterAnalyser::matchmapping(MatchMapping* n, int depth) {}
void LinterAnalyser::matchclass(MatchClass* n, int depth) {}
void LinterAnalyser::matchstar(MatchStar* n, int depth) {}
void LinterAnalyser::matchas(MatchAs* n, int depth) {}
void LinterAnalyser::matchor(MatchOr* n, int depth) {}

void LinterAnalyser::dicttype(DictType* n, int depth) {}
void LinterAnalyser::arraytype(ArrayType* n, int depth) {}
void LinterAnalyser::arrow(Arrow* n, int depth) {}
void LinterAnalyser::builtintype(BuiltinType* n, int depth) {}
void LinterAnalyser::tupletype(TupleType* n, int depth) {}
void LinterAnalyser::settype(SetType* n, int depth) {}
void LinterAnalyser::classtype(ClassType* n, int depth) {}

void LinterAnalyser::module(Module* stmt, int depth){};

void LinterAnalyser::interactive(Interactive* n, int depth) {}
void LinterAnalyser::functiontype(FunctionType* n, int depth) {}
void LinterAnalyser::expression(Expression* n, int depth) {}

}  // namespace lython
