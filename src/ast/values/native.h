#ifndef LYTHON_NATIVE_HEADER
#define LYTHON_NATIVE_HEADER

#include "dtypes.h"
#include "utilities/object.h"

namespace lython {

// TODO: allow native object to register their own method
struct NativeObject: GCObject {
    //
};
}  // namespace lython

#endif