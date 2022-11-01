#ifndef LYTHON_METADATA_H
#define LYTHON_METADATA_H

#include "dtypes.h"
// #include "ast/nodes.h"

namespace lython {

#define TYPES_METADATA(X) \
    X(String, String)     \
    X(char, Char)         \
    X(int8, Int8)         \
    X(int16, Int16)       \
    X(int32, Int32)       \
    X(int64, Int64)       \
    X(uint8, UInt8)       \
    X(uint16, UInt16)     \
    X(uint32, UInt32)     \
    X(uint64, UInt64)     \
    X(float32, Float32)   \
    X(float64, Float64)

namespace meta {
#define DEFINE_METADATA(type, tname)                                 \
    template <>                                                      \
    inline const char* type_name<type>() {                           \
        static const char* name = meta::register_type<type>(#tname); \
        return name;                                                 \
    }

TYPES_METADATA(DEFINE_METADATA)
}  // namespace meta

void metadata_init_names();

void track_static();

void register_globals();

}  // namespace lython

#endif
