#pragma once

#if 0
#include <string>
#include <tuple>
#include <functional>
#include <vector>

// first
#include "metadata_1.h"

namespace lython {

namespace meta {
namespace wrapper {
template <typename Sig>
struct signature;

struct NativeObject;

#if ! __linux__
#define LY_TYPENAME typename
#else
#define LY_TYPENAME
#endif

template <typename R, typename... Args>
struct signature<R(Args...)> {
    using arguments_t = typename std::tuple<Args...>;
    using return_t    = LY_TYPENAME R;
    using function_t  = LY_TYPENAME R(Args...);
    using freemethod_t = R (*)(Args...);

    using value_t   = lython::NativeObject*;
    using generic_t   = std::function<value_t(GCObject*, std::vector<value_t> const& args)>;
    using generic_c_t = value_t (*)(GCObject*, freemethod_t, std::vector<value_t> const& args);
};

template <typename R, typename O, typename... Args>
struct signature<R (O::*)(Args...)> {
    using arguments_t = typename std::tuple<Args...>;
    using return_t    = R;
    using class_t     = O;

    using packed_t   = typename std::tuple<R, Args...>;
    using freemethod_arg_t   = typename std::tuple<O*, Args...>;
    using freemethod_t = R (*)(O*, Args...);

    using value_t   = lython::NativeObject*;
    using generic_t = value_t (*)(GCObject*, O*, freemethod_t, std::vector<value_t> const& args);
};

template <typename Rx, typename Ty, size_t... Indices>
auto impl_free_method(Rx Ty::*method, std::index_sequence<Indices...>) ->
    typename signature<decltype(method)>::freemethod_t {
    using Arguments = typename signature<decltype(method)>::arguments_t;
    using FunctionT = typename signature<decltype(method)>::freemethod_t;

    // Allow the method to be captured without changing the lambda into a closure
    static Rx Ty::*static_method = method;

    // Save it to the C function type to ensure it works
    FunctionT lambda = [](Ty* self,
                          typename std::tuple_element<Indices, Arguments>::type... args) {  //
        return (self->*static_method)(args...);                                             //
    };
    return lambda;
}

template <typename Rx, typename Ty>
auto free_method(Rx Ty::*method) {
    using Arguments = typename signature<decltype(method)>::arguments_t;

    return impl_free_method(                                                               //
        std::forward<Rx Ty::*>(method),                                                    //
        std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Arguments>>>{}  //
    );                                                                                     //
}

template<int N, typename... Types>
int packargs(std::tuple<Types...>& destination, std::vector<lython::NativeObject*> const& args) {
    using TupleSize = typename std::tuple_size<std::remove_reference_t<std::tuple<Types...>>>;

    if constexpr (N > 0) {
        using ElemT = typename std::tuple_element<TupleSize::value - N, std::tuple<Types...>>::type;

        lython::NativeObject* val = args[TupleSize::value - N];

        // convert here
        std::get<TupleSize::value - N>(destination) = *(val->as<ElemT>());
        packargs<N - 1>(destination, args);
  
    }
    return 0;
}


template <typename Ret, typename Ty, typename... Args, size_t... I>
Ret apply_method(Ty* self,
                 Ret (Ty::*method)(Args...),
                 const std::tuple<Args...>& args,
                 std::index_sequence<I...>) {
    return (self->*method)(std::get<I>(args)...);
}

// Define a function that calls the helper function.
template <typename Ret, typename Ty, typename... Args>
Ret call_method(Ty* self, Ret (Ty::*method)(Args...), const std::tuple<Args...>& args) {
    return apply_method(self, method, args, std::index_sequence_for<Args...>{});
}

// Do I need the method special case? once the method is converted to a free method
// 
template <typename Rx, typename Ty>
auto wrap_method_generic(Rx Ty::* method) {
    using Arguments   = typename signature<decltype(method)>::arguments_t;
    using Packed      = typename signature<decltype(method)>::packed_t;
    using FunctionT   = typename signature<decltype(method)>::generic_t;
    using ReturnT     = typename signature<decltype(method)>::return_t;
    using FreeMethodT = typename signature<decltype(method)>::freemethod_t;

    using ValueT    = lython::NativeObject*;

    // Save it to the C function type to ensure it works
    FunctionT lambda = [](GCObject* root, Ty* self, FreeMethodT native, std::vector<ValueT> const& args) -> ValueT {  //
        // Unpack args to the correct Type into Packed

        Arguments tuple_args;

        packargs<std::tuple_size_v<std::remove_reference_t<Arguments>>>(tuple_args, args);

        // Allocate memory for the return value
        // this needs to be allocated by the owning object
        NativeValue<ReturnT>* wrapped_result = root->new_object<NativeValue<ReturnT>>();

        // Apply the free method
        (*wrapped_result->template as<ReturnT>()) = std::apply(native, std::tuple_cat(std::make_tuple(self), tuple_args));

        // call the function
        // (*wrapped_result->as<ReturnT>()) = call_method(self, native, tuple_args);

        return reinterpret_cast<ValueT>(wrapped_result);
        // return nullptr;
    };
    return lambda;  //
}

}  // namespace wrapper

template <typename Rx, typename Ty>
void new_method(std::string const& name, Rx Ty::*method) {
    ClassMetadata& registry = classmeta(type_id<Ty>());

    // Create a Free method
    // i.e transform obj.method(*args) into method(*obj, *args)
    // this function is compatible with a C pointer
    auto freemethod = wrapper::free_method(method);

    // Generate a wrapper around a method
    // that convert Values into the right type and return a generic value
    // this calls the freemethod that is being generated
    //
    auto function = wrapper::wrap_method_generic(method);
    
    registry.members.emplace_back(                  //
        name,                                       //
        type_id<Rx>(),                              //
        -1,                                         //
        sizeof(VoidFunction),
        reinterpret_cast<VoidFunction>(freemethod), // native method
        reinterpret_cast<VoidFunction>(function)    // wrapped method
    );
}

}  // namespace meta
}  // namespace lython

#endif