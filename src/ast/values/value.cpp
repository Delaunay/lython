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


meta::Member const* find_member(Value obj, String const& name) {
    meta::ClassMetadata const& meta = meta::classmeta(obj.tag);
    for(meta::Member const& member: meta.members) {
        if (String(member.name.c_str()) == name) {
            return &member;
        }
    }
    return nullptr;
}


uint8* value_memory(meta::ClassMetadata const& meta, Value& obj) {
    if (meta.size <= sizeof(Value::Holder) && meta.is_trivially_copyable) {
        return (uint8*)(&obj.value);
    } else {
        return (uint8*)(obj.value.obj);
    }
}


Value getattr(Value obj, String const& name) {
    meta::Member const* member = nullptr;
    meta::ClassMetadata const& meta = meta::classmeta(obj.tag);
    for(meta::Member const& mb: meta.members) {
        if (String(mb.name.c_str()) == name) {
            member = &mb;
            break;
        }
    }

    //
    if (member) {
        int type = member->type;
        meta::ClassMetadata const& attrmeta = meta::classmeta(type);

        Value property;
        property.tag = type;        

        void* dest = (void*)(&property.value);
        void* src = value_memory(meta, obj);

        // property is a value held in the object itself
        if (attrmeta.is_trivially_copyable && attrmeta.size <= sizeof(Value::Holder)) { 
            src = (uint8*)(src) + member->offset;
            memcpy(dest, src, member->size);
            return property;
        }

        // the object is going to be a pointer
        property.value.obj = (uint8*)(src) + member->offset;
        return property;
    }

    kwassert(false, "Not handled");
    return Value(_None());
}

Value getattrref(Value& obj, String const& name) {
    meta::Member const* member = nullptr;
    meta::ClassMetadata const& meta = meta::classmeta(obj.tag);
    for(meta::Member const& mb: meta.members) {
        if (String(mb.name.c_str()) == name) {
            member = &mb;
            break;
        }
    }

    //
    if (member) {
        int type = member->type;
        meta::ClassMetadata const& attrmeta = meta::classmeta(type);

        Value property;
        property.tag = attrmeta.weakref_type_id;        

        void* dest = (void*)(&property.value);
        void* src = value_memory(meta, obj);

        property.value.obj = (uint8*)(src) + member->offset;
        return property;
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


void setattr(Value& obj, String const& name, Value val) {
    meta::Member const* member = nullptr;
    meta::ClassMetadata const& meta = meta::classmeta(obj.tag);
    for(meta::Member const& mb: meta.members) {
        if (String(mb.name.c_str()) == name) {
            member = &mb;
            break;
        }
    }

    if (member) {
        int type = member->type;
        meta::ClassMetadata const& attrmeta = meta::classmeta(type);
        kwassert(type == val.tag, "must match attribute type");

        void* dest = value_memory(meta, obj) + member->offset;

        if (attrmeta.is_trivially_copyable && attrmeta.size <= sizeof(Value::Holder)) { 
            void* src = (void*)(&val.value);
            memcpy(dest, src, member->size);
            return;
        }

        // the object is going to be a pointer
        // property could be pointer or value
        // on our side Value will always be a pointer
       
        // this is pointer on pointer
        if (attrmeta.type_id == attrmeta.weakref_type_id) {
            dest = val.value.obj;
            return;
        } else {
            // if this is a move then we should zero out the source
            // so it cannot be used anymore
            // C++ classes that allocates memory
            // should only be used by one owner
            // Zero out the source
            // memset(src, 0, member->size);
            if (attrmeta.is_trivially_copyable) {
                // copy our pointer to the value
                void* src = val.value.obj;
                memcpy(dest, src, member->size);
                return;
            }

            // dest is a pointer to a member
            // but it needs a copy destination
            // this should call a copy/asignment
            Value valdest;
            valdest.tag = attrmeta.weakref_type_id;
            valdest.value.obj = dest;
            attrmeta.assign(valdest, val);
            return;
        }
    }
    kwassert(false, "Not handled");
}

}  // namespace lython
