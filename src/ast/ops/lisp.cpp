#include "ast/lisp.h"
#include "utilities/printing.h"
#include "ast/nodes.h"
#include "ast/visitor.h"
#include "logging/logging.h"
#include "utilities/strings.h"

namespace lython {

struct LispSexpTrait {
    using Trace   = std::false_type;
    using StmtRet = Sexp;
    using ExprRet = Sexp;
    using ModRet  = Sexp;
    using PatRet  = Sexp;
};

#define ReturnType Sexp

struct LispSexp: BaseVisitor<LispSexp, true, LispSexpTrait, int> {
    using Super = BaseVisitor<LispSexp, true, LispSexpTrait, int>;

    std::ostream& out;

    String indent(int level) {
        return String(' ', level * 2);
    }

    ReturnType
    sexp_body(Array<StmtNode*> const& body, int depth, int level, bool print_last = false) {
        // (
        //      (stmt)
        //      (stmt)
        // )
        Array<Sexp> items;
        for (auto& stmt: body) {
            items.push_back(exec(stmt, depth, level));
        }

        return Sexp::list(items);
    }

    ReturnType excepthandler(ExceptHandler const& self, int depth, int level) {
        out << '\n' << indent(level) << "except ";

        if (self.type.has_value()) {
            exec(self.type.value(), depth, level);
        }

        if (self.name.has_value()) {
            out << " as ";
            out << self.name.value();
        }

        out << ":\n";
        return sexp_body(self.body, depth, level + 1);
    }

    ReturnType matchcase(MatchCase const& self, int depth, int level) {
        out << indent(level) << "case ";
        exec(self.pattern, depth, level);

        if (self.guard.has_value()) {
            out << " if ";
            exec(self.guard.value(), depth, level);
        }

        out << ":\n";
        return sexp_body(self.body, depth, level + 1);
    }

    void arguments(Arguments const& self, int depth, int level);
    void withitem(WithItem const& self, int depth, int level);
    void alias(Alias const& self, int depth, int level);
    void keyword(Keyword const& self, int depth, int level);

