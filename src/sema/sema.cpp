#include "sema/sema.h"
#include "builtin/operators.h"
#include "dependencies/fmt.h"
#include "parser/format_spec.h"
#include "utilities/guard.h"
#include "utilities/helpers.h"
#include "utilities/printing.h"
#include "utilities/strings.h"

#include "dependencies/formatter.h"

#define KW_SANITY_CHECK 1

namespace lython {
Arrow* get_arrow(
    SemanticAnalyser* self, ExprNode* fun, ExprNode* type, int depth, int& offset, ClassDef*& cls);



bool SemanticAnalyser::add_name(StringRef name, ExprNode* value, ExprNode* type) {
    bindings.add(name, value, type);
    return true;
}

bool SemanticAnalyser::add_name(ExprNode* expr, ExprNode* value, ExprNode* type) {
    auto* name = cast<Name>(expr);

    if (name != nullptr) {
        name->ctx      = ExprContext::Store;
        name->type     = type;
        add_name(name->id, value, type);
        return true;
    }

    return false;
}

Name* SemanticAnalyser::make_ref(Node* parent, StringRef const& name, ExprNode* type) {
    return bindings.make_reference(parent, name, type);
}
Name* SemanticAnalyser::make_ref(Node* parent, String const& name, ExprNode* type) {
    return make_ref(parent, StringRef(name), type);
}

bool SemanticAnalyser::typecheck(
    ExprNode* lhs, TypeExpr* lhs_t, ExprNode* rhs, TypeExpr* rhs_t, CodeLocation const& loc) {

    if (lhs_t != nullptr && rhs_t != nullptr) {
        semalog.debug(loc,
                      "lhs_t: `{}` kind `{}` rhs_t: `{}` kind:{}",
                      str(lhs_t),
                      int(lhs_t->kind),
                      str(rhs_t),
                      int(rhs_t->kind));
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
    if (expr_t == nullptr) {
        return "";
    }

    auto*     cls_ref  = cast<Name>(expr_t);
    ClassDef* expr_cls = cast<ClassDef>(load_name(cls_ref));

    if (expr_cls != nullptr) {
        return join(".", Array<String>{expr_cls->cls_namespace, String(StringView(op))});
    }

    return "";
}

TypeExpr* SemanticAnalyser::boolop(BoolOp* n, int depth) {
    auto* bool_type        = make_ref(n, "bool", Type_t());
    bool  and_implemented  = false;
    bool  rand_implemented = false;
    auto* return_t         = bool_type;

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

        if (handler != nullptr) {
            n->native_operator = handler;
        } else {
            // Not a native operator
            BindingEntry* lhs_op_binding = bindings.find(StringRef(lhs_op));
            if (lhs_op_binding != nullptr) {
                Arrow* operator_type = cast<Arrow>(lhs_op_binding->type);

                lyassert(operator_type, "Bool operator needs to be Callable");
                lyassert(operator_type->args.size() == 2, "Bool operator requires 2 arguments");

                typecheck(lhs, lhs_t, nullptr, operator_type->args[0], LOC);
                typecheck(rhs, rhs_t, nullptr, operator_type->args[1], LOC);
                typecheck(
                    nullptr, operator_type->returns, nullptr, make_ref(n, "bool", Type_t()), LOC);
            } else {
                BindingEntry* rhs_op_binding = bindings.find(StringRef(rhs_op));
                if (rhs_op_binding != nullptr) {
                    Arrow* operator_type = cast<Arrow>(rhs_op_binding->type);

                    lyassert(operator_type, "Bool operator needs to be Callable");
                    lyassert(operator_type->args.size() == 2, "Bool operator requires 2 arguments");

                    typecheck(lhs, lhs_t, nullptr, operator_type->args[1], LOC);
                    typecheck(rhs, rhs_t, nullptr, operator_type->args[0], LOC);
                    typecheck(nullptr,
                              operator_type->returns,
                              nullptr,
                              make_ref(n, "bool", Type_t()),
                              LOC);
                }

                if (lhs_op_binding == nullptr && rhs_op_binding == nullptr) {
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
    auto* value_t = exec(n->value, depth);
    add_name(n->target, n->value, value_t);
    return value_t;
}

TypeExpr* SemanticAnalyser::resolve_variable(ExprNode* node) {

    Name* name = cast<Name>(node);

    if (name != nullptr) {
        lyassert(bindings.bindings.size() > 0, "");
        int last = int(bindings.bindings.size()) - 1;

        for (int i = last; i >= 0; i--) {
            BindingEntry const& entry = bindings.bindings[i];
            if (name->id == entry.name) {
                return (ExprNode*)entry.value;
            }
        }

        // lyassert(name->varid >= 0, "Type need to be resolved");
        // return static_cast<TypeExpr*>(bindings.get_value(name->varid));
    }

    return nullptr;
}

TypeExpr* SemanticAnalyser::compare(Compare* n, int depth) {

    auto* prev_t = exec(n->left, depth);
    auto* prev   = n->left;

    for (int i = 0; i < n->ops.size(); i++) {
        auto  op    = n->ops[i];
        auto* cmp   = n->comparators[i];
        auto* cmp_t = exec(cmp, depth);

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

    return make_ref(n, "bool", Type_t());
}

TypeExpr* SemanticAnalyser::binop(BinOp* n, int depth) {

    auto* lhs_t = exec(n->left, depth);
    auto* rhs_t = exec(n->right, depth);

    typecheck(n->left, lhs_t, n->right, rhs_t, LOC);

    TypeExpr*    type = resolve_variable(lhs_t);
    BuiltinType* blt  = cast<BuiltinType>(type);

    // Builtin type, all the operations are known
    if (blt) {
        String signature = join(String("-"), Array<String>{str(n->op), str(lhs_t), str(rhs_t)});

        kwdebug(semalog, "signature: {}", signature);
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
    auto* expr_t = exec(n->operand, depth);

    String signature = join("-", Array<String>{str(n->op), str(expr_t)});

    Function handler = get_native_unary_operation(signature);
    if (!handler) {
        SEMA_ERROR(n, UnsupportedOperand, str(n->op), expr_t, nullptr);
    }

    // lyassert(handler != nullptr, "Unary operation require native handler");
    n->native_operator = handler;

    // TODO: fetch native operator
    // TODO: check that op is defined for this type
    // use the op return type here
    return expr_t;
}
TypeExpr* SemanticAnalyser::lambda(Lambda* n, int depth) {
    Scope scope(bindings);
    auto* funtype = n->new_object<Arrow>();
    add_arguments(n->args, funtype, nullptr, depth);
    auto* type       = exec(n->body, depth);
    funtype->returns = type;
    return funtype;
}
TypeExpr* SemanticAnalyser::ifexp(IfExp* n, int depth) {
    auto* test_t = exec(n->test, depth);

    typecheck(n->test, test_t, nullptr, make_ref(n, "bool", Type_t()), LOC);
    auto* body_t   = exec(n->body, depth);
    auto* orelse_t = exec(n->orelse, depth);

    typecheck(nullptr, body_t, nullptr, orelse_t, LOC);
    return body_t;
}
TypeExpr* SemanticAnalyser::dictexpr(DictExpr* n, int depth) {
    TypeExpr* key_t = nullptr;
    TypeExpr* val_t = nullptr;

    for (int i = 0; i < n->keys.size(); i++) {
        auto* key_type = exec(n->keys[i], depth);
        auto* val_type = exec(n->values[i], depth);

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
        auto* val_type = exec(n->elts[i], depth);

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
        exec_with_ctx(ExprContext::Store, gen.target, depth);
        exec(gen.iter, depth);

        for (auto* if_: gen.ifs) {
            exec(if_, depth);
        }
    }

    auto* val_type = exec(n->elt, depth);

    auto* type  = n->new_object<ArrayType>();
    type->value = val_type;
    return type;
}
TypeExpr* SemanticAnalyser::generateexpr(GeneratorExp* n, int depth) {
    Scope scope(bindings);
    for (auto& gen: n->generators) {
        exec_with_ctx(ExprContext::Store, gen.target, depth);
        exec(gen.iter, depth);

        for (auto* if_: gen.ifs) {
            exec(if_, depth);
        }
    }

    auto* val_type = exec(n->elt, depth);
    auto* type     = n->new_object<ArrayType>();
    type->value    = val_type;
    return type;
}
TypeExpr* SemanticAnalyser::setcomp(SetComp* n, int depth) {
    Scope scope(bindings);
    for (auto& gen: n->generators) {
        exec_with_ctx(ExprContext::Store, gen.target, depth);
        exec(gen.iter, depth);

        for (auto* if_: gen.ifs) {
            exec(if_, depth);
        }
    }

    auto* val_type = exec(n->elt, depth);

    auto* type  = n->new_object<ArrayType>();
    type->value = val_type;
    return type;
}

TypeExpr* SemanticAnalyser::dictcomp(DictComp* n, int depth) {
    Scope scope(bindings);
    for (auto& gen: n->generators) {
        exec_with_ctx(ExprContext::Store, gen.target, depth);
        exec(gen.iter, depth);

        for (auto* if_: gen.ifs) {
            exec(if_, depth);
        }
    }

    auto* key_type = exec(n->key, depth);
    auto* val_type = exec(n->value, depth);

    auto* type  = n->new_object<DictType>();
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

BindingEntry const* SemanticAnalyser::lookup(Name_t* n) {
    if (n == nullptr) {
        return nullptr;
    }

    if (auto* found = bindings.find(n->id)) {
        //
        // n->ctx = ExprContext::Load;
        n->store_id = found->store_id;
        n->load_id  = len(bindings.bindings);


#if KW_SANITY_CHECK
        int idx = int (bindings.bindings.size()) - (n->load_id - n->store_id);
        if (idx < 0 || idx >= bindings.bindings.size()) {
            kwerror(outlog(), "Bad index got {} = (size: {}) - 1 - (load: {} store: {})", 
                idx, bindings.bindings.size(), n->load_id, n->store_id
            );
        }
        else {
            BindingEntry const* bndng = &bindings.bindings[idx];
            if (bndng != found) {
                kwerror(outlog(), "Expected to find {} but found {}", found->name, bndng->name);
            }
        }
#endif
        return found;
    }
    return nullptr;
}

Node* SemanticAnalyser::load_name(Name_t* n) {
    BindingEntry const* entry = lookup(n);
    if (entry) {
        return entry->value;
    }
    SEMA_ERROR(n, NameError, n, n->id);
    return nullptr;
#if 0
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
#endif
}

ClassDef* SemanticAnalyser::get_class(ExprNode* classref, int depth) {
    auto* cls_name = cast<Name>(classref);

    if (!cls_name) {
        return nullptr;
    }

    cls_name->ctx = ExprContext::Load;
    // lyassert(cls_name->ctx == ExprContext::Load, "Reference to the class should be loaded");
    auto* cls = cast<ClassDef>(load_name(cls_name));

    return cls;
}

Arrow* SemanticAnalyser::build_constructor_type(ExprNode* fun, ClassDef* cls, int depth) {
    static StringRef __init_name("__init__");
    static StringRef __new_name("__new__");

    // Find __new__ and __init__
    FunctionDef* __new  = nullptr;
    FunctionDef* __init = nullptr;

    for (StmtNode* stmt: cls->body) {
        if (FunctionDef* def = cast<FunctionDef>(stmt)) {
            if (def->name == __init_name) {
                __init = def;
            }
            if (def->name == __new_name) {
                __new = def;
                break;
            }
        }

        if (__new && __init) {
            break;
        }
    }

    kwdebug(semalog, "__init__ {} __new__ {}", (__init) != nullptr, __new != nullptr);

    //
    // Build the constructor type
    //
    int __new_arg_count  = __new ? int(__new->type->args.size()) : 0;
    int __init_arg_count = __init ? int(__init->type->args.size()) : 0;

    Arrow* ctor_t   = fun->new_object<Arrow>();
    ctor_t->returns = fun;

    for (int i = 0; i < std::max(__new_arg_count, __init_arg_count); i++) {
        TypeExpr* _narg_t = __new && i < __new_arg_count ? (__new->type->args[i]) : nullptr;
        TypeExpr* _iarg_t = __init && i < __init_arg_count ? (__init->type->args[i]) : nullptr;

        if (i == 0) {
            if (_narg_t) {
                // new takes a type as first argument
                typecheck(nullptr, _narg_t, nullptr, Type_t(), LOC);
            }
            if (_iarg_t) {
                // init takes an object as first argument
                typecheck(nullptr,
                          _iarg_t,
                          nullptr,
                          fun,
                          LOC);  // fun should be a Name that points to the class
            }
            continue;
        } else {
            if (_iarg_t && _narg_t) {
                // we expect __new__ and __init__ arg to match
                typecheck(nullptr, _narg_t, nullptr, _iarg_t, LOC);
            }

            TypeExpr* arg_t = _iarg_t ? _iarg_t : _narg_t;
            ctor_t->args.push_back(arg_t);
        }
    }

    cls->ctor_t = ctor_t;
    kwdebug(semalog, "Built constructor type {}", str(ctor_t));
    return ctor_t;
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
            kwwarn(semalog, "Could not resolve class");
            return nullptr;
        }
        TypeExpr* arrow = nullptr;

        if (cls->ctor_t != nullptr) {
            return cls->ctor_t;
        } else {
            return build_constructor_type(fun, cls, depth);
        }
    }
    default: break;
    }
    return nullptr;
}

bool SemanticAnalyser::reorder_arguments(Call* call, FunctionDef* def) {
    // return;
    // Transform as many argument as possible to positional
    //
    //  We should probably only allow *args && **kwargs at compile time
    //  as code generator helper
    //
    Array<Keyword>   args;
    Array<ExprNode*> final_args;
    Array<ExprNode*> var_args;
    Array<Keyword>   kw_args;

    Array<int> used_keywords;

    if (def->native) {
        // detected native function
        // native function do not have named arguments
        // so we cannot reorder
        if (call->keywords.size() > 0) {
            SEMA_ERROR(
                    call,
                    TypeError, 
                    fmt::format(
                        "TypeError: Native function only support positional arguments"
                    )
                );
            return false;
        }
        return true;
    }

    int i = 0;

    def->args.visit([&](ArgumentIter<false> const & arg) {
        Keyword kw;

        if (in(arg.kind, ArgumentKind::PosOnly, ArgumentKind::Regular)) {
            ExprNode* arg_value = arg.value;

            if (i < call->args.size()) {
                arg_value = call->args[i];
            }

            if (in(arg.kind, ArgumentKind::Regular)) {
                // Regular argument defined with a keyword
                for(int k = 0; k < call->keywords.size(); k++) {
                    if (arg.arg.arg == call->keywords[k].arg) {
                        arg_value = call->keywords[k].value;
                        used_keywords.push_back(k);
                        break;
                    }
                }
            }

            kw.arg = arg.arg.arg;
            kw.value = arg_value;
            args.push_back(kw);
            i += 1;
            return;
        }

        // push remaining args as variadic args
        // we probably want to save those in a different array
        if (in(arg.kind, ArgumentKind::VarArg)) {
            for(int k = i; k < call->args.size(); k++) {
                var_args.push_back(call->args[k]);
            }
            return;
        }

        if (in(arg.kind, ArgumentKind::KwOnly)) {
            ExprNode* arg_value = arg.value;
            
            for(int k = 0; k < call->keywords.size(); k++) {
                // The argument is defined in the call
                if (call->keywords[k].arg == arg.arg.arg) {
                    arg_value = call->keywords[k].value;
                    used_keywords.push_back(k);
                    break;
                }
            }

            kw.arg = arg.arg.arg;
            kw.value = arg_value;
            args.push_back(kw);
            return;
        }

        // Final kw arguments, that do not match anything
        if (in(arg.kind, ArgumentKind::KwArg)) {
            for(int k = 0; k < call->keywords.size(); k++) {
                if (!contains(used_keywords, k)) {
                    kw_args.push_back(kw);
                }
            }
        }
    });

    bool is_ok = true;
    final_args.reserve(args.size());
    for(Keyword kw: args) {
        if (kw.value == nullptr) {
            is_ok = false;
            SEMA_ERROR(
                call,
                TypeError, 
                fmt::format(
                    "TypeError: {} missing 1 required positional argument: '{}'",
                    str(call),
                    str(kw.arg)
                )
            );
        }
        final_args.push_back(kw.value);
    }

    // args - reordered (include posonly, regular & kwonly)
    // varargs list of args
    // kwargs dictionnary   
    call->args = final_args;
    call->varargs = var_args;
    call->keywords = kw_args;
    return is_ok;
}

TypeExpr* SemanticAnalyser::call(Call* n, int depth) {
    // Get the type of the function
    // if function function is an attribute obj.attr
    // this returns the type of the function
    auto* type = exec(n->func, depth);

    // 
    bool is_call_valid = false;
    if (Name* name = cast<Name>(n->func)) {
        BindingEntry const* entry = lookup(name);
        if (entry) {
            if (FunctionDef* def = cast<FunctionDef>(entry->value)){
                is_call_valid = reorder_arguments(n, def);
            }
        }
    }

    int       offset = 0;
    ClassDef* cls    = nullptr;
    auto*     arrow  = get_arrow(n->func, type, depth, offset, cls);

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

    // kwdebug(semalog, "n->func= {}: {}: arrow: {}", str(n->func), str(type), str(arrow));

    // self.a()
    if (Attribute* attr = cast<Attribute>(n->func)) {
        auto cls_t = exec(attr->value, depth);
        got->add_arg_type(cls_t);
    }

    // Method, insert the self argument since it is implicit
    if (offset == 1 && cls) {
        // got->add_arg_type(make_ref(got, strfat(cls->name)));
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
    if (arrow != nullptr && is_call_valid) {
        got->returns = arrow->returns;

        // (Point, Point) -> Point
        kwdebug(semalog, "fun type: {}, {}", str(got), n->args.size());
        kwdebug(semalog, "fun type: {}", str(arrow));

        typecheck(n, got, n->func, arrow, LOC);
    }

    if (arrow != nullptr) {
        return arrow->returns;
    }
    return nullptr;
}
TypeExpr* SemanticAnalyser::joinedstr(JoinedStr* n, int depth) {
    for (auto* value: n->values) {
        exec(value, depth);
    }
    return make_ref(n, "str", Type_t());
}

// TypeExpr* SemanticAnalyser::condjump(CondJump_t* n, int depth) {
//     return nullptr;
// }

TypeExpr* SemanticAnalyser::exported(Exported* n, int depth) {
    TypeExpr* return_type = nullptr;

    exported_stack.push_back(n);
    if (n->node && n->node->family() == NodeFamily::Expression) {
        return_type = exec(reinterpret_cast<ExprNode*>(n->node), depth);
    } else if (n->node && n->node->family() == NodeFamily::Statement) {
        return_type = exec(reinterpret_cast<StmtNode*>(n->node), depth);
    }

    exported_stack.pop_back();
    return return_type;
}

TypeExpr* SemanticAnalyser::formattedvalue(FormattedValue* n, int depth) {
    TypeExpr* vtype = exec(n->value, depth);

    // Objects are able to define their own format specifiers to replace the standard ones
    // [[fill]align][sign][#][0][minimumwidth][.precision][type]
    //          <     +    #  0    [0-9]*       .[0-9]*     b     e
    //          >     -                                     c     E
    //          =    ' '                                    d     f
    //          ^                                           o     F
    //                                                      x     g
    //                                                      X     G
    //                                                      n     n
    //                                                      None  %
    //
    // Conversion flag
    //      !r     {0!s:20}
    //      !s     {0!r:20}
    //
    //  class object:
    //      def __format__(self, format_spec):
    //          return format(str(self), format_spec)
    //
    // https://peps.python.org/pep-3101/

    // Here checking the type of the format spec is probably not the priority
    // but validating the format spec
    // the sinple case is if there is a single format_spec
    if (n->format_spec->values.size() == 1) {
        Constant* cst = cast<Constant>(n->format_spec->values[0]);

        if (cst != nullptr) {
            String          strspec = cst->value.as<String>();
            FormatSpecifier spec    = FormatSpecifier::parse(strspec);

            // FIXME: this should a SEMA error
            if (!spec.valid) {
                kwdebug(semalog,
                        "Format spec is not valid; parsed {} unparsed string: `{}`",  //
                        spec.__repr__(),                                              //
                        spec.unparsed                                                 //
                );
            }

            if (spec.is_float()) {
                // TODO
                // check that vtype is double or float
            }

            if (spec.is_integer()) {
                // TODO
                // check that vtype is an integer
            }
        }
    }

    // I think if there is more that means the spec is dynamic
    // then we are limited in our ability to validate it
    // we probably can extract the type it if it is defined the rest
    // will remain undefined
    for (auto* value: n->format_spec->values) {
        Constant* cst = cast<Constant>(value);
        if (cst != nullptr) {
            // Partial format spec
        } else {
            // Can we use the type here to guess which
            // part of the format spec this will fill out ?
            TypeExpr* etype = exec(value, depth);
        }
    }

    return nullptr;
}

TypeExpr* SemanticAnalyser::placeholder(Placeholder* n, int depth) { return nullptr; }
TypeExpr* SemanticAnalyser::constant(Constant* n, int depth) {
    switch (meta::ValueTypes(n->value.tag)) {
    case meta::ValueTypes::i8: return make_ref(n, "i8", Type_t());
    case meta::ValueTypes::i16: return make_ref(n, "i16", Type_t());
    case meta::ValueTypes::i32: return make_ref(n, "i32", Type_t());
    case meta::ValueTypes::i64: return make_ref(n, "i64", Type_t());

    case meta::ValueTypes::u8: return make_ref(n, "u8", Type_t());
    case meta::ValueTypes::u16: return make_ref(n, "u16", Type_t());
    case meta::ValueTypes::u32: return make_ref(n, "u32", Type_t());
    case meta::ValueTypes::u64: return make_ref(n, "u64", Type_t());

    case meta::ValueTypes::f32: return make_ref(n, "f32", Type_t());
    case meta::ValueTypes::f64: return make_ref(n, "f64", Type_t());
    case meta::ValueTypes::i1: return make_ref(n, "bool", Type_t());

    // case meta::ValueTypes::i8: return make_ref(n, "str", Type_t());
    default: break;
    }

    static int strid = meta::type_id<String>();
    if (n->value.tag == strid) {
        return make_ref(n, "str", Type_t());
    }

    return nullptr;
}

// This is only called when loading
TypeExpr* SemanticAnalyser::attribute(Attribute* n, int depth) {
    // <value>.<name>
    auto* type_t  = exec(n->value, depth);
    auto* class_t = get_class(type_t, depth);

    if (class_t == nullptr) {
        // class was not found, this is probably because the lookup
        // of the value failed, it should have produced a precise error
        // so we do not have to
        // SEMA_ERROR(NameError(n->value, str(n->value)));
        return nullptr;
    }

    if (class_t->type_id > -1) {
        // in the case of a method this does not work
        // because the type was not created
        // can we remove that piece of custom code
        // out and into the native module builder instead
        int tid;
        std::tie(n->attrid, tid) = meta::member_id(class_t->type_id, str(n->attr).c_str());

        if (tid > -1) {
            int i = 0;
            for (BindingEntry& bind: bindings.bindings) {
                if (bind.type_id == tid) {
                    Name* name = n->new_object<Name>();
                    name->id   = bind.name;
                    // name->varid = i;
                    name->type = bind.type;
                    return name;
                }
                i += 1;
            }
        }

        kwdebug(semalog, "native attribute not found");
        // return nullptr;
    }

    n->attrid = class_t->get_attribute(n->attr);
    if (n->attrid == -1) {
        SEMA_ERROR(n, AttributeError, class_t, n->attr);
        return nullptr;
    }

    n->resolved          = &class_t->attributes[n->attrid];
    ClassDef::Attr& attr = class_t->attributes[n->attrid];

    if (attr.type != nullptr && is_type(attr.type, depth, LOC)) {
        return attr.type;
    }
    return nullptr;
}

TypeExpr* SemanticAnalyser::attribute_assign(Attribute* n, int depth, TypeExpr* expected) {
    // self: Ref[class_t]
    n->ctx = ExprContext::Store;
    
    auto* type_t  = exec(n->value, depth);
    auto* class_t = get_class(type_t, depth);

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

    if (attr.type != nullptr && is_type(attr.type, depth, LOC)) {
        return attr.type;
    }
    return nullptr;
}

TypeExpr* SemanticAnalyser::subscript(Subscript* n, int depth) {
    auto* class_t = exec(n->value, depth);
    exec(n->slice, depth);
    // check that __getitem__ is defined in class_t
    return nullptr;
}
TypeExpr* SemanticAnalyser::starred(Starred* n, int depth) {
    // value should be of an expandable type
    auto* value_t = exec(n->value, depth);
    return nullptr;
}
TypeExpr* SemanticAnalyser::name(Name* n, int depth) {
    BindingEntry const* entry = lookup(n);
    n->ctx                    = expr_context;

    if (entry) {
        n->type = entry->type;
        return entry->type;
    }

    if (n->ctx == ExprContext::Store) {
        add_name(n, nullptr, nullptr);
        return nullptr;
    }

    SEMA_ERROR(n, NameError, n, n->id);
    return nullptr;

#if 0
    for(BindingEntry const& entry: bindings.bindings) {
        if (entry.name == n->id) {
            return entry.type;
        }
    }
    return nullptr;

    if (n->ctx == ExprContext::Store) {
        auto id  = bindings.add(n->id, n, nullptr);
        n->varid = id;
        n->size  = int(bindings.bindings.size());

        kwdebug(semalog, "Storing value for {} ({})", n->id, n->varid);
    } else {
        // Both delete & Load requires the variable to be defined first
        n->varid   = bindings.get_varid(n->id);
        n->size    = int(bindings.bindings.size());
        n->offset  = int(bindings.bindings.size()) - n->varid;
        n->dynamic = bindings.is_dynamic(n->varid);

        if (n->varid == -1) {
            kwdebug(semalog, "Value {} not found", n->id);
            SEMA_ERROR(n, NameError, n, n->id);
        }
    }

    // lyassert(n->varid != -1, "Should have been founds");

    auto* t = bindings.get_type(n->varid);
    if (t == nullptr) {
        kwdebug(semalog, "Value {} does not have a type", n->id);
    } else {
        kwdebug(semalog, "Loading value {}: {} of type {}", n->id, n->varid, str(t));
    }
    if (t->kind == NodeKind::Exported) {
        return exec(t, depth);
    }
    return t;
#endif
}

TypeExpr* SemanticAnalyser::comment(Comment* n, int depth) { return nullptr; }

TypeExpr* SemanticAnalyser::listexpr(ListExpr* n, int depth) {
    TypeExpr* val_t = nullptr;

    for (int i = 0; i < n->elts.size(); i++) {
        auto* val_type = exec(n->elts[i], depth);

        if (val_type == nullptr) {
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

        if (val_t == nullptr) {
            // FIXME Could not find tyep for n->elts[i]

        } else if (!is_type(val_t, depth, LOC)) {
            val_t = nullptr;
        }

        type->types.push_back(val_t);
    }

    n->type = type;
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
    int i = 0;
    TypeExpr* class_t = nullptr;
    if (def != nullptr) {
        class_t = make_ref(arrow, str(def->name), Type_t());
    }

    args.visit([&](ArgumentIter<false> const& iter) {
        if (in(iter.kind, ArgumentKind::VarArg, ArgumentKind::KwArg)) {
            kwdebug(semalog, "Unsupported argument type");
            return;
        }

        TypeExpr* ann_t = nullptr;
        TypeExpr* val_t = nullptr;

        bool ann_v = false;

        if(iter.arg.annotation.has_value()) {
            ann_t = exec(iter.arg.annotation.value(), depth);
            ann_v = is_type(ann_t, depth, LOC);
        }

        if (iter.value) {
            val_t = exec(iter.value, depth);
        }

        if (ann_v && val_t) {
            typecheck(nullptr, iter.arg.annotation.value(), iter.value, val_t, LOC);
        }

        TypeExpr* arg_t = ann_v ? iter.arg.annotation.value(): val_t;

        // First argument might be self: cls
        if (def != nullptr && i == 0 && class_t) {
            if (arg_t) {
                typecheck(nullptr, arg_t, nullptr, class_t, LOC);
            }
            arg_t = class_t;
        }
        
        // we could populate the default value here
        // but we would not want sema to think this is a constant
        add_name(iter.arg.arg, nullptr, arg_t);

        if (arrow != nullptr) {
            arrow->names.push_back(iter.arg.arg);

            if (!arrow->add_arg_type(arg_t)) {
                SEMA_ERROR(
                    arrow, TypeError, fmt::format("Cannot have a function type refer to itself"));
            }
        }

        i += 1;
    });
}

Arrow* SemanticAnalyser::functiondef_arrow(FunctionDef* n, StmtNode* class_t, int depth) {
    Arrow* type = n->new_object<Arrow>();

    {
        PopGuard ctx(semactx, SemaContext{false, true});

        if (n->returns.has_value()) {
            auto* return_t = n->returns.value();

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

Array<TypeExpr*> SemanticAnalyser::exec_body(Array<StmtNode*>& body, int depth) {

    Array<TypeExpr*> types;
    for (auto* stmt: body) {
        TypeExpr* tp = exec(stmt, depth);
        if (tp != nullptr) {
            types.push_back(tp);
        }
    }
    return types;
}

TypeExpr* SemanticAnalyser::functiondef(FunctionDef* n, int depth) {
    if (n->native != nullptr) {
        bindings.add(n->name, n, n->type);
        return n->type;
    }

    //
    // FIXME: need to change how arguments are added to the context
    // first all arguments that could be positional are added (posonly, regular, kwonly)
    // then varargs and kwargs are added last
    // 
    // Arguments are converted to positional whenever possible
    // so there is no overhead cost in using kw arguments
    // the overhead will come when varargs and kwargs are used
    //
    // Although, maybe we could convert them as well 
    // with more advanced tracing

    // if sema was already done on the function
    if (n->type && !eager) {
        kwinfo(semalog, "Send cached type {}", str(n->type));
        return n->type;
    }

    String funname = generate_function_name(n);

    PopGuard  _(namespaces, str(n->name));
    PopGuard  nested_stmt(nested, (StmtNode*)n);
    StmtNode* lst = nested_stmt.last(1, nullptr);

    // Insert the function into the global context
    // the arrow type is not created right away to prevent
    // circular typing
    bindings.add(funname, n, nullptr);

    // Enter function context
    Scope scope(bindings);

    // Create the function type from the arguments
    // this will also add the arguments to the context
    Arrow*    fun_type = functiondef_arrow(n, lst, depth);
    TypeExpr* return_t = fun_type->returns;

    // Update the function type at the very end
    bindings.set_type(funname, fun_type);

    // Infer return type from the body
    PopGuard ctx(semactx, SemaContext());
    auto     return_effective = exec_body(n->body, depth);

    if (return_t != nullptr) {
        // Annotated type takes precedence
        typecheck(nullptr,                  // lhs
                  return_t,                 // lhs_t
                  nullptr,                  // rhs
                  oneof(return_effective),  // rhs_t
                  LOC);
    }

    // do decorator last since we need to know our function signature to
    // typecheck them
    for (auto decorator: n->decorator_list) {
        auto* deco_t = exec(decorator.expr, depth);
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
            auto* fun = cast<FunctionDef>(stmt);
            n->insert_method(fun->name, fun, fun->type);

            if (str(fun->name) == "__init__") {
                kwinfo(semalog, "Found ctor");
                ctor[0] = fun;
            }

            methods.push_back(stmt);
            continue;
        }
        case NodeKind::Assign: {
            auto* assn = cast<Assign>(stmt);
            target     = assn->targets[0];
            value      = assn->value;
            break;
        }
        case NodeKind::AnnAssign: {
            auto* ann = cast<AnnAssign>(stmt);
            target    = ann->target;
            value     = ann->value.fold(nullptr);
            target_t  = is_type(ann->annotation, depth, LOC) ? ann->annotation : nullptr;
            break;
        }
        case NodeKind::Pass: continue;

        default: kwdebug(semalog, "Unhandled statement {}", str(stmt->kind)); continue;
        }

        auto* name = cast<Name>(target);

        // Not a variable, move on
        if (name == nullptr) {
            continue;
        }

        // try to deduce type fo the value
        if (value != nullptr) {
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
        n->insert_attribute(name->id, stmt, target_t != nullptr ? target_t : value_t);
    }
}

void SemanticAnalyser::record_ctor_attributes(ClassDef* n, FunctionDef* ctor, int depth) {
    if (ctor->args.args.empty()) {
        kwerror(semalog, "__init__ without self");
        return;
    }

    kwinfo(semalog, "Looking for attributes inside the ctor");
    auto self = ctor->args.args[0].arg;

    // we need the arguments in the scope so we can look them up
    Arrow arrow;

    Scope _(bindings);
    add_arguments(ctor->args, &arrow, n, depth);

    for (auto* stmt: ctor->body) {
        ExprNode* attr_expr = nullptr;
        ExprNode* value     = nullptr;
        TypeExpr* type      = nullptr;

        switch (stmt->kind) {
        case NodeKind::Assign: {
            auto* assn = cast<Assign>(stmt);
            attr_expr  = assn->targets[0];
            value      = assn->value;
            break;
        }
        case NodeKind::AnnAssign: {
            auto* ann = cast<AnnAssign>(stmt);
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

        auto* attr = cast<Attribute>(attr_expr);
        auto* name = cast<Name>(attr->value);

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
            type = exec(value, depth);

            if (!is_type(type, depth, LOC)) {
                type = nullptr;
            }
        }

        n->insert_attribute(attr->attr, stmt, type);
    }
}

TypeExpr* SemanticAnalyser::classdef(ClassDef* n, int depth) {
    PopGuard _(nested, n);
    PopGuard _n(namespaces, str(n->name));

    // a class is a new type
    // the type of a class is type
    int   id      = bindings.add(n->name, n, Type_t());
    Name* class_t = make_ref(n, str(n->name), Type_t());

    // TODO: go through bases and add their elements
    for (auto* base: n->bases) {
        exec(base, depth);
    }

    //
    for (auto& kw: n->keywords) {
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

    // __new__(cls) -> Object
    // __init__(self: Object) -> None

    // get __init__ and do a pass to insert its attribute to the class
    if (ctor != nullptr) {
        // Traverse body to look for our attributes
        record_ctor_attributes(n, ctor, depth);
    }
    // -----------------------------
    bool has_init = false;
    bool has_new  = false;

    // Process the remaining methods;
    for (auto& stmt: methods) {

        // fun_t is the arrow that is saved in the context populate self type
        auto* fun   = cast<FunctionDef>(stmt);
        auto* fun_t = cast<Arrow>(exec(stmt, depth));

        // For native, the type should be set by something else
        if (fun->native == nullptr) {
            if (fun_t != nullptr && fun_t->arg_count() > 0) {
                fun_t->set_arg_type(0, class_t);
            }
        }
    }

    // ----

    for (auto deco: n->decorator_list) {
        auto* deco_t = exec(deco.expr, depth);
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
    for (auto* target: n->targets) {
        exec(target, depth);
    }
    return nullptr;
}
TypeExpr* SemanticAnalyser::assign(Assign* n, int depth) {
    auto* type = exec(n->value, depth);

    // TODO: check if the assigned name already exist or not
    //
    if (n->targets.size() == 1) {
        auto* target = n->targets[0];

        if (target->kind == NodeKind::Name) {
            add_name(target, n->value, type);
        } else if (target->kind == NodeKind::Attribute) {
            // Deduce the type of the attribute from the value
            auto* target_t = attribute_assign(cast<Attribute>(n->targets[0]), depth, type);

            typecheck(target, target_t, n->value, type, LOC);
        } else {
            kwerror(semalog, "Assignment to an unsupported expression {}", str(target->kind));
        }

    } else {
        auto* types = cast<TupleType>(type);
        if (types == nullptr) {
            // TODO: unexpected type
            return type;
        }

        if (types->types.size() != n->targets.size()) {
            // TODO: Add type mismatch
            return type;
        }

        for (auto i = 0; i < types->types.size(); i++) {
            auto* target = n->targets[0];
            auto* name   = cast<Name>(target);
            auto* type   = types->types[0];

            add_name(n->targets[0], n->value, type);
        }
    }

    return type;
}
TypeExpr* SemanticAnalyser::augassign(AugAssign* n, int depth) {
    // Aug expects loading first then storing
    // LLVM needs to know that so we can get a loadable and writable address
    auto* expected_type = exec_with_ctx(ExprContext::LoadStore, n->target, depth);
    auto* type          = exec(n->value, depth);

    String signature = join("-", Array<String>{str(n->op), str(expected_type), str(type)});

    auto handler       = get_native_binary_operation(signature);
    n->native_operator = handler;

    if (handler == nullptr) {
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

    if (value != nullptr) {
        if (value_t == nullptr) {
            // FIXME could not find type for value
        } else if (!is_type(value_t, depth, LOC)) {
            value_t = nullptr;
        }
    }

    if (constraint != nullptr && value_t != nullptr) {
        // if we were able to deduce a type from the expression
        // make sure it matches the annotation constraint
        typecheck(n->target,   // a
                  constraint,  // int
                  value,       // 1
                  value_t,     // int
                  LOC);
    }

    if (value_t != nullptr && !ann_valid) {
        constraint = value_t;
        kwinfo(semalog,
               "Could fix annotation here was {} should be {}",
               str(n->annotation),
               str(value_t));

        // FIX annotation
        if (false) {
            n->annotation = value_t;
        }
    }

    if (n->target->kind == NodeKind::Name) {
        add_name(n->target, value, constraint);
    } else if (n->target->kind == NodeKind::Attribute) {
        // if it is an attribute make sure to update its type
        auto* attr_t = attribute_assign(cast<Attribute>(n->target), depth, constraint);

        // if attr has no type it will be added
        // if attr has a type then we need to check that the assignment type
        // matches
        typecheck(n->target, attr_t, nullptr, constraint, LOC);
    }

    return constraint;
}
TypeExpr* SemanticAnalyser::forstmt(For* n, int depth) {
    // This assume a function call
    // type could be an object with a __iter__ + __next__
    auto* iter_return_t = exec(n->iter, depth);

    // exec_with_ctx(ExprContext::Store, n->target, depth);

    // Handle unpacking here
    if (Name* name = cast<Name>(n->target)) {
        add_name(name, nullptr, iter_return_t);
    }

    // kwdebug(semalog, "iter type: {}: {}", str(n->iter), str(iter_t));

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
        auto* type = exec(item.context_expr, depth);

        if (item.optional_vars.has_value()) {
            auto* expr = item.optional_vars.value();
            if (expr->kind == NodeKind::Name) {
                auto* name = cast<Name>(expr);
                if (name != nullptr) {
                    // name->varid = bindings.add(name->id, expr, type);
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
        BindingEntry* entry = bindings.find(name);
        if (entry == nullptr) {
            SEMA_ERROR(n, NameError, n, name);
        }
    }
    return nullptr;
}
TypeExpr* SemanticAnalyser::nonlocal(Nonlocal* n, int depth) {
    for (auto& name: n->names) {
        BindingEntry* entry = bindings.find(name);
        if (entry == nullptr) {
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
    for (auto* elt: n->patterns) {
        exec(elt, depth);
    }
    return nullptr;
}
TypeExpr* SemanticAnalyser::matchmapping(MatchMapping* n, int depth) {
    for (auto* pat: n->patterns) {
        exec(pat, depth);
    }
    return nullptr;
}
TypeExpr* SemanticAnalyser::matchclass(MatchClass* n, int depth) {
    exec(n->cls, depth);
    for (auto* pat: n->patterns) {
        exec(pat, depth);
    }
    for (auto* pat: n->kwd_patterns) {
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
    for (auto* pat: n->patterns) {
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

    // create an entry point per module
    FunctionDef* entry = stmt->new_object<FunctionDef>();
    entry->name        = "__init__";

    // inser the module entry up top
    exec(entry, depth);

    for (auto* stmt: stmt->body) {
        if (in(stmt->kind, NodeKind::ClassDef, NodeKind::FunctionDef)) {
            // Sema only for definition, as they do not need to be evaluated
            exec(stmt, depth);
        } else {
            // Sema statement
            exec(stmt, depth);

            // This needs to be executed by the VM
            entry->body.push_back(stmt);
        }
    }
    stmt->__init__ = entry;

    return nullptr;
};

TypeExpr* SemanticAnalyser::interactive(Interactive* n, int depth) { return nullptr; }
TypeExpr* SemanticAnalyser::functiontype(FunctionType* n, int depth) { return Type_t(); }
TypeExpr* SemanticAnalyser::expression(Expression* n, int depth) { return exec(n->body, depth); }

bool SemanticAnalyser::has_errors() const { return !errors.empty(); }
void SemanticAnalyser::show_diagnostic(std::ostream& out, class AbstractLexer* lexer) {
    SemaErrorPrinter printer(std::cout, lexer);

    for (auto& diag: errors) {
        std::cout << "  ";
        printer.print(*diag.get());
        std::cout << "\n";
    }
}
}  // namespace lython
