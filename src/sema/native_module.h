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
        // TODO: handle imported types
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

            add_arg<N - 1, Types...>(arrow);
        }
        return 0;
    }

    template<typename T>
    ExprNode* lookup_type() {
        return Typename<std::remove_pointer_t<T>>::get_typename(module);
    }

    Arrow* maketype() {
        // using TupleSize = typename std::tuple_size<std::remove_reference_t<std::tuple<Args...>>>;
        constexpr int n = std::tuple_size_v<std::remove_reference_t<arguments_t>>;

        Arrow* arrow = module->new_object<Arrow>();
        arrow->args.resize(n);

        // arguments<Args...>(arrow);
        add_arg<n, Args...>(arrow);

        arrow->returns = lookup_type<return_t>();

        //std::cout << typeid(R(Args...)).name() << " " << str(arrow) << std::endl;
        return arrow;
    }
};


template <typename R, typename... Args>
Arrow* function_type_builder(Module* mod, R(*fun)(Args...)) {
    return FunctionTypeBuilder<R(Args...)>(mod).maketype();
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

#if 1

template<typename O, typename ...Args>
struct ConstructoHelper {
    static O* ctor(Args... args) {
        return new O(args...);
    }
};

template<typename FunctionType>
std::function<Value(void*, Array<Value>&)> cpp_lambda(FunctionType func) {
    return [func](void* mem, ScriptArgs& args) {
        return Interop<FunctionType>::wrapper(func, mem, args);
    };
}


// This builds a module before sema
// sema will go through it a build a Binding
struct NativeModuleBuilder {
    NativeModuleBuilder(String const&name, class ImportLib& importsys):
        module(importsys.newmodule(name))
     {}

    // builder.function<fun>("name")
    template<typename FunctionType>
    struct function_maker {
        using Wrapper = Interop<FunctionType>;
        
        NativeModuleBuilder& self;
        StringRef name;
        Function wrapped_function = nullptr;
        Arrow* function_type = nullptr;

        template<FunctionType fun>
        function_maker& set() {
            wrapped_function = Wrapper::template wrapper<fun>;
            function_type = function_type_builder(self.module, fun);
            return *this;
        }

        function_maker& set(FunctionType fun) {
            // How to make this work
            // I think the only way is to use std::function
            wrapped_function = cpp_lambda(fun);
            
            return *this;
        }

        NativeModuleBuilder& end() {
            FunctionDef* def = self.module->new_object<FunctionDef>();
            def->name = name;
            def->native = wrapped_function;
            def->type = function_type;
            self.module->body.push_back(def);
            return self;
        }
    };

    // def<test>()
    //  .fun("name")
    template<typename FunctionType>
    function_maker<FunctionType> function(String const& name) {
        return (function_maker<FunctionType>{*this, StringRef(name)});
    }

    template<typename O>
    struct NativeClassBinder {
        NativeModuleBuilder& self;
        Module* module;
        ClassDef* class_t = nullptr;
        
        // .klass()
        //      .constructor<int, int>()
        //
        template<typename ...Args>
        struct ctor_maker {
            NativeClassBinder& self;
            StringRef name;
            Function wrapped_function = nullptr;
            Arrow* function_type = nullptr;

            ctor_maker& set() {
                using Ctor =  ConstructoHelper<O, Args...>;
                using Wrapper = Interop<O*(Args...)>;
                wrapped_function = Wrapper::template wrapper<Ctor::ctor>;
                function_type = function_type_builder(self.module, Ctor::ctor);
                return *this;
            }

            NativeClassBinder& end() {
                FunctionDef* def = self.module->new_object<FunctionDef>();
                def->name = StringRef("__init__");
                def->native = wrapped_function;
                def->type = function_type;
                self.module->body.push_back(def);
                return self;
            }
        };

        template<typename ...Args>
        NativeClassBinder& constructor() {
            meta::register_members<O>();
            auto maker = ctor_maker<Args...>{*this};
            maker.set();
            maker.end();
            return *this;
        }

        template<typename FunctionType>
        struct method_maker {
            using Wrapper = Interop<FunctionType>;
            
            NativeClassBinder& self;
            StringRef name;
            Function wrapped_function = nullptr;
            Arrow* function_type = nullptr;

            template<FunctionType fun>
            method_maker& set() {
                wrapped_function = Wrapper::template wrapper<fun>;
                function_type = function_type_builder(self.module, Wrapper::template freemethod<fun>);
                return *this;
            }

            method_maker& set(FunctionType fun) {
                wrapped_function = cpp_lambda(fun)>;
                return *this;
            }

            NativeClassBinder& end() {
                FunctionDef* def = self.module->new_object<FunctionDef>();
                def->name = name;
                def->native = wrapped_function;
                self.module->body.push_back(def);
                return self;
            }
        };

        template<typename FunctionType>
        method_maker<FunctionType> method(String const& name) {
            return (method_maker<FunctionType>{*this, StringRef(name)});
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
         NativeClassBinder<T> builder{*this};
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