    void arg(Arg const& self, int depth, int level) {
        out << self.arg;

        if (self.annotation.has_value()) {
            out << ": ";
            exec(self.annotation.value(), depth, level);
        }
    }
#define FUNCTION_GEN(name, fun, rtype) rtype fun(const name* node, int depth, int level);

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
#undef VM

#undef FUNCTION_GEN
};

void comprehension(LispSexp& p, Comprehension const& self, int depth, int level);

ReturnType LispSexp::attribute(Attribute const* self, int depth, int level) {
    // obj.attr
    // (getattr obj "attr")
    Sexp val = exec(self->value, depth, level);
    return Sexp::list(
        Sexp::symbol("getattr"), 
        val, 
        Sexp::string(self->attr)
    );
}

ReturnType LispSexp::subscript(Subscript const* self, int depth, int level) {
    // obj[i]
    // (getitem obj i)
    Sexp slice = exec(self->slice, depth, level);

    return Sexp::list(
        Sexp::symbol("getitem"), exec(self->value, depth, level), slice);
}

ReturnType LispSexp::starred(Starred const* self, int depth, int level) {
    // *expr
    // (star expr)
    return Sexp::list(Sexp::symbol("star"), exec(self->value, depth, level));
}

ReturnType LispSexp::module(Module const* self, int depth, int level) {
    return sexp_body(self->body, depth, level);
}

ReturnType LispSexp::raise(Raise const* self, int depth, int level) {
    Array<Sexp> frags = {Sexp::symbol("raise")};

    if (self->exc.has_value()) {
        frags.push_back(exec(self->exc.value(), depth, level));
    }

    if (self->cause.has_value()) {
        frags.push_back(exec(self->cause.value(), depth, level));
    }
    // (raise exception cause)
    return Sexp::list(frags);
}

ReturnType LispSexp::assertstmt(Assert const* self, int depth, int level) {
    Array<Sexp> frags = {Sexp::symbol("assert")};

    frags.push_back(exec(self->test, depth, level));


    if (self->msg.has_value()) {
        frags.push_back(exec(self->msg.value(), depth, level));
    }

    // (assert cond "msg")
    return Sexp::list(frags);
}

ReturnType LispSexp::with(With const* self, int depth, int level) {
    // (with ((as a b) c) (
    //      body
    // ))

    Array<Sexp> frags = {Sexp::symbol("with")};

    Array<Sexp> items;
    for (auto& item: self->items) {

        auto expr = exec(item.context_expr, depth, level);

        if (item.optional_vars.has_value()) {
            // (as a b)
            Sexp name = exec(item.optional_vars.value(), depth, level);
            items.push_back(Sexp::list(Sexp::symbol("as"), expr, name));
        } else {
            items.push_back(expr);
        }
    }
    frags.push_back(Sexp::list(items));
    frags.push_back(sexp_body(self->body, depth, level + 1));
    return Sexp::list(frags);
}

ReturnType LispSexp::import(Import const* self, int depth, int level) {
    // (import (as longname short))
    Array<Sexp> frags = {Sexp::symbol("import")};

    for (auto& alias: self->names) {
        auto name = Sexp::symbol(alias.name);

        if (alias.asname.has_value()) {
            frags.push_back(Sexp::list(Sexp::symbol("as"), name, Sexp::string(alias.asname.value())));
        } else {
            frags.push_back(name);
        }
    }
    return Sexp::list(frags);
}

ReturnType LispSexp::importfrom(ImportFrom const* self, int depth, int level) {
    // (from abc (import ((as abc cdf) abc)))

    Array<Sexp> frags = {Sexp::symbol("from")};

    if (self->module.has_value()) {
        frags.push_back(Sexp::symbol(self->module.value()));
    }

    Array<Sexp> imports = {Sexp::symbol("import")};

    int i = 0;
    for (auto& alias: self->names) {
        auto name = Sexp::symbol(alias.name);

        if (alias.asname.has_value()) {
            imports.push_back(Sexp::list(Sexp::symbol("as"), name, alias.asname.value()));
        } else {
            imports.push_back(name);
        }
    }

    frags.push_back(Sexp::list(imports));
    return Sexp::list(frags);
}

ReturnType LispSexp::slice(Slice const* self, int depth, int level) {
    // (slice start end step)
    Array<Sexp> frags = {Sexp::symbol("slice")};

    if (self->lower.has_value()) {
        frags.push_back(exec(self->lower.value(), depth, level));
    }

    if (self->upper.has_value()) {
        frags.push_back(exec(self->upper.value(), depth, level));
    }

    if (self->step.has_value()) {
        frags.push_back(exec(self->step.value(), depth, level));
    }

    return Sexp::list(frags);
}

ReturnType LispSexp::tupleexpr(TupleExpr const* self, int depth, int level) {
    // (tuple ets...)
    Array<Sexp> frags = {Sexp::symbol("tuple")};

    for (auto& elem: self->elts) {
        frags.push_back(exec(elem, depth, level));
    }
    return Sexp::list(frags);
}

ReturnType LispSexp::listexpr(ListExpr const* self, int depth, int level) {
    // (list ets...)
    Array<Sexp> frags = {Sexp::symbol("list")};

    for (auto& elem: self->elts) {
        frags.push_back(exec(elem, depth, level));
    }
    return Sexp::list(frags);
}

ReturnType LispSexp::setexpr(SetExpr const* self, int depth, int level) {
    // (set ets...)
    Array<Sexp> frags = {Sexp::symbol("set")};

    for (auto& elem: self->elts) {
        frags.push_back(exec(elem, depth, level));
    }
    return Sexp::list(frags);
}

ReturnType LispSexp::dictexpr(DictExpr const* self, int depth, int level) {
    // (dict (k v)...)
    Array<Sexp> frags = {Sexp::symbol("dict")};

    for (int i = 0; i < self->keys.size(); i++) {
        auto key   = exec(self->keys[i], depth, level);
        auto value = exec(self->values[i], depth, level);

        frags.push_back(Sexp::list(key, value));
    }
    return Sexp::list(frags);
}

ReturnType LispSexp::matchvalue(MatchValue const* self, int depth, int level) {
    return exec(self->value, depth, level);
}

ReturnType LispSexp::matchsingleton(MatchSingleton const* self, int depth, int level) {
    self->value.print(out);
    return false;
}

ReturnType LispSexp::matchsequence(MatchSequence const* self, int depth, int level) {
    auto result = join(", ", self->patterns);
    out << "[" << result << "]";
    return false;
}

ReturnType LispSexp::matchmapping(MatchMapping const* self, int depth, int level) {
    Array<String> strs;
    strs.reserve(self->keys.size());

    for (int i = 0; i < self->keys.size(); i++) {
        // FIXME std::string -> String conversion
        strs.push_back(
            String(fmt::format("{}: {}", str(self->keys[i]), str(self->patterns[i])).c_str()));
    }

    String remains = "";
    if (self->rest.has_value()) {
        StringStream ss;
        ss << ", **" << self->rest.value();
        remains = ss.str();
    }

    out << "{" << join(", ", strs) << remains << "}";
    return false;
}

ReturnType LispSexp::matchclass(MatchClass const* self, int depth, int level) {
    exec(self->cls, depth, level);
    out << "(" << join(", ", self->patterns);

    if (self->patterns.size() > 0 && self->kwd_attrs.size() > 0) {
        out << ", ";
    }

    Array<String> kwdpat;
    kwdpat.reserve(self->kwd_attrs.size());

    for (int i = 0; i < self->kwd_attrs.size(); i++) {
        // FIXME std::string -> String conversion
        kwdpat.push_back(
            String(fmt::format("{}={}", self->kwd_attrs[i], str(self->kwd_patterns[i])).c_str()));
    }

    out << join(", ", kwdpat);
    out << ")";
    return false;
}

ReturnType LispSexp::matchstar(MatchStar const* self, int depth, int level) {
    out << "*";

    if (self->name.has_value()) {
        out << self->name.value();
    }
    return false;
}

ReturnType LispSexp::matchas(MatchAs const* self, int depth, int level) {
    if (self->pattern.has_value()) {
        exec(self->pattern.value(), depth, level);
    }

    if (self->name.has_value()) {
        out << " as " << self->name.value();
    }
    return false;
}

ReturnType LispSexp::matchor(MatchOr const* self, int depth, int level) {
    out << join(" | ", self->patterns);
    return false;
}

ReturnType LispSexp::ifstmt(If const* self, int depth, int level) {
    // (cond
    //      (pred result)
    //      (_    result)
    //)
    out << "if ";
    exec(self->test, depth, level);
    out << ":\n";
    sexp_body(self->body, depth, level + 1);

    for (int i = 0; i < self->tests.size(); i++) {
        auto& eliftest = self->tests[i];
        auto& elifbody = self->bodies[i];

        out << "\n" << indent(level) << "elif ";

        exec(eliftest, depth, level);
        out << ":\n";
        sexp_body(elifbody, depth, level + 1);
    }

    if (self->orelse.size()) {
        out << "\n" << indent(level) << "else:\n";
        sexp_body(self->orelse, depth, level + 1);
    }
    return false;
}

ReturnType LispSexp::match(Match const* self, int depth, int level) {
    // (match target (
    //      (pattern result)
    // ))
    out << "match ";
    exec(self->subject, depth, level);
    out << ":\n";

    int i = 0;
    for (auto& case_: self->cases) {
        matchcase(case_, depth, level + 1);

        if (i + 1 < self->cases.size()) {
            out << '\n';
        }
    }
    return false;
}

ReturnType LispSexp::lambda(Lambda const* self, int depth, int level) {
    // (lambda (x y) body)
    //
    out << "lambda ";
    arguments(self->args, depth, 0);
    out << ": ";
    exec(self->body, depth, level);
    return false;
}

ReturnType LispSexp::ifexp(IfExp const* self, int depth, int level) {
    // (if cond return else)
    out << "if ";
    exec(self->test, depth, level);
    out << ": ";
    exec(self->body, depth, level);
    out << " else ";
    exec(self->orelse, depth, level);
    return false;
}

ReturnType LispSexp::exported(Exported const* self, int depth, int level) {
    exec(self->node, depth, level);
    return false;
}


ReturnType LispSexp::listcomp(ListComp const* self, int depth, int level) {
    // (map (filter generator lambda) lambda)
    out << "[";
    exec(self->elt, depth, level);

    out << join(" ", self->generators);

    out << "]";
    return false;
}

ReturnType LispSexp::setcomp(SetComp const* self, int depth, int level) {
    // (map (filter generator lambda) lambda)
    out << "{";
    exec(self->elt, depth, level);

    out << join(" ", self->generators);

    out << "}";
    return false;
}

ReturnType LispSexp::generateexpr(GeneratorExp const* self, int depth, int level) {
    // (map (filter generator lambda) lambda)
    out << "(";
    exec(self->elt, depth, level);

    out << join(" ", self->generators);

    out << ")";
    return false;
}

ReturnType LispSexp::dictcomp(DictComp const* self, int depth, int level) {
    // (map (filter generator lambda) lambda)
    out << "{";
    exec(self->key, depth, level);
    out << ": ";
    exec(self->value, depth, level);

    out << join(" ", self->generators);
    out << "}";
    return false;
}

ReturnType LispSexp::await(Await const* self, int depth, int level) {
    // (await value)
    out << "await ";
    exec(self->value, depth, level);
    return false;
}

ReturnType LispSexp::yield(Yield const* self, int depth, int level) {
    // (yield value)
    out << "yield ";
    if (self->value.has_value()) {
        exec(self->value.value(), depth, level);
    }
    return false;
}

ReturnType LispSexp::yieldfrom(YieldFrom const* self, int depth, int level) {
    // (yield (from value))
    out << "yield from ";
    exec(self->value, depth, level);
    return false;
}

ReturnType LispSexp::call(Call const* self, int depth, int level) {
    // (function args...)
    exec(self->func, depth, level);
    out << "(";

    for (int i = 0; i < self->args.size(); i++) {
        exec(self->args[i], depth, level);

        if (i < self->args.size() - 1 || self->keywords.size() > 0)
            out << ", ";
    }

    for (int i = 0; i < self->keywords.size(); i++) {
        auto& key = self->keywords[i];

        out << self->keywords[i].arg;
        out << "=";

        exec(key.value, depth, level);

        if (i < self->keywords.size() - 1)
            out << ", ";
    }

    out << ")";
    return false;
}

ReturnType LispSexp::constant(Constant const* self, int depth, int level) {
    self->value.print(out);
    return false;
}

LispSexp::ExprRet LispSexp::placeholder(Placeholder_t self, int depth, int level) {
    return false;
} 

ReturnType LispSexp::namedexpr(NamedExpr const* self, int depth, int level) {
    exec(self->target, depth, level);
    out << " := ";
    exec(self->value, depth, level);
    return false;
}

ReturnType LispSexp::classdef(ClassDef const* self, int depth, int level) {
    int k = 0;
    for (auto decorator: self->decorator_list) {
        if (k > 0) {
            out << indent(level);
        }

        out << "@";
        exec(decorator, depth, level);
        out << "\n";
        k += 1;
    }

    if (self->decorator_list.size() > 0) {
        out << indent(level);
    }

    out << "class " << self->name;
    if (self->bases.size() + self->keywords.size() > 0) {
        out << '(';
    }

    out << join<ExprNode*>(", ", self->bases);

    if (self->bases.size() > 0 && self->keywords.size() > 0) {
        out << ", ";
    }

    Array<String> kwd;
    kwd.reserve(self->keywords.size());

    for (auto kw: self->keywords) {
        // FIXME std::string -> String conversion
        kwd.push_back(String(fmt::format("{}={}", str(kw.arg), str(kw.value)).c_str()));
    }

    out << join(", ", kwd);

    if (self->bases.size() + self->keywords.size() > 0) {
        out << ')';
    }

    out << ":\n";

    if (self->docstring.has_value()) {
        out << indent(level + 1) << "\"\"\"" << self->docstring.value() << "\"\"\"\n";
    }

    int assign = 1;
    k          = 0;
    for (auto& stmt: self->body) {

        assign = stmt->kind == NodeKind::Assign || stmt->kind == NodeKind::AnnAssign ||
                 stmt->kind == NodeKind::Pass;

        if (k > 0 && assign == 0) {
            out << "\n";
        }

        out << indent(level + 1);
        exec(stmt, depth, level + 1);

        out << "\n";
        k += 1;
    }
    return false;
}

ReturnType LispSexp::functiondef(FunctionDef const* self, int depth, int level) {
    int k = 0;
    for (auto decorator: self->decorator_list) {
        if (k > 0) {
            out << indent(level);
        }

        out << "@";
        exec(decorator, depth, level);
        out << "\n";
        k += 1;
    }

    if (self->decorator_list.size() > 0) {
        out << indent(level);
    }

    out << "def " << self->name << "(";
    arguments(self->args, depth, level);
    out << ")";

    if (self->returns.has_value()) {
        out << " -> ";
        exec(self->returns.value(), depth, level);
    }

    out << ":\n";

    if (self->docstring.has_value()) {
        out << indent(level + 1) << "\"\"\"" << self->docstring.value() << "\"\"\"\n";
    }

    sexp_body(self->body, depth, level + 1, true);

    out << "\n";
    return false;
}

ReturnType LispSexp::inlinestmt(Inline const* self, int depth, int level) {
    out << indent(level);

    int k = 0;
    for (auto& stmt: self->body) {
        exec(stmt, depth, level);

        if (k + 1 < self->body.size()) {
            out << "; ";
        }

        k += 1;
    }

    return false;
}

ReturnType LispSexp::forstmt(For const* self, int depth, int level) {
    out << "for ";
    exec(self->target, depth, -1);
    out << " in ";
    exec(self->iter, depth, -1);
    out << ":\n";
    sexp_body(self->body, depth, level + 1);

    if (self->orelse.size() > 0) {
        out << '\n';
        out << indent(level) << "else:\n";
        sexp_body(self->orelse, depth, level + 1);
    }

    return false;
}

ReturnType LispSexp::trystmt(Try const* self, int depth, int level) {
    out << "try:\n";
    sexp_body(self->body, depth, level + 1);

    for (auto& handler: self->handlers) {
        excepthandler(handler, depth, level);
    }

    if (self->orelse.size() > 0) {
        out << "\n" << indent(level) << "else:\n";
        sexp_body(self->orelse, depth, level + 1);
    }

    if (self->finalbody.size() > 0) {
        out << "\n" << indent(level) << "finally:\n";
        sexp_body(self->finalbody, depth, level + 1);
    }

    return false;
}

ReturnType LispSexp::compare(Compare const* self, int depth, int level) {
    exec(self->left, depth, level);

    for (int i = 0; i < self->ops.size(); i++) {
        print_op(out, self->ops[i]);
        exec(self->comparators[i], depth, level);
    }

    return false;
}

ReturnType LispSexp::binop(BinOp const* self, int depth, int level) {
    auto sself   = get_precedence(self);
    auto lhspred = get_precedence(self->left) < sself;
    auto rhspred = get_precedence(self->right) < sself;

    if (lhspred) {
        out << '(';
    }
    exec(self->left, depth, level);
    if (lhspred) {
        out << ')';
    }

    print_op(out, self->op, false);

    if (rhspred) {
        out << '(';
    }
    exec(self->right, depth, level);
    if (rhspred) {
        out << ')';
    }

    return false;
}

ReturnType LispSexp::boolop(BoolOp const* self, int depth, int level) {
    exec(self->values[0], depth, level);
    print_op(out, self->op);
    exec(self->values[1], depth, level);
    return false;
}

ReturnType LispSexp::unaryop(UnaryOp const* self, int depth, int level) {
    print_op(out, self->op);
    out << " ";
    exec(self->operand, depth, level);

    return false;
}

ReturnType LispSexp::whilestmt(While const* self, int depth, int level) {
    out << "while ";
    exec(self->test, depth, level);
    out << ":\n";
    sexp_body(self->body, depth, level + 1);

    if (self->orelse.size() > 0) {
        out << '\n' << indent(level) << "else:\n";
        sexp_body(self->orelse, depth, level + 1);
    }

    return false;
}

ReturnType LispSexp::returnstmt(Return const* self, int depth, int level) {
    out << "return ";

    if (self->value.has_value()) {
        exec(self->value.value(), depth, -1);
    }

    return false;
}

ReturnType LispSexp::deletestmt(Delete const* self, int depth, int level) {
    out << "del ";

    for (int i = 0; i < self->targets.size(); i++) {
        exec(self->targets[i], depth, level);

        if (i < self->targets.size() - 1)
            out << ", ";
    }

    return false;
}

ReturnType LispSexp::augassign(AugAssign const* self, int depth, int level) {
    exec(self->target, depth, -1);
    print_op(out, self->op, true);
    out << "= ";
    exec(self->value, depth, -1);
    return false;
}

ReturnType LispSexp::assign(Assign const* self, int depth, int level) {
    exec(self->targets[0], depth, -1);
    out << " = ";
    exec(self->value, depth, -1);
    return false;
}

ReturnType LispSexp::annassign(AnnAssign const* self, int depth, int level) {
    exec(self->target, depth, level);
    out << ": ";

    exec(self->annotation, depth, level);
    if (self->value.has_value()) {
        out << " = ";
        exec(self->value.value(), depth, level);
    }

    return false;
}

ReturnType LispSexp::pass(Pass const* self, int depth, int level) {
    out << "pass";
    return false;
}

ReturnType LispSexp::breakstmt(Break const* self, int depth, int level) {
    out << "break";
    return false;
}

ReturnType LispSexp::continuestmt(Continue const* self, int depth, int level) {
    out << "continue";
    return false;
}

ReturnType LispSexp::exprstmt(Expr const* self, int depth, int level) {
    if (self->value != nullptr)
        exec(self->value, depth, -1);

    return false;
}

ReturnType LispSexp::global(Global const* self, int depth, int level) {
    out << "global " << join(", ", self->names);
    return false;
}

ReturnType LispSexp::nonlocal(Nonlocal const* self, int depth, int level) {
    out << "nonlocal " << join(", ", self->names);
    return false;
}

ReturnType LispSexp::arrow(Arrow const* self, int depth, int level) {
    out << '(' << join<ExprNode*>(", ", self->args) << ") -> ";
    out << str(self->returns);
    return false;
}

ReturnType LispSexp::dicttype(DictType const* self, int depth, int level) {
    out << "Dict[";
    out << str(self->key);
    out << ", ";
    out << str(self->value) << "]";
    return false;
}

ReturnType LispSexp::settype(SetType const* self, int depth, int level) {
    out << "Set[";
    out << str(self->value) << "]";
    return false;
}

ReturnType LispSexp::name(Name const* self, int depth, int level) {
    out << self->id;
    return false;
}

ReturnType LispSexp::arraytype(ArrayType const* self, int depth, int level) {
    out << "Array[";
    out << str(self->value) << "]";
    return false;
}

ReturnType LispSexp::tupletype(TupleType const* self, int depth, int level) {
    out << "Tuple[";
    out << join<ExprNode*>(", ", self->types) << "]";
    return false;
}

ReturnType LispSexp::builtintype(BuiltinType const* self, int depth, int level) {
    out << self->name;
    return false;
}

ReturnType LispSexp::joinedstr(JoinedStr const* self, int depth, int level) {
    out << "JoinedStr";
    return false;
}

ReturnType LispSexp::formattedvalue(FormattedValue const* self, int depth, int indent) {
    out << "FormattedValue";
    return false;
}

ReturnType LispSexp::classtype(ClassType const* self, int depth, int level) {
    out << self->def->name;
    return false;
}


void comprehension(LispSexp& p, Comprehension const& self, int depth, int level) {
    out << " for ";
    p.exec(self.target, depth, level);
    out << " in ";
    p.exec(self.iter, depth, level);

    for (auto expr: self.ifs) {
        out << " if ";
        p.exec(expr, depth, level);
    }
}

void LispSexp::keyword(Keyword const& self, int depth, int level) {
    out << self.arg;
    if (self.value != nullptr) {
        out << " = ";
        exec(self.value, depth, level);
    }
}

void LispSexp::alias(Alias const& self, int depth, int level) {
    out << self.name;
    if (self.asname.has_value()) {
        out << " as " << self.asname.value();
    }
}

ReturnType LispSexp::functiontype(FunctionType const* self, int depth, int indent) { return true; }

ReturnType LispSexp::expression(Expression const* self, int depth, int level) { return true; }

ReturnType LispSexp::interactive(Interactive const* self, int depth, int level) { return true; }

void LispSexp::withitem(WithItem const& self, int depth, int level) {
    exec(self.context_expr, depth, level);
    if (self.optional_vars.has_value()) {
        out << " as ";
        exec(self.optional_vars.value(), depth, level);
    }
}

void LispSexp::arguments(Arguments const& self, int depth, int level) {
    int i = 0;

    for (auto& arg: self.args) {
        out << arg.arg;

        if (arg.annotation.has_value()) {
            out << ": ";
            exec(arg.annotation.value(), depth, level);
        }

        auto default_offset = self.args.size() - 1 - i;
        if (self.defaults.size() > 0 && default_offset < self.defaults.size()) {
            if (arg.annotation.has_value()) {
                out << " = ";
            } else {
                out << "=";
            }
            exec(self.defaults[default_offset], depth, -1);
        }

        if (i + 1 < self.args.size()) {
            out << ", ";
        }
        i += 1;
    }

    if (self.vararg.has_value()) {
        if (self.args.size() > 0) {
            out << ", ";
        }

        out << "*" << self.vararg.value().arg;
    }

    if ((self.args.size() > 0 || self.vararg.has_value()) && self.kwonlyargs.size() > 0) {
        out << ", ";
    }

    i = 0;
    for (auto& kw: self.kwonlyargs) {
        out << kw.arg;

        if (kw.annotation.has_value()) {
            out << ": ";
            exec(kw.annotation.value(), depth, level);
        }

        auto default_offset = self.kwonlyargs.size() - 1 - i;
        if (self.kw_defaults.size() > 0 && default_offset < self.kw_defaults.size()) {
            if (kw.annotation.has_value()) {
                out << " = ";
            } else {
                out << "=";
            }
            exec(self.kw_defaults[default_offset], depth, -1);
        }

        if (i + 1 < self.kwonlyargs.size()) {
            out << ", ";
        }
        i += 1;
    }

    if (self.kwarg.has_value()) {
        if (self.kwonlyargs.size() > 0) {
            out << ", ";
        }
        out << "**" << self.kwarg.value().arg;
    }
}

int get_precedence(Node const* node) {
    if (node->kind == NodeKind::BinOp) {
        BinOp* op = (BinOp*)(node);
        // clang-format off
        switch (op->op) {
            case BinaryOperator::Add: return 1;
            case BinaryOperator::Sub: return 1;
            case BinaryOperator::Mult: return 2;
            case BinaryOperator::Div: return 2;
            case BinaryOperator::Pow: return 3;
            case BinaryOperator::BitXor: return 3;
        }
        // clang-format on
        return 10;
    }
    return 1000;
}

void lispsexp(Node const* obj) {
    LispSexp p;
    p.exec<bool>(obj, 0);
}
void lispsexp(ExprNode const* obj) {
    LispSexp p;
    p.exec(obj, 0, 0);
}
void lispsexp(Pattern const* obj) {
    LispSexp p;
    p.exec(obj, 0, 0);
}
void lispsexp(StmtNode const* obj) {
    LispSexp p;
    p.exec(obj, 0, 0);
}
void lispsexp(ModNode const* obj) {
    LispSexp p;
    p.exec(obj, 0, 0);
}

}  // namespace lython
