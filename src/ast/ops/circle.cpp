#include "utilities/printing.h"
#include "ast/nodes.h"
#include "ast/visitor.h"
#include "compatibility/compatibility.h"
#include "dependencies/fmt.h"
#include "lexer/unlex.h"
#include "logging/logging.h"
#include "parser/parsing_error.h"
#include "utilities/strings.h"

namespace lython {

struct CircleTrait {
    using Trace   = std::true_type;
    using StmtRet = bool;
    using ExprRet = bool;
    using ModRet  = bool;
    using PatRet  = bool;

    enum
    { MaxRecursionDepth = LY_MAX_VISITOR_RECURSION_DEPTH };
};

#define ReturnType bool

// Circle should not happen
// Weird things can happen during sema where we create/resolve types
// this is here so we can do a sanity check and prevent stack overflows while debugging
struct Circle: BaseVisitor<Circle, true, CircleTrait> {
    using Super = BaseVisitor<Circle, true, CircleTrait>;
    Array<Node const*> visited;

    bool in(Node const* obj, int depth) {
        if (obj == nullptr) {
            return false;
        }

        for (Node const* item: visited) {
            if (item == obj) {
                kwtrace(outlog(), depth, "Duplicate is: {}", meta::type_name(item->class_id));
                return true;
            }
        }

        if (cast<Name>(obj) == nullptr) {
            visited.push_back(obj);
        }
        return false;
    }

    ReturnType exec(ModNode_t* mod, int depth) {
        if (in(mod, depth)) {
            return true;
        }

        return Super::exec(mod, depth);
    }
    ReturnType exec(Pattern_t* pat, int depth) {
        if (in(pat, depth)) {
            return true;
        }

        return Super::exec(pat, depth);
    }
    ReturnType exec(ExprNode_t* expr, int depth) {
        if (in(expr, depth)) {
            return true;
        }

        return Super::exec(expr, depth);
    }
    ReturnType exec(StmtNode_t* stmt, int depth) {
        if (in(stmt, depth)) {
            return true;
        }

        return Super::exec(stmt, depth);
    }

    template <typename T> 
    bool any_of(Array<T> const& elts, int depth) {
        // std::any_of produce so much asm crap in gcc-12 & clang-15
        // zig++ generates fine code though
        // msvc generates the worst code by far
        for (auto const& elt: elts) {  // NOLINT
            if (exec(elt, depth))
                return true;
        }
        return false;
    }

    ReturnType exec_body(Array<StmtNode*> const& body, int depth) { return any_of(body, depth); }

    ReturnType excepthandler(ExceptHandler const& self, int depth) {
        if (self.type.has_value()) {
            if (exec(self.type.value(), depth))
                return true;
        }

        return exec(self.comment, depth) || exec_body(self.body, depth + 1);
    }

    ReturnType matchcase(MatchCase const& self, int depth) {
        if (exec(self.pattern, depth)) {
            return true;
        }

        if (self.guard.has_value()) {
            if (exec(self.guard.value(), depth))
                return true;
        }

        return exec(self.comment, depth) || exec_body(self.body, depth + 1);
    }

    bool comprehension(Comprehension const& self, int depth);
    bool arguments(Arguments const& self, int depth);
    bool withitem(WithItem const& self, int depth);
    bool alias(Alias const& self, int depth);
    bool keyword(Keyword const& self, int depth);

    bool arg(Arg const& self, int depth) {
        if (self.annotation.has_value()) {
            if (exec(self.annotation.value(), depth)) {
                return true;
            }
        }
        return false;
    }

#define FUNCTION_GEN(name, fun, rtype) rtype fun(const name* node, int depth);

#define X(name, _)
#define SECTION(name)
#define EXPR(name, fun)  FUNCTION_GEN(name, fun, ReturnType)
#define STMT(name, fun)  FUNCTION_GEN(name, fun, ReturnType)
#define MOD(name, fun)   FUNCTION_GEN(name, fun, ReturnType)
#define MATCH(name, fun) FUNCTION_GEN(name, fun, ReturnType)
#define VM(name, fun) 

