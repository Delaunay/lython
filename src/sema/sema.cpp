#include "ast/magic.h"
#include "sema/sema.h"
#include "utilities/strings.h"

namespace lython {

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

TypeExpr *SemanticAnalyser::boolop(BoolOp *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::namedexpr(NamedExpr *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::binop(BinOp *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::unaryop(UnaryOp *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::lambda(Lambda *n, int depth) {
    Scope scope(bindings);
    auto  type = exec(n->body, depth);
    return type;
}
TypeExpr *SemanticAnalyser::ifexp(IfExp *n, int depth) {
    exec(n->test, depth);
    exec(n->body, depth);
    exec(n->orelse, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::dictexpr(DictExpr *n, int depth) {
    TypeExpr *key_type = nullptr;
    TypeExpr *val_type = nullptr;

    for (int i = 0; i < n->keys.size(); i++) {
        key_type = exec(n->keys[i], depth);
        val_type = exec(n->values[i], depth);
    }

    DictType *type = n->new_object<DictType>();
    type->key      = key_type;
    type->value    = val_type;
    return type;
}
TypeExpr *SemanticAnalyser::setexpr(SetExpr *n, int depth) {
    TypeExpr *val_type = nullptr;

    for (int i = 0; i < n->elts.size(); i++) {
        val_type = exec(n->elts[i], depth);
    }

    SetType *type = n->new_object<SetType>();
    type->value   = val_type;
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
TypeExpr *SemanticAnalyser::await(Await *n, int depth) {
    exec(n->value, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::yield(Yield *n, int depth) {
    auto r = exec<TypeExpr>(n->value, depth);
    if (r.has_value()) {
        return r.value();
    }
    return nullptr;
}
TypeExpr *SemanticAnalyser::yieldfrom(YieldFrom *n, int depth) {
    exec(n->value, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::compare(Compare *n, int depth) { return nullptr; }
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
TypeExpr *SemanticAnalyser::constant(Constant *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::attribute(Attribute *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::subscript(Subscript *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::starred(Starred *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::name(Name *n, int depth) {
    n->varid = get_varid(n->id);
    return get_type(n->varid);
}
TypeExpr *SemanticAnalyser::listexpr(ListExpr *n, int depth) {
    TypeExpr *val_type = nullptr;

    for (int i = 0; i < n->elts.size(); i++) {
        val_type = exec(n->elts[i], depth);
    }

    ArrayType *type = n->new_object<ArrayType>();
    type->value     = val_type;
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

void SemanticAnalyser::add_arguments(Arguments &args, Arrow *arrow) {
    for (auto &arg: args.args) {
        TypeExpr *type = nullptr;
        if (arg.annotation.has_value()) {
            type = arg.annotation.value();
        }
        add(arg.arg, nullptr, type);

        if (arrow) {
            arrow->args.push_back(type);
        }
    }

    for (auto &arg: args.kwonlyargs) {
        TypeExpr *type = nullptr;
        if (arg.annotation.has_value()) {
            type = arg.annotation.value();
        }
        add(arg.arg, nullptr, type);

        if (arrow) {
            arrow->args.push_back(type);
        }
    }
}

void SemanticAnalyser::dump() const {
    auto big   = String(40, '-');
    auto small = String(20, '-');
    auto sep   = fmt::format("{:>40}-+-{:>20}-+-{}", big, small, small);

    std::cout << sep << '\n';
    std::cout << fmt::format("{:40} | {:20} | {}", "name", "type", "value") << "\n";
    std::cout << sep << '\n';
    for (auto &e: bindings) {
        print(std::cout, e);
    }
    std::cout << sep << '\n';
}

TypeExpr *SemanticAnalyser::functiondef(FunctionDef *n, int depth) {
    auto  id = add(n->name, n, nullptr);
    Scope scope(bindings);

    auto type = n->new_object<Arrow>();
    add_arguments(n->args, type);

    auto return_effective = exec<TypeExpr>(n->body, depth);

    if (n->returns.has_value()) {
        type->returns = n->returns.value();
    }

    set_type(id, type);
    dump();
    return nullptr;
}
TypeExpr *SemanticAnalyser::classdef(ClassDef *n, int depth) {
    int id;
    // I might have to always run the forward pass
    if (!forwardpass /*|| depth > 1*/) {
        id = add(n->name, n, nullptr);
    } else {
        id = get_varid(n->name);
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
    exec<TypeExpr>(n->targets, depth);
    return exec(n->value, depth);
}
TypeExpr *SemanticAnalyser::augassign(AugAssign *n, int depth) {
    exec(n->target, depth);
    auto type = exec(n->value, depth);
    return type;
}
TypeExpr *SemanticAnalyser::annassign(AnnAssign *n, int depth) {
    exec(n->target, depth);
    // TODO: type check here
    auto type = exec<TypeExpr>(n->value, depth);

    if (type.has_value()) {
        return type.value();
    }
    return nullptr;
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
        if (item.optional_vars.has_value()) {
            exec(item.optional_vars.value(), depth);
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
TypeExpr *SemanticAnalyser::global(Global *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::nonlocal(Nonlocal *n, int depth) {
    // n->names
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

TypeExpr *SemanticAnalyser::dicttype(DictType *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::arraytype(ArrayType *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::arrow(Arrow *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::builtintype(BuiltinType *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::tupletype(TupleType *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::settype(SetType *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::classtype(ClassType *n, int depth) { return nullptr; }

} // namespace lython
