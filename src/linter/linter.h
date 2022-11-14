#ifndef LYTHON_LINTER_HEADER
#define LYTHON_LINTER_HEADER

#include "ast/magic.h"
#include "ast/ops.h"
#include "ast/visitor.h"
#include "sema/bindings.h"
#include "sema/builtin.h"
#include "sema/errors.h"
#include "utilities/strings.h"

#define LINTER_ERROR(exception)    \
    error("{}", exception.what()); \
    errors.push_back(std::unique_ptr<SemaException>(new exception));

namespace lython {

Array<String> python_paths();

struct LinterVisitorTrait {
    using StmtRet = void;
    using ExprRet = void;
    using ModRet  = void;
    using PatRet  = void;
    using IsConst = std::false_type;
    using Trace   = std::true_type;

    enum
    { MaxRecursionDepth = LY_MAX_VISITOR_RECURSION_DEPTH };
};

struct LinterContext {
    bool yield = false;
};

struct LinterException: LythonException {
    LinterException(std::string const& msg): cached_message(msg) {}

    LinterException(): cached_message("") {}

    virtual const char* what() const NOTHROW override final {
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

/* Linter Analyzer
 */
struct LinterAnalyser: BaseVisitor<LinterAnalyser, false, LinterVisitorTrait> {
    public:
    virtual ~LinterAnalyser() {}

    Array<std::unique_ptr<SemaException>> errors;

    Node* load_name(Name_t* variable);

    template <typename T, typename... Args>
    void linter_error(Node* Node, lython::CodeLocation const& loc, Args... args) {
        errors.push_back(std::unique_ptr<LinterException>(new T(std::forward(args)...)));

        LinterException* exception = errors[errors.size() - 1];
        // use the LOC from parent function
        lython::log(lython::LogLevel::Error, loc, "{}", exception->what());
    }

#define LINTER_ERROR(expr, exception, ...) linter_error(expr, exception, LOC, __VA_ARGS__)

#define FUNCTION_GEN(name, fun) virtual TypeExpr* fun(name* n, int depth);

#define X(name, _)
#define SSECTION(name)
#define MOD(name, fun)   FUNCTION_GEN(name, fun)
#define EXPR(name, fun)  FUNCTION_GEN(name, fun)
#define STMT(name, fun)  FUNCTION_GEN(name, fun)
#define MATCH(name, fun) FUNCTION_GEN(name, fun)

    NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef FUNCTION_GEN
};

}  // namespace lython

#endif