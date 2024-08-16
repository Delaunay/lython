#ifndef LYTHON_NODEKIND_HEADER
#define LYTHON_NODEKIND_HEADER

#include <cstdint>

namespace lython {

// To make this more generic, I could have a StringDB that assign a integer to a constant string
// the string would be the class name and the integer would become the RTTI
// Custom RTTI
enum class NodeKind : int8_t
{

// clang-format off
// Check X-MACRO trick
// this is used to code gen a bunch of functions/types
#define NODEKIND_ENUM(X, SECTION, EXPR, STMT, MOD, MATCH, VM)\
    X(Invalid, invalid)             \
    SECTION(EXPR_START)             \
    EXPR(Exported, exported)        \
    EXPR(BoolOp, boolop)            \
    EXPR(NamedExpr, namedexpr)      \
    EXPR(BinOp, binop)              \
    EXPR(UnaryOp, unaryop)          \
    EXPR(Lambda, lambda)            \
    EXPR(IfExp, ifexp)              \
    EXPR(DictExpr, dictexpr)        \
    EXPR(SetExpr, setexpr)          \
    EXPR(ListComp, listcomp)        \
    EXPR(GeneratorExp, generateexpr)\
    EXPR(SetComp, setcomp)          \
    EXPR(DictComp, dictcomp)        \
    EXPR(Await, await)              \
    EXPR(Yield, yield)              \
    EXPR(YieldFrom, yieldfrom)      \
    EXPR(Compare, compare)          \
    EXPR(Call, call)                \
    EXPR(JoinedStr, joinedstr)      \
    EXPR(FormattedValue, formattedvalue)\
    EXPR(Constant, constant)        \
    EXPR(Placeholder, placeholder)  \
    EXPR(Attribute, attribute)      \
    EXPR(Subscript, subscript)      \
    EXPR(Starred, starred)          \
    EXPR(Name, name)                \
    EXPR(ListExpr, listexpr)        \
    EXPR(TupleExpr, tupleexpr)      \
    EXPR(Slice, slice)              \
    EXPR(DictType, dicttype)        \
    EXPR(ArrayType, arraytype)      \
    EXPR(TupleType, tupletype)      \
    EXPR(Arrow, arrow)              \
    EXPR(ClassType, classtype)      \
    EXPR(SetType, settype)          \
    EXPR(BuiltinType, builtintype)  \
    EXPR(Comment, comment)          \
    SECTION(EXPR_END)               \
    SECTION(MODULE_START)               \
    MOD(Module, module)                 \
    MOD(Interactive, interactive)       \
    MOD(Expression, expression)         \
    MOD(FunctionType, functiontype)     \
    SECTION(MODULE_END)                 \
    SECTION(STMT_START)                 \
    STMT(InvalidStatement, invalidstmt) \
    STMT(FunctionDef, functiondef)      \
    STMT(ClassDef, classdef)            \
    STMT(Return, returnstmt)            \
    STMT(Delete, deletestmt)            \
    STMT(Assign, assign)                \
    STMT(AugAssign, augassign)          \
    STMT(AnnAssign, annassign)          \
    STMT(For, forstmt)                  \
    STMT(While, whilestmt)              \
    STMT(If, ifstmt)                    \
    STMT(With, with)                    \
    STMT(Raise, raise)                  \
    STMT(Try, trystmt)                  \
    STMT(Assert, assertstmt)            \
    STMT(Import, import)                \
    STMT(ImportFrom, importfrom)        \
    STMT(Global, global)                \
    STMT(Nonlocal, nonlocal)            \
    STMT(Expr, exprstmt)                \
    STMT(Pass, pass)                    \
    STMT(Break, breakstmt)              \
    STMT(Continue, continuestmt)        \
    STMT(Match, match)                  \
    STMT(Inline, inlinestmt)            \
    SECTION(STMT_END)                   \
    SECTION(PAT_START)                  \
    MATCH(MatchValue, matchvalue)       \
    MATCH(MatchSingleton, matchsingleton)   \
    MATCH(MatchSequence, matchsequence)     \
    MATCH(MatchMapping, matchmapping)       \
    MATCH(MatchClass, matchclass)           \
    MATCH(MatchStar, matchstar)             \
    MATCH(MatchAs, matchas)                 \
    MATCH(MatchOr, matchor)                 \
    SECTION(PAT_END)                        \
    SECTION(VM_START)                       \
    VM(CondJump, condjump)                  \
    VM(Jump, jump)                          \
    VM(VMStmt, vmstmt)                      \
    VM(VMNativeFunction, nativefunction)

    #define X(name, _) name,
    #define SSECTION(name) name,
    #define EXPR(name, _) name,
    #define STMT(name, _) name,
    #define MOD(name, _) name,
    #define MATCH(name, _) name,
    #define VM(name, _) name,

    NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH, VM)

    #undef X
    #undef SSECTION
    #undef EXPR
    #undef STMT
    #undef MOD
    #undef MATCH
    #undef VM

    Size
};
// clang-format off


#define KW_PASS(name, fun)
#define KW_SECTION_PASS(name)

#define KW_FOREACH_EXPR(EXPR) NODEKIND_ENUM(KW_PASS, KW_SECTION_PASS, EXPR, KW_PASS, KW_PASS, KW_PASS, KW_PASS)
#define KW_FOREACH_STMT(STMT) NODEKIND_ENUM(KW_PASS, KW_SECTION_PASS, KW_PASS, STMT, KW_PASS, KW_PASS, KW_PASS)
#define KW_FOREACH_MOD(MOD)   NODEKIND_ENUM(KW_PASS, KW_SECTION_PASS, KW_PASS, KW_PASS, MOD, KW_PASS, KW_PASS)
#define KW_FOREACH_PAT(MATCH) NODEKIND_ENUM(KW_PASS, KW_SECTION_PASS, KW_PASS, KW_PASS, KW_PASS, MATCH, KW_PASS)
#define KW_FOREACH_VM(VM)     NODEKIND_ENUM(KW_PASS, KW_SECTION_PASS, KW_PASS, KW_PASS, KW_PASS, KW_PASS, VM)
#define KW_FOREACH_AST(ALL)   NODEKIND_ENUM(KW_PASS, KW_SECTION_PASS, ALL, ALL, ALL, ALL, KW_PASS)
#define KW_FOREACH_ALL(ALL)   NODEKIND_ENUM(KW_PASS, KW_SECTION_PASS, ALL, ALL, ALL, ALL, ALL)

}

#endif