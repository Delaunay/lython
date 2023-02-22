#ifndef LYTHON_TESTS_CASES_HEADER
#define LYTHON_TESTS_CASES_HEADER

#include "ast/nodes.h"
#include "dtypes.h"

#include <catch2/catch_all.hpp>
#include <sstream>

using namespace lython;

#define GENCASES(name) Array<TestCase> const& name##_error_examples();

#define X(name, _)
#define SSECTION(name)
#define EXPR(name, _) GENCASES(name)
#define STMT(name, _) GENCASES(name)
#define MOD(name, _)
#define MATCH(name, _)

NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef GENTEST

#endif