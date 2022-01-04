#include "sema/sema.h"
#include "ast/magic.h"
#include "utilities/strings.h"

namespace lython {
ClassDef *get_class(Bindings const &bindings, ExprNode *classref);
Arrow *   get_arrow(SemanticAnalyser *self, ExprNode *fun, ExprNode *type, int depth, int &offset);

bool SemanticAnalyser::add_name(ExprNode *expr, ExprNode *value, ExprNode *type) {
    auto name = cast<Name>(expr);

    if (name) {
        name->ctx = ExprContext::Store;
        bindings.add(name->id, value, type);
        return true;
    }

    return false;
}

bool SemanticAnalyser::typecheck(ExprNode *lhs, TypeExpr *lhs_t, ExprNode *rhs, TypeExpr *rhs_t,
                                 CodeLocation const &loc) {

    if (lhs_t && rhs_t)
        debug("{} {} {} {} {}", str(lhs_t), lhs_t->kind, str(rhs_t), rhs_t->kind, loc.repr());

    auto match = equal(lhs_t, rhs_t);

    if (!match) {
        SEMA_ERROR(TypeError(lhs, lhs_t, rhs, rhs_t, loc));
    }
    return match;
}

TypeExpr *SemanticAnalyser::boolop(BoolOp *n, int depth) {
    auto   bool_type        = make_ref(n, "bool");
    bool   and_implemented  = false;
    bool   rand_implemented = false;
    auto   return_t         = bool_type;
    String magic            = "";
    String rmagic           = "";

    if (n->op == BoolOperator::And) {
        magic  = "__and__";
        rmagic = "__rand__";
    }
    if (n->op == BoolOperator::Or) {
        magic  = "__or__";
        rmagic = "__ror__";
    }

    auto lhs_t = exec(n->values[0], depth);
    for (int i = 1; i < n->values.size(); i++) {
        auto rhs_t = exec(n->values[i], depth);

        //
        //  TODO: we could create a builtin file that define
        //  bool as a class that has the __and__ attribute
        //  that would harmonize this code
        //
        // if not a bool we need to check for
        //  * __and__ inside the lhs
        //  * __rand__ inside the rhs
        if (!equal(lhs_t, bool_type)) {
            auto cls = get_class(bindings, lhs_t);

            if (cls == nullptr) {
                SEMA_ERROR(UnsupportedOperand(str(n->op), lhs_t, rhs_t));
                return nullptr;
            }

            auto fun = getattr(cls, magic);

            if (fun == nullptr) {
                SEMA_ERROR(UnsupportedOperand(str(n->op), lhs_t, rhs_t));
                return nullptr;
            }

            // This is a standard call now
            auto arrow_expr = exec(fun, depth);
            auto arrow      = cast<Arrow>(arrow_expr);

            // Generate the Call Arrow
            auto got = n->new_object<Arrow>();
            got->args.push_back(nullptr); // lhs_t
            got->args.push_back(rhs_t);
            got->returns = arrow->returns;

            typecheck(n, got, nullptr, arrow, LOC);
            lhs_t = arrow->returns;
        }
    }

    return return_t;
}
TypeExpr *SemanticAnalyser::namedexpr(NamedExpr *n, int depth) {
    auto value_t = exec(n->value, depth);
    add_name(n->target, n->value, value_t);
    return value_t;
}
TypeExpr *SemanticAnalyser::binop(BinOp *n, int depth) {

    auto lhs_t = exec(n->left, depth);
    auto rhs_t = exec(n->right, depth);

    typecheck(n->left, lhs_t, n->right, rhs_t, LOC);

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
    add_arguments(n->args, funtype, nullptr, depth);
    auto type        = exec(n->body, depth);
    funtype->returns = type;
    return funtype;
}
TypeExpr *SemanticAnalyser::ifexp(IfExp *n, int depth) {
    auto test_t = exec(n->test, depth);

    typecheck(n->test, test_t, nullptr, make_ref(n, "bool"), LOC);
    auto body_t   = exec(n->body, depth);
    auto orelse_t = exec(n->orelse, depth);

    typecheck(nullptr, body_t, nullptr, orelse_t, LOC);
    return body_t;
}
TypeExpr *SemanticAnalyser::dictexpr(DictExpr *n, int depth) {
    TypeExpr *key_t = nullptr;
    TypeExpr *val_t = nullptr;

    for (int i = 0; i < n->keys.size(); i++) {
        auto key_type = exec(n->keys[i], depth);
        auto val_type = exec(n->values[i], depth);

        if (key_t != nullptr && val_t != nullptr) {
            typecheck(n->keys[i], key_type, nullptr, key_t, LOC);
            typecheck(n->values[i], val_type, nullptr, val_t, LOC);
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
            typecheck(n->elts[i], val_type, nullptr, val_t, LOC);
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

//  def fun(a: int, b:int) -> float:
//      pass
//
// Arrow1: (int, int) -> float
//
// Arrow2: (typeof(arg1), typeof(arg2)) -> ..
//
// <fun>(arg1 arg2 ...)
//

ClassDef *get_class(Bindings const &bindings, ExprNode *classref) {
    auto cls_name = cast<Name>(classref);
    if (!cls_name) {
        return nullptr;
    }
    // Class is a compile-time value
    auto cls_node = bindings.get_value(cls_name->varid);
    auto cls      = cast<ClassDef>(cls_node);
    return cls;
}

Arrow *get_arrow(SemanticAnalyser *self, ExprNode *fun, ExprNode *type, int depth, int &offset) {
    if (type == nullptr) {
        return nullptr;
    }
    switch (type->kind) {
    case NodeKind::Arrow: {
        offset = 0;
        return cast<Arrow>(type);
    }
    case NodeKind::BuiltinType: {
        if (!equal(type, Type_t())) {
            return nullptr;
        }
        auto cls  = get_class(self->bindings, fun);
        auto init = getattr(cls, "__init__");
        offset    = 1;

        if (init == nullptr) {
            debug("Use default ctor");
            Arrow *arrow = fun->new_object<Arrow>();
            arrow->args.push_back(type);
            arrow->returns = fun;
            return arrow;
        } else {
            debug("Got a custom ctor");
            auto   init_t = self->exec(init, depth);
            Arrow *arrow  = cast<Arrow>(init_t);
            // sema should already have run for this
            // arrow->args[0] = type;
            // arrow->returns = fun;
            return arrow;
        }
    }
    }
    return nullptr;
}

TypeExpr *SemanticAnalyser::call(Call *n, int depth) {
    auto type   = exec(n->func, depth);
    int  offset = 0;
    auto arrow  = get_arrow(this, n->func, type, depth, offset);

    if (arrow == nullptr) {
        SEMA_ERROR(TypeError(fmt::format("{} is not callable", str(n->func))));
    }

    // Create the matching Arrow type for this call
    Arrow *got = n->new_object<Arrow>();
    if (arrow != nullptr) {
        got->args.reserve(arrow->args.size());
    }
    if (offset == 1) {
        got->args.push_back(type);
    }
    for (auto &arg: n->args) {
        got->args.push_back(exec(arg, depth));
    }

    // FIXME: we do not know the returns so we just use the one we have
    if (arrow != nullptr) {
        got->returns = arrow->returns;
        typecheck(n, got, n->func, arrow, LOC);
    }

    for (auto &kw: n->keywords) {
        // TODO: Handle keywods
        exec(kw.value, depth);
    }

    if (arrow != nullptr) {
        return arrow->returns;
    }
    return nullptr;
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
    auto type_t = exec(n->value, depth);

    auto class_t = get_class(bindings, type_t);

    if (class_t == nullptr) {
        SEMA_ERROR(NameError(n->value, str(n->value)));
        return nullptr;
    }

    auto attr = getattr(class_t, str(n->attr));

    if (attr == nullptr) {
        SEMA_ERROR(AttributeError(class_t, n->attr));
    }

    auto attr_t = exec(attr, depth);

    return attr_t;
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
        auto id  = bindings.add(n->id, n, nullptr);
        n->varid = id;

        debug("Storing value for {} ({})", n->id, n->varid);
    } else {
        // Both delete & Load requires the variable to be defined first
        n->varid = bindings.get_varid(n->id);
        if (n->varid == -1) {
            debug("Value {} not found", n->id);
            SEMA_ERROR(NameError(n, n->id));
        }
    }

    auto t = bindings.get_type(n->varid);
    if (t == nullptr) {
        debug("Value {} does not have a type", n->id);
    } else {
        debug("Loading value {}: {} of type {}", n->id, n->varid, str(t));
    }
    return t;
}
TypeExpr *SemanticAnalyser::listexpr(ListExpr *n, int depth) {
    TypeExpr *val_t = nullptr;

    for (int i = 0; i < n->elts.size(); i++) {
        auto val_type = exec(n->elts[i], depth);

        if (val_t != nullptr) {
            typecheck(n->elts[i], val_type, nullptr, val_t, LOC);
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

void SemanticAnalyser::add_arguments(Arguments &args, Arrow *arrow, ClassDef *def, int depth) {
    TypeExpr *class_t = nullptr;
    if (def != nullptr) {
        class_t = make_ref(arrow, str(def->name));
    }
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
            typecheck(type, typetype, nullptr, Type_t(), LOC);
        }

        // if default value & annotation types must match
        if (type && dvalue_t) {
            typecheck(arg.annotation.value(), type, dvalue, dvalue_t, LOC);
        }

        // if no annotation use default value type
        if (type == nullptr) {
            type = dvalue_t;
        }

        // if it is a method populate the type of self
        // TODO: fix for staticmethod & class methods
        if (class_t != nullptr && i == 0) {
            type = class_t;
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
            typecheck(type, typetype, nullptr, Type_t(), LOC);
        }

        // if default value & annotation types must match
        if (type && dvalue_t) {
            typecheck(arg.annotation.value(), type, dvalue, dvalue_t, LOC);
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
    // if sema was already done on the function
    if (n->type) {
        return n->type;
    }

    PopGuard nested_stmt(nested, (StmtNode *)n);

    // Add the function name first to handle recursive calls
    auto  id = bindings.add(n->name, n, nullptr);
    Scope scope(bindings);

    auto type = n->new_object<Arrow>();
    add_arguments(n->args, type, cast<ClassDef>(nested_stmt.last(1, nullptr)), depth);

    auto return_effective = exec<TypeExpr *>(n->body, depth);

    if (n->returns.has_value()) {
        // Annotated type takes precedence
        auto return_t = n->returns.value();
        auto typetype = exec(return_t, depth);

        typecheck(return_t, typetype, nullptr, Type_t(), LOC);

        type->returns = return_t;
        typecheck(n->returns.value(), return_t, nullptr, oneof(return_effective), LOC);
    }

    bindings.set_type(id, type);

    // do decorator last since we need to know our function signature to
    // typecheck them
    for (auto decorator: n->decorator_list) {
        auto deco_t = exec(decorator, depth);
        // TODO check the signature here
    }

    // bindings.dump(std::cout);
    n->type = type;
    return type;
}
TypeExpr *SemanticAnalyser::classdef(ClassDef *n, int depth) {
    PopGuard _(nested, n);

    int id = bindings.add(n->name, n, Type_t());

    // TODO: go through bases and add their elements
    for (auto base: n->bases) {
        exec(base, depth);
    }

    //
    for (auto kw: n->keywords) {
        exec(kw.value, depth);
    }

    Array<StmtNode *> methods;
    FunctionDef *     ctor = nullptr;

    // Do a first pass to look for special methods such as __init__
    for (auto &stmt: n->body) {
        Scope scope(bindings);

        auto fun = cast<FunctionDef>(stmt);
        if (fun) {
            n->insert_attribute(fun->name, fun);

            if (str(fun->name) == "__init__") {
                ctor = fun;
            } else {
                methods.push_back(stmt);
            }
            continue;
        }

        auto attr = cast<Assign>(stmt);
        if (attr) {
            // TODO:  get the target type
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
            auto type = exec<TypeExpr *>(attras->value, depth);

            auto typetype = exec(attras->annotation, depth);
            typecheck(attras->annotation, typetype, nullptr, Type_t(), LOC);

            if (type.has_value()) {
                typecheck(attras->target, attras->annotation, attras->value.value(), type.value(),
                          LOC);
            }

            auto name = cast<Name>(attras->target);
            if (name) {
                n->insert_attribute(name->id, stmt, attras->annotation);
            }
            continue;
        }

        debug("Unhandled statement {}", str(stmt->kind));
    }

    // get __init__ and do a pass to insert its attribute to the class
    auto class_t = make_ref(n, str(n->name));

    if (ctor != nullptr) {
        auto ctor_t = cast<Arrow>(exec(ctor, depth));

        if (ctor_t != nullptr) {
            ctor_t->args[0] = class_t;
            ctor_t->returns = class_t;
        }
    }
    // -----------------------------

    // Process the remaining methods;
    for (auto &stmt: methods) {
        Scope scope(bindings);

        // fun_t is the arrow that is saved in the context
        // populate self type
        auto fun   = cast<FunctionDef>(stmt);
        auto fun_t = cast<Arrow>(exec(stmt, depth));
        if (fun_t != nullptr) {
            fun_t->args[0] = class_t;
        }

        if (fun != nullptr) {
            fun->type = fun_t;
        }
    }

    // ----

    for (auto deco: n->decorator_list) {
        auto deco_t = exec(deco, depth);
        // TODO: check signature here
    }
    return Type_t();
}
TypeExpr *SemanticAnalyser::returnstmt(Return *n, int depth) {
    auto v = exec<TypeExpr *>(n->value, depth);
    if (v.has_value()) {
        return v.value();
    }
    return None_t();
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
        auto target = n->targets[0];

        if (target->kind == NodeKind::Name) {
            add_name(target, n->value, type);
        } else if (target->kind == NodeKind::Attribute) {
            auto target_t = exec(target, depth);
            typecheck(target, target_t, n->value, type, LOC);
        } else {
            error("Assignment to an unsupported expression {}", str(target->kind));
        }

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

    typecheck(n->value, type, n->target, expected_type, LOC);
    return type;
}

//! Annotation takes priority over the deduced type
//! this enbles users to use annotation to debug
TypeExpr *SemanticAnalyser::annassign(AnnAssign *n, int depth) {
    auto constraint = n->annotation;
    auto typetype   = exec(n->annotation, depth);

    // Type annotation must be a type
    typecheck(n->annotation, typetype, nullptr, Type_t(), LOC);

    auto type = exec<TypeExpr *>(n->value, depth);

    ExprNode *value = nullptr;

    if (type.has_value()) {
        // if we were able to deduce a type from the expression
        // make sure it matches the annotation constraint
        typecheck(n->target, constraint,          //
                  n->value.value(), type.value(), //
                  LOC);

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
            SEMA_ERROR(NameError(n, name));
        }
    }
    return nullptr;
}
TypeExpr *SemanticAnalyser::nonlocal(Nonlocal *n, int depth) {
    for (auto &name: n->names) {
        auto varid = bindings.get_varid(name);
        if (varid == -1) {
            SEMA_ERROR(NameError(n, name));
        }
    }
    return nullptr;
}
TypeExpr *SemanticAnalyser::exprstmt(Expr *n, int depth) { return exec(n->value, depth); }
TypeExpr *SemanticAnalyser::pass(Pass *n, int depth) { return None_t(); }
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

TypeExpr *SemanticAnalyser::dicttype(DictType *n, int depth) { return Type_t(); }
TypeExpr *SemanticAnalyser::arraytype(ArrayType *n, int depth) { return Type_t(); }
TypeExpr *SemanticAnalyser::arrow(Arrow *n, int depth) { return Type_t(); }
TypeExpr *SemanticAnalyser::builtintype(BuiltinType *n, int depth) { return Type_t(); }
TypeExpr *SemanticAnalyser::tupletype(TupleType *n, int depth) { return Type_t(); }
TypeExpr *SemanticAnalyser::settype(SetType *n, int depth) { return Type_t(); }
TypeExpr *SemanticAnalyser::classtype(ClassType *n, int depth) { return Type_t(); }

} // namespace lython
