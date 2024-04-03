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

    Generator():
        NativeObject(meta::type_id<Generator>())
    {}

    ConstantValue __next__(struct TreeEvaluator& vm);

    // Scope use to evaluate this generator
    Bindings scope;

    // Function we execute
    FunctionDef* generator;

    bool is_native() const override { return false; }
    bool is_pointer() const override { return false; }
    bool is_valid() const override { return true; }
    int8* _memory() override { return nullptr; }
};
}  // namespace lython

#endif