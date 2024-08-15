#pragma once

#include "sema/bindings.h"
#include "ast/nodes.h"
#include "utilities/object.h"
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

    using Arguments             = typename Interop<FunctionType>::Arguments;
    using ReturnType            = typename Interop<FunctionType>::ReturnType;
    using CanonicalFunctionType = typename Interop<FunctionType>::FreeMethodType;

    enum {
        arg_count = arg_count<FunctionType>()
    };

    // Bindings const* bindings;
    Module* module = nullptr;

    FunctionTypeBuilder(Module* module):
        module(module)
    {}

    template<int N>
    int add_arg(Arrow* arrow) {
        if constexpr (N > 0) {
            using ElemT = typename std::tuple_element<arg_count - N, Arguments>::type;

            arrow->args[arg_count - N] = lookup_type<ElemT>();

            add_arg<N - 1>(arrow);
        }
        return 0;
    }

    template<typename T>
    ExprNode* lookup_type() {
        return Typename<std::remove_pointer_t<T>>::get_typename(module);
    }

    Arrow* maketype() {
        Arrow* arrow = module->new_object<Arrow>();
        arrow->args.resize(arg_count);

        add_arg<arg_count>(arrow);

        arrow->returns = lookup_type<ReturnType>();
        return arrow;
    }
};


template <typename FunctionType>
Arrow* function_type_builder(Module* mod) {
    return FunctionTypeBuilder<FunctionType>(mod).maketype();
}

#if 1

template<typename O, typename ...Args>
struct ConstructoHelper {

    // FIXME: if O is small it might not need allocation
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

template<typename Owner>
FunctionDef* make_native_function(Owner* module, StringRef name, Array<String> const& argnames, Function implementation, Arrow* type) {
    FunctionDef* def = module->template new_object<FunctionDef>();
    def->name = StringRef(name);
    def->native = implementation;
    def->type = type;

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

    module->body.push_back(def);
    return def;
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

    // Deprecated ?
    // this might allow us to add more meta data in the future
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
            FunctionDef* def = make_native_function(
                self.module,
                name,
                argnames,
                wrapped_function,
                function_type
            );
            return self;
        }
    };


    // .def<&add>("add", "a", "b")
    template<auto Fun, typename ...Names>
    NativeModuleBuilder& def(String const& funname, Names... argnames) {
        using Wrapper = Interop<decltype(Fun)>;
        using FreeMethodType = typename Wrapper::FreeMethodType;

        static_assert(arg_count<decltype(Fun)>() >= sizeof...(argnames), "Number of arguments must match number of strings");

        FunctionDef* def = make_native_function(
            module,
            funname,
            Array<String>{argnames...},
            Wrapper::template wrapper<Fun>,
            function_type_builder<FreeMethodType>(module)
        );
        return *this;
    }

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

        NativeModuleBuilder& end() {
            return self;
        }

        Module* get_module() {
            return module;
        }
        
        template<typename ...Args>
        using ConstructorMaker = function_maker<
            NativeClassBinder<O>, 
            decltype (ConstructoHelper<O, Args...>::template ctor)    
        >;

        template<typename ...Args>
        auto constructor(std::initializer_list<String> argnames) {
            if (sizeof...(Args) < argnames.size()) {
                // error: "More names than arguments"
            }

            meta::register_members<O>();

            using Wrapper = Interop<O(Args...)>;
            using FreeMethodType = typename Wrapper::FreeMethodType;

            FunctionDef* def = make_native_function(
                class_t,
                StringRef("__init__"),
                Array<String>(argnames),
                Wrapper::constructor,
                function_type_builder<FreeMethodType>(module)
            );

            // add the def to class_t so vm can find __init__ when looking for it

            return *this;
        }

        template<auto Fun, typename ...Names>
        NativeClassBinder& def(String const& funname, Names... argnames) {
            using Wrapper = Interop<decltype(Fun)>;
            using FreeMethodType = typename Wrapper::FreeMethodType;

            static_assert(arg_count<FreeMethodType>() >= sizeof...(argnames), "Number of arguments must match number of strings");

            FunctionDef* def = make_native_function(
                class_t,
                funname,
                Array<String>{argnames...},
                Wrapper::template wrapper<Fun>,
                function_type_builder<FreeMethodType>(module)
            );
            return *this;
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