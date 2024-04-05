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

void free_value(Value val, void (*deleter)(void*)) {
    // we don't know the type here
    // we have to tag the value to know no memory was allocated
    // if (sizeof(T) <= sizeof(Value::Holder)) {
    //     return;
    // }

    if (val.is_object()) {
        if (deleter != nullptr) {
            deleter(val.value.obj);
        }

        // NOTE: this only nullify current value so other copy of this value
        // might still think the value is valid
        // one thing we can do is allocate the memory using a pool.
        // on free the memory returns to the pool and it is marked as invalid
        // copied value will be able to check for the mark until the memory is reused
        // then same issue would be still be possible
        val.value.obj = nullptr;  // just in case
    }
}

std::ostream& operator<<(std::ostream& os, _None const& v) { return os << "None"; }

std::ostream& operator<<(std::ostream& os, Value const& v) {
    switch (meta::ValueTypes(v.tag)) {
#define CASE(type, name) \
    case meta::ValueTypes::name: return os << v.value.name;
        KIWI_VALUE_TYPES(CASE)
#undef CASE

    case meta::ValueTypes::Max: break;
    }

    // we could insert a function for a given typeid
    return os << "obj";
}

}  // namespace lython