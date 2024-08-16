#ifndef LYTHON_VALUE_EXCEPTION_HEADER
#define LYTHON_VALUE_EXCEPTION_HEADER

// Actual method signature is WIP
// most mght become references

#include "dtypes.h"
#include "vm/tree.h"
#include "utilities/allocator.h"

namespace lython {


struct KIGNORE() _LyException: public GCObject {
    _LyException(Array<StackTrace> const& traces):
        traces(traces)
    {}

    Array<StackTrace> traces;

    // type of the exception
    // this is to help with exception matching
    ExprNode* type = nullptr;

    // Custom exception generated
    // this is what is created when raise AttributeError(...)
    // is called
    Value custom;  

    Value cause;
};


// template <>
// struct lython::meta::ReflectionTrait<_LyException> {
//     static int register_members() {
//         lython::meta::new_member<_LyException, Array<StackTrace>>("traces");
//         lython::meta::new_member<_LyException, ExprNode*>("type");
//         lython::meta::new_member<_LyException, ConstantValue>("custom");
//         return 0;
//     }
// };

// // Here we make exception hold the stack trace but the stack trace should be held by the VM
// // the Exception object is just going to hold some user specified info
// struct lyException: public NativeValue<_LyException> {

//     lyException(Array<StackTrace> const& traces): 
//         NativeValue<_LyException>(traces) 
//     {}

//     // type
//     // message
//     String _type = String("AssertionError");
//     String _message = String("Very bad");

//     virtual String message() { return _message; }

//     virtual String type_str() { return _type; }

//     Array<StackTrace>& traces() {
//         return object.traces;
//     }

//     ExprNode* type() {
//         return object.type;
//     }

//     ConstantValue custom() {
//         return object.custom;
//     }  
// };

}  // namespace lython

#endif