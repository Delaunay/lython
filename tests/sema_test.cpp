#include "ast/magic.h"
#include "lexer/buffer.h"
#include "parser/parser.h"
#include "sema/sema.h"
#include "utilities/strings.h"

#include <catch2/catch.hpp>
#include <sstream>

#include "logging/logging.h"

using namespace lython;

template <typename T>
Array<String> const &examples() {
    static Array<String> ex = {};
    return ex;
}

inline Array<String> sema_it(String code) {
    StringBuffer reader(code);
    Module       module;

    Lexer  lex(reader);
    Parser parser(lex);

    info("{}", "Parse");
    Module *mod = parser.parse_module();
    assert(mod->body.size() > 0, "Should parse more than one expression");

    info("{}", "Sema");
    SemanticAnalyser sema;
    sema.exec(mod, 0);

    Array<String> errors;
    for (auto &err: sema.errors) {
        errors.push_back(err.message);
    }

    delete mod;
    return errors;
}

#define GENTEST(name)                                        \
    TEMPLATE_TEST_CASE("SEMA_" #name, #name, name) {         \
        info("Testing {}", str(nodekind<TestType>()));       \
        Array<String> const &samples = examples<TestType>(); \
                                                             \
        for (auto &code: samples) {                          \
            REQUIRE(sema_it(code) == Array<String>());       \
            info("<<<<<<<<<<<<<<<<<<<<<<<< DONE");           \
        }                                                    \
    }

#define X(name, _)
#define SSECTION(name)
#define EXPR(name, _) GENTEST(name)
#define STMT(name, _) GENTEST(name)
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

template <>
Array<String> const &examples<FunctionDef>() {
    static Array<String> ex = {
        "c = 1\n"
        "e = 2\n"
        "f = 3\n"
        "def a(b: c, d: e = f):\n"
        "    return b + d",
    };
    return ex;
}

template <>
Array<String> const &examples<With>() {
    static Array<String> ex = {
        "a = 1\n"
        "with a as b:\n"
        "    return b",
    };
    return ex;
}
