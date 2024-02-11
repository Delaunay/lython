#ifndef LYTHON_SEMA_BUILTIN_HEADER
#define LYTHON_SEMA_BUILTIN_HEADER

#include "ast/nodes.h"

namespace lython {

using TypeExpr = ExprNode;

ExprNode* False();
ExprNode* True();
ExprNode* None();

struct undefined_Type;
struct undefined_None;
struct undefined_Module;

#define BUILTIN_TYPES(TYPE)         \
    TYPE(Type, undefined_Type)      \
    TYPE(None, undefined_None)      \
    TYPE(i8, int8)                  \
    TYPE(i16, int16)                \
    TYPE(i32, int32)                \
    TYPE(i64, int64)                \
    TYPE(f32, float)                \
    TYPE(f64, float64)              \
    TYPE(u8, uint8)                 \
    TYPE(u16, uint16)               \
    TYPE(u32, uint32)               \
    TYPE(u64, uint64)               \
    TYPE(str, String)               \
    TYPE(bool, bool)                \
    TYPE(Module, undefined_Module)

#define TYPE(name, _) TypeExpr* name##_t();

BUILTIN_TYPES(TYPE)

#undef TYPE

}  // namespace lython

#endif