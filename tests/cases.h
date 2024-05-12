#ifndef LYTHON_TESTS_CASES_HEADER
#define LYTHON_TESTS_CASES_HEADER

#include "ast/nodes.h"
#include "dtypes.h"
#include "parser/parsing_error.h"

#include "libtest.h"

#include <catch2/catch_all.hpp>
#include <sstream>

using namespace lython;

#define GENCASES(name) Array<TestCase> const &name##_examples();

#define X(name, _)
#define SSECTION(name)
#define EXPR(name, _) GENCASES(name)
#define STMT(name, _) GENCASES(name)
#define MOD(name, _)
#define MATCH(name, _)
#define VM(name, _)

NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH, VM)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef GENTEST

// Name Error
String NE(String const &name);

// Not Callable
String NC(std::string const &name);

// Type Error
String TE(String const &lhs_v, String const &lhs_t, String const &rhs_v, String const &rhs_t);

// Attribute Error
String AE(String const &name, String const &attr);

// UnsipportedOperand
String UO(String const &op, String const &lhs, String const &rhs);

// Import Error
String IE(String const &import, String const &name);

// ModuleNotFoundError
String MNFE(String const &module);

#endif