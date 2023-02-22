#ifndef LYTHON_NATIVE_HEADER
#define LYTHON_NATIVE_HEADER

#include "dtypes.h"
#include "utilities/object.h"

namespace lython {

// TODO: allow native object to register their own methods
struct NativeObject: GCObject {

    virtual bool is_native() const { return true; }

};
}  // namespace lython

#endif