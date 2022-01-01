#ifndef LYTHON_SEMA_ERROR_HEADER
#define LYTHON_SEMA_ERROR_HEADER

#include "ast/nodes.h"
#include "sema/builtin.h"

namespace lython {

struct SemaException: LythonException {
    SemaException(std::string const &msg): cached_message(msg) {}

    SemaException(): cached_message("") {}

    virtual const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override final {
        generate_message();
        return cached_message.c_str();
    }

    void generate_message() const {
        if (cached_message.size() > 0) {
            return;
        }

        cached_message = message();
    }

    virtual std::string message() const = 0;

    mutable std::string cached_message;
};

/*
 *
 * Examples
 * --------
 * >>> class Name:
 * ...     x = 1
 * ...
 * >>> a = Name()
 * >>> a.n
 * Traceback (most recent call last):
 *   File "<stdin>", line 1, in <module>
 * AttributeError: 'Name' object has no attribute 'n'
 */
struct AttributeError: public SemaException {};

/*
 * Examples
 * --------
 * >>> x
 * Traceback (most recent call last):
 *   File "<stdin>", line 1, in <module>
 * NameError: name 'x' is not defined
 */
struct NameError: public SemaException {
    NameError(Node *code, StringRef name): code(code), name(name) {}

    std::string message() const override;

    Node *    code;
    StringRef name;
};

/*
 * Examples
 * --------
 * >>> x = 1
 * >>> x(1)
 * Traceback (most recent call last):
 *  File "<stdin>", line 1, in <module>
 * TypeError: 'int' object is not callable
 *
 */
struct TypeError: public SemaException {
    TypeError(std::string const &msg): SemaException(msg) {}

    TypeError(ExprNode *lhs, TypeExpr *lhs_t, ExprNode *rhs, TypeExpr *rhs_t,
              CodeLocation const &loc):
        lhs_v(lhs),
        lhs_t(lhs_t), rhs_v(rhs), rhs_t(rhs_t) {}

    std::string message() const override;

    static std::string message(String const &lhs_v, String const &lhs_t, String const &rhs_v,
                               String const &rhs_t);

    // Source code info
    ExprNode *lhs_v = nullptr;
    TypeExpr *lhs_t = nullptr;
    ExprNode *rhs_v = nullptr;
    TypeExpr *rhs_t = nullptr;
};

} // namespace lython

#endif
