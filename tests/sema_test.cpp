#include "ast/magic.h"
#include "lexer/buffer.h"
#include "parser/parser.h"
#include "sema/sema.h"
#include "utilities/strings.h"

#include <catch2/catch.hpp>
#include <sstream>

#include "logging/logging.h"

#include "cases.h"

using namespace lython;

inline Tuple<TypeExpr *, Array<String>> sema_it(String code, Module *&mod) {
    StringBuffer reader(code);
    Lexer        lex(reader);
    Parser       parser(lex);

    info("{}", "Parse");
    mod = parser.parse_module();
    assert(mod->body.size() > 0, "Should parse more than one expression");

    info("{}", "Sema");
    SemanticAnalyser sema;
    sema.exec(mod, 0);

    BindingEntry &entry = sema.bindings.bindings.back();

    Array<String> errors;
    for (auto &err: sema.errors) {
        errors.push_back(err.message);
    }

    return std::make_tuple(entry.type, errors);
}

Array<String> expected_errors(TestCase const &test) {
    Array<String> r;
    r.reserve(test.undefined.size());
    for (auto &undef: test.undefined) {
        r.push_back("Undefined variable " + undef);
    }

    return r;
}

void run_testcase(String const &name, Array<TestCase> cases) {
    info("Testing {}", name);

    Array<String> errors;
    TypeExpr *    deduced_type = nullptr;
    for (auto &c: cases) {
        Module *mod;

        if (c.exception == "") {
            std::tie(deduced_type, errors) = sema_it(c.code, mod);

            REQUIRE(errors == expected_errors(c));

            if (c.expected_type != "") {
                REQUIRE(c.expected_type == str(deduced_type));
            }
            delete mod;
        } else {
            REQUIRE_THROWS_WITH(sema_it(c.code, mod), std::string(c.exception.c_str()));
        }
        info("<<<<<<<<<<<<<<<<<<<<<<<< DONE");
    }
}

#define GENTEST(name)                                               \
    TEMPLATE_TEST_CASE("SEMA_" #name, #name, name) {                \
        run_testcase(str(nodekind<TestType>()), name##_examples()); \
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
