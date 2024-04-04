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

inline ExprNode* new_typename(GCObject* root, StringRef name) {
    Name* expr = root->new_object<Name>();
    expr->id = name;
    return expr;
}

template<typename T>
struct Typename{
    // Generic lookup of the type inside the module
    static ExprNode* get_typename(Module* mod) {  
        for(StmtNode* node: mod->body) {
            // we do not have its type name just its type id
            // custom type should be a classdef though 
            if (ClassDef* def = cast<ClassDef>(node)) {
                if (def->type_id == meta::type_id<T>()) {
                    return new_typename(mod, def->name);
                }
            }
        } 
        return new_typename(mod, StringRef("<unknown>")); 
    }
};

#define TYPE(name, native)                                          \
    template<>                                                      \
    struct Typename<native> {                                       \
        static ExprNode* get_typename(Module* mod) {                \
            return new_typename(mod, StringRef(#name));             \
        }                                                           \
    };
    
    BUILTIN_TYPES(TYPE)

#undef TYPE


template <typename R, typename... Args>
struct FunctionTypeBuilder<R(Args...)> {
    using arguments_t = typename std::tuple<Args...>;
    using return_t    = LY_TYPENAME R;
    using function_t  = LY_TYPENAME R(Args...);

    // Bindings const* bindings;
    Module* module = nullptr;

    FunctionTypeBuilder(Module* module):
        module(module)
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
        return Typename<std::remove_pointer_t<T>>::get_typename(module);

        #if 0
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
        #endif
    }

    Arrow* function() {
        // using TupleSize = typename std::tuple_size<std::remove_reference_t<std::tuple<Args...>>>;
        constexpr int n = std::tuple_size_v<std::remove_reference_t<arguments_t>>;

        Arrow* arrow = module->new_object<Arrow>();
        arrow->args.resize(n);

        // arguments<Args...>(arrow);
        add_arg<n, Args...>(arrow);

        arrow->returns = lookup_type<return_t>();
        return arrow;
    }
};


template<typename R, typename ...Args>
Arrow* function_type_builder(Module* mod, R(*function)(Args...)) {
    FunctionTypeBuilder<R(Args...)> builder(mod);

    return builder.function();
}

// MetaData: type -> id
// Bendings: type -> id -> BuiltinType
// 


//
// Those insert the native bindings into sema to be call by the VM
//
/*
template<typename R, typename ...Args>
void register_native_function(GCObject* root, Bindings& binding, String const& name, R(*function)(Args...)) {
    FunctionDef* self = root->new_object<FunctionDef>();
    StringRef identifier(name);
    self->name = identifier;
    self->native = wrap_native(function);

    Arrow* funtype = function_type_builder(root, function);

    binding.add(
        self->name,
        self,
        funtype
    );
}
*/

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

    FunctionDef* ctor_fun = root->new_object<FunctionDef>();
    ctor_fun->type = ctor_t;
    ctor_fun->name = name;
    ctor_fun->native = [](GCObject* mem, Array<Constant*> const& args) -> Constant* {
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

namespace meta {
template<typename Func>
struct function_traits;

template<typename R, typename... Args>
struct function_traits<R(*)(Args...)> {
    using function_type = R(*)(Args...);
    using function = std::function<R(Args...)>;
};

template<typename R, typename... Args>
struct function_traits<std::function<R(Args...)>> {
    using function_type = R(*)(Args...);
    using function = std::function<R(Args...)>;
};

template<typename Func>
using function_type_of = typename function_traits<std::decay_t<Func>>::function_type;

}

#if 0
// This builds a module before sema
// sema will go through it a build a Binding
struct NativeModuleBuilder {
    NativeModuleBuilder(String const&name, class ImportLib& importsys):
        module(importsys.newmodule(name))
     {}

    template<typename R, typename ...Args>
    NativeModuleBuilder& function(String const& name, R(*function)(Args...)) {
        FunctionDef* self = module->new_object<FunctionDef>();
        StringRef identifier(name);
        self->name = identifier;
        self->native = wrap_native(function);
        self->type = function_type_builder(module, function);

        module->body.push_back(self);
        return *this;
    }

    template<typename O>
    struct NativeClassBinder {
        Module* module;
        ClassDef* class_t = nullptr;

        template<typename ...Args>
        NativeClassBinder& constructor() {
            meta::register_members<O>();

            FunctionDef* self = class_t->new_object<FunctionDef>();
            StringRef identifier("__init__");
            self->name = identifier;

            FunctionTypeBuilder<O*(Args...)> builder(module);
            self->type = builder.function();
            
            self->native = [](GCObject* mem, Array<Constant*> const& args) -> Constant* {
                // that is a lot of back to back allocation
                // we can could combine them in one
                // or/and remove some intermediate, NativePointer is probably not that necessary
                NativePointer<O>* obj = mem->new_object<NativePointer<O>>(); // Allocate
                
                auto arguments = Interop<O(Args...)>::from_script(args);

                void* memory = malloc(sizeof(O) * 1);                        // Allocate
                O* rawobj = helper::ctor<O>(memory, arguments);

                obj->set_pointer(rawobj);
                Constant* val = mem->new_object<Constant>();
                val->value = ConstantValue(obj);
                return val;                      // Allocate
            };
            class_t->body.push_back(self);
            return *this;
        }

        template<typename R, typename ...Args>
        NativeClassBinder& method(String const& name, R(*function)(Args...)) {
            FunctionDef* self = class_t->new_object<FunctionDef>();
            StringRef identifier(name);
            self->name = identifier;
            self->native = wrap_native(function);
            self->type = function_type_builder(module, function);
            class_t->body.push_back(self);
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
         builder.module = module;
         builder.class_t = module->new_object<ClassDef>();
         builder.class_t->name = name;
         builder.class_t->type_id = meta::type_id<T>();
         module->body.push_back(builder.class_t);
         return builder;
    }

    struct Module* module;
};
#endif

}