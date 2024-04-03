#pragma once

#include "ast/nodes.h"

namespace lython {

// Binary
Dict<StringRef, Function> build_native_binary_operators();

Dict<StringRef, Function> const& native_binary_operators();

Function get_native_binary_operation(StringRef signature);

// Bool
Dict<StringRef, Function> build_native_bool_operators();

Dict<StringRef, Function> const& native_bool_operators();

Function get_native_bool_operation(StringRef signature);

// Unary
Dict<StringRef, Function> build_native_unary_operators();

Dict<StringRef, Function> const& native_unary_operators();

Function get_native_unary_operation(StringRef signature);

// Cmp
Dict<StringRef, Function> build_native_cmp_operators();

Dict<StringRef, Function> const& native_cmp_operators();

Function get_native_cmp_operation(StringRef signature);

}  // namespace lython