    NODEKIND_ENUM(X, SECTION, EXPR, STMT, MOD, MATCH, VM)

#undef X
#undef SECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef FUNCTION_GEN
} LY_ALIGN(32);

ReturnType Circle::attribute(Attribute const* self, int depth) { return exec(self->value, depth); }

ReturnType Circle::subscript(Subscript const* self, int depth) {
    return exec(self->value, depth) || exec(self->slice, depth);
}

ReturnType Circle::starred(Starred const* self, int depth) { return exec(self->value, depth); }

ReturnType Circle::module(Module const* self, int depth) { return exec_body(self->body, depth); }

ReturnType Circle::raise(Raise const* self, int depth) {
    if (self->exc.has_value()) {
        if (exec(self->exc.value(), depth))
            return true;
    }

    if (self->cause.has_value()) {
        if (exec(self->cause.value(), depth)) {
            return true;
        }
    }
    return false;
}

ReturnType Circle::assertstmt(Assert const* self, int depth) {
    if (exec(self->test, depth)) {
        return true;
    }

    if (self->msg.has_value()) {
        if (exec(self->msg.value(), depth)) {
            return true;
        }
    }

    return false;
}

ReturnType Circle::with(With const* self, int depth) {
    for (auto const& item: self->items) {
        if (exec(item.context_expr, depth)) {
            return true;
        }

        if (item.optional_vars.has_value()) {
            if (exec(item.optional_vars.value(), depth)) {
                return true;
            }
        }
    }

    return exec(self->comment, depth) || exec_body(self->body, depth + 1);
}

ReturnType Circle::import(Import const* self, int depth) { return false; }

ReturnType Circle::importfrom(ImportFrom const* self, int depth) { return false; }

ReturnType Circle::slice(Slice const* self, int depth) {
    if (self->lower.has_value()) {
        if (exec(self->lower.value(), depth)) {
            return true;
        }
    }

    if (self->upper.has_value()) {
        if (exec(self->upper.value(), depth))
            return true;
    }

    if (self->step.has_value()) {
        if (exec(self->step.value(), depth))
            return true;
    }
    return false;
}

ReturnType Circle::tupleexpr(TupleExpr const* self, int depth) { return any_of(self->elts, depth); }

ReturnType Circle::listexpr(ListExpr const* self, int depth) { return any_of(self->elts, depth); }

ReturnType Circle::setexpr(SetExpr const* self, int depth) { return any_of(self->elts, depth); }

ReturnType Circle::dictexpr(DictExpr const* self, int depth) {
    for (int i = 0; i < self->keys.size(); i++) {
        if (exec(self->keys[i], depth) || exec(self->values[i], depth)) {
            return true;
        }
    }
    return false;
}

ReturnType Circle::matchvalue(MatchValue const* self, int depth) {
    return exec(self->value, depth);
}

ReturnType Circle::matchsingleton(MatchSingleton const* self, int depth) { return false; }

ReturnType Circle::matchsequence(MatchSequence const* self, int depth) {
    return any_of(self->patterns, depth);
}

ReturnType Circle::matchmapping(MatchMapping const* self, int depth) {
    for (int i = 0; i < self->keys.size(); i++) {
        if (exec(self->keys[i], depth) || exec(self->patterns[i], depth)) {
            return true;
        }
    }
    return false;
}

ReturnType Circle::matchclass(MatchClass const* self, int depth) {
    return exec(self->cls, depth) ||         //
           any_of(self->patterns, depth) ||  //
           any_of(self->kwd_patterns, depth);
}

ReturnType Circle::matchstar(MatchStar const* self, int depth) { return false; }

ReturnType Circle::matchas(MatchAs const* self, int depth) {
    if (self->pattern.has_value()) {
        if (exec(self->pattern.value(), depth)) {
            return true;
        }
    }
    return false;
}

ReturnType Circle::matchor(MatchOr const* self, int depth) { return any_of(self->patterns, depth); }

ReturnType Circle::ifstmt(If const* self, int depth) {

    if (exec(self->test, depth) || exec(self->comment, depth) || exec_body(self->body, depth + 1)) {
        return true;
    }

    for (int i = 0; i < self->tests.size(); i++) {
        auto const& eliftest = self->tests[i];
        auto const& elifbody = self->bodies[i];

        if (exec(eliftest, depth) || exec(self->tests_comment[i], depth) ||
            exec_body(elifbody, depth + 1)) {
            return true;
        }
    }

    if (!self->orelse.empty()) {
        if (exec(self->else_comment, depth) || exec_body(self->orelse, depth + 1)) {
            return true;
        }
    }
    return false;
}

ReturnType Circle::match(Match const* self, int depth) {
    if (exec(self->subject, depth) || exec(self->comment, depth)) {
        return true;
    }

    for (auto& case_: self->cases) {  // NOLINT
        if (matchcase(case_, depth + 1)) {
            return true;
        }
    }
    return false;
}

ReturnType Circle::lambda(Lambda const* self, int depth) {
    return arguments(self->args, depth) || exec(self->body, depth);
}

ReturnType Circle::ifexp(IfExp const* self, int depth) {
    return exec(self->body, depth) || exec(self->test, depth) || exec(self->orelse, depth);
}

ReturnType Circle::listcomp(ListComp const* self, int depth) {
    if (exec(self->elt, depth)) {
        return true;
    }

    for (int i = 0; i < self->generators.size(); i++) {
        if (comprehension(self->generators[i], depth)) {
            return true;
        }
    }
    return false;
}

ReturnType Circle::setcomp(SetComp const* self, int depth) {
    if (exec(self->elt, depth)) {
        return true;
    }

    for (int i = 0; i < self->generators.size(); i++) {
        if (comprehension(self->generators[i], depth)) {
            return true;
        }
    }
    return false;
}

ReturnType Circle::generateexpr(GeneratorExp const* self, int depth) {
    if (exec(self->elt, depth)) {
        return true;
    }

    for (int i = 0; i < self->generators.size(); i++) {
        if (comprehension(self->generators[i], depth)) {
            return true;
        }
    }
    return false;
}

ReturnType Circle::dictcomp(DictComp const* self, int depth) {

    if (exec(self->key, depth) || exec(self->value, depth)) {
        return true;
    }

    for (int i = 0; i < self->generators.size(); i++) {
        if (comprehension(self->generators[i], depth)) {
            return true;
        }
    }
    return false;
}

ReturnType Circle::await(Await const* self, int depth) { return exec(self->value, depth); }

ReturnType Circle::yield(Yield const* self, int depth) {
    return self->value.has_value() && exec(self->value.value(), depth);
}

ReturnType Circle::yieldfrom(YieldFrom const* self, int depth) { return exec(self->value, depth); }

ReturnType Circle::call(Call const* self, int depth) {
    if (exec(self->func, depth) || any_of(self->args, depth)) {
        return true;
    }

    for (int i = 0; i < self->keywords.size(); i++) {
        auto const& key = self->keywords[i];

        if (exec(key.value, depth)) {
            return true;
        }
    }

    return false;
}

ReturnType Circle::constant(Constant const* self, int depth) { return false; }

Circle::ExprRet Circle::placeholder(Placeholder_t* self, int depth) {
    return false;
} 

ReturnType Circle::namedexpr(NamedExpr const* self, int depth) {
    return exec(self->target, depth) || exec(self->value, depth);
}

ReturnType Circle::classdef(ClassDef const* self, int depth) {
    for (auto decorator: self->decorator_list) {
        if (exec(decorator.expr, depth) || exec(decorator.comment, depth)) {
            return true;
        }
    }

    for (auto kw: self->keywords) {  // NOLINT
        if (exec(kw.value, depth)) {
            return true;
        }
    }

    if (any_of(self->bases, depth) || exec(self->comment, depth)) {
        return true;
    }

    if (self->docstring.has_value() && exec(self->docstring.value().comment, depth)) {
        return true;
    }

    for (auto& stmt: self->body) {  // NOLINT
        if (exec(stmt, depth + 1) || exec(stmt->comment, depth)) {
            return true;
        }
    }
    return false;
}

ReturnType Circle::functiondef(FunctionDef const* self, int depth) {
    for (auto decorator: self->decorator_list) {
        if (exec(decorator.expr, depth) || exec(decorator.comment, depth)) {
            return true;
        }
    }

    if (arguments(self->args, depth)) {
        return true;
    }

    if (self->returns.has_value() && exec(self->returns.value(), depth)) {
        return true;
    }

    if (exec(self->comment, depth)) {
        return true;
    }

    if (self->docstring.has_value() && exec(self->docstring.value().comment, depth)) {
        return true;
    }

    return exec_body(self->body, depth + 1) || exec(self->type, depth);
}

ReturnType Circle::inlinestmt(Inline const* self, int depth) {
    return exec_body(self->body, depth);
}

ReturnType Circle::forstmt(For const* self, int depth) {
    if (exec(self->target, depth) || exec(self->iter, depth) || exec(self->comment, depth)) {
        return true;
    }

    if (exec_body(self->body, depth + 1)) {
        return true;
    }

    if (!self->orelse.empty()) {
        if (exec(self->else_comment, depth) || exec_body(self->orelse, depth + 1)) {
            return true;
        }
    }

    return false;
}

ReturnType Circle::trystmt(Try const* self, int depth) {
    if (exec(self->comment, depth) || exec_body(self->body, depth + 1)) {
        return true;
    }

    for (auto const& handler: self->handlers) {
        if (excepthandler(handler, depth)) {
            return true;
        }
    }

    if (!self->orelse.empty()) {
        if (exec(self->else_comment, depth) || exec_body(self->orelse, depth + 1)) {
            return true;
        }
    }

    if (!self->finalbody.empty()) {

        if (exec(self->finally_comment, depth) || exec_body(self->finalbody, depth + 1)) {
            return true;
        }
    }

    return false;
}

ReturnType Circle::compare(Compare const* self, int depth) {
    if (exec(self->left, depth)) {
        return true;
    }

    int n = int(self->comparators.size());
    for (int i = 0; i < self->ops.size(); i++) {

        if (i < n && exec(self->comparators[i], depth)) {
            return true;
        }
    }

    return false;
}

ReturnType Circle::binop(BinOp const* self, int depth) {
    return exec(self->left, depth) || exec(self->right, depth);
}

ReturnType Circle::invalidstmt(InvalidStatement const* self, int depth) { return false; }

ReturnType Circle::boolop(BoolOp const* self, int depth) {

    int m = self->opcount + 1;
    int n = int(self->values.size());

    for (int i = 0; i < m; i++) {

        if (i < n && exec(self->values[i], depth)) {
            return true;
        }
    }

    return false;
}

ReturnType Circle::unaryop(UnaryOp const* self, int depth) { return exec(self->operand, depth); }

ReturnType Circle::whilestmt(While const* self, int depth) {
    if (exec(self->test, depth) || exec(self->comment, depth) || exec_body(self->body, depth + 1)) {
        return true;
    }

    if (!self->orelse.empty()) {
        return exec(self->else_comment, depth) || exec_body(self->orelse, depth + 1);
    }

    return false;
}

ReturnType Circle::returnstmt(Return const* self, int depth) {
    if (self->value.has_value()) {
        if (exec(self->value.value(), depth)) {
            return true;
        }
    }

    return false;
}

ReturnType Circle::deletestmt(Delete const* self, int depth) {
    for (int i = 0; i < self->targets.size(); i++) {
        if (exec(self->targets[i], depth)) {
            return true;
        }
    }
    return false;
}

ReturnType Circle::augassign(AugAssign const* self, int depth) {
    return exec(self->target, depth) || exec(self->value, depth);
}

ReturnType Circle::assign(Assign const* self, int depth) {
    return exec(self->targets[0], depth) || exec(self->value, depth);
}

ReturnType Circle::annassign(AnnAssign const* self, int depth) {
    if (exec(self->target, depth) || exec(self->annotation, depth)) {
        return true;
    }

    if (self->value.has_value()) {
        if (exec(self->value.value(), depth)) {
            return true;
        }
    }

    return false;
}

ReturnType Circle::pass(Pass const* self, int depth) { return false; }

ReturnType Circle::breakstmt(Break const* self, int depth) { return false; }

ReturnType Circle::continuestmt(Continue const* self, int depth) { return false; }

ReturnType Circle::exprstmt(Expr const* self, int depth) {
    if (self->value != nullptr)
        if (exec(self->value, depth)) {
            return true;
        }

    return false;
}

ReturnType Circle::global(Global const* self, int depth) { return false; }

ReturnType Circle::nonlocal(Nonlocal const* self, int depth) { return false; }

ReturnType Circle::arrow(Arrow const* self, int depth) {
    for (ExprNode* node: self->args) {
        if (exec(node, depth)) {
            return true;
        }
    }
    return exec(self->returns, depth);
}

ReturnType Circle::dicttype(DictType const* self, int depth) {
    return exec(self->key, depth) || exec(self->value, depth);
}

ReturnType Circle::settype(SetType const* self, int depth) { return false; }

ReturnType Circle::name(Name const* self, int depth) { 
    //
    return false; 
}

ReturnType Circle::arraytype(ArrayType const* self, int depth) { return exec(self->value, depth); }

ReturnType Circle::tupletype(TupleType const* self, int depth) {
    return any_of(self->types, depth);
}

ReturnType Circle::builtintype(BuiltinType const* self, int depth) { return false; }

ReturnType Circle::joinedstr(JoinedStr const* self, int depth) { return false; }

ReturnType Circle::formattedvalue(FormattedValue const* self, int depth) { return false; }

ReturnType Circle::classtype(ClassType const* self, int depth) { return false; }

// Helper
// ==================================================

bool Circle::comprehension(Comprehension const& self, int depth) {

    return exec(self.target, depth) ||  //
           exec(self.iter, depth) ||    //
           any_of(self.ifs, depth);
}

bool Circle::keyword(Keyword const& self, int depth) {
    return (self.value != nullptr) && exec(self.value, depth);
}

bool Circle::alias(Alias const& self, int depth) { return false; }

ReturnType Circle::functiontype(FunctionType const* self, int depth) { return false; }

ReturnType Circle::expression(Expression const* self, int depth) { return false; }

ReturnType Circle::interactive(Interactive const* self, int depth) { return false; }

bool Circle::withitem(WithItem const& self, int depth) {
    if (exec(self.context_expr, depth)) {
        return true;
    }

    if (self.optional_vars.has_value()) {

        if (exec(self.optional_vars.value(), depth)) {
            return true;
        }
    }

    return false;
}

ReturnType Circle::comment(Comment const* n, int depth) { return false; }


ReturnType Circle::exported(Exported const * n, int depth) {
    //return exec(n->node, depth);
    return false;
}


bool Circle::arguments(Arguments const& self, int depth) {
    int i = 0;

    for (auto const& arg: self.args) {
        if (arg.annotation.has_value() && exec(arg.annotation.value(), depth)) {
            return true;
        }

        auto default_offset = self.args.size() - 1 - i;
        if (!self.defaults.empty() && default_offset < self.defaults.size()) {
            if (exec(self.defaults[default_offset], depth)) {
                return true;
            }
        }

        i += 1;
    }

    i = 0;
    for (auto const& kw: self.kwonlyargs) {
        if (kw.annotation.has_value()) {
            if (exec(kw.annotation.value(), depth)) {
                return true;
            }
        }

        auto default_offset = self.kwonlyargs.size() - 1 - i;
        if (!self.kw_defaults.empty() && default_offset < self.kw_defaults.size()) {
            if (exec(self.kw_defaults[default_offset], depth)) {
                return true;
            }
        }

        i += 1;
    }

    return false;
}

// ReturnType Circle::condjump(CondJump_t* n, int depth) {
//     return false; 
// }

bool has_circle(ExprNode const* obj) { return Circle().exec(obj, 0); }
bool has_circle(Pattern const* obj) { return Circle().exec(obj, 0); }
bool has_circle(StmtNode const* obj) { return Circle().exec(obj, 0); }
bool has_circle(ModNode const* obj) { return Circle().exec(obj, 0); }

}  // namespace lython
