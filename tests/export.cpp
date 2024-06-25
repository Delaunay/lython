// #include "cases.h"
// #include "cases_sample.h"
// #include "cases_vm.h"

//
#define CATCH_CONFIG_RUNNER
#include <catch2/catch_all.hpp>
#include <sstream>

#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "logging/logging.h"
#include "parser/parser.h"
#include "parser/format_spec.h"
#include "utilities/strings.cpp"

//
#include "libtest.h"

using namespace lython;


// TypeError
// NameError
// AttributeError
//
// Add Inheritance lookup
Array<TestCase> sema_cases() {
    static Array<TestCase> ex = {

        /*

        {
            "class Custom:\n" // Compare op
            "    def __gt__(self, a) -> int:\n"
            "        retrun 1\n"
            "\n"
            "a = Custom()\n"
            "a > True\n" //
        },
        {
            "class Custom:\n" // Bin Operator
            "    def __add__(self, a: int) -> int:\n"
            "        retrun 1\n"
            "\n"
            "a = Custom()\n"
            "a + 1\n" // Works
        },
        {
            "class Custom:\n" // Aug Operator
            "    def __iadd__(self, a: int) -> int:\n"
            "        retrun 1\n"
            "\n"
            "a = Custom()\n"
            "a += 1\n" // Works
        },
        {
            "class Custom:\n" // Unary Operator
            "    def __pos__(self) -> int:\n"
            "        retrun 1\n"
            "\n"
            "a = Custom()\n"
            "+a\n" // Works
        },
        {
            "class Custom:\n"
            "    def __add__(self, a: int) -> int:\n"
            "        retrun 1\n"
            "\n"
            "a = Custom()\n"
            "a + 2.0\n" // TypeError
        },
        {
            "class CustomRAnd:\n"
            "    def __rand__(self, a) -> int:\n"
            "        retrun 1\n"
            "\n"
            "class Name:\n"
            "    pass\n"
            "\n"
            "a = CustomRAnd()\n"
            "b and a\n" // <= lookup if __rand__ to call __rand__(a, b)
        },

        // */
    };
    return ex;
}

#if 0
TEST_CASE("Export") {
    #define GENTEST(name)   \
        lython::transition("vm", str(lython::nodekind<name>()), lython::name##_vm_examples());

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