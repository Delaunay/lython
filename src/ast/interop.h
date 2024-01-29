#pragma once

#include <functional>

#include "ast/nodes.h"
#include "ast/constant.h"

namespace lython {

template<typename Sig> 
class Interop {

};

// TODO:
//  Avoid memory allocation here,
//  Change the call so the return object is already allocated
template<typename R, typename ...Args> 
struct Interop<R(Args...)> {
    using NativeArgs = std::tuple<Args...>;
    using ScriptValue = lython::Constant*;
    using ScriptArgs = Array<ScriptValue>;
    using ScriptFunction_C = ScriptValue (*)(GCObject*, R(*)(Args...), ScriptArgs const&);

    // Convert script arguments to native arguments
    template<int N, typename... Types>
    static int packargs(std::tuple<Types...>& destination, ScriptArgs const& args) {
        using TupleSize = typename std::tuple_size<std::remove_reference_t<std::tuple<Types...>>>;

        if constexpr (N > 0) {
            using ElemT = typename std::tuple_element<TupleSize::value - N, std::tuple<Types...>>::type;

            ScriptValue erased = args[TupleSize::value - N];
            ConstantValue const& value = erased->value;

            if (value.type() == ConstantValue::TObject) 
            {
                NativeObject* object = value.get<NativeObject*>();
                std::get<TupleSize::value - N>(destination) = *(object->as<ElemT>());
            } 
            else 
            {
                std::get<TupleSize::value - N>(destination) = value.get<ElemT>();
            }

            packargs<N - 1>(destination, args);
        }

        return 0;
    }

    template<typename T>
    static Constant* allocate_return_value(GCObject* mem, T*& value) {

        if (!in(meta::type_id<T>(), meta::type_id<double>())) {
            NativeValue<T>* wrapped_result = mem->new_object<NativeValue<T>>();
            Constant* return_value = mem->new_object<Constant>(wrapped_result);
            value = wrapped_result->template as<T>();
            return return_value;
        }

        Constant* return_value = mem->new_object<Constant>(T());
        value = return_value->value.address<T>();
        return return_value;
    }

    static ScriptFunction_C wrap() {
        // Save it to the C function type to ensure it works
        return [](GCObject* mem, R(*fun)(Args...), ScriptArgs const& args) -> ScriptValue {  //
            // Unpack args to the correct Type into Packed
            NativeArgs tuple_args;

            packargs<std::tuple_size_v<std::remove_reference_t<NativeArgs>>>(tuple_args, args);

            // Allocate memory for the return value
            // this needs to be allocated by the owning object
            // NativeValue<R>* wrapped_result = mem->new_object<NativeValue<R>>();
            // Constant* return_value = mem->new_object<Constant>(wrapped_result);
            
            R* value = nullptr;
            Constant* rval = allocate_return_value(mem, value);

            // Apply the free method
            (*value) = std::apply(           //
                fun,                                                    //
                std::tuple_cat(tuple_args)                              //
            );

            return rval;
        };
    }
};

template<typename R, typename ...Args> 
BuiltinType::WrappedNativeFunction wrap_native(R(*fun)(Args...)) {
    return [fun](GCObject*mem, Array<Constant*> const& arguments) {
        return Interop<R(Args...)>::wrap()(mem, fun, arguments);
    };
}

}