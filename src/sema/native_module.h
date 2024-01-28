#pragma once

#include "sema/bindings.h"
#include "ast/nodes.h"
#include "utilities/object.h"
#include "utilities/metadata_method.h"

namespace lython {

template <typename Sig>
struct FunctionTypeBuilder;

#if not __linux__
#define LY_TYPENAME typename
#else
#define LY_TYPENAME
#endif

template <typename R, typename... Args>
struct FunctionTypeBuilder<R(Args...)> {
    using arguments_t = typename std::tuple<Args...>;
    using return_t    = LY_TYPENAME R;
    using function_t  = LY_TYPENAME R(Args...);

    Bindings const* bindings;

    FunctionTypeBuilder(Bindings const* bind):
        bindings(bind)
    {}

    template<int N, typename... Types>
    int add_arg(Arrow* arrow) {
        using TupleSize = typename std::tuple_size<std::remove_reference_t<std::tuple<Types...>>>;

        if constexpr (N > 0) {
            using ElemT = typename std::tuple_element<TupleSize::value - N, std::tuple<Types...>>::type;

            arrow->args[TupleSize::value - N] = lookup_type<ElemT>();

            // convert here
            add_arg<N - 1, Types...>(arrow);
        }
        return 0;
    }

    template<typename T>
    ExprNode* lookup_type() {
        // Here we need to match builtin type to the argument
        // we also need to match native types that might have been
        // added by this native module
        auto tid = meta::type_id<T>();
        for(BindingEntry const& entry: bindings->bindings) {
            if (entry.type_id == tid) {
                return (ExprNode*)(entry.value);
            }
        }
        return nullptr;
    }

    Arrow* function(GCObject* root) {
        // using TupleSize = typename std::tuple_size<std::remove_reference_t<std::tuple<Args...>>>;
        constexpr int n = std::tuple_size_v<std::remove_reference_t<arguments_t>>;

        Arrow* arrow = root->new_object<Arrow>();
        arrow->args.resize(n);

        // arguments<Args...>(arrow);
        add_arg<n, Args...>(arrow);

        arrow->returns = lookup_type<return_t>();
        return arrow;
    }
};


template<typename R, typename ...Args>
Arrow* function_type_builder(GCObject* root, Bindings const& bind, R(*function)(Args...)) {
    FunctionTypeBuilder<R(Args...)> builder(&bind);

    return builder.function(root);
}

// MetaData: type -> id
// Bendings: type -> id -> BuiltinType
// 

template<typename R, typename ...Args>
void register_native_function(GCObject* root, Bindings& binding, String const& name, R(*function)(Args...)) {
    BuiltinType* self = root->new_object<BuiltinType>();
    StringRef identifier(name);
    self->name = identifier;
    self->native_function = meta::wrapper::wrap_function_generic(function);

    Arrow* funtype = function_type_builder(root, binding, function);

    binding.add(
        self->name,
        self,
        funtype
    );
}


class NativeModule {

    template<typename R, typename ...Args>
    void add_function(StringRef name, R(*function)(Args...)) {
        
    };

    /*
    void add_native_function(StringRef name, NativeFunction fun) {
        BuiltinType* self = _root.new_object<BuiltinType>();
        self->name = name;
        self->native_function = fun;

        // here
        // wrap_function_generic;

        Arrow* type = _types.new_object<Arrow>();
        _members.add(
            self->name,
            self,
            function_type_builder(&_root, _members, fun)
        );
    }

    void add_native_macro(StringRef name, NativeMacro fun) {
        BuiltinType* self = _root.new_object<BuiltinType>();
        self->name = name;
        // self->native_macro = fun;

        Arrow* type = _types.new_object<Arrow>();
        _members.add(
            self->name,
            self,
            type
        );
    }

    void add_class(StringRef name) {
        // Sema lower method as free function
        // here we could pass the native module to sema or register it as if it was
        // sema
        cls = _root.new_object<ClassDef>();

        _members.add(
            name,
            cls,
            cls
        );

        // cls->add ...
    } */

    Bindings _members;
    Expression _root;
    Expression _types;
};


class MathModule: public NativeModule {


};


class BuiltinModule: public NativeModule {


};

}