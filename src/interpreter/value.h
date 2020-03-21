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
    invalid,
    #define X(type) pod_##type,
        POD_TYPES(X)
    #undef X
    pod_str,
    obj_closure,
    obj_object,     // class instance object
    obj_class,      // class type
    obj_module,     // imported package
    obj_none
};


struct str{};

template <typename T>
ValueKind retrieve_tag(){
    return ValueKind::invalid;
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

template<typename V>
struct _State{
    struct Entry{
        Entry(V v, String const& name = ""):
            v(v), n(name)
        {}

        V      v;
        String n;
    };

    Array<Entry> env;

    std::ostream& dump(std::ostream& out){
        out << String(50, '-') << '\n';
        out << env.size() << std::endl;

        for(auto i = 0ul; i < env.size(); ++i){
            out << fmt::format("{:4d} | {:20} | {}", i, env.at(i).n, env.at(i).v) << std::endl;
        }

        out << String(50, '-') << '\n';
        return out;
    }

    void push(V v, String const& name = ""){
        env.emplace_back(v, name);
    }

    void pop(std::ptrdiff_t n){
        env.erase(env.begin() + n, env.end());
    }

    template<typename T = int>
    T size() const {
        return T(env.size());
    }

    template<typename T>
    V operator[] (T i) const {
        assert(env.size() > i, "Index should be inside size()");
        return env[env.size() - std::size_t(i)].v;
    }
};

struct Value {
    friend Value new_object(AST::Struct const *type);

    using BuiltinImpl = std::function<Value(_State<Value>&)>;

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

    std::ostream& print(std::ostream& out, int depth = 0) const;

    template<typename Stream = std::ostream>
    friend Stream& operator<<(Stream& out, Value const& val) {
        val.print(out);
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

    template<typename T>
    T get() const {
        throw TypeError("{}(const): expected {} got {}", to_string(retrieve_tag<T>()), to_string(tag));
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

    Value(AST::Function const* fun, _State<Value>* env = nullptr);
    Value(BuiltinImpl fun, _State<Value>* env = nullptr);
    Value(AST::Struct const* cstruct);
    Value(_State<Value>& state, String const& name = "-");

    Value():
        tag(ValueKind::obj_none)
    {}

    bool operator==(Value const& v) const;

private:
    Value(ValueKind tag):
        tag(tag)
    {}
};

using State = _State<Value>;

Value new_object(const AST::Struct *type);

namespace value {
// Object
struct Struct: public ValueHolder{
    using Attributes = Array<Value>;

    Struct(AST::Struct const* type);

    void set_attribute(int idx, Value val);
    void set_attribute(StringRef name, Value val);

    Value get_attributes(int idx);
    Value get_attributes(StringRef name);

    AST::Struct const* type;
    Attributes attributes;

    std::ostream& print(std::ostream& out, int depth = 0) const;
};

// Result of a closure
struct Closure: public ValueHolder {
    using BuiltinImpl = std::function<Value(State&)>;

    Closure(AST::Function const* fun, State* env, BuiltinImpl builtin):
        fun(fun), env(env), builtin(builtin)
    {}

    AST::Function const * fun = nullptr;
    State*                env = nullptr;
    BuiltinImpl           builtin;

    std::ostream& print(std::ostream& out, int depth = 0) const;
};

// Struct Type
struct Class: public ValueHolder{
    Class(AST::Struct const* cstruct = nullptr):
        fun(cstruct)
    {}

    AST::Struct const* fun;

    std::ostream& print(std::ostream& out, int depth = 0) const;
};

struct Module: public ValueHolder{
    Module(State& state, String const& name): state(state), name(name)
    {}

    State state;
    String name;

    std::ostream& print(std::ostream& out, int = 0) const {
        return out << "Module<" << name << ">";
    }
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


template<>
inline value::Struct const* Value::get() const
{
    if (this->tag != ValueKind::obj_object)
        throw TypeError("{}: expected {} got {}", to_string(ValueKind::obj_object), to_string(tag));

    return static_cast<value::Struct const*>(_obj.get());
}

template<>
inline value::Closure const* Value::get() const
{
    if (this->tag != ValueKind::obj_closure)
        throw TypeError("{}: expected {} got {}", to_string(ValueKind::obj_object), to_string(tag));

    return static_cast<value::Closure const*>(_obj.get());
}

template<>
inline value::Class const* Value::get() const
{
    if (this->tag != ValueKind::obj_class)
        throw TypeError("{}: expected {} got {}", to_string(ValueKind::obj_class), to_string(tag));

    return static_cast<value::Class const*>(_obj.get());
}


}

#endif
