#include "ast/values/value.h"

namespace lython {

bool Value::operator==(Value const& val) const {
    if (tag == val.tag) {
        // is there a risk that when the value is smaller some garbage remain ?
        switch (meta::ValueTypes(tag)) {

#define CASE(type, name) \
    case meta::ValueTypes::name: return value.name == val.value.name;
            KIWI_VALUE_TYPES(CASE)
#undef CASE
        }

        // To make it work  on arbitrary value we need to 
        // fetch the comparison operator for a particular type id
        // we could save the operator inside the type registry
    }

    return false;
}

Value binary_invoke(void* ctx, Value fun, Value a, Value b) {
    Array<Value> value_args = {a, b};
    return fun.as<Function>()(ctx, value_args);
}

Value unary_invoke(void* ctx, Value fun, Value a) {
    Array<Value> value_args = {a};
    return fun.as<Function>()(ctx, value_args);
}

GetterError Value::global_err = GetterError{false};

std::ostream& ostream_op(std::ostream& os, bool const& v) { 
    if (v) 
        return os << "True"; 
    return os << "False"; 
}
std::ostream& ostream_op(std::ostream& os, uint64 const& v) { return os << v; }
std::ostream& ostream_op(std::ostream& os, int64 const& v) { return os << v; }
std::ostream& ostream_op(std::ostream& os, uint32 const& v) { return os << v; }
std::ostream& ostream_op(std::ostream& os, int32 const& v) { return os << v; }
std::ostream& ostream_op(std::ostream& os, uint16 const& v) { return os << v; }
std::ostream& ostream_op(std::ostream& os, int16 const& v) { return os << v; }
std::ostream& ostream_op(std::ostream& os, uint8 const& v) { return os << v; }
std::ostream& ostream_op(std::ostream& os, int8 const& v) { return os << v; }
std::ostream& ostream_op(std::ostream& os, float32 const& v) { 
    if (v == static_cast<int>(v))
        return os << fmt::format("{:.1f}", v); 

    return os << v; 
}
std::ostream& ostream_op(std::ostream& os, float64 const& v) { 
    if (v == static_cast<int>(v))
        return os << fmt::format("{:.1f}", v); 

    return os << v;  
}
std::ostream& ostream_op(std::ostream& os, Function const& v) { return os << "Function"; }
std::ostream& ostream_op(std::ostream& os, _None const& v) { return os << "None"; }

std::ostream& operator<<(std::ostream& os, Value const& v) {
    switch (meta::ValueTypes(v.tag)) {
#define CASE(type, name)                            \
    case meta::ValueTypes::name:                    \
            return ostream_op(os, v.value.name);
        
        KIWI_VALUE_TYPES(CASE)
#undef CASE

    case meta::ValueTypes::Max: break;
    }

    static int strtid = meta::type_id<String>();

    if (strtid == v.tag) {
        return os << '"' << v.as<String const&>() << '"';
    }

    // we could insert a function for a given typeid
    return os << "obj";
}

}  // namespace lython