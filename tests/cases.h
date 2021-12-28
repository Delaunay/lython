#ifndef LYTHON_TESTS_CASES_HEADER
#define LYTHON_TESTS_CASES_HEADER

#include "ast/nodes.h"
#include "dtypes.h"
#include "parser/parsing_error.h"

#include <catch2/catch.hpp>
#include <sstream>

using namespace lython;

struct TestCase {
    TestCase(String const &c, Array<String> const &u = Array<String>(), String const &t = "",
             String const &ptr = ""):
        code(c),
        undefined(u), expected_type(t), exception(ptr) {}

    String        code;
    Array<String> undefined;
    String        expected_type;
    String        exception;
};

#define GENCASES(name) Array<TestCase> const &name##_examples();

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