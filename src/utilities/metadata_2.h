#pragma once

namespace lython {

namespace meta {
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

//
// Small type reserve their ID
//

#define TYPEID_SPEC(type, name)                                     \
    template <>                                                     \
    struct _type_id<type> {                                         \
        static constexpr int id() { return int(ValueTypes::name); } \
    };

    KIWI_VALUE_TYPES(TYPEID_SPEC)
#undef TYPEID_SPEC

#define DEFINE_METADATA(type_, tname)                                 \
    template <>                                                      \
    inline const char* type_name<type_>() {                           \
        static const char* name = meta::register_type<type_>(#tname); \
        return name;                                                 \
    }

TYPES_METADATA(DEFINE_METADATA)




}  // namespace meta

}
