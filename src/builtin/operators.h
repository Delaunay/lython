#pragma once

#include "ast/nodes.h"

namespace lython {

// Binary
Dict<StringRef, BinOp::NativeBinaryOp> build_native_binary_operators();

Dict<StringRef, BinOp::NativeBinaryOp> const& native_binary_operators();

BinOp::NativeBinaryOp get_native_binary_operation(StringRef signature);

// Bool
Dict<StringRef, BoolOp::NativeBoolyOp> build_native_bool_operators();

Dict<StringRef, BoolOp::NativeBoolyOp> const& native_bool_operators();

BoolOp::NativeBoolyOp get_native_bool_operation(StringRef signature);

// Unary
Dict<StringRef, UnaryOp::NativeUnaryOp> build_native_unary_operators();

Dict<StringRef, UnaryOp::NativeUnaryOp> const& native_unary_operators();

UnaryOp::NativeUnaryOp get_native_unary_operation(StringRef signature);

// Cmp
Dict<StringRef, Compare::NativeCompOp> build_native_cmp_operators();

Dict<StringRef, Compare::NativeCompOp> const& native_cmp_operators();

Compare::NativeCompOp get_native_cmp_operation(StringRef signature);

}  // namespace lython