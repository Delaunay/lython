#ifndef LYTHON_EXCEPTION_HEADER
#define LYTHON_EXCEPTION_HEADER

// Actual method signature is WIP
// most mght become references

#include "dtypes.h"
#include "native.h"
#include "vm/tree.h"

namespace lython {

// Here we make exception hold the stack trace but the stack trace should be held by the VM
// the Exception object is just going to hold some user specified info
struct lyException: public NativeObject {

    lyException(Array<StackTrace> const& traces): traces(traces) {}

    Array<StackTrace> traces;

    // type of the exception
    // this is to help with exception matching
    ExprNode* type;

    // Custom exception generated
    // this is what is created when raise AttributeError(...)
    // is called
    ConstantValue custom;
};
}  // namespace lython

#endif