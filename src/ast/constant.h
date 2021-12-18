#ifndef LYTHON_CONSTANT_HEADER
#define LYTHON_CONSTANT_HEADER

#include <iostream>

namespace lython {

struct ConstantValue {
    public:
    struct invalid_t {};
    struct none_t {};

    ; // clang-format off
    #define ConstantType(POC, CPX)       \
        POD(Invalid, invalid_t, invalid) \
        POD(Int, int, integer)           \
        POD(Float, float32, singlef)     \
        POD(Double, float64, doublef)    \
        CPX(String, String, string)      \
        POD(Bool, bool, boolean)         \
        POD(None, none_t, none)

    enum Type {
        #define ENUM(a)      T##a,
        #define POD(a, b, c) ENUM(a)
        #define CPX(a, b, c) ENUM(a)

        ConstantType(POD, CPX)
 
        #undef CPX
        #undef POD
        #undef ENUM
        
    };

    #define POD(k, type, name) ConstantValue(type v): kind(T##k) { value.name = v; }
    #define CPX(k, type, name) ConstantValue(type v): kind(TInvalid) { set_##name(v); }

    ConstantType(POD, CPX);
    
    #undef CPX 
    #undef POD
    // clang-format on

    ConstantValue() = default;

    ConstantValue(ConstantValue const &vv): kind(TInvalid) { copy_union(vv.kind, vv.value); }

    ~ConstantValue() { remove_cpx(); }

    ConstantValue &operator=(ConstantValue const &vv) {
        copy_union(vv.kind, vv.value);
        return *this;
    }

    void print(std::ostream &out) const;

    bool is_none() const { return kind == TNone; }

    // clang-format off
    bool is_pod() const {
        switch (kind) {
        #define POD(kind, type, name) case T##kind: return true;
        #define CPX(kind, type, name) case T##kind: return false;

        ConstantType(POD, CPX);
        
        #undef CPX
        #undef POD
        }
    }
    // clang-format on

    Type type() const { return kind; }

    private:
    // ast.Str, ast.Bytes, ast.NameConstant, ast.Ellipsis
    union ValueVariant {
        ValueVariant() {}
        ~ValueVariant(){}

        ; // clang-format off
        #define ATTR(type, name)      type name;
        #define POD(kind, type, name) ATTR(type, name)
        #define CPX(kind, type, name) ATTR(type, name)

        ConstantType(POD, CPX);

        #undef CPX
        #undef POD
        #undef ATTR
        // clang-format on
    };

    ValueVariant value;
    Type         kind = TInvalid;

    template <typename T>
    void set_cpx(Type ktype, T &memory, const T &data) {
        if (kind != ktype) {
            new (&memory) T(data);
        } else {
            memory = data;
        }
        kind = ktype;
    }

    ; // clang-format off
    #define POD(kind, type, name)
    #define CPX(kind, type, name)\
    void set_##name(const type &data) { set_cpx(T##kind, value.name, data); }
    
    ConstantType(POD, CPX);

    #undef CPX
    #undef POD
    // clang-format on

    void remove_cpx() {
        // clang-format off
        switch (kind) {

        #define POD(kind, type, name) case T##kind: return;
        #define CPX(kind, type, name) case T##kind: value.name.~type(); break;

        ConstantType(POD, CPX);
        
        #undef CPX
        #undef POD
        }
        // clang-format on
    }

    void copy_union(Type k, ValueVariant const &v) {
        if (kind != k) {
            remove_cpx();
        }

        switch (k) {
            ; // clang-format off
            #define POD(k, type, name)\
            case T##k:{\
                value.name = v.name;\
                break;\
            }
            
            #define CPX(k, type, name)\
            case T##k:{\
                set_##name(v.name);\
                break;\
            }

            ConstantType(POD, CPX);

            #undef CPX
            #undef POD
            // clang-format on
        }
        kind = k;
    }
};

} // namespace lython

#endif