#ifndef LYTHON_EXCEPTION_HEADER
#define LYTHON_EXCEPTION_HEADER

// Actual method signature is WIP
// most mght become references

#include "dtypes.h"
#include "native.h"

namespace lython {

struct Exception: public NativeObject {
    public:
    virtual NativeObject __next__();
};
}  // namespace lython

#endif