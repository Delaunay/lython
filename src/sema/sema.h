#ifndef LYTHON_SEMA_HEADER
#define LYTHON_SEMA_HEADER

#include "ast/ops.h"
#include "ast/visitor.h"
#include "sema/bindings.h"
#include "sema/builtin.h"
#include "sema/errors.h"
#include "sema/importlib.h"
#include "utilities/printing.h"
#include "utilities/strings.h"

// #define SEMA_ERROR(exception)      \
//     kwerror("{}", exception.what()); \
//     errors.push_back(std::unique_ptr<SemaException>(new exception));

namespace lython {

struct SemaVisitorTrait {
    using StmtRet = TypeExpr*;
    using ExprRet = TypeExpr*;
    using ModRet  = TypeExpr*;
    using PatRet  = TypeExpr*;
    using IsConst = std::false_type;
    using Trace   = std::true_type;

    enum
    { MaxRecursionDepth = LY_MAX_VISITOR_RECURSION_DEPTH };
};

struct SemaContext {
    bool yield = false;
    bool arrow = false;
};

/* The semantic analysis (SEM-A) happens after the parsing, the AST can be assumed to be
 * syntactically correct its job is to detect issues that could prevent a succesful compilation.
 *
 * Errors caught in that process are undeclared variables and mistypings,
 * this includes missing attributes, missing methods
 *
 * In addition, our SEM-A will deduce types (i.e variables inherit the type of the expressions,
 * this is NOT type inference) and allocate a register to each variables.
 *
 * To support type deduction SEM-A returns the type of the analysed expression,
 * the deduction can then be used for typechecking.
 *
 * Type deduction is a weaker form of type inference where the type of the parent parent
 * expression is deduced from the children. In the future we might add full type inference. Type
 * deduction will still be useful then as it will reduce the cost of type inference for the
 * trivial cases.
 *
 * Type deduction alone should provide a satisfactory development experience, as the user should
 * only have to specify the type of the arguments which is good practice anyway as it serves as
 * documentation.
 *
 * Notes
 * -----
 *
 * SEM-A does a quick first pass through the module to insert definitions
 * that are used before their definitions. This allow us to get away
 * with not forward-declaring everything, but it means some analytics
 * will get delayed until the end. In the case of mutually recursive definitions
 * forward declaration is required so typing can be checked.
 *
 * SEM-A will add type annotation & reorder arguments wherever it can.
 * This has the goal and standardizing the code & simplifying its execution
 * later on.
 *
 * You can inspect the change by saving the resulting AST.
 * You could implement an automatic formatter that executes semantic analysis
 * to format & complete the code. The completed code will then take less time
 * to compile as the representation will be easier to analyse.
 *
 * Raises
 * ------
 *
 * TypeError
 *      Raised when types between expression mismatch
 *
 * UnsupportedOperand
 *      Raised when using an operand on an object that does not support it
 *
 * AttributeError
 *      Raised when using an object attribute that does not exist
 *
 * NameError
 *      Raised when using an undefined variable
 *
 * ModuleNotFoundError
 *      Raised when importing a module that was not found
 *
 * ImportError
 *      Raised when importing a statement that was not found from a module
 *
 */
struct SemanticAnalyser: public BaseVisitor<SemanticAnalyser, false, SemaVisitorTrait> {
    Bindings bindings;  // This should be outside of sema so it can live on after sema
    bool     forwardpass = false;
    Array<std::unique_ptr<SemaException>> errors;
    Array<StmtNode*>                      nested;
    Array<String>                         namespaces;
    Dict<StringRef, bool>                 flags;
    ImportLib*                            importsys = nullptr;
    Array<Exported*>                      exported_stack;
    bool                                  eager        = false;
    ExprContext                           expr_context = ExprContext::Load;

    Logger& semalog = lython::outlog();

    // Should I remove the types for the runtime info
    // the type can have their own query struct
    // which might or might not be included in the final binary
    SemanticAnalyser(ImportLib* import = ImportLib::instance()): importsys(import) {}

    // maybe conbine the semacontext with samespace
    Array<SemaContext> semactx;

