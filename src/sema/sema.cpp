#include "sema/sema.h"
#include "ast/magic.h"
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
    auto values_t = exec<ExprNode *>(n->values, depth);
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
    auto r = exec<TypeExpr *>(n->value, depth);
    if (r.has_value()) {
        return r.value();
    }
    return nullptr;
}
TypeExpr *SemanticAnalyser::yieldfrom(YieldFrom *n, int depth) { return exec(n->value, depth); }
TypeExpr *SemanticAnalyser::compare(Compare *n, int depth) {

    auto prev_t = exec(n->left, depth);
    auto prev   = n->left;

    for (int i = 0; i < n->ops.size(); i++) {
        auto op    = n->ops[i];
        auto cmp   = n->comparators[i];
        auto cmp_t = exec(cmp, depth);

        //
        // prev <op> cmp

        // TODO: typecheck op
        // op: (prev_t -> cmp_t -> bool)

        // --
        prev   = cmp;
        prev_t = cmp_t;
    }

    return make_ref(n, "bool");
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
    switch (n->value.type()) {
    case ConstantValue::TInt:
        return make_ref(n, "i32");
    case ConstantValue::TFloat:
        return make_ref(n, "f32");
    case ConstantValue::TDouble:
        return make_ref(n, "f64");
    case ConstantValue::TString:
        return make_ref(n, "str");
    case ConstantValue::TBool:
        return make_ref(n, "bool");
    default:
        return nullptr;
    }

    return nullptr;
}
TypeExpr *SemanticAnalyser::attribute(Attribute *n, int depth) {
    auto class_t = exec(n->value, depth);
    // TODO: check that attr is defined in class_t
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
    if (n->ctx == ExprContext::Store) {
        debug("{}", "here storing value");
        auto id  = bindings.add(n->id, n, nullptr);
        n->varid = id;
    } else {
        // Both delete & Load requires the variable to be defined first
        n->varid = bindings.get_varid(n->id);
        if (n->varid == -1) {
            errors.push_back(
                SemanticError{nullptr, n, nullptr,
                              String(fmt::format("Undefined variable {}", n->id).c_str()), LOC});
        }
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
    exec<TypeExpr *>(n->lower, depth);
    exec<TypeExpr *>(n->upper, depth);
    exec<TypeExpr *>(n->step, depth);
    return nullptr;
}

void SemanticAnalyser::add_arguments(Arguments &args, Arrow *arrow, int depth) {

    for (int i = 0, n = int(args.args.size()); i < n; i++) {
        auto      arg      = args.args[i];
        ExprNode *dvalue   = nullptr;
        TypeExpr *dvalue_t = nullptr;

        int d = int(args.defaults.size()) - (n - i);

        if (d >= 0) {
            dvalue   = args.defaults[d];
            dvalue_t = exec(dvalue, depth);
        }

        TypeExpr *type = nullptr;
        if (arg.annotation.has_value()) {
            type = arg.annotation.value();

            auto typetype = exec(type, depth);
            typecheck(typetype, type_Type());
        }

        // if default value & annotation types must match
        if (type && dvalue_t) {
            typecheck(type, dvalue_t);
        }

        // if no annotation use default value type
        if (type == nullptr) {
            type = dvalue_t;
        }

        // we could populate the default value here
        // but we would not want sema to think this is a constant
        bindings.add(arg.arg, nullptr, type);

        if (arrow) {
            arrow->args.push_back(type);
        }
    }

    for (int i = 0, n = args.kwonlyargs.size(); i < n; i++) {
        auto arg = args.kwonlyargs[i];

        ExprNode *dvalue   = nullptr;
        TypeExpr *dvalue_t = nullptr;

        int d = int(args.kw_defaults.size()) - (n - i);

        if (d >= 0) {
            dvalue   = args.kw_defaults[d];
            dvalue_t = exec(dvalue, depth);
        }

        TypeExpr *type = nullptr;
        if (arg.annotation.has_value()) {
            type = arg.annotation.value();

            auto typetype = exec(type, depth);
            typecheck(typetype, type_Type());
        }

        // if default value & annotation types must match
        if (type && dvalue_t) {
            typecheck(type, dvalue_t);
        }

        // if no annotation use default value type
        if (type == nullptr) {
            type = dvalue_t;
        }

        bindings.add(arg.arg, nullptr, type);

        if (arrow) {
            arrow->args.push_back(type);
        }
    }
}

TypeExpr *SemanticAnalyser::functiondef(FunctionDef *n, int depth) {
    // Add the function name first to handle recursive calls
    auto  id = bindings.add(n->name, n, nullptr);
    Scope scope(bindings);

    auto type = n->new_object<Arrow>();
    add_arguments(n->args, type, depth);

    auto return_effective = exec<TypeExpr *>(n->body, depth);

    if (n->returns.has_value()) {
        // Annotated type takes precedence
        auto return_t = n->returns.value();
        auto typetype = exec(return_t, depth);
        typecheck(typetype, type_Type());

        type->returns = return_t;
        typecheck(return_t, oneof(return_effective));
    }

    bindings.set_type(id, type);

    // do decorator last since we need to know our function signature to
    // typecheck them
    for (auto decorator: n->decorator_list) {
        auto deco_t = exec(decorator, depth);
        // TODO check the signature here
    }

    // bindings.dump(std::cout);
    return type;
}
TypeExpr *SemanticAnalyser::classdef(ClassDef *n, int depth) {
    int       id;
    TypeExpr *class_t = nullptr;

    // I might have to always run the forward pass
    if (!forwardpass /*|| depth > 1*/) {
        auto t   = n->new_object<Name>();
        id       = bindings.add(n->name, n, t);
        t->id    = n->name;
        t->varid = id;
        class_t  = t;

    } else {
        id      = bindings.get_varid(n->name);
        class_t = bindings.get_type(id);
    }

    // TODO: go through bases and add their elements
    for (auto base: n->bases) {
        exec(base, depth);
    }

    //
    for (auto kw: n->keywords) {
        exec(kw.value, depth);
    }

    Array<StmtNode *> secondpass;
    FunctionDef *     ctor;

    for (auto &stmt: n->body) {
        Scope scope(bindings);

        auto fun = cast<FunctionDef>(stmt);
        if (fun) {
            n->insert_attribute(fun->name, fun);

            if (str(fun->name) == "__init__") {
                ctor = fun;
            } else {
                secondpass.push_back(stmt);
            }
            continue;
        }

        auto attr = cast<Assign>(stmt);
        if (attr) {
            auto targets_t = exec(attr->value, depth);

            for (auto target: attr->targets) {
                auto name = cast<Name>(target);
                if (name) {
                    n->insert_attribute(name->id, stmt);
                }
            }

            continue;
        }

        auto attras = cast<AnnAssign>(stmt);
        if (attras) {
            auto type     = exec<TypeExpr *>(attras->value, depth);
            auto target_t = exec(attras->annotation, depth);

            if (type.has_value()) {
                typecheck(type.value(), target_t);
            }

            auto name = cast<Name>(attras->target);
            if (name) {
                n->insert_attribute(name->id, stmt, target_t);
            }
            continue;
        }

        debug("Unhandled statement {}", str(stmt->kind));
    }

    // get __init__ and do a pass to insert its attribute to the class

    Array<StmtNode *> secondpass2;
    for (auto &stmt: secondpass2) {
        Scope scope(bindings);

        auto fun_t = exec(stmt, depth);
    }

    for (auto deco: n->decorator_list) {
        auto deco_t = exec(deco, depth);
        // TODO: check signature here
    }
    return class_t;
}
TypeExpr *SemanticAnalyser::returnstmt(Return *n, int depth) {
    auto v = exec<TypeExpr *>(n->value, depth);
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
    auto constraint = n->annotation;
    auto typetype   = exec(n->annotation, depth);

    // Type annotation must be a type
    typecheck(typetype, type_Type());

    auto type = exec<TypeExpr *>(n->value, depth);

    ExprNode *value = nullptr;

    if (type.has_value()) {
        // if we were able to deduce a type from the expression
        // make sure it matches the annotation constraint
        typecheck(constraint, type.value());
        value = n->value.value();
        return type.value();
    }

    add_name(n->target, value, constraint);
    return constraint;
}
TypeExpr *SemanticAnalyser::forstmt(For *n, int depth) {
    auto iter_t = exec(n->iter, depth);

    // TODO: use iter_t to set target types
    exec(n->target, depth);

    // TODO: check consistency of return types
    auto return_t1 = exec<TypeExpr *>(n->body, depth);
    auto return_t2 = exec<TypeExpr *>(n->orelse, depth);

    return oneof(return_t1);
}
TypeExpr *SemanticAnalyser::whilestmt(While *n, int depth) {
    exec(n->test, depth);
    exec<TypeExpr *>(n->body, depth);
    auto types = exec<TypeExpr *>(n->orelse, depth);
    return oneof(types);
}
TypeExpr *SemanticAnalyser::ifstmt(If *n, int depth) {
    exec(n->test, depth);
    auto types = exec<TypeExpr *>(n->body, depth);

    for (int i = 0; i < n->tests.size(); i++) {
        exec(n->tests[i], depth);
        exec<TypeExpr *>(n->bodies[i], depth);
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
                if (name != nullptr) {
                    name->varid = bindings.add(name->id, expr, type);
                }
            } else {
                // FIXME: is this even possible ?
                exec(item.optional_vars.value(), depth);
            }
        }
    }

    auto types = exec<TypeExpr *>(n->body, depth);
    return oneof(types);
}
TypeExpr *SemanticAnalyser::raise(Raise *n, int depth) {
    exec<TypeExpr *>(n->exc, depth);
    exec<TypeExpr *>(n->cause, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::trystmt(Try *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::assertstmt(Assert *n, int depth) {
    exec(n->test, depth);
    exec<TypeExpr *>(n->msg, depth + 1);
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
        exec<TypeExpr *>(b.guard, depth + 1);
        types = exec<TypeExpr *>(b.body, depth + 1);
    }

    return oneof(types);
}
TypeExpr *SemanticAnalyser::inlinestmt(Inline *n, int depth) {
    auto types = exec<TypeExpr *>(n->body, depth);
    return oneof(types);
}

TypeExpr *SemanticAnalyser::matchvalue(MatchValue *n, int depth) {
    exec(n->value, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::matchsingleton(MatchSingleton *n, int depth) { return nullptr; }
TypeExpr *SemanticAnalyser::matchsequence(MatchSequence *n, int depth) {
    for (auto &elt: n->patterns) {
        exec(elt, depth);
    }
    return nullptr;
}
TypeExpr *SemanticAnalyser::matchmapping(MatchMapping *n, int depth) {
    for (auto pat: n->patterns) {
        exec(pat, depth);
    }
    return nullptr;
}
TypeExpr *SemanticAnalyser::matchclass(MatchClass *n, int depth) {
    exec(n->cls, depth);
    for (auto pat: n->patterns) {
        exec(pat, depth);
    }
    for (auto pat: n->kwd_patterns) {
        exec(pat, depth);
    }
    return nullptr;
}
TypeExpr *SemanticAnalyser::matchstar(MatchStar *n, int depth) {
    // TODO: need to get the type from the target
    if (n->name.has_value()) {
        bindings.add(n->name.value(), n, nullptr);
    }
    return nullptr;
}
TypeExpr *SemanticAnalyser::matchas(MatchAs *n, int depth) {
    // TODO: need to get the type from the target
    if (n->name.has_value()) {
        bindings.add(n->name.value(), n, nullptr);
    }
    exec<TypeExpr *>(n->pattern, depth);
    return nullptr;
}
TypeExpr *SemanticAnalyser::matchor(MatchOr *n, int depth) {
    for (auto pat: n->patterns) {
        exec(pat, depth);
    }
    return nullptr;
}

BuiltinType make_type(String const &name) {
    auto expr = BuiltinType();
    expr.name = "Type";
    return expr;
}

#define TYPE(name)                                  \
    TypeExpr *type_##name() {                       \
        static BuiltinType type = make_type(#name); \
        return &type;                               \
    }

BUILTIN_TYPES(TYPE)

#undef TYPE

TypeExpr *SemanticAnalyser::dicttype(DictType *n, int depth) { return type_Type(); }
TypeExpr *SemanticAnalyser::arraytype(ArrayType *n, int depth) { return type_Type(); }
TypeExpr *SemanticAnalyser::arrow(Arrow *n, int depth) { return type_Type(); }
TypeExpr *SemanticAnalyser::builtintype(BuiltinType *n, int depth) { return type_Type(); }
TypeExpr *SemanticAnalyser::tupletype(TupleType *n, int depth) { return type_Type(); }
TypeExpr *SemanticAnalyser::settype(SetType *n, int depth) { return type_Type(); }
TypeExpr *SemanticAnalyser::classtype(ClassType *n, int depth) { return type_Type(); }

} // namespace lython
