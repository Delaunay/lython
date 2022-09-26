#pragma once

#include "ast/constant.h"
#include <cmath>

namespace lython {

// -
// Unary (T) -> T
// -

template <typename T, typename Implementation>
struct UnaryOperation {
    struct Params {
        T r;
        T a;
    };

    static ConstantValue vm(ConstantValue const& a) {
        return ConstantValue(Implementation::call(a.get<T>()));
    }

    static void call(Params& params) { params.r = Implementation::call(params.a); }

    static T call(T a) { return Implementation::call(a); }
};

template <typename T>
struct Invert: public UnaryOperation<T, Invert<T>> {
    static T call(T a) { return ~a; }
};

template <typename T>
struct Not: public UnaryOperation<T, Not<T>> {
    static T call(T a) { return !a; }
};

template <typename T>
struct UAdd: public UnaryOperation<T, UAdd<T>> {
    static T call(T a) { return +a; }
};

template <typename T>
struct USub: public UnaryOperation<T, USub<T>> {
    static T call(T a) { return -a; }
};

template <>
struct USub<uint8>: public UnaryOperation<uint8, USub<uint8>> {
    static int8 call(uint8 a) { return -int8(a); }
};

template <>
struct USub<uint16>: public UnaryOperation<uint16, USub<uint16>> {
    static int16 call(uint16 a) { return -int16(a); }
};

template <>
struct USub<uint32>: public UnaryOperation<uint32, USub<uint32>> {
    static int32 call(uint32 a) { return -int32(a); }
};

template <>
struct USub<uint64>: public UnaryOperation<uint64, USub<uint64>> {
    static int64 call(uint64 a) { return -int64(a); }
};

// -
// Binary (T, T) -> T
// -
template <typename T, typename Implementation>
struct BinaryOperation {
    struct Params {
        T r;
        T a;
        T b;
    };

    static void call(Params& params) { params.r = Implementation::call(params.a, params.b); }

    static ConstantValue vm(ConstantValue const& a, ConstantValue const& b) {
        return ConstantValue(Implementation::call(a.get<T>(), b.get<T>()));
    }

    static T call(T a, T b) { return Implementation::call(a, b); }
};

template <typename T>
struct Add: public BinaryOperation<T, Add<T>> {
    static T call(T a, T b) { return a + b; }
};

template <typename T>
struct Sub: public BinaryOperation<T, Sub<T>> {
    static T call(T a, T b) { return a - b; }
};

template <typename T>
struct Pow: public BinaryOperation<T, Pow<T>> {
    static T call(T a, T b) { return T(std::pow(a, b)); }
};

template <typename T>
struct LShift: public BinaryOperation<T, LShift<T>> {
    static T call(T a, T b) { return a << b; }
};

template <typename T>
struct RShift: public BinaryOperation<T, RShift<T>> {
    static T call(T a, T b) { return a >> b; }
};

template <typename T>
struct Mult: public BinaryOperation<T, Mult<T>> {
    static T call(T a, T b) { return a * b; }
};

template <typename T>
struct Div: public BinaryOperation<T, Div<T>> {
    static T call(T a, T b) { return a / b; }
};

template <typename T>
struct Mod: public BinaryOperation<T, Mod<T>> {
    static T call(T a, T b) { return a % b; }
};

template <>
struct Mod<float>: public BinaryOperation<float, Mod<float>> {
    static float call(float a, float b) { return fmodf(a, b); }
};

template <>
struct Mod<double>: public BinaryOperation<double, Mod<double>> {
    static double call(double a, double b) { return fmod(a, b); }
};

template <typename T>
struct And: public BinaryOperation<T, And<T>> {
    static T call(T a, T b) { return a && b; }
};

template <typename T>
struct Or: public BinaryOperation<T, Or<T>> {
    static T call(T a, T b) { return a || b; }
};

template <typename T>
struct BitAnd: public BinaryOperation<T, BitAnd<T>> {
    static T call(T a, T b) { return a & b; }
};

template <typename T>
struct BitOr: public BinaryOperation<T, BitOr<T>> {
    static T call(T a, T b) { return a | b; }
};

template <typename T>
struct BitXor: public BinaryOperation<T, BitXor<T>> {
    static T call(T a, T b) { return a ^ b; }
};

// -
// Comparison (T, T) -> bool
// -
template <typename T, typename Implementation>
struct ComparisonOperation {
    struct Params {
        bool r : 1;
        T    a;
        T    b;
    };

    static ConstantValue vm(ConstantValue const& a, ConstantValue const& b) {
        return ConstantValue(Implementation::call(a.get<T>(), b.get<T>()));
    }

    static void call(Params& params) { params.r = Implementation::call(params.a, params.b); }

    static T call(T a, T b) { return Implementation::call(a, b); }
};

template <typename T>
struct Eq: public ComparisonOperation<T, Eq<T>> {
    static T call(T a, T b) { return a == b; }
};

template <typename T>
struct NotEq: public ComparisonOperation<T, NotEq<T>> {
    static T call(T a, T b) { return a != b; }
};

template <typename T>
struct Lt: public ComparisonOperation<T, Lt<T>> {
    static T call(T a, T b) { return a < b; }
};

template <typename T>
struct LtE: public ComparisonOperation<T, LtE<T>> {
    static T call(T a, T b) { return a <= b; }
};

template <typename T>
struct Gt: public ComparisonOperation<T, Gt<T>> {
    static T call(T a, T b) { return a > b; }
};

template <typename T>
struct GtE: public ComparisonOperation<T, GtE<T>> {
    static T call(T a, T b) { return a >= b; }
};

template <typename T>
struct Is: public ComparisonOperation<T, Is<T>> {
    static T call(T a, T b) { return a == b; }
};

template <typename T>
struct IsNot: public ComparisonOperation<T, IsNot<T>> {
    static T call(T a, T b) { return a != b; }
};

template <typename T>
struct In: public ComparisonOperation<T, In<T>> {
    static T call(T a, T b) { return a == b; }
};

template <typename T>
struct NotIn: public ComparisonOperation<T, NotIn<T>> {
    static T call(T a, T b) { return a != b; }
};

}  // namespace lython