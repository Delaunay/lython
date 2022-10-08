#ifndef LYTHON_GENERATOR_HEADER
#define LYTHON_GENERATOR_HEADER

// Actual method signature is WIP
// most mght become references

#include "dtypes.h"
#include "native.h"

namespace lython {

/* Compile time representation of an object
 */
struct Object: public NativeObject {
    public:
};
}  // namespace lython

#endif