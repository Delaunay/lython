#ifndef LYTHON_NATIVE_HEADER
#define LYTHON_NATIVE_HEADER

#include "dtypes.h"
#include "utilities/object.h"

namespace lython {

struct NativeObject: GCObject {};
}  // namespace lython

#endif