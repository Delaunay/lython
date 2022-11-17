#ifndef LYTHON_SEMA_ERROR_HEADER
#define LYTHON_SEMA_ERROR_HEADER

#include <ostream>

#include "ast/nodes.h"
#include "printer/error_printer.h"
#include "sema/builtin.h"

namespace lython {

struct SemaError {};

StmtNode* get_parent_stmt(Node* node);

struct SemaException: public LythonException {
    ExprNode* expr = nullptr;
    StmtNode* stmt = nullptr;

    SemaException(ExprNode* expr): expr(expr), stmt(get_parent_stmt(expr)), cached_message("") {}

    SemaException(StmtNode* stmt, std::string const& msg): stmt(stmt), cached_message(msg) {}

    SemaException(std::string const& msg): cached_message(msg) {}

    SemaException(): cached_message("") {}

    virtual const char* what() const LY_NOEXCEPT override final {
        generate_message();
        return cached_message.c_str();
    }

    void generate_message() const {
        if (cached_message.size() > 0) {
            return;
        }

        cached_message = message();
    }

    void set_node(Node* node) {
        if (node && node->family() == NodeFamily::Expression) {
            set_expr((ExprNode*)node);
        }

        if (node && node->family() == NodeFamily::Statement) {
            set_stmt((StmtNode*)node);
        }
    }

    void set_expr(ExprNode* expression) {
        expr = expression;
        stmt = get_parent_stmt(expr);
    }

    void set_stmt(StmtNode* statement) { stmt = statement; }

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
struct AttributeError: public SemaException {
    AttributeError(ClassDef* obj, StringRef attr):
        obj(obj), attr(attr)  //
    {}

    std::string message() const override;

    static std::string message(String const& name, String const& attr);

    ClassDef* obj;
    StringRef attr;
};

/*
 * Examples
 * --------
 * >>> x
 * Traceback (most recent call last):
 *   File "<stdin>", line 1, in <module>
 * NameError: name 'x' is not defined
 */
struct NameError: public SemaException {
    NameError(Node* code, StringRef name): code(code), name(name) {}

    std::string message() const override;

    Node*     code;
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
    TypeError(std::string const& msg): SemaException(msg) {}

    TypeError(
        ExprNode* lhs, TypeExpr* lhs_t, ExprNode* rhs, TypeExpr* rhs_t, CodeLocation const& loc):
        lhs_v(lhs),
        lhs_t(lhs_t), rhs_v(rhs), rhs_t(rhs_t) {}

    std::string message() const override;

    static std::string
    message(String const& lhs_v, String const& lhs_t, String const& rhs_v, String const& rhs_t);

    // Source code info
    ExprNode* lhs_v = nullptr;
    TypeExpr* lhs_t = nullptr;
    ExprNode* rhs_v = nullptr;
    TypeExpr* rhs_t = nullptr;
};

struct UnsupportedOperand: public SemaException {
    UnsupportedOperand(String const& str, TypeExpr* lhs_t, TypeExpr* rhs_t):
        operand(str), lhs_t(lhs_t), rhs_t(rhs_t) {}

    std::string message() const override;

    static std::string message(String const& op, String const& lhs_t, String const& rhs_t);

    String    operand;
    TypeExpr* lhs_t = nullptr;
    TypeExpr* rhs_t = nullptr;
};

struct RecursiveDefinition: public SemaException {
    RecursiveDefinition(String const& str, ExprNode* fun, ClassDef* cls):
        msg(str), fun(fun), cls(cls) {}

    std::string message() const override;

    static std::string message(ExprNode const* lhs_t, ClassDef const* rhs_t);

    String    msg;
    ExprNode* fun = nullptr;
    ClassDef* cls = nullptr;
};

struct ModuleNotFoundError: public SemaException {
    ModuleNotFoundError(StringRef const& mod): module(mod) {}

    std::string message() const override;

    static std::string message(String const& module);

    StringRef module;
};

struct ImportError: public SemaException {
    ImportError(StringRef const& mod, StringRef const& name): module(mod), name(name) {}

    std::string message() const override;

    static std::string message(String const& module, String const& name);

    StringRef module;
    StringRef name;
};

struct SemaErrorPrinter: public BaseErrorPrinter {
    SemaErrorPrinter(std::ostream& out, class AbstractLexer* lexer = nullptr):
        BaseErrorPrinter(out, lexer)  //
    {}

    void print(SemaException const& err);
};

}  // namespace lython

#endif
