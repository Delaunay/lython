#ifndef LYTHON_CONSTANT_HEADER
#define LYTHON_CONSTANT_HEADER

#include "dtypes.h"
#include <iostream>

#define CLANG_FMT_FIX

namespace lython {

struct NativeObject;

template<typename T>
struct _accessor {
    static T* get(struct ConstantValue& self) { return nullptr; }
};

/*
template<typename T>
struct Point {
    T x;
    T y;
};

template<typename T>
struct Vector {
    T x;
    T y;
    T z;
};

template<typename T>
struct Color {
    T r;
    T g;
    T b;
    T a;
};

template<typename T>
struct Transform2D {
    T matrix[9];
};


template<typename T>
struct Transform3D {
    T matrix[16];
};

*/

struct ConstantValue {
    public:
    struct invalid_t {
        bool operator==(invalid_t const& t) const { return true; }
    };

    struct none_t {
        bool operator==(none_t const& t) const { return true; }
    };

    static ConstantValue const& none() {
        static ConstantValue n{none_t()};
        return n;
    }

#define ConstantType(POD, CPX)                      \
    POD(Invalid, ConstantValue::invalid_t, invalid) \
    POD(i8, int8, i8)                               \
    POD(i16, int16, i16)                            \
    POD(i32, int32, i32)                            \
    POD(i64, int64, i64)                            \
    POD(u8, uint8, u8)                              \
    POD(u16, uint16, u16)                           \
    POD(u32, uint32, u32)                           \
    POD(u64, uint64, u64)                           \
    POD(f32, float32, f32)                          \
    POD(f64, float64, f64)                          \
    POD(Bool, bool, boolean)                        \
    POD(None, ConstantValue::none_t, none)          \
    POD(Object, NativeObject*, object)              \
    POD(Function, meta::VoidFunction, function)     \
    CPX(String, String, string)

#define NUMERIC_CONSTANT(NUM) \
    NUM(i8, int8, i8)         \
    NUM(i16, int16, i16)      \
    NUM(i32, int32, i32)      \
    NUM(i64, int64, i64)      \
    NUM(u8, uint8, u8)        \
    NUM(u16, uint16, u16)     \
    NUM(u32, uint32, u32)     \
    NUM(u64, uint64, u64)     \
    NUM(f32, float32, f32)    \
    NUM(f64, float64, f64)    \
    NUM(Bool, bool, boolean)

    // clang-format off
    enum Type {
        #define ENUM(a)      T##a,
        #define POD(a, b, c) ENUM(a)
        #define CPX(a, b, c) ENUM(a)

        ConstantType(POD, CPX)

        #undef CPX
        #undef POD
        #undef ENUM

    };

    #define POD(k, type, name) explicit ConstantValue(type v): kind(T##k) { value.name = v; }
    #define CPX(k, type, name) explicit ConstantValue(type v): kind(TInvalid) { set_##name(v); }

    ConstantType(POD, CPX);

    #undef CPX
    #undef POD
    // clang-format on

    ConstantValue() = default;

    ConstantValue(ConstantValue const& vv): kind(TInvalid) { copy_union(vv.kind, vv.value); }

    ~ConstantValue() { remove_cpx(); }

    ConstantValue& operator=(ConstantValue const& vv) {
        copy_union(vv.kind, vv.value);
        return *this;
    }

    bool operator==(ConstantValue const& v) const {
        if (v.kind != kind) {
            return false;
        }

        // clang-format off
        switch (kind) {
        #define CMP(kind, type, name) case T##kind: return value.name == v.value.name;

        #define POD(kind, type, name) CMP(kind, type, name)
        #define CPX(kind, type, name) CMP(kind, type, name)

        ConstantType(POD, CPX);

        #undef CPX
        #undef POD
        }
        // clang-format on

        return false;
    }

    void print(std::ostream& out) const;

    void debug_print(std::ostream& out) const;

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

    template <typename T>
    T const& get() const {
        return value.i64;
    }

    template<typename T>
    friend struct _accessor;

    template<typename T>
    T* address() { return _accessor<T>::get(*this); }


    Type get_kind() const {
        return kind;
    }

    friend struct NativeObject;

    private:
    // ast.Str, ast.Bytes, ast.NameConstant, ast.Ellipsis
    union ValueVariant {
        ValueVariant() {}
        ~ValueVariant(){}

        ;  // clang-format off
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

    void _print_object(std::ostream& out) const;

    template <typename T>
    void set_cpx(Type ktype, T& memory, const T& data) {
        if (kind != ktype) {
            new (&memory) T(data);
        } else {
            memory = data;
        }
        kind = ktype;
    }

    ;  // clang-format off
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

    void copy_union(Type k, ValueVariant const& v) {
        if (kind != k) {
            remove_cpx();
        }

        switch (k) {
            ;  // clang-format off
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

// Explicit specialization needs to be declare outisde of the class
#define POD(kind, type, name)                             \
    template <>                                           \
    inline type const& ConstantValue::get<type>() const { \
        return value.name;                                \
    }
#define CPX(kind, type, name)                             \
    template <>                                           \
    inline type const& ConstantValue::get<type>() const { \
        return value.name;                                \
    }

ConstantType(POD, CPX);

#undef CPX
#undef POD


#define ENUM(k, T, n)                                                   \
    template<>                                                          \
    struct _accessor<T> {                                               \
        static T* get(ConstantValue& self) { return &self.value.n; }    \
    };

#define POD(k, t, n) ENUM(k, t, n)
#define CPX(a, b, c) 

ConstantType(POD, CPX)

#undef CPX
#undef POD
#undef ENUM

}  // namespace lython

#endif