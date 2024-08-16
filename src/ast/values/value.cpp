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
    case meta::ValueTypes::Max:
        return false;
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
std::ostream& ostream_op(std::ostream& os, _Invalid const& v) { return os << "Invalid"; }

std::ostream& ostream_op(std::ostream& os, Pointf const& v) { return os << "Point(" << v.x << ", " << v.y << ")"; }
std::ostream& ostream_op(std::ostream& os, Pointi const& v) { return os << "Point(" << v.x << ", " << v.y << ")"; }
std::ostream& ostream_op(std::ostream& os, Color const& v) { return os << "Color("<< v.r << ", " << v.g << v.b << ", " << v.a << ")"; }

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

    auto& registry = meta::TypeRegistry::instance();
    auto& meta = registry.id_to_meta[v.tag];

    if (meta.printer) {
        meta.printer(os, v);
        return os;
    }

    // we could insert a function for a given typeid
    return os << "obj";
}

std::ostream& Value::print(std::ostream& os) const {
    return os << (*this);
}

String Value::__repr__() const {
    StringStream ss;
    debug_print(ss);
    return ss.str();
}

std::ostream& Value::debug_print(std::ostream& os) const {
    switch (meta::ValueTypes(tag)) {
#define CASE(type, name)                            \
    case meta::ValueTypes::name:                    \
            return os << value.name << ": " << #type;
        
        KIWI_VALUE_TYPES(CASE)
#undef CASE

    case meta::ValueTypes::Max: break;
    }

    meta::ClassMetadata& meta = meta::classmeta(tag);
    if (meta.printer) {
        meta.printer(os, *this);
        return os << ": " << meta.name;
    }
    return os << "obj: " << meta.name;
}


bool Value::destroy() {
    meta::ClassMetadata& metadata = meta::classmeta(tag);

    if (metadata.deleter) {
        metadata.deleter(nullptr, *this);
        return true;
    }
    return false;
}

Value Value::copy() const {
    auto& registry = meta::TypeRegistry::instance();
    auto& meta = registry.id_to_meta[tag];
    if (meta.copier) {
        return meta.copier(*this);
    }
    return Value();
}

Value Value::ref() {
    // skip lookup for common types
    switch (meta::ValueTypes(tag)) {
#define CASE(type, name)                            \
    case meta::ValueTypes::name:                    \
            return _ref<type>::ref(*this);
        KIWI_VALUE_TYPES(CASE)
#undef CASE

    case meta::ValueTypes::Max: break;
    }

    auto& registry = meta::TypeRegistry::instance();
    auto& meta = registry.id_to_meta[tag];
    if (meta.ref) {
        return meta.ref(*this);
    }
    
    return *this;
}

std::size_t Value::hash() const {
    auto& registry = meta::TypeRegistry::instance();
    auto& meta = registry.id_to_meta[tag];
    if (meta.hasher) {
        return meta.hasher(*this);
    }
    return 0;
}

bool register_metadata(int type_id, const char* name, ValueDeleter deleter, ValueCopier copier, ValuePrinter printer, ValueHash hasher, ValueRef     refmaker) {
    auto& registry = meta::TypeRegistry::instance();
    auto& meta = registry.id_to_meta[type_id];
    meta.name    = name;

    meta.deleter = deleter;
    meta.copier  = copier;
    meta.printer = printer;
    meta.hasher  = hasher;
    meta.ref     = refmaker;

    return true;
}

uint8* value_memory(meta::ClassMetadata const& meta, Value& obj) {
    if (meta.size <= sizeof(Value::Holder) && meta.is_trivially_copyable) {
        return (uint8*)(&obj.value);
    } else {
        return (uint8*)(obj.value.obj);
    }
}


Value getattr(Value obj, String const& name) 
{
    meta::ClassMetadata& meta = meta::classmeta(obj.tag);
    for(meta::Property& member: meta.members) {
        if (String(member.name.c_str()) == name) {
            int type = member.type;
            meta::ClassMetadata const& attrmeta = meta::classmeta(type);
            return member.getattr(obj);
        }
    }

    kwassert(false, "Not handled");
    return Value(_None());
}

Value make_pointer(int tag, void* ptr) {
    Value vptr;
    vptr.tag = tag;
    vptr.value.obj = ptr;
    return vptr;
};


Value setattr(Value& obj, String const& name, Value val) {
    meta::ClassMetadata& meta = meta::classmeta(obj.tag);
    for(meta::Property& member: meta.members) {
        if (String(member.name.c_str()) == name) {
            int type = member.type;
            meta::ClassMetadata const& attrmeta = meta::classmeta(type);
            member.setattr(obj, val);
            return Value(true);
        }
    }

    kwassert(false, "Not handled");
    return Value(false);
}


Value Value::operator() (void* ctx, Array<Value> args) {
    if (meta::type_id<Function>() == tag) {
        return as<Function>()(ctx, args);
    }
    return Value();
}

namespace meta {

Value Property::getattr(void* obj) {
    return impl_getattr(obj);
}

}

}  // namespace lython
