#ifndef LYTHON_SEMA_BUILTIN_HEADER
#define LYTHON_SEMA_BUILTIN_HEADER

#include "ast/nodes.h"

namespace lython {

using TypeExpr = ExprNode;

ExprNode* False();
ExprNode* True();
ExprNode* None();

#define BUILTIN_TYPES(TYPE) \
    TYPE(Type)              \
    TYPE(None)              \
    TYPE(i8)                \
    TYPE(i16)               \
    TYPE(i32)               \
    TYPE(i64)               \
    TYPE(f32)               \
    TYPE(f64)               \
    TYPE(u8)                \
    TYPE(u16)               \
    TYPE(u32)               \
    TYPE(u64)               \
    TYPE(str)               \
    TYPE(bool)              \
    TYPE(Module)

#define TYPE(name) TypeExpr* name##_t();

BUILTIN_TYPES(TYPE)

#undef TYPE

}  // namespace lython

#endif