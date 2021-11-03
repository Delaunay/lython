#include "ast/magic.h"
#include "sema/sema.h"
#include "utilities/strings.h"

namespace lython {

void Bindings::dump(std::ostream &out) const {
    auto big   = String(40, '-');
    auto small = String(20, '-');
    auto sep   = fmt::format("{:>40}-+-{:>20}-+-{}", big, small, small);

    out << sep << '\n';
    out << fmt::format("{:40} | {:20} | {}", "name", "type", "value") << "\n";
    out << sep << '\n';
    for (auto &e: bindings) {
        print(out, e);
    }
    out << sep << '\n';
}

inline std::ostream &print(std::ostream &out, BindingEntry const &entry) {
    String n = str(entry.name);
    String v = str(entry.value);
    String t = str(entry.type);

    auto frags = split('\n', v);

    out << fmt::format("{:>40} | {:>20} | {}", n, t, frags[0]) << '\n';
    for (int i = 1; i < frags.size(); i++) {
        if (strip(frags[i]) == "") {
            continue;
        }
        out << fmt::format("{:>40} | {:>20} | {}", "", "", frags[i]) << '\n';
    }
    return out;
}

bool SemanticAnalyser::add_name(ExprNode *expr, ExprNode *value, ExprNode *type) {
    auto name = cast<Name>(expr);

    if (name) {
        name->ctx = ExprContext::Store;
        bindings.add(name->id, value, type);
        return true;
    }

    return false;
}

TypeExpr *SemanticAnalyser::boolop(BoolOp *n, int depth) {
    auto values_t = exec<ExprNode>(n->values, depth);
    // TODO: check that op is defined for those types
    // use the op return type here
    return values_t[0];
}
TypeExpr *SemanticAnalyser::namedexpr(NamedExpr *n, int depth) {
    auto value_t = exec(n->value, depth);
    add_name(n->target, n->value, value_t);
    return value_t;
}
TypeExpr *SemanticAnalyser::binop(BinOp *n, int depth) {

    auto lhs_t = exec(n->left, depth);
    auto rhs_t = exec(n->right, depth);
    typecheck(lhs_t, rhs_t);

    // TODO: check that op is defined for those types
    // use the op return type here
    return lhs_t;
}
TypeExpr *SemanticAnalyser::unaryop(UnaryOp *n, int depth) {
    auto expr_t = exec(n->operand, depth);

    // TODO: check that op is defined for this type
    // use the op return type here
    return expr_t;
}
TypeExpr *SemanticAnalyser::lambda(Lambda *n, int depth) {
    Scope scope(bindings);
    auto  funtype = n->new_object<Arrow>();
    add_arguments(n->args, funtype, depth);
    auto type        = exec(n->body, depth);
    funtype->returns = type;
    return funtype;
}
TypeExpr *SemanticAnalyser::ifexp(IfExp *n, int depth) {
    auto test_t = exec(n->test, depth);
    // typecheck(test_t, bool_t);
    auto body_t   = exec(n->body, depth);
    auto orelse_t = exec(n->orelse, depth);

    typecheck(body_t, orelse_t);
    return body_t;
}
TypeExpr *SemanticAnalyser::dictexpr(DictExpr *n, int depth) {
    TypeExpr *key_t = nullptr;
    TypeExpr *val_t = nullptr;

    for (int i = 0; i < n->keys.size(); i++) {
        auto key_type = exec(n->keys[i], depth);
        auto val_type = exec(n->values[i], depth);

        if (key_t != nullptr && val_t != nullptr) {
            typecheck(key_type, key_t);
            typecheck(val_type, val_t);
        } else {
            key_t = key_type;
            val_t = val_type;
        }
    }

    DictType *type = n->new_object<DictType>();
    type->key      = key_t;
    type->value    = val_t;
    return type;
}
TypeExpr *SemanticAnalyser::setexpr(SetExpr *n, int depth) {
    TypeExpr *val_t = nullptr;

    for (int i = 0; i < n->elts.size(); i++) {
        auto val_type = exec(n->elts[i], depth);

        if (val_t != nullptr) {
            typecheck(val_type, val_t);
        } else {
            val_t = val_type;
        }
    }

    SetType *type = n->new_object<SetType>();
    type->value   = val_t;
    return type;
}
TypeExpr *SemanticAnalyser::listcomp(ListComp *n, int depth) {
    Scope scope(bindings);
    for (auto &gen: n->generators) {
        exec(gen.target, depth);
        exec(gen.iter, depth);

        for (auto if_: gen.ifs) {
            exec(if_, depth);
        }
    }

    auto val_type = exec(n->elt, depth);

    auto type   = n->new_object<ArrayType>();
    type->value = val_type;
    return type;
}
TypeExpr *SemanticAnalyser::generateexpr(GeneratorExp *n, int depth) {
    Scope scope(bindings);
    for (auto &gen: n->generators) {
        exec(gen.target, depth);
        exec(gen.iter, depth);

        for (auto if_: gen.ifs) {
            exec(if_, depth);
        }
    }

    auto val_type = exec(n->elt, depth);

    auto type   = n->new_object<ArrayType>();
    type->value = val_type;
    return type;
}
TypeExpr *SemanticAnalyser::setcomp(SetComp *n, int depth) {
    Scope scope(bindings);
    for (auto &gen: n->generators) {
        exec(gen.target, depth);
        exec(gen.iter, depth);

        for (auto if_: gen.ifs) {
            exec(if_, depth);
        }
    }

    auto val_type = exec(n->elt, depth);

    auto type   = n->new_object<ArrayType>();
    type->value = val_type;
    return type;
}
TypeExpr *SemanticAnalyser::dictcomp(DictComp *n, int depth) {
    Scope scope(bindings);
    for (auto &gen: n->generators) {
        exec(gen.target, depth);
        exec(gen.iter, depth);

        for (auto if_: gen.ifs) {
            exec(if_, depth);
        }
    }

    auto key_type = exec(n->key, depth);
    auto val_type = exec(n->value, depth);

    auto type   = n->new_object<DictType>();
    type->key   = key_type;
    type->value = val_type;
    return type;
}
TypeExpr *SemanticAnalyser::await(Await *n, int depth) { return exec(n->value, depth); }
TypeExpr *SemanticAnalyser::yield(Yield *n, int depth) {
    auto r = exec<TypeExpr>(n->value, depth);
    if (r.has_value()) {
        return r.value();
    }
    return nullptr;
}
TypeExpr *SemanticAnalyser::yieldfrom(YieldFrom *n, int depth) { return exec(n->value, depth); }
TypeExpr *SemanticAnalyser::compare(Compare *n, int depth) {
    for (auto cmp: n->comparators) {
        exec(cmp, depth);
    }

    // TODO: return bool here
    return nullptr;
}
TypeExpr *SemanticAnalyser::call(Call *n, int depth) {
    auto type = exec(n->func, depth);

    for (auto &arg: n->args) {
        exec(arg, depth);
    }

    for (auto &kw: n->keywords) {
        exec(kw.value, depth);
    }

    // type check argument with function def
    // fetch return type inside arrow
    return type;
}
TypeExpr *SemanticAnalyser::joinedstr(JoinedStr *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::formattedvalue(FormattedValue *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::constant(Constant *n, int depth) {
    // TODO: get type from constant
    return nullptr;
}
TypeExpr *SemanticAnalyser::attribute(Attribute *n, int depth) {
    auto class_t = exec(n->value, depth);
    // check that attr is defined in class_t
    // n->attr
    return nullptr;
}
TypeExpr *SemanticAnalyser::subscript(Subscript *n, int depth) {
    auto class_t = exec(n->value, depth);
    exec(n->slice, depth);
    // check that __getitem__ is defined in class_t
    return nullptr;
}
TypeExpr *SemanticAnalyser::starred(Starred *n, int depth) {
    // value should be of an expandable type
    auto value_t = exec(n->value, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::name(Name *n, int depth) {
    n->varid = bindings.get_varid(n->id);
    if (n->varid == -1) {
        errors.push_back(SemanticError{
            nullptr, n, nullptr, String(fmt::format("Undefined variable {}", n->id).c_str()), LOC});
    }
    return bindings.get_type(n->varid);
}
TypeExpr *SemanticAnalyser::listexpr(ListExpr *n, int depth) {
    TypeExpr *val_t = nullptr;

    for (int i = 0; i < n->elts.size(); i++) {
        auto val_type = exec(n->elts[i], depth);

        if (val_t != nullptr) {
            typecheck(val_type, val_t);
        } else {
            val_t = val_type;
        }
    }

    ArrayType *type = n->new_object<ArrayType>();
    type->value     = val_t;
    return type;
}
TypeExpr *SemanticAnalyser::tupleexpr(TupleExpr *n, int depth) {
    TypeExpr * val_type = nullptr;
    TupleType *type     = n->new_object<TupleType>();
    type->types.reserve(n->elts.size());

    for (int i = 0; i < n->elts.size(); i++) {
        val_type = exec(n->elts[i], depth);
        type->types.push_back(val_type);
    }

    return type;
}
TypeExpr *SemanticAnalyser::slice(Slice *n, int depth) {
    exec<TypeExpr>(n->lower, depth);
    exec<TypeExpr>(n->upper, depth);
    exec<TypeExpr>(n->step, depth);
    return nullptr;
}

void SemanticAnalyser::add_arguments(Arguments &args, Arrow *arrow, int depth) {
    for (auto &arg: args.args) {
        TypeExpr *type = nullptr;
        if (arg.annotation.has_value()) {
            type = arg.annotation.value();
            exec(type, depth);
        }
        bindings.add(arg.arg, nullptr, type);

        if (arrow) {
            arrow->args.push_back(type);
            exec(type, depth);
        }
    }

    for (auto &arg: args.kwonlyargs) {
        TypeExpr *type = nullptr;
        if (arg.annotation.has_value()) {
            type = arg.annotation.value();
        }
        bindings.add(arg.arg, nullptr, type);

        if (arrow) {
            arrow->args.push_back(type);
        }
    }
}

TypeExpr *SemanticAnalyser::functiondef(FunctionDef *n, int depth) {
    auto  id = bindings.add(n->name, n, nullptr);
    Scope scope(bindings);

    auto type = n->new_object<Arrow>();
    add_arguments(n->args, type, depth);

    auto return_effective = exec<TypeExpr>(n->body, depth);

    if (n->returns.has_value()) {
        type->returns = n->returns.value();
    }

    bindings.set_type(id, type);
    bindings.dump(std::cout);
    return nullptr;
}
TypeExpr *SemanticAnalyser::classdef(ClassDef *n, int depth) {
    int id;
    // I might have to always run the forward pass
    if (!forwardpass /*|| depth > 1*/) {
        id = bindings.add(n->name, n, nullptr);
    } else {
        id = bindings.get_varid(n->name);
    }

    Scope scope(bindings);
    auto  types = exec<TypeExpr>(n->body, depth);
    return oneof(types);
}
TypeExpr *SemanticAnalyser::returnstmt(Return *n, int depth) {
    auto v = exec<TypeExpr>(n->value, depth);
    if (v.has_value()) {
        return v.value();
    }
    return nullptr;
}
TypeExpr *SemanticAnalyser::deletestmt(Delete *n, int depth) {
    for (auto target: n->targets) {
        exec(target, depth);
    }
    return nullptr;
}
TypeExpr *SemanticAnalyser::assign(Assign *n, int depth) {
    auto type = exec(n->value, depth);

    if (n->targets.size() == 1) {
        add_name(n->targets[0], n->value, type);
    } else {
        auto types = cast<TupleType>(type);
        if (!types) {
            // TODO: unexpected type
            return type;
        }

        if (types->types.size() != n->targets.size()) {
            // TODO: Add type mismatch
            return type;
        }

        for (auto i = 0; i < types->types.size(); i++) {
            auto target = n->targets[0];
            auto name   = cast<Name>(target);
            auto type   = types->types[0];

            add_name(n->targets[0], n->value, type);
        }
    }

    return type;
}
TypeExpr *SemanticAnalyser::augassign(AugAssign *n, int depth) {
    auto expected_type = exec(n->target, depth);
    auto type          = exec(n->value, depth);

    typecheck(type, expected_type);
    return type;
}

//! Annotation takes priority over the deduced type
//! this enbles users to use annotation to debug
TypeExpr *SemanticAnalyser::annassign(AnnAssign *n, int depth) {

    auto      constraint = n->annotation;
    auto      type       = exec<TypeExpr>(n->value, depth);
    ExprNode *value      = nullptr;

    if (type.has_value()) {
        typecheck(constraint, type.value());
        value = n->value.value();
        return type.value();
    }

    add_name(n->target, value, constraint);
    return constraint;
}
TypeExpr *SemanticAnalyser::forstmt(For *n, int depth) {
    exec(n->target, depth);
    exec(n->iter, depth);
    auto types = exec<TypeExpr>(n->body, depth);
    exec<TypeExpr>(n->orelse, depth);
    return oneof(types);
}
TypeExpr *SemanticAnalyser::whilestmt(While *n, int depth) {
    exec(n->test, depth);
    exec<TypeExpr>(n->body, depth);
    auto types = exec<TypeExpr>(n->orelse, depth);
    return oneof(types);
}
TypeExpr *SemanticAnalyser::ifstmt(If *n, int depth) {
    exec(n->test, depth);
    auto types = exec<TypeExpr>(n->body, depth);

    for (int i = 0; i < n->tests.size(); i++) {
        exec(n->tests[i], depth);
        exec<TypeExpr>(n->bodies[i], depth);
    }

    return oneof(types);
}
TypeExpr *SemanticAnalyser::with(With *n, int depth) {
    for (auto &item: n->items) {
        auto type = exec(item.context_expr, depth);

        if (item.optional_vars.has_value()) {
            auto expr = item.optional_vars.value();
            if (expr->kind == NodeKind::Name) {
                auto name = cast<Name>(expr);
                bindings.add(name->id, expr, type);
            } else {
                // FIXME: is this even possible ?
                exec(item.optional_vars.value(), depth);
            }
        }
    }

    auto types = exec<TypeExpr>(n->body, depth);
    return oneof(types);
}
TypeExpr *SemanticAnalyser::raise(Raise *n, int depth) {
    exec<TypeExpr>(n->exc, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::trystmt(Try *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::assertstmt(Assert *n, int depth) {
    exec(n->test, depth);
    exec<TypeExpr>(n->msg, depth + 1);
    return nullptr;
}
TypeExpr *SemanticAnalyser::import(Import *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::importfrom(ImportFrom *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::global(Global *n, int depth) {
    for (auto &name: n->names) {
        auto varid = bindings.get_varid(name);
        if (varid == -1) {
            errors.push_back(
                SemanticError{n, nullptr, nullptr,
                              String(fmt::format("Undefined variable {}", name).c_str()), LOC});
        }
    }
    return nullptr;
}
TypeExpr *SemanticAnalyser::nonlocal(Nonlocal *n, int depth) {
    for (auto &name: n->names) {
        auto varid = bindings.get_varid(name);
        if (varid == -1) {
            errors.push_back(
                SemanticError{n, nullptr, nullptr,
                              String(fmt::format("Undefined variable {}", name).c_str()), LOC});
        }
    }
    return nullptr;
}
TypeExpr *SemanticAnalyser::exprstmt(Expr *n, int depth) { return exec(n->value, depth); }
TypeExpr *SemanticAnalyser::pass(Pass *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::breakstmt(Break *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::continuestmt(Continue *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::match(Match *n, int depth) {
    exec(n->subject, depth);

    Array<TypeExpr *> types;
    for (auto &b: n->cases) {
        exec(b.pattern, depth + 1);
        exec<TypeExpr>(b.guard, depth + 1);
        types = exec<TypeExpr>(b.body, depth + 1);
    }

    return oneof(types);
}
TypeExpr *SemanticAnalyser::inlinestmt(Inline *n, int depth) {
    auto types = exec<TypeExpr>(n->body, depth);
    return oneof(types);
}

TypeExpr *SemanticAnalyser::matchvalue(MatchValue *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::matchsingleton(MatchSingleton *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::matchsequence(MatchSequence *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::matchmapping(MatchMapping *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::matchclass(MatchClass *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::matchstar(MatchStar *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::matchas(MatchAs *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::matchor(MatchOr *n, int depth) { return nullptr; }

// typetype is the type of a type
// i.e
//  1 is of type int
//  int si of type Type
TypeExpr *typetype() {
    static TypeExpr type = []() {
        auto expr = BuiltinType();
        expr.name = "Type";
        return expr;
    }();
    return &type;
}

TypeExpr *SemanticAnalyser::dicttype(DictType *n, int depth) { return typetype(); }
TypeExpr *SemanticAnalyser::arraytype(ArrayType *n, int depth) { return typetype(); }
TypeExpr *SemanticAnalyser::arrow(Arrow *n, int depth) { return typetype(); }
TypeExpr *SemanticAnalyser::builtintype(BuiltinType *n, int depth) { return typetype(); }
TypeExpr *SemanticAnalyser::tupletype(TupleType *n, int depth) { return typetype(); }
TypeExpr *SemanticAnalyser::settype(SetType *n, int depth) { return typetype(); }
TypeExpr *SemanticAnalyser::classtype(ClassType *n, int depth) { return typetype(); }

} // namespace lython
