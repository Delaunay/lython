#ifndef LYTHON_VALUE_HEADER
#define LYTHON_VALUE_HEADER

#include <iostream>

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
    obj_string
};

template <typename T>
VTag retrieve_tag(){
    return VTag(-1);
}

#define X(type) template<> inline VTag retrieve_tag<type>() { return pod_##type; }
  POD_TYPES(X)
#undef X

namespace AbstractSyntaxTree {
    class Function;
}



struct Value{
public: // Object Value
    using BuiltinImpl = std::function<Value(Array<Value>&)>;

    struct Closure{
        AbstractSyntaxTree::Function* fun = nullptr;
        Array<Value> env;
        BuiltinImpl builtin;
    };

public:
    VTag tag;

    union {
        #define X(type) type v_##type;
          POD_TYPES(X)
        #undef X
    } pod_data;

    Closure v_closure;
    String v_str;

    //! T: type to retrieve the value as
    //! VTag: type held inside the Value obj
    template<typename T, VTag tag>
    T as(){ return _DataFetcher<T, tag>::fetch(this);}

    std::ostream& print(std::ostream& out){
        switch (tag) {
            #define X(type) case pod_##type:{\
                return out << #type << '(' << pod_data.v_##type << ')';\
            }
                POD_TYPES(X)
            #undef X

            case obj_closure: { return out << "closure";}
            case obj_string: { return out << "String(" << v_str << ")";}
        }
        return out;
    }

private: // cant have partial specialization of functions
    template<typename T, VTag tag>
    struct _DataFetcher{
          static T fetch(Value* v){
              std::cout << "default: ";
              v->print(std::cout);
              return T();
          }
    };

    #define X(type)\
        template<typename T>\
        struct _DataFetcher<T, pod_##type>{\
            static T fetch(Value* v){ return T(v->pod_data.v_##type); }\
        };
      POD_TYPES(X)
    #undef X

public: // constructor
    #define X(type) Value(type o_##type): tag(pod_##type){ pod_data.v_##type = o_##type; }
      POD_TYPES(X)
    #undef X

      Value(AbstractSyntaxTree::Function* fun, Array<Value>  env): tag(obj_closure){
          v_closure = {fun, env, nullptr};
      }

      Value(BuiltinImpl fun, Array<Value>  env): tag(obj_closure){
          v_closure = {nullptr, env, fun};
      }

    Value(String str): tag(obj_string){
        v_str    = std::move(str);
    }
};

}

#endif
