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
#undef VM

#undef GENTEST


#endif