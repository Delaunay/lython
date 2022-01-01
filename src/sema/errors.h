#ifndef LYTHON_SEMA_ERROR_HEADER
#define LYTHON_SEMA_ERROR_HEADER

#include "ast/nodes.h"
#include "sema/builtin.h"

namespace lython {

struct SemaException: LythonException {};

struct AttributeError: public SemaException {};

struct NameError: public SemaException {
    NameError(Node *code, StringRef name, CodeLocation const &loc):
        code(code), name(name), loc(loc) {}

    Node *       code;
    StringRef    name;
    CodeLocation loc;
};

struct TypeError: public SemaException {
    TypeError(std::string const &msg, CodeLocation const &loc): cached_message(msg), loc(loc) {}

    TypeError(ExprNode *lhs, TypeExpr *lhs_t, ExprNode *rhs, TypeExpr *rhs_t,
              CodeLocation const &loc):
        lhs_v(lhs),
        lhs_t(lhs_t), rhs_v(rhs), rhs_t(rhs_t), loc(loc) {}

    virtual const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW {
        message();
        return cached_message.c_str();
    }

    std::string const &message() const;

    // Source code info
    ExprNode *lhs_v = nullptr;
    TypeExpr *lhs_t = nullptr;
    ExprNode *rhs_v = nullptr;
    TypeExpr *rhs_t = nullptr;

    // Compiler Debug location
    CodeLocation loc;

    mutable std::string cached_message;
};

} // namespace lython

#endif
