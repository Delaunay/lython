// #include "cases.h"
#include "cases_sample.h"

#define CATCH_CONFIG_RUNNER
#include <catch2/catch_all.hpp>
#include <sstream>

#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "logging/logging.h"
#include "parser/parser.h"
#include "parser/format_spec.h"
#include "utilities/strings.cpp"

#if 0
TEST_CASE("Export") {
    #define GENTEST(name)   \
        transition("cases", str(lython::nodekind<name>()), name##_examples());

    #define X(name, _)
    #define SSECTION(name)
    #define EXPR(name, _) GENTEST(name)
    #define STMT(name, _) GENTEST(name)
    #define MOD(name, _)
    #define MATCH(name, _)
    #define VM(n, m)

    NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH, VM)

    #undef X
    #undef SSECTION
    #undef EXPR
    #undef STMT
    #undef MOD
    #undef MATCH
    #undef VM
}
#endif


int main(int argc, char const* argv[]) {
    // transition("cases", str(nodekind<TestType>()), name##_examples());

    int result = Catch::Session().run(argc, argv);

    return result;
}