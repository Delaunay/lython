
#include "builtin/operators.inc"
#include "ast/nodes.h"
#include "dependencies/coz_wrap.h"
#include "utilities/names.h"

namespace lython {
Dict<StringRef, BinOp::NativeBinaryOp> build_native_binary_operators() {
    // FIXME: add return type, the return type can be different
    Dict<StringRef, BinOp::NativeBinaryOp> map;

    // clang-format off
#define JOIN(op, t1, t2) op-t1-t2
#define JOIN1(op, t1) op-t1
    // clang-format on

#define LAMBDA(op, type) op<type>::vm;

    // clang-format off
    // Float
    map[StringRef(STR(JOIN(Add, f32, f32)))] = LAMBDA(Add, float32);
    map[StringRef(STR(JOIN(Add, f64, f64)))] = LAMBDA(Add, float64);

    map[StringRef(STR(JOIN(Sub, f32, f32)))] = LAMBDA(Sub, float32);
    map[StringRef(STR(JOIN(Sub, f64, f64)))] = LAMBDA(Sub, float64);

    map[StringRef(STR(JOIN(Mult, f32, f32)))] = LAMBDA(Mult, float32);
    map[StringRef(STR(JOIN(Mult, f64, f64)))] = LAMBDA(Mult, float64);

    map[StringRef(STR(JOIN(Div, f32, f32)))] = LAMBDA(Div, float32);
    map[StringRef(STR(JOIN(Div, f64, f64)))] = LAMBDA(Div, float64);

    map[StringRef(STR(JOIN(Mod, f32, f32)))] = LAMBDA(Mod, float32);
    map[StringRef(STR(JOIN(Mod, f64, f64)))] = LAMBDA(Mod, float64);

    map[StringRef(STR(JOIN(Pow, f32, f32)))] = LAMBDA(Pow, float32);
    map[StringRef(STR(JOIN(Pow, f64, f64)))] = LAMBDA(Pow, float64);

    // Integer

    map[StringRef(STR(JOIN(Add, i8, i8)))]   = LAMBDA(Add, int8);
    map[StringRef(STR(JOIN(Add, i16, i16)))] = LAMBDA(Add, int16);
    map[StringRef(STR(JOIN(Add, i32, i32)))] = LAMBDA(Add, int32);
    map[StringRef(STR(JOIN(Add, i64, i64)))] = LAMBDA(Add, int64);

    map[StringRef(STR(JOIN(Sub, i8, i8)))]   = LAMBDA(Sub, int8);
    map[StringRef(STR(JOIN(Sub, i16, i16)))] = LAMBDA(Sub, int16);
    map[StringRef(STR(JOIN(Sub, i32, i32)))] = LAMBDA(Sub, int32);
    map[StringRef(STR(JOIN(Sub, i64, i64)))] = LAMBDA(Sub, int64);

    map[StringRef(STR(JOIN(Mult, i8, i8)))]   = LAMBDA(Mult, int8);
    map[StringRef(STR(JOIN(Mult, i16, i16)))] = LAMBDA(Mult, int16);
    map[StringRef(STR(JOIN(Mult, i32, i32)))] = LAMBDA(Mult, int32);
    map[StringRef(STR(JOIN(Mult, i64, i64)))] = LAMBDA(Mult, int64);

    map[StringRef(STR(JOIN(Div, i8, i8)))]   = LAMBDA(Div, int8);
    map[StringRef(STR(JOIN(Div, i16, i16)))] = LAMBDA(Div, int16);
    map[StringRef(STR(JOIN(Div, i32, i32)))] = LAMBDA(Div, int32);
    map[StringRef(STR(JOIN(Div, i64, i64)))] = LAMBDA(Div, int64);

    map[StringRef(STR(JOIN(Mod, i8, i8)))]   = LAMBDA(Mod, int8);
    map[StringRef(STR(JOIN(Mod, i16, i16)))] = LAMBDA(Mod, int16);
    map[StringRef(STR(JOIN(Mod, i32, i32)))] = LAMBDA(Mod, int32);
    map[StringRef(STR(JOIN(Mod, i64, i64)))] = LAMBDA(Mod, int64);

    map[StringRef(STR(JOIN(Pow, i8, i8)))]   = LAMBDA(Pow, int8);
    map[StringRef(STR(JOIN(Pow, i16, i16)))] = LAMBDA(Pow, int16);
    map[StringRef(STR(JOIN(Pow, i32, i32)))] = LAMBDA(Pow, int32);
    map[StringRef(STR(JOIN(Pow, i64, i64)))] = LAMBDA(Pow, int64);

    map[StringRef(STR(JOIN(LShift, i8, i8)))]   = LAMBDA(LShift, int8);
    map[StringRef(STR(JOIN(LShift, i16, i16)))] = LAMBDA(LShift, int16);
    map[StringRef(STR(JOIN(LShift, i32, i32)))] = LAMBDA(LShift, int32);
    map[StringRef(STR(JOIN(LShift, i64, i64)))] = LAMBDA(LShift, int64);

    map[StringRef(STR(JOIN(RShift, i8, i8)))]   = LAMBDA(RShift, int8);
    map[StringRef(STR(JOIN(RShift, i16, i16)))] = LAMBDA(RShift, int16);
    map[StringRef(STR(JOIN(RShift, i32, i32)))] = LAMBDA(RShift, int32);
    map[StringRef(STR(JOIN(RShift, i64, i64)))] = LAMBDA(RShift, int64);

    map[StringRef(STR(JOIN(BitOr, i8, i8)))]   = LAMBDA(BitOr, int8);
    map[StringRef(STR(JOIN(BitOr, i16, i16)))] = LAMBDA(BitOr, int16);
    map[StringRef(STR(JOIN(BitOr, i32, i32)))] = LAMBDA(BitOr, int32);
    map[StringRef(STR(JOIN(BitOr, i64, i64)))] = LAMBDA(BitOr, int64);

    map[StringRef(STR(JOIN(BitXor, i8, i8)))]   = LAMBDA(BitXor, int8);
    map[StringRef(STR(JOIN(BitXor, i16, i16)))] = LAMBDA(BitXor, int16);
    map[StringRef(STR(JOIN(BitXor, i32, i32)))] = LAMBDA(BitXor, int32);
    map[StringRef(STR(JOIN(BitXor, i64, i64)))] = LAMBDA(BitXor, int64);

    map[StringRef(STR(JOIN(BitAnd, i8, i8)))]   = LAMBDA(BitAnd, int8);
    map[StringRef(STR(JOIN(BitAnd, i16, i16)))] = LAMBDA(BitAnd, int16);
    map[StringRef(STR(JOIN(BitAnd, i32, i32)))] = LAMBDA(BitAnd, int32);
    map[StringRef(STR(JOIN(BitAnd, i64, i64)))] = LAMBDA(BitAnd, int64);

    // Unsigned
    map[StringRef(STR(JOIN(Add, u8, u8)))]   = LAMBDA(Add, uint8);
    map[StringRef(STR(JOIN(Add, u16, u16)))] = LAMBDA(Add, uint16);
    map[StringRef(STR(JOIN(Add, u32, u32)))] = LAMBDA(Add, uint32);
    map[StringRef(STR(JOIN(Add, u64, u64)))] = LAMBDA(Add, uint64);

    map[StringRef(STR(JOIN(Sub, u8, u8)))]   = LAMBDA(Sub, uint8);
    map[StringRef(STR(JOIN(Sub, u16, u16)))] = LAMBDA(Sub, uint16);
    map[StringRef(STR(JOIN(Sub, u32, u32)))] = LAMBDA(Sub, uint32);
    map[StringRef(STR(JOIN(Sub, u64, u64)))] = LAMBDA(Sub, uint64);

    map[StringRef(STR(JOIN(Mult, u8, u8)))]   = LAMBDA(Mult, uint8);
    map[StringRef(STR(JOIN(Mult, u16, u16)))] = LAMBDA(Mult, uint16);
    map[StringRef(STR(JOIN(Mult, u32, u32)))] = LAMBDA(Mult, uint32);
    map[StringRef(STR(JOIN(Mult, u64, u64)))] = LAMBDA(Mult, uint64);

    map[StringRef(STR(JOIN(Div, u8, u8)))]   = LAMBDA(Div, uint8);
    map[StringRef(STR(JOIN(Div, u16, u16)))] = LAMBDA(Div, uint16);
    map[StringRef(STR(JOIN(Div, u32, u32)))] = LAMBDA(Div, uint32);
    map[StringRef(STR(JOIN(Div, u64, u64)))] = LAMBDA(Div, uint64);

    map[StringRef(STR(JOIN(Mod, u8, u8)))]   = LAMBDA(Mod, uint8);
    map[StringRef(STR(JOIN(Mod, u16, u16)))] = LAMBDA(Mod, uint16);
    map[StringRef(STR(JOIN(Mod, u32, u32)))] = LAMBDA(Mod, uint32);
    map[StringRef(STR(JOIN(Mod, u64, u64)))] = LAMBDA(Mod, uint64);

    map[StringRef(STR(JOIN(Pow, u8, u8)))]   = LAMBDA(Pow, uint8);
    map[StringRef(STR(JOIN(Pow, u16, u16)))] = LAMBDA(Pow, uint16);
    map[StringRef(STR(JOIN(Pow, u32, u32)))] = LAMBDA(Pow, uint32);
    map[StringRef(STR(JOIN(Pow, u64, u64)))] = LAMBDA(Pow, uint64);

    map[StringRef(STR(JOIN(LShift, u8, u8)))]   = LAMBDA(LShift, uint8);
    map[StringRef(STR(JOIN(LShift, u16, u16)))] = LAMBDA(LShift, uint16);
    map[StringRef(STR(JOIN(LShift, u32, u32)))] = LAMBDA(LShift, uint32);
    map[StringRef(STR(JOIN(LShift, u64, u64)))] = LAMBDA(LShift, uint64);

    map[StringRef(STR(JOIN(RShift, u8, u8)))]   = LAMBDA(RShift, uint8);
    map[StringRef(STR(JOIN(RShift, u16, u16)))] = LAMBDA(RShift, uint16);
    map[StringRef(STR(JOIN(RShift, u32, u32)))] = LAMBDA(RShift, uint32);
    map[StringRef(STR(JOIN(RShift, u64, u64)))] = LAMBDA(RShift, uint64);

    map[StringRef(STR(JOIN(BitOr, u8, u8)))]   = LAMBDA(BitOr, uint8);
    map[StringRef(STR(JOIN(BitOr, u16, u16)))] = LAMBDA(BitOr, uint16);
    map[StringRef(STR(JOIN(BitOr, u32, u32)))] = LAMBDA(BitOr, uint32);
    map[StringRef(STR(JOIN(BitOr, u64, u64)))] = LAMBDA(BitOr, uint64);

    map[StringRef(STR(JOIN(BitXor, u8, u8)))]   = LAMBDA(BitXor, uint8);
    map[StringRef(STR(JOIN(BitXor, u16, u16)))] = LAMBDA(BitXor, uint16);
    map[StringRef(STR(JOIN(BitXor, u32, u32)))] = LAMBDA(BitXor, uint32);
    map[StringRef(STR(JOIN(BitXor, u64, u64)))] = LAMBDA(BitXor, uint64);

    map[StringRef(STR(JOIN(BitAnd, u8, u8)))]   = LAMBDA(BitAnd, uint8);
    map[StringRef(STR(JOIN(BitAnd, u16, u16)))] = LAMBDA(BitAnd, uint16);
    map[StringRef(STR(JOIN(BitAnd, u32, u32)))] = LAMBDA(BitAnd, uint32);
    map[StringRef(STR(JOIN(BitAnd, u64, u64)))] = LAMBDA(BitAnd, uint64);
    // clang-format on

    return map;
}

template <typename K, typename V>
V get(Dict<K, V> const& map, K key, V value) {
    using Iterator = typename Dict<K, V>::const_iterator;

    Iterator maybe = map.find(key);

    if (maybe == map.end()) {
        return value;
    }

    return maybe->second;
}

Dict<StringRef, BinOp::NativeBinaryOp> const& native_binary_operators() {
    static Dict<StringRef, BinOp::NativeBinaryOp> ops = build_native_binary_operators();
    return ops;
}

BinOp::NativeBinaryOp get_native_binary_operation(StringRef signature) {
    return get(native_binary_operators(), signature, BinOp::NativeBinaryOp());
}

// Bool
Dict<StringRef, BoolOp::NativeBoolyOp> build_native_bool_operators() {
    Dict<StringRef, BoolOp::NativeBoolyOp> map;

    // clang-format off
    map[StringRef(STR(JOIN(and, bool, bool)))] = LAMBDA(And, bool);
    map[StringRef(STR(JOIN(or, bool, bool)))] = LAMBDA(Or, bool);
    // clang-format on
    return map;
}

Dict<StringRef, BoolOp::NativeBoolyOp> const& native_bool_operators() {
    static auto ops = build_native_bool_operators();
    return ops;
}

BoolOp::NativeBoolyOp get_native_bool_operation(StringRef signature) {
    return get(native_bool_operators(), signature, BoolOp::NativeBoolyOp());
}

// Unary
Dict<StringRef, UnaryOp::NativeUnaryOp> build_native_unary_operators() {
    Dict<StringRef, UnaryOp::NativeUnaryOp> map;

    // clang-format off
    map[StringRef(STR(JOIN1(Invert, u8)))]  = LAMBDA(Invert, uint8);
    map[StringRef(STR(JOIN1(Invert, u16)))] = LAMBDA(Invert, uint16);
    map[StringRef(STR(JOIN1(Invert, u32)))] = LAMBDA(Invert, uint32);
    map[StringRef(STR(JOIN1(Invert, u64)))] = LAMBDA(Invert, uint64);

    map[StringRef(STR(JOIN1(Not   , u8)))]  = LAMBDA(Not   , uint8);
    map[StringRef(STR(JOIN1(Not   , u16)))] = LAMBDA(Not   , uint16);
    map[StringRef(STR(JOIN1(Not   , u32)))] = LAMBDA(Not   , uint32);
    map[StringRef(STR(JOIN1(Not   , u64)))] = LAMBDA(Not   , uint64);

    map[StringRef(STR(JOIN1(UAdd  , u8)))]  = LAMBDA(UAdd  , uint8);
    map[StringRef(STR(JOIN1(UAdd  , u16)))] = LAMBDA(UAdd  , uint16);
    map[StringRef(STR(JOIN1(UAdd  , u32)))] = LAMBDA(UAdd  , uint32);
    map[StringRef(STR(JOIN1(UAdd  , u64)))] = LAMBDA(UAdd  , uint64);

    map[StringRef(STR(JOIN1(USub  , u8)))]  = LAMBDA(USub  , uint8);
    map[StringRef(STR(JOIN1(USub  , u16)))] = LAMBDA(USub  , uint16);
    map[StringRef(STR(JOIN1(USub  , u32)))] = LAMBDA(USub  , uint32);
    map[StringRef(STR(JOIN1(USub  , u64)))] = LAMBDA(USub  , uint64);

    map[StringRef(STR(JOIN1(Invert, i8)))]  = LAMBDA(Invert, int8);
    map[StringRef(STR(JOIN1(Invert, i16)))] = LAMBDA(Invert, int16);
    map[StringRef(STR(JOIN1(Invert, i32)))] = LAMBDA(Invert, int32);
    map[StringRef(STR(JOIN1(Invert, i64)))] = LAMBDA(Invert, int64);

    map[StringRef(STR(JOIN1(Not   , i8)))]  = LAMBDA(Not   , int8);
    map[StringRef(STR(JOIN1(Not   , i16)))] = LAMBDA(Not   , int16);
    map[StringRef(STR(JOIN1(Not   , i32)))] = LAMBDA(Not   , int32);
    map[StringRef(STR(JOIN1(Not   , i64)))] = LAMBDA(Not   , int64);

    map[StringRef(STR(JOIN1(UAdd  , i8)))]  = LAMBDA(UAdd  , int8);
    map[StringRef(STR(JOIN1(UAdd  , i16)))] = LAMBDA(UAdd  , int16);
    map[StringRef(STR(JOIN1(UAdd  , i32)))] = LAMBDA(UAdd  , int32);
    map[StringRef(STR(JOIN1(UAdd  , i64)))] = LAMBDA(UAdd  , int64);

    map[StringRef(STR(JOIN1(USub  , i8)))]  = LAMBDA(USub  , int8);
    map[StringRef(STR(JOIN1(USub  , i16)))] = LAMBDA(USub  , int16);
    map[StringRef(STR(JOIN1(USub  , i32)))] = LAMBDA(USub  , int32);
    map[StringRef(STR(JOIN1(USub  , i64)))] = LAMBDA(USub  , int64);

    // Floats
    map[StringRef(STR(JOIN1(UAdd  , f32)))] = LAMBDA(UAdd  , float32);
    map[StringRef(STR(JOIN1(USub  , f32)))] = LAMBDA(USub  , float32);
    map[StringRef(STR(JOIN1(UAdd  , f64)))] = LAMBDA(UAdd  , float64);
    map[StringRef(STR(JOIN1(USub  , f64)))] = LAMBDA(USub  , float64);

    // clang-format on

    return map;
}

Dict<StringRef, UnaryOp::NativeUnaryOp> const& native_unary_operators() {
    static auto ops = build_native_unary_operators();
    return ops;
}

UnaryOp::NativeUnaryOp get_native_unary_operation(StringRef signature) {
    return get(native_unary_operators(), signature, UnaryOp::NativeUnaryOp());
}

// Cmp
Dict<StringRef, Compare::NativeCompOp> build_native_cmp_operators() {
    Dict<StringRef, Compare::NativeCompOp> map;

    // clang-format off
    map[StringRef(STR(JOIN(Eq   , u8, u8)))] = LAMBDA(Eq   , uint8);
    map[StringRef(STR(JOIN(NotEq, u8, u8)))] = LAMBDA(NotEq, uint8);
    map[StringRef(STR(JOIN(Lt   , u8, u8)))] = LAMBDA(Lt   , uint8);
    map[StringRef(STR(JOIN(LtE  , u8, u8)))] = LAMBDA(LtE  , uint8);
    map[StringRef(STR(JOIN(Gt   , u8, u8)))] = LAMBDA(Gt   , uint8);
    map[StringRef(STR(JOIN(GtE  , u8, u8)))] = LAMBDA(GtE  , uint8);
    map[StringRef(STR(JOIN(Is   , u8, u8)))] = LAMBDA(Is   , uint8);
    map[StringRef(STR(JOIN(IsNot, u8, u8)))] = LAMBDA(IsNot, uint8);

    map[StringRef(STR(JOIN(Eq   , u16, u16)))] = LAMBDA(Eq   , uint16);
    map[StringRef(STR(JOIN(NotEq, u16, u16)))] = LAMBDA(NotEq, uint16);
    map[StringRef(STR(JOIN(Lt   , u16, u16)))] = LAMBDA(Lt   , uint16);
    map[StringRef(STR(JOIN(LtE  , u16, u16)))] = LAMBDA(LtE  , uint16);
    map[StringRef(STR(JOIN(Gt   , u16, u16)))] = LAMBDA(Gt   , uint16);
    map[StringRef(STR(JOIN(GtE  , u16, u16)))] = LAMBDA(GtE  , uint16);
    map[StringRef(STR(JOIN(Is   , u16, u16)))] = LAMBDA(Is   , uint16);
    map[StringRef(STR(JOIN(IsNot, u16, u16)))] = LAMBDA(IsNot, uint16);

    map[StringRef(STR(JOIN(Eq   , u32, u32)))] = LAMBDA(Eq   , uint32);
    map[StringRef(STR(JOIN(NotEq, u32, u32)))] = LAMBDA(NotEq, uint32);
    map[StringRef(STR(JOIN(Lt   , u32, u32)))] = LAMBDA(Lt   , uint32);
    map[StringRef(STR(JOIN(LtE  , u32, u32)))] = LAMBDA(LtE  , uint32);
    map[StringRef(STR(JOIN(Gt   , u32, u32)))] = LAMBDA(Gt   , uint32);
    map[StringRef(STR(JOIN(GtE  , u32, u32)))] = LAMBDA(GtE  , uint32);
    map[StringRef(STR(JOIN(Is   , u32, u32)))] = LAMBDA(Is   , uint32);
    map[StringRef(STR(JOIN(IsNot, u32, u32)))] = LAMBDA(IsNot, uint32);

    map[StringRef(STR(JOIN(Eq   , u64, u64)))] = LAMBDA(Eq   , uint64);
    map[StringRef(STR(JOIN(NotEq, u64, u64)))] = LAMBDA(NotEq, uint64);
    map[StringRef(STR(JOIN(Lt   , u64, u64)))] = LAMBDA(Lt   , uint64);
    map[StringRef(STR(JOIN(LtE  , u64, u64)))] = LAMBDA(LtE  , uint64);
    map[StringRef(STR(JOIN(Gt   , u64, u64)))] = LAMBDA(Gt   , uint64);
    map[StringRef(STR(JOIN(GtE  , u64, u64)))] = LAMBDA(GtE  , uint64);
    map[StringRef(STR(JOIN(Is   , u64, u64)))] = LAMBDA(Is   , uint64);
    map[StringRef(STR(JOIN(IsNot, u64, u64)))] = LAMBDA(IsNot, uint64);

    map[StringRef(STR(JOIN(Eq   , i8, i8)))] = LAMBDA(Eq   , int8);
    map[StringRef(STR(JOIN(NotEq, i8, i8)))] = LAMBDA(NotEq, int8);
    map[StringRef(STR(JOIN(Lt   , i8, i8)))] = LAMBDA(Lt   , int8);
    map[StringRef(STR(JOIN(LtE  , i8, i8)))] = LAMBDA(LtE  , int8);
    map[StringRef(STR(JOIN(Gt   , i8, i8)))] = LAMBDA(Gt   , int8);
    map[StringRef(STR(JOIN(GtE  , i8, i8)))] = LAMBDA(GtE  , int8);
    map[StringRef(STR(JOIN(Is   , i8, i8)))] = LAMBDA(Is   , int8);
    map[StringRef(STR(JOIN(IsNot, i8, i8)))] = LAMBDA(IsNot, int8);

    map[StringRef(STR(JOIN(Eq   , i16, i16)))] = LAMBDA(Eq   , int16);
    map[StringRef(STR(JOIN(NotEq, i16, i16)))] = LAMBDA(NotEq, int16);
    map[StringRef(STR(JOIN(Lt   , i16, i16)))] = LAMBDA(Lt   , int16);
    map[StringRef(STR(JOIN(LtE  , i16, i16)))] = LAMBDA(LtE  , int16);
    map[StringRef(STR(JOIN(Gt   , i16, i16)))] = LAMBDA(Gt   , int16);
    map[StringRef(STR(JOIN(GtE  , i16, i16)))] = LAMBDA(GtE  , int16);
    map[StringRef(STR(JOIN(Is   , i16, i16)))] = LAMBDA(Is   , int16);
    map[StringRef(STR(JOIN(IsNot, i16, i16)))] = LAMBDA(IsNot, int16);

    map[StringRef(STR(JOIN(Eq   , i32, i32)))] = LAMBDA(Eq   , int32);
    map[StringRef(STR(JOIN(NotEq, i32, i32)))] = LAMBDA(NotEq, int32);
    map[StringRef(STR(JOIN(Lt   , i32, i32)))] = LAMBDA(Lt   , int32);
    map[StringRef(STR(JOIN(LtE  , i32, i32)))] = LAMBDA(LtE  , int32);
    map[StringRef(STR(JOIN(Gt   , i32, i32)))] = LAMBDA(Gt   , int32);
    map[StringRef(STR(JOIN(GtE  , i32, i32)))] = LAMBDA(GtE  , int32);
    map[StringRef(STR(JOIN(Is   , i32, i32)))] = LAMBDA(Is   , int32);
    map[StringRef(STR(JOIN(IsNot, i32, i32)))] = LAMBDA(IsNot, int32);

    map[StringRef(STR(JOIN(Eq   , i64, i64)))] = LAMBDA(Eq   , int64);
    map[StringRef(STR(JOIN(NotEq, i64, i64)))] = LAMBDA(NotEq, int64);
    map[StringRef(STR(JOIN(Lt   , i64, i64)))] = LAMBDA(Lt   , int64);
    map[StringRef(STR(JOIN(LtE  , i64, i64)))] = LAMBDA(LtE  , int64);
    map[StringRef(STR(JOIN(Gt   , i64, i64)))] = LAMBDA(Gt   , int64);
    map[StringRef(STR(JOIN(GtE  , i64, i64)))] = LAMBDA(GtE  , int64);
    map[StringRef(STR(JOIN(Is   , i64, i64)))] = LAMBDA(Is   , int64);
    map[StringRef(STR(JOIN(IsNot, i64, i64)))] = LAMBDA(IsNot, int64);

    map[StringRef(STR(JOIN(Eq   , f32, f32)))] = LAMBDA(Eq   , float32);
    map[StringRef(STR(JOIN(NotEq, f32, f32)))] = LAMBDA(NotEq, float32);
    map[StringRef(STR(JOIN(Lt   , f32, f32)))] = LAMBDA(Lt   , float32);
    map[StringRef(STR(JOIN(LtE  , f32, f32)))] = LAMBDA(LtE  , float32);
    map[StringRef(STR(JOIN(Gt   , f32, f32)))] = LAMBDA(Gt   , float32);
    map[StringRef(STR(JOIN(GtE  , f32, f32)))] = LAMBDA(GtE  , float32);
    map[StringRef(STR(JOIN(Is   , f32, f32)))] = LAMBDA(Is   , float32);
    map[StringRef(STR(JOIN(IsNot, f32, f32)))] = LAMBDA(IsNot, float32);

    map[StringRef(STR(JOIN(Eq   , f64, f64)))] = LAMBDA(Eq   , float64);
    map[StringRef(STR(JOIN(NotEq, f64, f64)))] = LAMBDA(NotEq, float64);
    map[StringRef(STR(JOIN(Lt   , f64, f64)))] = LAMBDA(Lt   , float64);
    map[StringRef(STR(JOIN(LtE  , f64, f64)))] = LAMBDA(LtE  , float64);
    map[StringRef(STR(JOIN(Gt   , f64, i64)))] = LAMBDA(Gt   , float64);
    map[StringRef(STR(JOIN(GtE  , f64, f64)))] = LAMBDA(GtE  , float64);
    map[StringRef(STR(JOIN(Is   , f64, f64)))] = LAMBDA(Is   , float64);
    map[StringRef(STR(JOIN(IsNot, f64, f64)))] = LAMBDA(IsNot, float64);

    // clang-format on
    return map;
}

Dict<StringRef, Compare::NativeCompOp> const& native_cmp_operators() {
    static auto ops = build_native_cmp_operators();
    return ops;
}

Compare::NativeCompOp get_native_cmp_operation(StringRef signature) {
    return get(native_cmp_operators(), signature, Compare::NativeCompOp());
}

}  // namespace lython