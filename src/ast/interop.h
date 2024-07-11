#pragma once

#include <functional>

#include "ast/nodes.h"

#if 0

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
    using ScriptValue = lython::Value;
    using ScriptArgs = Array<ScriptValue>;
    using ScriptFunction_C = ScriptValue (*)(void*, R(*)(Args...), ScriptArgs const&);

    static NativeArgs from_script(ScriptArgs const& args) {
        NativeArgs arguments;

        packargs<std::tuple_size_v<std::remove_reference_t<NativeArgs>>>(arguments, args);

        return arguments;
    }

    // Convert script arguments to native arguments
    template<int N, typename... Types>
    static int packargs(std::tuple<Types...>& destination, ScriptArgs const& args) {
        using TupleSize = typename std::tuple_size<std::remove_reference_t<std::tuple<Types...>>>;

        if constexpr (N > 0) {
            using ElemT = typename std::tuple_element<TupleSize::value - N, std::tuple<Types...>>::type;

            ScriptValue erased = args[TupleSize::value - N];
            ConstantValue const& value = erased->value;
            ElemT& dest = std::get<TupleSize::value - N>(destination);

            // FIXME
            // raise an error on type mismatch ?
            
            // Object
            if constexpr (std::is_pointer<ElemT>::value) {
                using ObjectT = typename std::remove_pointer<ElemT>::type;

                NativeObject* object = value.get<NativeObject*>();

                dest = object->as<ObjectT>();
            }
            else {
                // this check does not need to happen at runtime
                if (value.type() == ConstantValue::TObject) 
                {
                    NativeObject* object = value.get<NativeObject*>();
                    dest = *(object->as<ElemT>());
                } 
                else 
                {
                    dest = value.get<ElemT>();
                }
            }

            packargs<N - 1>(destination, args);
        }

        return 0;
    }

    template<typename T>
    static Constant* allocate_return_value(GCObject* mem, T*& value) {

        if (!in(meta::type_id<T>(), 
                meta::type_id<double>(),
                meta::type_id<float>(),
                meta::type_id<int8>(),
                meta::type_id<int16>(),
                meta::type_id<int32>(),
                meta::type_id<int64>(),
                meta::type_id<uint8>(),
                meta::type_id<uint16>(),
                meta::type_id<uint32>(),
                meta::type_id<uint64>(),
                meta::type_id<bool>()
                )) {
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
        return [](void* mem, R(*fun)(Args...), ScriptArgs const& args) -> ScriptValue {  //
            // Unpack args to the correct Type into Packed
            NativeArgs arguments = from_script(args);

            // R* value = nullptr;
            // Constant* rval = allocate_return_value(mem, value);
            
            // Apply the free method
            return make_value<R>(std::apply(  //
                fun,                //
                arguments           //
            ));

            // convert the raw value to our generic constant
            // HERE
            // return rval;
        };
    }
};

}
#endif