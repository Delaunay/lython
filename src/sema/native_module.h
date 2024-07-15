#pragma once

#include "sema/bindings.h"
#include "ast/nodes.h"
#include "utilities/object.h"
#include "utilities/metadata_method.h"
#include "ast/interop.h"
#include "sema/importlib.h"

namespace lython {

/*
template <typename Sig>
struct FunctionTypeBuilder;
*/

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

template <typename FunctionType>
struct FunctionTypeBuilder {
    /*
    using arguments_t = typename std::tuple<Args...>;
    using return_t    = LY_TYPENAME R;
    using function_t  = LY_TYPENAME R(Args...);
    */

    using arguments_t = typename Interop<FunctionType>::Arguments;
    using return_t    = typename Interop<FunctionType>::ReturnType;
    using function_t  = typename Interop<FunctionType>::FreeMethodType;

    // Bindings const* bindings;
    Module* module = nullptr;

    FunctionTypeBuilder(Module* module):
        module(module)
    {}

    template<int N>
    int add_arg(Arrow* arrow) {
        using TupleSize = typename std::tuple_size<arguments_t>;

        if constexpr (N > 0) {
            using ElemT = typename std::tuple_element<TupleSize::value - N, arguments_t>::type;

            arrow->args[TupleSize::value - N] = lookup_type<ElemT>();

            add_arg<N - 1>(arrow);
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
        add_arg<n>(arrow);

        arrow->returns = lookup_type<return_t>();

        //std::cout << typeid(R(Args...)).name() << " " << str(arrow) << std::endl;
        return arrow;
    }
};


template <typename FunctionType>
Arrow* function_type_builder(Module* mod) {
    return FunctionTypeBuilder<FunctionType>(mod).maketype();
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

    using ctor_t = decltype(ConstructoHelper::ctor);

};

template<typename FunctionType>
std::function<Value(void*, Array<Value>&)> cpp_lambda(FunctionType func) {
    return [func](void* mem, Array<Value>& args) {
        return Interop<FunctionType>::wrapper(func, mem, args);
    };
}


// This builds a module before sema
// sema will go through it a build a Binding
struct NativeModuleBuilder {
    NativeModuleBuilder(String const&name, class ImportLib& importsys):
        module(importsys.newmodule(name))
     {}


    Module* get_module() {
        return module;
    }

    // builder.function<fun>("name")
    template<typename Parent, typename FunctionType>
    struct function_maker {
        using Wrapper = Interop<FunctionType>;
        
        Parent&       self;
        StringRef     name;
        Function      wrapped_function = nullptr;
        Arrow*        function_type = nullptr;
        Array<String> argnames;

        template<FunctionType fun>
        function_maker& set() {
            wrapped_function = Wrapper::template wrapper<fun>;

            using FreeMethodType = \
                typename Interop<FunctionType>::FreeMethodType;

            function_type = function_type_builder<FreeMethodType>(
                    self.get_module()
            );
            return *this;
        }

        template<typename ...Args>
        function_maker& args(Args... names) {
            argnames = Array<String>{
                names...
            };
            return *this;
        }

        function_maker& set(FunctionType fun) {
            // How to make this work
            // I think the only way is to use std::function
            wrapped_function = cpp_lambda(fun);
            
            return *this;
        }

        Parent& end() {
            FunctionDef* def = self.module->template new_object<FunctionDef>();
            def->name = name;
            def->native = wrapped_function;
            def->type = function_type;
            int i = 0;

            if (def->type && def->type->args.size() > 0) {
                for(String const& argname: argnames) {
                    Arg arg;
                    arg.arg = argname;
                    arg.annotation = def->type->args[i];
                    def->args.args.push_back(arg);
                    i += 1;
                }
            }
            self.module->body.push_back(def);
            return self;
        }
    };

    // def<test>()
    //  .fun("name")
    template<typename FunctionType>
    function_maker<NativeModuleBuilder, FunctionType> function(String const& name) {
        return (function_maker<NativeModuleBuilder, FunctionType>{*this, StringRef(name)});
    }

    template<typename O>
    struct NativeClassBinder {
        NativeModuleBuilder& self;
        Module* module;
        ClassDef* class_t = nullptr;
        
        // .klass()
        //      .constructor<int, int>()
        //


        Module* get_module() {
            return module;
        }
        
        template<typename ...Args>
        using ConstructorMaker = function_maker<
            NativeClassBinder<O>, 
            decltype (ConstructoHelper<O, Args...>::template ctor)    
        >;

        template<typename ...Args>
        auto constructor() {
            meta::register_members<O>();
            return ConstructorMaker<Args...>{*this, StringRef("__init__")};
        }

        template<typename FunctionType>
        using MethodMaker = function_maker<
            NativeClassBinder<O>, 
            FunctionType
        >;
    
        template<typename FunctionType>
        auto method(String const& name) {
            return MethodMaker<FunctionType>{*this, StringRef(name)};
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