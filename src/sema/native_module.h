#pragma once

#include "sema/bindings.h"
#include "ast/nodes.h"
#include "utilities/object.h"
#include "utilities/metadata_method.h"
#include "ast/interop.h"
#include "sema/importlib.h"

namespace lython {

template <typename Sig>
struct FunctionTypeBuilder;

#ifndef LY_TYPENAME 
#if ! __linux__
#define LY_TYPENAME typename
#else
#define LY_TYPENAME
#endif
#endif

template <typename R, typename... Args>
struct FunctionTypeBuilder<R(Args...)> {
    using arguments_t = typename std::tuple<Args...>;
    using return_t    = LY_TYPENAME R;
    using function_t  = LY_TYPENAME R(Args...);

    Bindings const* bindings;
    GCObject* _root = nullptr;

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
        int i = 0;

        for(BindingEntry const& entry: bindings->bindings) {
            if (entry.type_id == tid) {
                Name* name = _root->new_object<Name>();
                name->varid = i;
                name->id = entry.name;
                return name;;
            }
            i += 1;
        }
        return nullptr;
    }

    Arrow* function(GCObject* root) {
        // using TupleSize = typename std::tuple_size<std::remove_reference_t<std::tuple<Args...>>>;
        constexpr int n = std::tuple_size_v<std::remove_reference_t<arguments_t>>;
        _root = root;

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


//
// Those insert the native bindings into sema to be call by the VM
//
template<typename R, typename ...Args>
void register_native_function(GCObject* root, Bindings& binding, String const& name, R(*function)(Args...)) {
    BuiltinType* self = root->new_object<BuiltinType>();
    StringRef identifier(name);
    self->name = identifier;
    self->native_function = wrap_native(function);

    Arrow* funtype = function_type_builder(root, binding, function);

    binding.add(
        self->name,
        self,
        funtype
    );
}

namespace helper {
template<typename T, typename Tuple, std::size_t... Is>
    T* _ctor(void* memory, Tuple&& tuple, std::index_sequence<Is...>) {
        return new(memory) T(std::get<Is>(std::forward<Tuple>(tuple))...);
    }

    template<typename T, typename Tuple>
    T* ctor(void* memory, Tuple&& tuple) {
        return _ctor<T>(
            memory, 
            std::forward<Tuple>(tuple),
            std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{}
        );
    }
}

template<typename T, typename ...Args>
void create_constructor(GCObject* root, Bindings& binding, StringRef name) {
    //Arrow* ctor_t = root->new_object<Arrow>();
    //ctor_t->returns = class_t;
    //ctor_t->args; // ....

    FunctionTypeBuilder<T*(Args...)> builder(&binding);
    Arrow* ctor_t = builder.function(root);

    BuiltinType* ctor_fun = root->new_object<BuiltinType>();
    ctor_fun->name = name;
    ctor_fun->native_function = [](GCObject* mem, Array<Constant*> const& args) -> Constant* {
        // that is a lot of back to back allocation
        // we can could combine them in one
        // or/and remove some intermediate, NativePointer is probably not that necessary
        NativePointer<T>* obj = mem->new_object<NativePointer<T>>(); // Allocate
        
        auto arguments = Interop<T(Args...)>::from_script(args);

        void* memory = malloc(sizeof(T) * 1);                        // Allocate
        T* rawobj = helper::ctor<T>(memory, arguments);

        obj->set_pointer(rawobj);
        Constant* val = mem->new_object<Constant>();
        val->value = ConstantValue(obj);
        return val;                      // Allocate
    };

    binding.add(
        name,
        ctor_fun,
        ctor_t
    );
}

template<typename T, typename ...Args>
void register_native_object(GCObject* root, Bindings& binding, String const& name) {
    BuiltinType* self = root->new_object<BuiltinType>();
    StringRef identifier(name);
    self->name = identifier;
    // ---

    // Add the Native object to the binding
    int varid = binding.add(self->name, nullptr, Type_t(), meta::type_id<T*>());

    Name* class_t = root->new_object<Name>();
    class_t->varid = varid;
    class_t->id = identifier;

    create_constructor<T, Args...>(root, binding, StringRef("name"));
}



// This builds a module before sema
// sema will go through it a build a Binding
struct NativeModuleBuilder {
    NativeModuleBuilder(String const&name, class ImportLib& importsys):
        module(importsys.newmodule(name))
     {}

    template<typename R, typename ...Args>
    NativeModuleBuilder& function(String const& name, R(*function)(Args...)) {
        BuiltinType* self = module->new_object<BuiltinType>();
        StringRef identifier(name);
        self->name = identifier;
        self->native_function = wrap_native(function);

        Expr* stmt = module->new_object<Expr>();
        stmt->value = self;
        module->body.push_back(stmt);
        return *this;
    }

    template<typename O>
    struct NativeClassBinder {

        template<typename ...Args>
        NativeClassBinder& constructor() {
            return *this;
        }

        template<typename ...Args>
        NativeClassBinder& method() {
            return *this;
        }
    

        template<typename T>
        NativeClassBinder& attribute(String const& name) {
            return *this;
        }

        // method + attribute
        NativeClassBinder& member() {
            return *this;
        }

        // !?
        template<typename ...Args>
        NativeClassBinder& virtual_method() {
            return *this;
        }
    };

    template<typename T>
    NativeClassBinder<T> klass(String const& name) {
         NativeClassBinder<T> builder;
         return builder;
    }

    struct Module* module;
};


}