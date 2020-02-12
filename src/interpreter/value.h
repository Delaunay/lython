#ifndef LYTHON_VALUE_HEADER
#define LYTHON_VALUE_HEADER

#include <iostream>
#include <fmt/core.h>

#include "../Types.h"
#include "../logging/logging.h"
#include "../utilities/allocator.h"

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
    X(char   )

enum VTag {
    #define X(type) pod_##type,
        POD_TYPES(X)
    #undef X
    obj_closure,
    obj_string,
    obj_object,
    obj_none
};

template <typename T>
VTag retrieve_tag(){
    return VTag(-1);
}

#define X(type) template<> inline VTag retrieve_tag<type>() { return pod_##type; }
  POD_TYPES(X)
#undef X


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
    }

struct Value {
public: // Object Value
    using BuiltinImpl = std::function<Value(Array<Value>&)>;

    struct Closure {
        AST::Function const * fun = nullptr;
        Array<Value>          env;
        BuiltinImpl           builtin;
    };

    struct Object {
        // Dict<String, Value> attributes;
    };

public:
    VTag tag;

    union {
        #define X(type) type v_##type;
          POD_TYPES(X)
        #undef X
    } pod_data;

    Object  v_object;
    Closure v_closure;
    String v_str;

public:
    String str(){
        StringStream ss;
        print(ss);
        return ss.str();
    }

    std::ostream& print(std::ostream& out) const {
        switch (tag) {
            #define X(type) case pod_##type:{\
                return out << #type << '(' << pod_data.v_##type << ')';\
            }
                POD_TYPES(X)
            #undef X

            case obj_closure: { return out << "closure";}
            case obj_object: {
                return out << "object";
            }
            case obj_string: { return out << "String(" << v_str << ")";}
            case obj_none: return out << "None";
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
        throw std::runtime_error("invalid get expression, type mismatch!");
    }

    Closure& get_closure();

public: // constructor
    #define X(type) Value(type o_##type): tag(pod_##type){ pod_data.v_##type = o_##type; }
      POD_TYPES(X)
    #undef X

    Value(AST::Function const* fun, Array<Value>  env): tag(obj_closure){
        v_closure = {fun, std::move(env), nullptr};
    }

    Value(BuiltinImpl fun, Array<Value>  env): tag(obj_closure){
        v_closure = {nullptr, std::move(env), std::move(fun)};
    }

    Value(String str): tag(obj_string){
        v_str    = std::move(str);
    }

    bool operator==(Value const& v) const{
        if (v.tag != tag){
            return false;
        }

        switch(tag){
        #define X(type)\
            case pod_##type:{\
                 return pod_data.v_##type == v.pod_data.v_##type;\
            }
            POD_TYPES(X)
        #undef X

        default:
            return false;
        }
    }
    };

#define X(type)\
    template<>\
    inline type Value::get(){\
        if (this->tag != pod_##type)\
            throw TypeError("{}: expected {} got {}", obj_object, this->tag);\
        return this->pod_data.v_##type;\
    }
    POD_TYPES(X)
#undef X

template<>
inline Value::Closure& Value::get()
{
    if (this->tag != obj_closure)
        throw TypeError("{}: expected {} got {}", obj_object, this->tag);
    return this->v_closure;
}

template<>
inline Value::Object& Value::get()
{
    if (this->tag != obj_object)
        throw TypeError("{}: expected {} got {}", obj_object, this->tag);
    return this->v_object;
}

inline Value::Closure& Value::get_closure(){
    return get<Closure&>();
}

}

#endif
