#ifndef LYTHON_GENERATOR_HEADER
#define LYTHON_GENERATOR_HEADER

// Actual method signature is WIP
// most mght become references

#include "dtypes.h"
#include "native.h"
#include "sema/bindings.h"

namespace lython {

// this value is created when a function yields instead of returning
struct Generator: public NativeObject {
    public:
    ConstantValue __next__(struct TreeEvaluator& vm);

    // Scope use to evaluate this generator
    Bindings scope;

    // Function we execute
    FunctionDef* generator;
};
}  // namespace lython

#endif