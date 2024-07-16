
#include "ast/values/value.h"




namespace lython {


// Currently Kiwi only supports C function pointers
// so no extra memory is required

// to support more we would need to create a new
// Function object that would type erase the differences
// between bound methods, lambdas and C function pointers

// We do not need to support bound methods on the C++ side
// Kiwi runtime will handle it similar to how Kiwi methods are handled
//  --> less complexity
//
// capturing lambdas might be useful but I worry that
// having that implicit state be hidden of kiwi might not be a good idea
template<typename FunctionType>
struct FunctionPtr {

    FunctionType function;
    void* object = nullptr;

    // invoke == Interop<FunctionType>::invoke(this)
};


template<typename FunctionType>
FunctionPtr<typename Interop<FunctionType>::FreeMethodType> function(FunctionType function) {
    return {typename Interop<FunctionType>::freemethod};
}


template<typename O, typename FunctionType>
FunctionPtr<typename Interop<FunctionType>::FreeMethodType> method(O* self, FunctionType function) {
    return {typename Interop<FunctionType>::freemethod, self};
}


}