#include "sema/sema.h"
#include "ast/magic.h"
#include "builtin/operators.h"
#include "dependencies/fmt.h"
#include "utilities/guard.h"
#include "utilities/strings.h"

namespace lython {
Arrow* get_arrow(
    SemanticAnalyser* self, ExprNode* fun, ExprNode* type, int depth, int& offset, ClassDef*& cls);

bool SemanticAnalyser::add_name(ExprNode* expr, ExprNode* value, ExprNode* type) {
    auto name = cast<Name>(expr);

    if (name) {
        name->ctx = ExprContext::Store;
        bindings.add(name->id, value, type);
        return true;
    }

    return false;
}

bool SemanticAnalyser::typecheck(
    ExprNode* lhs, TypeExpr* lhs_t, ExprNode* rhs, TypeExpr* rhs_t, CodeLocation const& loc) {

    if (lhs_t && rhs_t) {
        lython::log(lython::LogLevel::Debug,
                    loc,
                    "lhs_t: {} kind {} rhs_t: {} kind:{}",
                    str(lhs_t),
                    lhs_t->kind,
                    str(rhs_t),
                    rhs_t->kind);
    }

    auto match = equal(lhs_t, rhs_t);

    if (!match) {
        SEMA_ERROR(lhs, TypeError, lhs, lhs_t, rhs, rhs_t, loc);
    }
    return match;
}

bool SemanticAnalyser::is_type(TypeExpr* type, int depth, lython::CodeLocation const& loc) {
    TypeExpr* value_t_t = exec(type, depth);

    return typecheck(type,       // int
                     value_t_t,  // Type
                     nullptr,    //
                     Type_t(),   // Type
                     loc);
}

Tuple<ClassDef*, FunctionDef*>
SemanticAnalyser::find_method(TypeExpr* class_type, String const& methodname, int depth) {
    ClassDef* cls = get_class(class_type, depth);

    if (cls == nullptr) {
        return std::make_tuple(nullptr, nullptr);
    }

    TypeExpr* op_type = nullptr;

    return std::make_tuple(cls, cast<FunctionDef>(getattr(cls, methodname, op_type)));
}

// classname.__and__
String SemanticAnalyser::operator_function(TypeExpr* expr_t, StringRef op) {
    auto      cls_ref  = cast<Name>(expr_t);
    ClassDef* expr_cls = cast<ClassDef>(load_name(cls_ref));

    if (expr_cls) {
        return join(".", Array<String>{expr_cls->cls_namespace, String(StringView(op))});
    }

    return "";
}

TypeExpr* SemanticAnalyser::boolop(BoolOp* n, int depth) {
    auto bool_type        = make_ref(n, "bool");
    bool and_implemented  = false;
    bool rand_implemented = false;
    auto return_t         = bool_type;

    StringRef magic  = operator_magic_name(n->op, false);
    StringRef rmagic = operator_magic_name(n->op, true);

    // TODO: what about inheritance ?
    ExprNode* lhs    = n->values[0];
    TypeExpr* lhs_t  = exec(lhs, depth);
    String    lhs_op = operator_function(lhs_t, magic);

    ExprNode* rhs   = nullptr;
    TypeExpr* rhs_t = nullptr;
    String    rhs_op;

    for (int i = 1; i < n->values.size(); i++) {
        rhs   = n->values[i];
        rhs_t = exec(rhs, depth);

        String signature = join("-", Array<String>{str(n->op), str(lhs_t), str(rhs_t)});
        auto   handler   = get_native_bool_operation(signature);

        if (handler) {
            n->native_operator = handler;
        } else {
            // Not a native operator
            int lhs_op_varid = bindings.get_varid(lhs_op);
            if (lhs_op_varid > 0) {
                // found the function we are calling
                n->varid = lhs_op_varid;

                Arrow* operator_type = cast<Arrow>(bindings.get_type(lhs_op_varid));

                assert(operator_type, "Bool operator needs to be Callable");
                assert(operator_type->args.size() == 2, "Bool operator requires 2 arguments");

                typecheck(lhs, lhs_t, nullptr, operator_type->args[0], LOC);
                typecheck(rhs, rhs_t, nullptr, operator_type->args[1], LOC);
                typecheck(nullptr, operator_type->returns, nullptr, make_ref(n, "bool"), LOC);
            } else {
                int rhs_op_varid = bindings.get_varid(rhs_op);
                if (rhs_op_varid > 0) {
                    // found the function we are calling
                    n->varid = rhs_op_varid;

                    Arrow* operator_type = cast<Arrow>(bindings.get_type(rhs_op_varid));

                    assert(operator_type, "Bool operator needs to be Callable");
                    assert(operator_type->args.size() == 2, "Bool operator requires 2 arguments");

                    typecheck(lhs, lhs_t, nullptr, operator_type->args[1], LOC);
                    typecheck(rhs, rhs_t, nullptr, operator_type->args[0], LOC);
                    typecheck(nullptr, operator_type->returns, nullptr, make_ref(n, "bool"), LOC);
                }

                if (lhs_op_varid == -1 && rhs_op_varid == -1) {
                    SEMA_ERROR(n, UnsupportedOperand, str(n->op), lhs_t, rhs_t);
                }
            }
        }

        lhs_t  = rhs_t;
        lhs    = rhs;
        lhs_op = operator_function(lhs_t, magic);

        /*
        //
        //  TODO: we could create a builtin file that define
        //  bool as a class that has the __and__ attribute
        //  that would harmonize this code
        //
        // if not a bool we need to check for
        //  * __and__ inside the lhs
        //  * __rand__ inside the rhs
        if (!equal(lhs_t, bool_type)) {

            ClassDef*    cls = nullptr;
            FunctionDef* fun = nullptr;

            std::tie(cls, fun) = find_method(lhs_t, magic);

            if (fun == nullptr) {
                SEMA_ERROR(UnsupportedOperand(str(n->op), lhs_t, rhs_t));
                return nullptr;
            }

            // This is a standard call now
            auto arrow_expr = exec(fun, depth);
            auto arrow      = cast<Arrow>(arrow_expr);

            // Generate the Call Arrow
            auto got = n->new_object<Arrow>();
            got->args.push_back(make_ref(got, str(cls->name)));  // lhs_t
            got->args.push_back(rhs_t);
            got->returns = arrow->returns;

            typecheck(n, got, nullptr, arrow, LOC);
            lhs_t = arrow->returns;
        }
        */
    }

    return return_t;
}
TypeExpr* SemanticAnalyser::namedexpr(NamedExpr* n, int depth) {
    auto value_t = exec(n->value, depth);
    add_name(n->target, n->value, value_t);
    return value_t;
}

TypeExpr* SemanticAnalyser::resolve_variable(ExprNode* node) {

    Name* name = cast<Name>(node);

    if (name) {
        assert(name->varid >= 0, "Type need to be resolved");
        return static_cast<TypeExpr*>(bindings.get_value(name->varid));
    }

    return nullptr;
}

TypeExpr* SemanticAnalyser::compare(Compare* n, int depth) {

    auto prev_t = exec(n->left, depth);
    auto prev   = n->left;

    for (int i = 0; i < n->ops.size(); i++) {
        auto op    = n->ops[i];
        auto cmp   = n->comparators[i];
        auto cmp_t = exec(cmp, depth);

        // Check if we have a native function to handle this
        String signature = join(String("-"), Array<String>{str(op), str(prev_t), str(cmp_t)});

        // TODO: get return type
        auto handler = get_native_cmp_operation(signature);
        n->native_operator.push_back(handler);

        if (!handler) {
            SEMA_ERROR(n, UnsupportedOperand, str(op), prev_t, cmp_t);
        }
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

TypeExpr* SemanticAnalyser::binop(BinOp* n, int depth) {

    auto lhs_t = exec(n->left, depth);
    auto rhs_t = exec(n->right, depth);

    typecheck(n->left, lhs_t, n->right, rhs_t, LOC);

    TypeExpr*    type = resolve_variable(lhs_t);
    BuiltinType* blt  = cast<BuiltinType>(type);

    // Builtin type, all the operations are known
    if (blt) {
        String signature = join(String("-"), Array<String>{str(n->op), str(lhs_t), str(rhs_t)});

        debug("signature: {}", signature);
        n->native_operator = get_native_binary_operation(signature);

        // FIXME: get return type
        return lhs_t;
    } else {
        // TODO: Not a builtin type, so it is user defined
        // need to check for a magic method that overload the right operator

        // Get the return value of the custom operator
        return lhs_t;
    }

    // TODO: check that op is defined for those types
    // use the op return type here
    return lhs_t;
}
TypeExpr* SemanticAnalyser::unaryop(UnaryOp* n, int depth) {
    auto expr_t = exec(n->operand, depth);

    String signature = join("-", Array<String>{str(n->op), str(expr_t)});

    UnaryOp::NativeUnaryOp handler = get_native_unary_operation(signature);
    if (!handler) {
        SEMA_ERROR(n, UnsupportedOperand, str(n->op), expr_t, nullptr);
    }

    // assert(handler != nullptr, "Unary operation require native handler");
    n->native_operator = handler;

    // TODO: fetch native operator
    // TODO: check that op is defined for this type
    // use the op return type here
    return expr_t;
}
TypeExpr* SemanticAnalyser::lambda(Lambda* n, int depth) {
    Scope scope(bindings);
    auto  funtype = n->new_object<Arrow>();
    add_arguments(n->args, funtype, nullptr, depth);
    auto type        = exec(n->body, depth);
    funtype->returns = type;
    return funtype;
}
TypeExpr* SemanticAnalyser::ifexp(IfExp* n, int depth) {
    auto test_t = exec(n->test, depth);

    typecheck(n->test, test_t, nullptr, make_ref(n, "bool"), LOC);
    auto body_t   = exec(n->body, depth);
    auto orelse_t = exec(n->orelse, depth);

    typecheck(nullptr, body_t, nullptr, orelse_t, LOC);
    return body_t;
}
TypeExpr* SemanticAnalyser::dictexpr(DictExpr* n, int depth) {
    TypeExpr* key_t = nullptr;
    TypeExpr* val_t = nullptr;

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

    DictType* type = n->new_object<DictType>();
    type->key      = key_t;
    type->value    = val_t;
    return type;
}
TypeExpr* SemanticAnalyser::setexpr(SetExpr* n, int depth) {
    TypeExpr* val_t = nullptr;

    for (int i = 0; i < n->elts.size(); i++) {
        auto val_type = exec(n->elts[i], depth);

        if (val_t != nullptr) {
            typecheck(n->elts[i], val_type, nullptr, val_t, LOC);
        } else {
            val_t = val_type;
        }
    }

    SetType* type = n->new_object<SetType>();
    type->value   = val_t;
    return type;
}
TypeExpr* SemanticAnalyser::listcomp(ListComp* n, int depth) {
    Scope scope(bindings);
    for (auto& gen: n->generators) {
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
TypeExpr* SemanticAnalyser::generateexpr(GeneratorExp* n, int depth) {
    Scope scope(bindings);
    for (auto& gen: n->generators) {
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
TypeExpr* SemanticAnalyser::setcomp(SetComp* n, int depth) {
    Scope scope(bindings);
    for (auto& gen: n->generators) {
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

TypeExpr* SemanticAnalyser::dictcomp(DictComp* n, int depth) {
    Scope scope(bindings);
    for (auto& gen: n->generators) {
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
TypeExpr* SemanticAnalyser::await(Await* n, int depth) { return exec(n->value, depth); }
TypeExpr* SemanticAnalyser::yield(Yield* n, int depth) {
    get_context().yield = true;

    auto r = exec<TypeExpr*>(n->value, depth);
    if (r.has_value()) {
        return r.value();
    }
    return nullptr;
}
TypeExpr* SemanticAnalyser::yieldfrom(YieldFrom* n, int depth) { return exec(n->value, depth); }

//  def fun(a: int, b:int) -> float:
//      pass
//
// Arrow1: (int, int) -> float
//
// Arrow2: (typeof(arg1), typeof(arg2)) -> ..
//
// <fun>(arg1 arg2 ...)
//

Node* SemanticAnalyser::load_name(Name_t* n) {
    if (n == nullptr) {
        return nullptr;
    }

    Node* result = nullptr;
    int   varid  = 0;

    if (n->dynamic) {
        // Local variables | Arguments
        assert(n->offset != -1, "Reference should have a reverse lookup offset");
        varid  = int(bindings.bindings.size()) - n->offset;
        result = bindings.get_value(varid);
    } else {
        // Global variables
        result = bindings.get_value(n->varid);
    }

    return result;
}

ClassDef* SemanticAnalyser::get_class(ExprNode* classref, int depth) {
    auto cls_name = cast<Name>(classref);

    if (!cls_name) {
        return nullptr;
    }

    cls_name->ctx = ExprContext::Load;
    // assert(cls_name->ctx == ExprContext::Load, "Reference to the class should be loaded");
    auto cls = cast<ClassDef>(load_name(cls_name));

    return cls;
}

Arrow*
SemanticAnalyser::get_arrow(ExprNode* fun, ExprNode* type, int depth, int& offset, ClassDef*& cls) {

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

        // Fetch the class type inside the binding
        cls = get_class(fun, depth);

        if (cls == nullptr) {
            warn("Could not resolve class");
            return nullptr;
        }
        TypeExpr* arrow = nullptr;

        // The methods were added to the context outside of the class itself
        // here we need to resolve the method which is inside the context

        // NOTE: we should call __new__ here implictly
        //

        String ctor_name = String(cls->name) + String(".__init__");
        int    ctorvarid = bindings.get_varid(ctor_name);
        Node*  ctor      = bindings.get_value(ctorvarid);

        // Now we can switch the function being called from being the class
        // to being the constructor of the class
        assert(fun->kind == NodeKind::Name, "Expect a reference to the class");

        // update the ref to point to the constructor if we found it
        Name* fun_ref = cast<Name>(fun);
        if (fun_ref && ctorvarid != -1) {
            fun_ref->varid   = ctorvarid;
            fun_ref->size    = int(bindings.bindings.size()) - fun_ref->varid;
            fun_ref->dynamic = bindings.is_dynamic(fun_ref->varid);
        } else {
            warn("Constructor was not found");
        }

        FunctionDef* init = cast<FunctionDef>(ctor);
        // auto init = getattr(cls, "__init__", arrow);
        offset = 1;

        if (init == nullptr) {
            debug("Use default ctor");
            auto   cls_ref = make_ref(fun, cls->name);
            Arrow* arrow   = fun->new_object<Arrow>();
            arrow->add_arg_type(cls_ref);
            arrow->returns = cls_ref;
            return arrow;
        } else {
            // ctor should already have been sema'ed
            if (init->type) {
                return init->type;
            }

            // if not then we are doing it right now and this is a circle
            SEMA_ERROR(fun, RecursiveDefinition, "", fun, cls);
            return nullptr;
        }
    }
    default: break;
    }
    return nullptr;
}

TypeExpr* SemanticAnalyser::call(Call* n, int depth) {
    // Get the type of the function
    auto type = exec(n->func, depth);

    int       offset = 0;
    ClassDef* cls    = nullptr;
    auto      arrow  = get_arrow(n->func, type, depth, offset, cls);

    if (arrow == nullptr) {
        SEMA_ERROR(n, TypeError, fmt::format("{} is not callable", str(n->func)));
    }

    // Sort kwargs to make them positional
    // Get Arguments and generate an arrow from it

    // Create the matching Arrow type for this call
    Arrow* got = n->new_object<Arrow>();
    if (arrow != nullptr) {
        got->args.reserve(arrow->arg_count());
    }

    // Method, insert the self argument since it is implicit
    if (offset == 1 && cls) {
        got->add_arg_type(make_ref(got, str(cls->name)));
    }

    for (auto& arg: n->args) {
        got->add_arg_type(exec(arg, depth));
    }

    Dict<StringRef, ExprNode*> kwargs;
    for (auto& kw: n->keywords) {
        kwargs[kw.arg] = exec(kw.value, depth);
    }

    if (arrow != nullptr) {
        for (int i = int(got->arg_count()); i < arrow->names.size(); i++) {
            auto name = arrow->names[i];

            auto item = kwargs.find(name);
            if (item == kwargs.end()) {
                auto value = got->defaults[name];
                if (value) {
                    SEMA_ERROR(n, TypeError, fmt::format("Arguement {} is not set", name));
                }
                // Got default use the expected type
                got->add_arg_type(arrow->args[i]);
                continue;
            }

            got->add_arg_type(item->second);
        }
    }

    // FIXME: we do not know the returns so we just use the one we have
    if (arrow != nullptr) {
        if (got->arg_count() != arrow->arg_count()) {
            // we do not really that check
            // SEMA_ERROR(TypeError(" missing {} required positional arguments"));
            //
        }

        got->returns = arrow->returns;
        typecheck(n, got, n->func, arrow, LOC);
    }

    if (arrow != nullptr) {
        return arrow->returns;
    }
    return nullptr;
}
TypeExpr* SemanticAnalyser::joinedstr(JoinedStr* n, int depth) { return nullptr; }
TypeExpr* SemanticAnalyser::formattedvalue(FormattedValue* n, int depth) { return nullptr; }
TypeExpr* SemanticAnalyser::constant(Constant* n, int depth) {
    switch (n->value.type()) {
    case ConstantValue::Ti8: return make_ref(n, "i8");
    case ConstantValue::Ti16: return make_ref(n, "i16");
    case ConstantValue::Ti32: return make_ref(n, "i32");
    case ConstantValue::Ti64: return make_ref(n, "i64");

    case ConstantValue::Tu8: return make_ref(n, "u8");
    case ConstantValue::Tu16: return make_ref(n, "u16");
    case ConstantValue::Tu32: return make_ref(n, "u32");
    case ConstantValue::Tu64: return make_ref(n, "u64");

    case ConstantValue::Tf32: return make_ref(n, "f32");
    case ConstantValue::Tf64: return make_ref(n, "f64");
    case ConstantValue::TString: return make_ref(n, "str");
    case ConstantValue::TBool: return make_ref(n, "bool");
    default: return nullptr;
    }

    return nullptr;
}

// This is only called when loading
TypeExpr* SemanticAnalyser::attribute(Attribute* n, int depth) {
    // <value>.<name>
    auto type_t  = exec(n->value, depth);
    auto class_t = get_class(type_t, depth);

    if (class_t == nullptr) {
        // class was not found, this is probably because the lookup
        // of the value failed, it should have produced a precise error
        // so we do not have to
        // SEMA_ERROR(NameError(n->value, str(n->value)));
        return nullptr;
    }

    n->attrid = class_t->get_attribute(n->attr);
    if (n->attrid == -1) {
        SEMA_ERROR(n, AttributeError, class_t, n->attr);
        return nullptr;
    }

    ClassDef::Attr& attr = class_t->attributes[n->attrid];

    if (attr.type && is_type(attr.type, depth, LOC)) {
        return attr.type;
    }
    return nullptr;
}

TypeExpr* SemanticAnalyser::attribute_assign(Attribute* n, int depth, TypeExpr* expected) {
    // self: Ref[class_t]
    auto type_t  = exec(n->value, depth);
    auto class_t = get_class(type_t, depth);

    if (class_t == nullptr) {
        SEMA_ERROR(n, NameError, n->value, str(n->value));
        return nullptr;
    }

    n->attrid = class_t->get_attribute(n->attr);
    if (n->attrid == -1) {
        SEMA_ERROR(n, AttributeError, class_t, n->attr);
        return nullptr;
    }

    // Update attribute type when we are in an assignment
    ClassDef::Attr& attr = class_t->attributes[n->attrid];
    if (n->attrid > 0 && attr.type == nullptr) {
        attr.type = expected;
    }

    if (attr.type && is_type(attr.type, depth, LOC)) {
        return attr.type;
    }
    return nullptr;
}

TypeExpr* SemanticAnalyser::subscript(Subscript* n, int depth) {
    auto class_t = exec(n->value, depth);
    exec(n->slice, depth);
    // check that __getitem__ is defined in class_t
    return nullptr;
}
TypeExpr* SemanticAnalyser::starred(Starred* n, int depth) {
    // value should be of an expandable type
    auto value_t = exec(n->value, depth);
    return nullptr;
}
TypeExpr* SemanticAnalyser::name(Name* n, int depth) {
    if (n->ctx == ExprContext::Store) {
        auto id  = bindings.add(n->id, n, nullptr);
        n->varid = id;
        n->size  = int(bindings.bindings.size());

        debug("Storing value for {} ({})", n->id, n->varid);
    } else {
        // Both delete & Load requires the variable to be defined first
        n->varid   = bindings.get_varid(n->id);
        n->size    = int(bindings.bindings.size());
        n->offset  = int(bindings.bindings.size()) - n->varid;
        n->dynamic = bindings.is_dynamic(n->varid);

        if (n->varid == -1) {
            debug("Value {} not found", n->id);
            SEMA_ERROR(n, NameError, n, n->id);
        }
    }

    // assert(n->varid != -1, "Should have been founds");

    auto t = bindings.get_type(n->varid);
    if (t == nullptr) {
        debug("Value {} does not have a type", n->id);
    } else {
        debug("Loading value {}: {} of type {}", n->id, n->varid, str(t));
    }
    return t;
}

TypeExpr* SemanticAnalyser::comment(Comment* n, int depth) { return nullptr; }

TypeExpr* SemanticAnalyser::listexpr(ListExpr* n, int depth) {
    TypeExpr* val_t = nullptr;

    for (int i = 0; i < n->elts.size(); i++) {
        auto val_type = exec(n->elts[i], depth);

        if (!val_type) {
            // FIXME Could not find tyep for n->elts[i]
        } else if (val_t == nullptr && is_type(val_type, depth, LOC)) {
            val_t = val_type;
        } else if (val_t != nullptr) {
            typecheck(n->elts[i], val_type, nullptr, val_t, LOC);
        }
    }

    ArrayType* type = n->new_object<ArrayType>();
    type->value     = val_t;
    return type;
}
TypeExpr* SemanticAnalyser::tupleexpr(TupleExpr* n, int depth) {
    TupleType* type = n->new_object<TupleType>();
    type->types.reserve(n->elts.size());

    for (int i = 0; i < n->elts.size(); i++) {
        TypeExpr* val_t = exec(n->elts[i], depth);

        if (!val_t) {
            // FIXME Could not find tyep for n->elts[i]

        } else if (!is_type(val_t, depth, LOC)) {
            val_t = nullptr;
        }

        type->types.push_back(val_t);
    }

    return type;
}
TypeExpr* SemanticAnalyser::slice(Slice* n, int depth) {
    exec<TypeExpr*>(n->lower, depth);
    exec<TypeExpr*>(n->upper, depth);
    exec<TypeExpr*>(n->step, depth);
    return nullptr;
}

// name : annotation = value
// void SemanticAnalyser::add_argument(
//     Identifier const& name, TypeExpr* annotation, ExprNode* value, Arrow* arrow, int depth) {

//     TypeExpr* value_t = exec(value, depth);

// }

void SemanticAnalyser::add_arguments(Arguments& args, Arrow* arrow, ClassDef* def, int depth) {
    TypeExpr* class_t = nullptr;
    if (def != nullptr) {
        class_t = make_ref(arrow, str(def->name));
    }

    auto resolve_argument = [&](TypeExpr* annotation, ExprNode* value) -> TypeExpr* {
        TypeExpr* value_t       = nullptr;
        bool      value_t_valid = false;
        bool      ann_valid     = false;

        if (value) {
            value_t = exec(value, depth);  // Infer the type of the value

            if (!value_t) {
                // FXIME: Could not find type for value
            } else {
                value_t_valid = is_type(value_t, depth, LOC);
            }
        }

        if (annotation != nullptr) {
            ann_valid = is_type(annotation, depth, LOC);
        }

        if (ann_valid && value_t_valid) {
            typecheck(nullptr, annotation, value, value_t, LOC);
        }

        if (ann_valid) {
            return annotation;
        }

        if (value_t_valid) {
            return value_t;
        }

        return nullptr;
    };

    for (int i = 0, n = int(args.args.size()); i < n; i++) {
        Arg arg = args.args[i];

        ExprNode* value      = nullptr;
        ExprNode* annotation = arg.annotation.has_value() ? arg.annotation.value() : nullptr;

        int d = int(args.defaults.size()) - (n - i);
        if (d >= 0) {
            value                    = args.defaults[d];
            arrow->defaults[arg.arg] = true;
        }

        TypeExpr* type = resolve_argument(annotation, value);

        // if it is a method populate the type of self
        // TODO: fix for staticmethod & class methods
        if (class_t != nullptr && i == 0) {
            debug("Insert class type");
            type = class_t;
        }

        // we could populate the default value here
        // but we would not want sema to think this is a constant
        bindings.add(arg.arg, nullptr, type);

        if (arrow) {
            arrow->names.push_back(arg.arg);

            if (!arrow->add_arg_type(type)) {
                SEMA_ERROR(
                    arrow, TypeError, fmt::format("Cannot have a function type refer to itself"));
            }
        }
    }

    for (int i = 0, n = int(args.kwonlyargs.size()); i < n; i++) {
        auto arg = args.kwonlyargs[i];

        ExprNode* value      = nullptr;
        ExprNode* annotation = arg.annotation.has_value() ? arg.annotation.value() : nullptr;

        int d = int(args.kw_defaults.size()) - (n - i);

        if (d >= 0) {
            value                    = args.kw_defaults[d];
            arrow->defaults[arg.arg] = true;
        }

        TypeExpr* type = resolve_argument(annotation, value);

        bindings.add(arg.arg, nullptr, type);

        if (arrow) {
            arrow->names.push_back(arg.arg);
            if (!arrow->add_arg_type(type)) {
                SEMA_ERROR(
                    arrow, TypeError, fmt::format("Cannot have a function type refer to itself"));
            }
        }
    }
}

Arrow* SemanticAnalyser::functiondef_arrow(FunctionDef* n, StmtNode* class_t, int depth) {
    Arrow* type = n->new_object<Arrow>();

    {
        PopGuard ctx(semactx, SemaContext{false, true});

        if (n->returns.has_value()) {
            auto return_t = n->returns.value();

            if (is_type(return_t, depth, LOC)) {
                type->returns = n->returns.value();
            }
        }

        // The arguments will be rolled back

        add_arguments(n->args, type, cast<ClassDef>(class_t), depth);
        // --
    }

    return type;
}

String SemanticAnalyser::generate_function_name(FunctionDef* n) {
    // Generate the function name
    String funname = String(n->name);
    if (namespaces.size() > 0) {
        String cls_namespace = join(".", namespaces);
        funname              = fmtstr("{}.{}", cls_namespace, funname);
    }

    return funname;
}

TypeExpr* SemanticAnalyser::functiondef(FunctionDef* n, int depth) {
    // if sema was already done on the function
    if (n->type) {
        info("Send cached type {}", str(n->type));
        return n->type;
    }

    String funname = generate_function_name(n);

    PopGuard  _(namespaces, str(n->name));
    PopGuard  nested_stmt(nested, (StmtNode*)n);
    StmtNode* lst = nested_stmt.last(1, nullptr);

    // Insert the function into the global context
    // the arrow type is not created right away to prevent
    // circular typing
    auto id = bindings.add(funname, n, nullptr);

    // Enter function context
    Scope scope(bindings);

    // Create the function type from the arguments
    // this will also add the arguments to the context
    Arrow*    fun_type = functiondef_arrow(n, lst, depth);
    TypeExpr* return_t = fun_type->returns;

    // Update the function type at the very end
    bindings.set_type(id, fun_type);

    // Infer return type from the body
    PopGuard ctx(semactx, SemaContext());
    auto     return_effective = exec<TypeExpr*>(n->body, depth);

    if (n->returns.has_value()) {
        // Annotated type takes precedence
        typecheck(n->returns.value(), return_t, nullptr, oneof(return_effective), LOC);
    }

    // do decorator last since we need to know our function signature to
    // typecheck them
    for (auto decorator: n->decorator_list) {
        auto deco_t = exec(decorator.expr, depth);
        // TODO check the signature here
    }

    n->type      = fun_type;
    n->generator = get_context().yield;
    return fun_type;
}

void SemanticAnalyser::record_attributes(ClassDef*               n,
                                         Array<StmtNode*> const& body,
                                         Array<StmtNode*>&       methods,
                                         FunctionDef**           ctor,
                                         int                     depth) {

    for (auto& stmt: n->body) {
        Scope scope(bindings);

        // Assignment
        ExprNode* target   = nullptr;
        TypeExpr* target_t = nullptr;

        ExprNode* value   = nullptr;
        TypeExpr* value_t = nullptr;

        switch (stmt->kind) {
        case NodeKind::FunctionDef: {
            // Look for special functions
            auto fun = cast<FunctionDef>(stmt);
            n->insert_attribute(fun->name, fun);

            if (str(fun->name) == "__init__") {
                info("Found ctor");
                ctor[0] = fun;
            } else {
                methods.push_back(stmt);
            }
            continue;
        }
        case NodeKind::Assign: {
            auto assn = cast<Assign>(stmt);
            target    = assn->targets[0];
            value     = assn->value;
            break;
        }
        case NodeKind::AnnAssign: {
            auto ann = cast<AnnAssign>(stmt);
            target   = ann->target;
            value    = ann->value.fold(nullptr);
            target_t = is_type(ann->annotation, depth, LOC) ? ann->annotation : nullptr;
            break;
        }
        default: debug("Unhandled statement {}", str(stmt->kind)); continue;
        }

        auto name = cast<Name>(target);

        // Not a variable, move on
        if (name == nullptr) {
            continue;
        }

        // try to deduce type fo the value
        if (value) {
            value_t = exec(value, depth);

            if (!is_type(value_t, depth, LOC)) {
                value_t = nullptr;
            }
        }

        // if both types are available do a type check
        if (target_t != nullptr && value_t != nullptr) {
            typecheck(target, target_t, value, value_t, LOC);
        }

        // insert attribute using the annotation first
        n->insert_attribute(name->id, stmt, target_t ? target_t : value_t);
    }
}

void record_ctor_attributes(SemanticAnalyser* sema, ClassDef* n, FunctionDef* ctor, int depth) {
    if (ctor->args.args.size() <= 0) {
        error("__init__ without self");
        return;
    }

    info("Looking for attributes inside the ctor");
    auto self = ctor->args.args[0].arg;

    // we need the arguments in the scope so we can look them up
    Arrow arrow;

    Scope _(sema->bindings);
    sema->add_arguments(ctor->args, &arrow, n, depth);

    for (auto stmt: ctor->body) {
        ExprNode* attr_expr = nullptr;
        ExprNode* value     = nullptr;
        TypeExpr* type      = nullptr;

        switch (stmt->kind) {
        case NodeKind::Assign: {
            auto assn = cast<Assign>(stmt);
            attr_expr = assn->targets[0];
            value     = assn->value;
            break;
        }
        case NodeKind::AnnAssign: {
            auto ann  = cast<AnnAssign>(stmt);
            attr_expr = ann->target;
            value     = ann->value.has_value() ? ann->value.value() : nullptr;
            type      = ann->annotation;
            break;
        }
        default: break;
        }

        // if stmt is a comment
        if (attr_expr == nullptr) {
            continue;
        }

        if (attr_expr->kind != NodeKind::Attribute) {
            continue;
        }

        auto attr = cast<Attribute>(attr_expr);
        auto name = cast<Name>(attr->value);

        if (name == nullptr) {
            continue;
        }

        if (name->id != self) {
            continue;
        }

        //
        // TODO: think about this, sema will reexecute that
        // the type should be added else where because we are inside a function
        // and we are processing only a subset of statements
        //
        // TODO: maybe we should nto rely on `sema->exec(ctor, depth)`
        //
        // Try to guess the attribute type
        if (type == nullptr && value != nullptr) {
            type = sema->exec(value, depth);
        }

        n->insert_attribute(attr->attr, stmt, type);
    }

    return;
}
TypeExpr* SemanticAnalyser::classdef(ClassDef* n, int depth) {
    PopGuard _(nested, n);
    PopGuard _n(namespaces, str(n->name));

    // a class is a new type
    // the type of a class is type
    int   id      = bindings.add(n->name, n, Type_t());
    Name* class_t = make_ref(n, str(n->name), id);

    // TODO: go through bases and add their elements
    for (auto base: n->bases) {
        exec(base, depth);
    }

    //
    for (auto kw: n->keywords) {
        exec(kw.value, depth);
    }

    Array<StmtNode*> methods;
    FunctionDef*     ctor = nullptr;

    // we insert methods as regular functions
    // this makes the structure flat and makes lookup O(1)
    // instead of possible O(n) if the method is netested n times

    String cls_namespace = join(".", namespaces);
    n->cls_namespace     = cls_namespace;

    // Do a first pass to look for special methods such as __init__
    // the attributes here are also static
    record_attributes(n, n->body, methods, &ctor, depth);
    // -----------------------------

    // get __init__ and do a pass to insert its attribute to the class
    if (ctor != nullptr) {
        // Traverse body to look for our attributes
        record_ctor_attributes(this, n, ctor, depth);

        // add the constructor to the context
        auto ctor_t = cast<Arrow>(exec(ctor, depth));

        // Fix ctor type
        if (ctor_t != nullptr && ctor_t->arg_count() > 0) {
            ctor_t->set_arg_type(0, class_t);
            ctor_t->returns = class_t;
        }
    }
    // -----------------------------

    // Process the remaining methods;
    for (auto& stmt: methods) {

        // fun_t is the arrow that is saved in the context
        // populate self type
        auto fun   = cast<FunctionDef>(stmt);
        auto fun_t = cast<Arrow>(exec(stmt, depth));

        if (fun_t != nullptr && fun_t->arg_count() > 0) {
            fun_t->set_arg_type(0, class_t);
        }
    }

    // ----

    for (auto deco: n->decorator_list) {
        auto deco_t = exec(deco.expr, depth);
        // TODO: check signature here
    }

    return Type_t();
}

TypeExpr* SemanticAnalyser::invalidstmt(InvalidStatement_t* n, int depth) {
    // ignore it, parser already showed this error
    return None_t();
}
TypeExpr* SemanticAnalyser::returnstmt(Return* n, int depth) {
    auto v = exec<TypeExpr*>(n->value, depth);
    if (v.has_value()) {
        return v.value();
    }
    return None_t();
}
TypeExpr* SemanticAnalyser::deletestmt(Delete* n, int depth) {
    for (auto target: n->targets) {
        exec(target, depth);
    }
    return nullptr;
}
TypeExpr* SemanticAnalyser::assign(Assign* n, int depth) {
    auto type = exec(n->value, depth);

    // TODO: check if the assigned name already exist or not
    //
    if (n->targets.size() == 1) {
        auto target = n->targets[0];

        if (target->kind == NodeKind::Name) {
            add_name(target, n->value, type);
        } else if (target->kind == NodeKind::Attribute) {
            // Deduce the type of the attribute from the value
            auto target_t = attribute_assign(cast<Attribute>(n->targets[0]), depth, type);

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
TypeExpr* SemanticAnalyser::augassign(AugAssign* n, int depth) {
    auto expected_type = exec(n->target, depth);
    auto type          = exec(n->value, depth);

    String signature = join("-", Array<String>{str(n->op), str(expected_type), str(type)});

    auto handler       = get_native_binary_operation(signature);
    n->native_operator = handler;

    if (!handler) {
        SEMA_ERROR(n, UnsupportedOperand, str(n->op), expected_type, type);
    }

    typecheck(n->value, type, n->target, expected_type, LOC);
    return type;
}

//! Annotation takes priority over the deduced type
//! this enbles users to use annotation to debug
TypeExpr* SemanticAnalyser::annassign(AnnAssign* n, int depth) {
    TypeExpr* constraint = n->annotation;
    bool      ann_valid  = is_type(n->annotation, depth, LOC);

    ExprNode* value   = n->value.fold(nullptr);
    TypeExpr* value_t = exec<TypeExpr*>(n->value, depth).fold(nullptr);

    if (value) {
        if (!value_t) {
            // FIXME could not find type for value
        } else if (!is_type(value_t, depth, LOC)) {
            value_t = nullptr;
        }
    }

    if (constraint && value_t) {
        // if we were able to deduce a type from the expression
        // make sure it matches the annotation constraint
        typecheck(n->target,   // a
                  constraint,  // int
                  value,       // 1
                  value_t,     // int
                  LOC);
    }

    if (value_t && !ann_valid) {
        constraint = value_t;
        info("Could fix annotation here was {} should be {}", str(n->annotation), str(value_t));

        // FIX annotation
        if (false) {
            n->annotation = value_t;
        }
    }

    if (n->target->kind == NodeKind::Name) {
        add_name(n->target, value, constraint);
    } else if (n->target->kind == NodeKind::Attribute) {
        // if it is an attribute make sure to update its type
        auto attr_t = attribute_assign(cast<Attribute>(n->target), depth, constraint);

        // if attr has no type it will be added
        // if attr has a type then we need to check that the assignment type
        // matches
        typecheck(n->target, attr_t, nullptr, constraint, LOC);
    }

    return constraint;
}
TypeExpr* SemanticAnalyser::forstmt(For* n, int depth) {
    auto iter_t = exec(n->iter, depth);

    // TODO: use iter_t to set target types
    exec(n->target, depth);

    // TODO: check consistency of return types
    auto return_t1 = exec<TypeExpr*>(n->body, depth);
    auto return_t2 = exec<TypeExpr*>(n->orelse, depth);

    return oneof(return_t1);
}
TypeExpr* SemanticAnalyser::whilestmt(While* n, int depth) {
    exec(n->test, depth);
    exec<TypeExpr*>(n->body, depth);
    auto types = exec<TypeExpr*>(n->orelse, depth);
    return oneof(types);
}
TypeExpr* SemanticAnalyser::ifstmt(If* n, int depth) {
    exec(n->test, depth);
    auto types = exec<TypeExpr*>(n->body, depth);

    for (int i = 0; i < n->tests.size(); i++) {
        exec(n->tests[i], depth);
        exec<TypeExpr*>(n->bodies[i], depth);
    }

    return oneof(types);
}
TypeExpr* SemanticAnalyser::with(With* n, int depth) {
    for (auto& item: n->items) {
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

    auto types = exec<TypeExpr*>(n->body, depth);
    return oneof(types);
}
TypeExpr* SemanticAnalyser::raise(Raise* n, int depth) {
    // TODO:
    exec<TypeExpr*>(n->exc, depth);
    exec<TypeExpr*>(n->cause, depth);
    return nullptr;
}
TypeExpr* SemanticAnalyser::trystmt(Try* n, int depth) {

    Array<TypeExpr*> return_t1;
    Array<TypeExpr*> return_t2;
    Array<TypeExpr*> return_t3;
    Array<TypeExpr*> return_t4;

    return_t1 = exec<TypeExpr*>(n->body, depth);

    for (ExceptHandler& handler: n->handlers) {
        Scope _(bindings);

        TypeExpr* exception_type = nullptr;

        if (handler.type.has_value()) {
            exception_type = handler.type.value();

            TypeExpr* type = exec(exception_type, depth);
            typecheck(exception_type, type, nullptr, make_ref(n, "Type"), LOC);
        }

        if (handler.name.has_value()) {
            bindings.add(handler.name.value(), nullptr, exception_type);
        }

        return_t2 = exec<TypeExpr*>(handler.body, depth);
    }

    return_t3 = exec<TypeExpr*>(n->orelse, depth);
    return_t4 = exec<TypeExpr*>(n->orelse, depth);

    // TODO:
    return oneof(return_t1);
}
TypeExpr* SemanticAnalyser::assertstmt(Assert* n, int depth) {
    // TODO:
    // we need to build the AssertionError exception
    exec(n->test, depth);
    exec<TypeExpr*>(n->msg, depth + 1);
    return nullptr;
}

// This means the binding lookup for variable should stop before the global scope :/
TypeExpr* SemanticAnalyser::global(Global* n, int depth) {
    for (auto& name: n->names) {
        auto varid = bindings.get_varid(name);
        if (varid == -1) {
            SEMA_ERROR(n, NameError, n, name);
        }
    }
    return nullptr;
}
TypeExpr* SemanticAnalyser::nonlocal(Nonlocal* n, int depth) {
    for (auto& name: n->names) {
        auto varid = bindings.get_varid(name);
        if (varid == -1) {
            SEMA_ERROR(n, NameError, n, name);
        }
    }
    return nullptr;
}
TypeExpr* SemanticAnalyser::exprstmt(Expr* n, int depth) { return exec(n->value, depth); }
TypeExpr* SemanticAnalyser::pass(Pass* n, int depth) { return None_t(); }
TypeExpr* SemanticAnalyser::breakstmt(Break* n, int depth) {
    // check that we are inside a loop
    return nullptr;
}
TypeExpr* SemanticAnalyser::continuestmt(Continue* n, int depth) {
    // check that we are inside a loop
    return nullptr;
}
TypeExpr* SemanticAnalyser::match(Match* n, int depth) {
    exec(n->subject, depth);

    Array<TypeExpr*> types;
    for (auto& b: n->cases) {
        exec(b.pattern, depth + 1);
        exec<TypeExpr*>(b.guard, depth + 1);
        types = exec<TypeExpr*>(b.body, depth + 1);
    }

    return oneof(types);
}
TypeExpr* SemanticAnalyser::inlinestmt(Inline* n, int depth) {
    auto types = exec<TypeExpr*>(n->body, depth);
    return oneof(types);
}

TypeExpr* SemanticAnalyser::matchvalue(MatchValue* n, int depth) {
    exec(n->value, depth);
    return nullptr;
}
TypeExpr* SemanticAnalyser::matchsingleton(MatchSingleton* n, int depth) { return nullptr; }
TypeExpr* SemanticAnalyser::matchsequence(MatchSequence* n, int depth) {
    for (auto& elt: n->patterns) {
        exec(elt, depth);
    }
    return nullptr;
}
TypeExpr* SemanticAnalyser::matchmapping(MatchMapping* n, int depth) {
    for (auto pat: n->patterns) {
        exec(pat, depth);
    }
    return nullptr;
}
TypeExpr* SemanticAnalyser::matchclass(MatchClass* n, int depth) {
    exec(n->cls, depth);
    for (auto pat: n->patterns) {
        exec(pat, depth);
    }
    for (auto pat: n->kwd_patterns) {
        exec(pat, depth);
    }
    return nullptr;
}
TypeExpr* SemanticAnalyser::matchstar(MatchStar* n, int depth) {
    // TODO: need to get the type from the target
    if (n->name.has_value()) {
        bindings.add(n->name.value(), n, nullptr);
    }
    return nullptr;
}
TypeExpr* SemanticAnalyser::matchas(MatchAs* n, int depth) {
    // TODO: need to get the type from the target
    if (n->name.has_value()) {
        bindings.add(n->name.value(), n, nullptr);
    }
    exec<TypeExpr*>(n->pattern, depth);
    return nullptr;
}
TypeExpr* SemanticAnalyser::matchor(MatchOr* n, int depth) {
    for (auto pat: n->patterns) {
        exec(pat, depth);
    }
    return nullptr;
}

TypeExpr* SemanticAnalyser::dicttype(DictType* n, int depth) { return Type_t(); }
TypeExpr* SemanticAnalyser::arraytype(ArrayType* n, int depth) { return Type_t(); }
TypeExpr* SemanticAnalyser::arrow(Arrow* n, int depth) { return Type_t(); }
TypeExpr* SemanticAnalyser::builtintype(BuiltinType* n, int depth) { return Type_t(); }
TypeExpr* SemanticAnalyser::tupletype(TupleType* n, int depth) { return Type_t(); }
TypeExpr* SemanticAnalyser::settype(SetType* n, int depth) { return Type_t(); }
TypeExpr* SemanticAnalyser::classtype(ClassType* n, int depth) { return Type_t(); }

TypeExpr* SemanticAnalyser::module(Module* stmt, int depth) {
    // TODO: Add a forward pass that simply add functions & variables
    // to the context so the SEMA can look everything up
    exec<TypeExpr*>(stmt->body, depth);
    return nullptr;
};

TypeExpr* SemanticAnalyser::interactive(Interactive* n, int depth) { return nullptr; }
TypeExpr* SemanticAnalyser::functiontype(FunctionType* n, int depth) { return Type_t(); }
TypeExpr* SemanticAnalyser::expression(Expression* n, int depth) { return exec(n->body, depth); }

}  // namespace lython
