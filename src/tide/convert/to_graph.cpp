#include "tide/convert/to_graph.h"
#include "builtin/operators.h"
#include "utilities/guard.h"
#include "utilities/printing.h"
#include "utilities/strings.h"

namespace lython {

using StmtRet = ToGraph::StmtRet;
using ExprRet = ToGraph::ExprRet;
using ModRet  = ToGraph::ModRet;
using PatRet  = ToGraph::PatRet;

ExprRet ToGraph::exported(Exported_t* n, int depth) { return nullptr; }
// ExprRet ToGraph::condjump(CondJump_t* n, int depth) { return nullptr; }

ExprRet ToGraph::boolop(BoolOp_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::namedexpr(NamedExpr_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::compare(Compare_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::binop(BinOp_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    new_input(graph, n->left, depth);
    new_input(graph, n->right, depth);
    return new_output(graph, depth);
}
ExprRet ToGraph::unaryop(UnaryOp_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    new_input(graph, n->operand, depth);
    return new_output(graph, depth);
}
ExprRet ToGraph::lambda(Lambda_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::ifexp(IfExp_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    new_input(graph, n->test, depth);
    new_input(graph, n->body, depth);
    new_input(graph, n->orelse, depth);
    return new_output(graph, depth);
}
ExprRet ToGraph::dictexpr(DictExpr_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::setexpr(SetExpr_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::listcomp(ListComp_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::generateexpr(GeneratorExp_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::setcomp(SetComp_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::dictcomp(DictComp_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::await(Await_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::yield(Yield_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::yieldfrom(YieldFrom_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::call(Call_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);

    // if pure the output is the value
    // if not it is going to be the exec pin
    // but if the function returns a value it will probably get assigned
    // to a variable which has its exec pin
    //
    // The only case where the call can be "pure" is if
    // the call is inside an expression
    // this is a bit tough to visualize
    //
    // Non pure call essentially make an implicit set var
    // and cannot be nested inside expressions
    // but in python they could

    // UNREAL specific
    // NOTE: Pure do not cache the results
    // so a pure call gets called multiple times
    // while a impure call would not be cached
    // it is counter intuitive because Pure calls should always
    // return the same value i.e should be cachable
    // but non pure could return different values on different calls
    // so they should not be cached
    //
    //
    // We should change on our end the naming to cached/non-cached
    // cached will create the implicit Set node underneath

    // and check if the call is not pure
    if (current_exec_pin != nullptr) {
        GraphNodePinBase* exec_prev = new_exec_input(graph, depth);
        GraphNodePinBase* exec_next = new_exec_output(graph, depth);
        current_exec_pin            = exec_next;
    }

    return new_output(graph, depth);
}
ExprRet ToGraph::placeholder(Placeholder_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::import(Import_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::importfrom(ImportFrom_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::joinedstr(JoinedStr_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::formattedvalue(FormattedValue_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::constant(Constant_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::attribute(Attribute_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::subscript(Subscript_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::starred(Starred_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::name(Name_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::listexpr(ListExpr_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::tupleexpr(TupleExpr_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::slice(Slice_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::dicttype(DictType_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::arraytype(ArrayType_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::arrow(Arrow_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::builtintype(BuiltinType_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::tupletype(TupleType_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::settype(SetType_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::classtype(ClassType_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}
ExprRet ToGraph::comment(Comment_t* n, int depth) {
    GraphNodeBase* graph = new_object<GraphNode>(n);
    return new_output(graph, depth);
}

StmtRet ToGraph::functiondef(FunctionDef_t* n, int depth) {
    // Create a node for the arguments
    GraphNodeBase*    args      = new_object<GraphNode>(n);
    GraphNodePinBase* exec_next = new_exec_output(args, depth);

    for (Arg& arg: n->args.args) {
        GraphNodePinBase* output = new_output(args, depth);
        // HERE insert for lookup
    }

    for (StmtNode* stmt: n->body) {
        current_exec_pin = exec_next;
        current_exec_pin = exec(stmt, depth);
    }

    // returns should already have been created though
    // GraphNodeBase* returns = new_object<GraphNode>(n);
    // GraphNodePinBase* returned_value = new_input(args, nullptr, depth);

    return nullptr;
}
StmtRet ToGraph::classdef(ClassDef_t* n, int depth) {
    // Lets not support this
    return StmtRet();
}

StmtRet ToGraph::invalidstmt(InvalidStatement_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);

    //
    GraphNodePinBase* exec_next = new_exec_output(graph, depth);
    return exec_next;
}

StmtRet ToGraph::returnstmt(Return_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);
    return nullptr;
}
StmtRet ToGraph::deletestmt(Delete_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);

    //
    GraphNodePinBase* exec_next = new_exec_output(graph, depth);
    return exec_next;
}
StmtRet ToGraph::assign(Assign_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);

    //
    GraphNodePinBase* exec_next = new_exec_output(graph, depth);
    return exec_next;
}
StmtRet ToGraph::augassign(AugAssign_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);

    //
    GraphNodePinBase* exec_next = new_exec_output(graph, depth);
    return exec_next;
}
StmtRet ToGraph::annassign(AnnAssign_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);

    //
    GraphNodePinBase* exec_next = new_exec_output(graph, depth);
    return exec_next;
}
StmtRet ToGraph::forstmt(For_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);
    GraphNodePinBase* exec_body = new_exec_output(graph, depth);

    for (StmtNode* stmt: n->body) {
        current_exec_pin = exec_body;
        current_exec_pin = exec(stmt, depth);
    }

    //
    GraphNodePinBase* exec_next = new_exec_output(graph, depth);
    return exec_next;
}
StmtRet ToGraph::whilestmt(While_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);
    GraphNodePinBase* exec_body = new_exec_output(graph, depth);
    GraphNodePinBase* exec_cond = new_exec_output(graph, depth);

    //
    GraphNodePinBase* exec_next = new_exec_output(graph, depth);
    return exec_next;
}

StmtRet ToGraph::ifstmt(If_t* n, int depth) {
    GraphNodeBase* graph     = new_object<GraphNode>(n);
    auto           exec_prev = new_exec_input(graph, depth);
    new_input(graph, n->test, depth);

    GraphNodePinBase* exec_true = new_exec_output(graph, depth);
    for (StmtNode* stmt: n->body) {
        // When processing body we could insert a Sequential Node
        // to tidy up the code a bit
        current_exec_pin = exec_true;
        exec_true        = exec(stmt, depth);
        current_exec_pin = exec_true;
    }

    GraphNodePinBase* exec_false = new_exec_output(graph, depth);
    for (StmtNode* stmt: n->orelse) {
        current_exec_pin = exec_false;
        exec_false       = exec(stmt, depth);
        current_exec_pin = exec_false;
    }

    // Join the flow
    GraphNodeBase*    join    = new_object<GraphNode>(n);
    GraphNodePinBase* join_in = new_exec_input(join, depth);
    join_in->pins().push_back(exec_true);
    join_in->pins().push_back(exec_false);

    //
    return new_exec_output(join, depth);
}
StmtRet ToGraph::with(With_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);
    GraphNodePinBase* exec_body = new_exec_output(graph, depth);

    for (StmtNode* stmt: n->body) {
        current_exec_pin = exec_body;
        current_exec_pin = exec(stmt, depth);
    }
    //
    GraphNodePinBase* exec_next = new_exec_output(graph, depth);
    return exec_next;
}

StmtRet ToGraph::raise(Raise_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);
    return nullptr;
}
StmtRet ToGraph::trystmt(Try_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);

    GraphNodeBase*    join    = new_object<GraphNode>(n);
    GraphNodePinBase* join_in = new_exec_input(join, depth);

    GraphNodePinBase* exec_normal = new_exec_output(graph, depth);
    for (StmtNode* stmt: n->body) {
        current_exec_pin = exec_normal;
        current_exec_pin = exec(stmt, depth);
    }
    join->pins().push_back(current_exec_pin);

    for (auto& handler: n->handlers) {
        // TODO: this should match the exception expression

        GraphNodePinBase* exec_handler = new_exec_output(graph, depth);
        for (StmtNode* stmt: handler.body) {
            current_exec_pin = exec_handler;
            current_exec_pin = exec(stmt, depth);
        }
        join->pins().push_back(current_exec_pin);
    }

    GraphNodePinBase* exec_orelse = new_exec_output(graph, depth);
    for (StmtNode* stmt: n->orelse) {
        current_exec_pin = exec_orelse;
        current_exec_pin = exec(stmt, depth);
    }
    join->pins().push_back(current_exec_pin);

    GraphNodePinBase* exec_finalbody = new_exec_output(graph, depth);
    for (StmtNode* stmt: n->finalbody) {
        current_exec_pin = exec_finalbody;
        current_exec_pin = exec(stmt, depth);
    }
    join->pins().push_back(current_exec_pin);

    return new_exec_output(join, depth);
}
StmtRet ToGraph::assertstmt(Assert_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);

    //
    GraphNodePinBase* exec_next = new_exec_output(graph, depth);
    return exec_next;
}
StmtRet ToGraph::global(Global_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);

    //
    GraphNodePinBase* exec_next = new_exec_output(graph, depth);
    return exec_next;
}
StmtRet ToGraph::nonlocal(Nonlocal_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);

    //
    GraphNodePinBase* exec_next = new_exec_output(graph, depth);
    return exec_next;
}
StmtRet ToGraph::exprstmt(Expr_t* n, int depth) {
    // Hum, is this possible
    return exec(n->value, depth);
}

StmtRet ToGraph::pass(Pass_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);
    return nullptr;
}
StmtRet ToGraph::breakstmt(Break_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);
    return nullptr;
}
StmtRet ToGraph::continuestmt(Continue_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);
    return nullptr;
}
StmtRet ToGraph::match(Match_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);

    // Joined
    GraphNodeBase* join    = new_object<GraphNode>(n);
    auto           join_in = new_exec_input(join, depth);

    // All the match case are a new output execution pin
    for (MatchCase& match: n->cases) {
        GraphNodePinBase* output = new_exec_output(graph, depth);

        // the match pattern is an input
        // TODO

        // branching
        for (StmtNode* stmt: match.body) {
            current_exec_pin = output;
            current_exec_pin = exec(stmt, depth);
        }

        join_in->pins().push_back(current_exec_pin);
        current_exec_pin = nullptr;
    }

    //
    return new_exec_output(join, depth);
}

StmtRet ToGraph::inlinestmt(Inline_t* n, int depth) {
    GraphNodeBase*    graph     = new_object<GraphNode>(n);
    GraphNodePinBase* exec_prev = new_exec_input(graph, depth);

    for (StmtNode* stmt: n->body) {
        current_exec_pin = exec(stmt, depth);
    }
    //
    return new_exec_output(graph, depth);
}

PatRet ToGraph::matchvalue(MatchValue_t* n, int depth) { return PatRet(); }
PatRet ToGraph::matchsingleton(MatchSingleton_t* n, int depth) { return PatRet(); }
PatRet ToGraph::matchsequence(MatchSequence_t* n, int depth) { return PatRet(); }
PatRet ToGraph::matchmapping(MatchMapping_t* n, int depth) { return PatRet(); }
PatRet ToGraph::matchclass(MatchClass_t* n, int depth) { return PatRet(); }
PatRet ToGraph::matchstar(MatchStar_t* n, int depth) { return PatRet(); }
PatRet ToGraph::matchas(MatchAs_t* n, int depth) { return PatRet(); }
PatRet ToGraph::matchor(MatchOr_t* n, int depth) { return PatRet(); }

ModRet ToGraph::module(Module_t* mod, int depth) {
    for (StmtNode* stmt: mod->body) {
        current_exec_pin = exec(stmt, depth);
    }
};
ModRet ToGraph::interactive(Interactive_t* n, int depth) { return ModRet(); }
ModRet ToGraph::functiontype(FunctionType_t* n, int depth) { return ModRet(); }
ModRet ToGraph::expression(Expression_t* n, int depth) { return ModRet(); }

}  // namespace lython
