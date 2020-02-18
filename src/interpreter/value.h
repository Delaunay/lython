#ifndef LYTHON_VALUE_HEADER
#define LYTHON_VALUE_HEADER

#include <iostream>
#include <spdlog/fmt/bundled/core.h>

#include "dtypes.h"
#include "ast/names.h"
#include "logging/logging.h"
#include "utilities/allocator.h"

namespace lython {

// have to remove int8 and uint8 from here since it is the same as uchar and char
#define POD_TYPES(X)\
    X(uint64 )\
    X(int64  )\
    X(uint32 )\
    X(int32  )\
    X(uint16 )\
    X(int16  )\
    X(float32)\
    X(float64)\
    X(uchar  )\
    X(char   )\

enum class ValueKind: uint64 {
    #define X(type) pod_##type,
        POD_TYPES(X)
    #undef X
    pod_str,
    obj_closure,
    obj_object,     // class instance object
    obj_class,      // class type
    obj_none
};


struct str{};

template <typename T>
ValueKind retrieve_tag(){
    return ValueKind(-1);
}

#define X(type)\
template<>\
inline ValueKind retrieve_tag<type>(){\
    return ValueKind::pod_##type;\
}
POD_TYPES(X)
#undef X

String to_string(ValueKind kind);

// NEW_EXCEPTION(TypeError)

class TypeError: public Exception{
public:
    template<typename ... Args>
    TypeError(const char* fmt, const Args& ... args):
        Exception(fmt, "TypeError", args...)
    {}
};


namespace AST {
    class Function;
    class Node;
    class Struct;
}

namespace value {
    struct ValueHolder{};
    struct Struct;
    struct Class;
    struct Closure;
}

struct Value {
    friend Value new_object();

    using BuiltinImpl = std::function<Value(Array<Value>&)>;

    ValueKind tag;

private:
    union {
    #define X(type) type v_##type;
        POD_TYPES(X)
    #undef X
    } _data;

    // Complex Object Holder
    std::shared_ptr<value::ValueHolder> _obj = nullptr;

public:
    String str(){
        StringStream ss;
        print(ss);
        return ss.str();
    }

    std::ostream& print(std::ostream& out) const {
        switch (tag) {
            #define X(type) case ValueKind::pod_##type:{\
                return out << #type << '(' << _data.v_##type << ')';\
            }
                POD_TYPES(X)
            #undef X
            case ValueKind::pod_str:
                return out << "String(" << StringDatabase::instance()[_data.v_uint64] << ")";
            case ValueKind::obj_closure:
                return out << "closure";
            case ValueKind::obj_class:
                return out << "class";
            case ValueKind::obj_object:
                return out << "object";
            case ValueKind::obj_none:
                return out << "None";
            }
        return out;
    }

public:
    //! T: type to retrieve the value as
    //! VTag: type held inside the Value obj
    template<typename T, typename V>
    T cast(){ return T(get<V>());}

    template<typename T>
    T get() {
        throw TypeError("{}: expected {} got {}", to_string(retrieve_tag<T>()), to_string(tag));
    }

    value::Closure* get_closure();

public:
    // constructor
    #define X(type)\
    Value(type o_##type):\
        tag(ValueKind::pod_##type){\
        _data.v_##type = o_##type;\
    }

    POD_TYPES(X)
    #undef X

    // I think we can only do this for compile time string
    // need a way to handle runtime string differently
    Value(StringRef const& str): tag(ValueKind::pod_str){
      _data.v_uint64 = str.ref;
    }

    // Delegate to stringref
    Value(String const& str):
        Value(get_string(str))
    {}

    Value(AST::Function const* fun, Array<Value>  env);
    Value(BuiltinImpl fun, Array<Value>  env);
    Value(AST::Struct const* cstruct);

    Value():
        tag(ValueKind::obj_none)
    {}

    bool operator==(Value const& v) const{
        if (v.tag != tag){
            return false;
        }

        switch(tag){
            #define X(type)\
            case ValueKind::pod_##type:{\
                 return _data.v_##type == v._data.v_##type;\
            }
            POD_TYPES(X)
            #undef X

        default:
            return false;
        }
    }

private:
    Value(ValueKind tag):
        tag(tag)
    {}
};

Value new_object();

namespace value {
// Object
struct Struct: public ValueHolder{
    using AttrDict = Dict<StringRef, Value, string_ref_hash>;
    AttrDict attributes;
};

// Result of a closure
struct Closure: public ValueHolder {
    using BuiltinImpl = std::function<Value(Array<Value>&)>;

    Closure(AST::Function const* fun, Array<Value> env, BuiltinImpl builtin):
        fun(fun), env(env), builtin(builtin)
    {}

    AST::Function const * fun = nullptr;
    Array<Value>          env;
    BuiltinImpl           builtin;
};

// Struct Type
struct Class: public ValueHolder{
    Class(AST::Struct const* cstruct = nullptr):
        fun(cstruct)
    {}

    AST::Struct const* fun;
};
}

#define X(type)\
    template<>\
    inline type Value::get(){\
        if (this->tag != ValueKind::pod_##type)\
            throw TypeError("{}: expected {} got {}", to_string(retrieve_tag<type>()), to_string(tag));\
        return this->_data.v_##type;\
    }
    POD_TYPES(X)
#undef X

template<>
inline value::Closure* Value::get()
{
    if (this->tag != ValueKind::obj_closure)
        throw TypeError("{}: expected {} got {}", to_string(ValueKind::obj_object), to_string(tag));

    return get_closure();
}

template<>
inline value::Class* Value::get()
{
    if (this->tag != ValueKind::obj_class)
        throw TypeError("{}: expected {} got {}", to_string(ValueKind::obj_class), to_string(tag));

    return static_cast<value::Class*>(_obj.get());
}

template<>
inline value::Struct* Value::get()
{
    if (this->tag != ValueKind::obj_object)
        throw TypeError("{}: expected {} got {}", to_string(ValueKind::obj_object), to_string(tag));

    return static_cast<value::Struct*>(_obj.get());
}


}

#endif