    bool has_errors() const;

    BindingEntry const* lookup(Name_t* n);

    SemaContext& get_context() {
        static SemaContext global_ctx;
        if (semactx.size() == 0) {
            return global_ctx;
        }
        return semactx[semactx.size() - 1];
    }

    template <typename... Args>
    TypeExpr* exec_with_ctx(ExprContext ctx, Args... args) {
        ExprContext old = expr_context;
        expr_context    = ctx;
        TypeExpr* r     = exec(args...);
        expr_context    = old;
        return r;
    }

    void show_diagnostic(std::ostream& out, class AbstractLexer* lexer = nullptr);

    bool is_type(TypeExpr* node, int depth, lython::CodeLocation const& loc);

    bool reorder_arguments(Call* call, FunctionDef* def);

    template <typename T, typename... Args>
    void sema_error(Node* node, lython::CodeLocation const& loc, Args... args) {
        errors.push_back(std::unique_ptr<SemaException>(new T(args...)));
        SemaException* exception = errors[errors.size() - 1].get();

        // Populate location info
        exception->set_node(node);

        // use the LOC from parent function
        lython::outlog().log(lython::LogLevel::Error, loc, "{}", exception->what());
    }

#define SEMA_ERROR(expr, exception, ...) sema_error<exception>(expr, LOC, __VA_ARGS__)

    public:
    virtual ~SemanticAnalyser() {}

    StmtNode* current_namespace() {
        if (nested.size() > 0) {
            return nested[nested.size() - 1];
        }
        return nullptr;
    }

    Tuple<ClassDef*, FunctionDef*>
    find_method(TypeExpr* class_type, String const& methodname, int depth);

    bool typecheck(
        ExprNode* lhs, TypeExpr* lhs_t, ExprNode* rhs, TypeExpr* rhs_t, CodeLocation const& loc);

    bool add_name(StringRef name, ExprNode* value, ExprNode* type);
    bool add_name(ExprNode* expr, ExprNode* value, ExprNode* type);

    String operator_function(TypeExpr* expr_t, StringRef op);

    Arrow* functiondef_arrow(FunctionDef* n, StmtNode* class_t, int depth);
    void   record_ctor_attributes(ClassDef* n, FunctionDef* ctor, int depth);

    String generate_function_name(FunctionDef* n);

    Arrow* get_arrow(ExprNode* fun, ExprNode* type, int depth, int& offset, ClassDef*& cls);

    Arrow* build_constructor_type(ExprNode* fun, ClassDef* cls, int depth);

    TypeExpr* oneof(Array<TypeExpr*> types) {
        if (types.size() > 0) {
            return types[0];
        }
        return nullptr;
    }

    Node* load_name(Name_t* variable);

    Array<TypeExpr*> exec_body(Array<StmtNode*>& body, int depth);

    Name* make_ref(Node* parent, StringRef const& name, ExprNode* type = nullptr);
    Name* make_ref(Node* parent, String const& name, ExprNode* type = nullptr);

    void record_attributes(ClassDef*               n,
                           Array<StmtNode*> const& body,
                           Array<StmtNode*>&       methods,
                           FunctionDef**           ctor,
                           int                     depth);

    ClassDef* get_class(ExprNode* classref, int depth);
    TypeExpr* resolve_variable(ExprNode* node);

    TypeExpr* attribute_assign(Attribute* n, int depth, TypeExpr* expected);

    void add_arguments(Arguments& args, Arrow*, ClassDef* def, int);

#define FUNCTION_GEN(name, fun) virtual TypeExpr* fun(name* n, int depth);

#define X(name, _)
#define SSECTION(name)
#define MOD(name, fun)   FUNCTION_GEN(name, fun)
#define EXPR(name, fun)  FUNCTION_GEN(name, fun)
#define STMT(name, fun)  FUNCTION_GEN(name, fun)
#define MATCH(name, fun) FUNCTION_GEN(name, fun)
#define VM(name, fun)

    NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH, VM)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH
#undef VM

#undef FUNCTION_GEN
};

}  // namespace lython

#endif