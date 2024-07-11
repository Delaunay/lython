
#include <catch2/catch_all.hpp>
#include <sstream>

// Kiwi
#include "logging/logging.h"
#include "lowering/SSA.h"
#include "utilities/printing.h"
#include "lexer/buffer.h"
#include "parser/parser.h"
#include "revision_data.h"
#include "sema/sema.h"
#include "utilities/strings.h"

// Test
#include "libtest.h"

using namespace lython;

void ssa_it(TestCase const& testcase) {


    StringBuffer reader(testcase.get_code());
    Lexer        lex(reader);
    Parser       parser(lex);

    kwinfo(outlog(), "{}", "Parse");
    Module* mod = parser.parse_module();
    lyassert(mod->body.size() > 0, "Should parse more than one expression");

    // SSA inherit from the copy tree
    {
        using CopyWalk = TreeWalk<StaticSingleAssignment, false, SSAVisitorTrait>;

        kwinfo(outlog(), "{}", "SSA");
        CopyWalk cpy;
        auto newmod = cpy.module(mod, 0);
        Module* cpy_mod = cast<Module>(newmod);

        std::cout << "\n";
        std::cout << str(mod);
        std::cout << "\n====\n";
        std::cout << str(cpy_mod);
        std::cout << "\n";

        REQUIRE(str(mod) == str(cpy_mod));
    }

    // SSA
    if (testcase.has("SSA"))
    {
        kwinfo(outlog(), "{}", "SSA");
        StaticSingleAssignment ssa;
        auto newmod = ssa.module(mod, 0);
        Module* ssa_mod = cast<Module>(newmod);

        std::cout << "\n";
        std::cout << str(mod);
        std::cout << "\n====\n";
        std::cout << str(ssa_mod);
        std::cout << "\n";

        REQUIRE(str(ssa_mod) == testcase.get_one("SSA"));
    }

}

void run_testcase(String const& folder, String const& name, Array<TestCase> cases) {
    kwinfo(outlog(), "Testing {}", name);

    Array<String> errors;
    TypeExpr*     deduced_type = nullptr;

    cases = get_test_cases(folder, name, cases);

    int i = 0;
    for (TestCase const& c: cases) {
        StringStream ss;
        ss << name << "_" << i;
        String filename = ss.str();

        SECTION(filename.c_str()) {
            ssa_it(c);
        }
        i += 1;
    }
}

#define GENTEST(name)                                               \
    TEMPLATE_TEST_CASE("SSA_" #name, #name, name) {                 \
        auto cases = get_test_cases("cases", #name);                                           \
        run_testcase("SSA", str(lython::nodekind<TestType>()), cases); \
    }

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

#undef GENTEST
