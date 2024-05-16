#include "utilities/printing.h"
#include "lexer/buffer.h"
#include "parser/parser.h"
#include "revision_data.h"
#include "sema/sema.h"
#include "utilities/strings.h"

#include <catch2/catch_all.hpp>
#include <sstream>

#include "logging/logging.h"

#include "cases.h"

// Path to repository on current system

String test_modules_path() { return String(_SOURCE_DIRECTORY) + "/code"; }

using namespace lython;

void run_testcase(String const& folder, String const& name, Array<TestCase> cases);

TEST_CASE("SEMA_FunctionDef_Typing") {
    static Array<TestCase> ex = {
        {
            "def fun():\n"
            "    return x\n",  // Name error
            {
                NE("x"),
            },
        },
        {
            "def fun(a: i32) -> i32:\n"
            "    return a\n"
            "x = fun(1)\n"  // Works
        },
        {
            "def fun(a: i32) -> i32:\n"
            "    return a\n"
            "x: i32 = fun(1)\n"  // Works
        },

        {
            "def fun(a: i32) -> i32:\n"
            "    return a\n"
            "x = fun(1.0)\n",  // Type Error
            {
                TE("fun(1.0)", "(f64) -> i32", "fun", "(i32) -> i32"),
            },
        },

        {
            "def fun(a: i32) -> i32:\n"
            "    return a\n"
            "x: f32 = fun(1)\n",  // Type Error
            {
                TE("x", "f32", "fun(1)", "i32"),
            },
        },

        {
            "def fun(a: i32, b: f64) -> i32:\n"
            "    return a\n"
            "x: i32 = fun(b=1.0, a=1)\n",  // works
        },

        {
            "def fun(a: i32 = 1, b: f64 = 1.1) -> i32:\n"
            "    return a\n"
            "x: i32 = fun()\n",  // works
        },

        {
            "def fun(a: i32, b: f64 = 1.1) -> i32:\n"
            "    return a\n"
            "x: i32 = fun()\n",  // missing argument
            TestErrors({
                // TE("Argument a is not defined");
            }),
        },
    };

    run_testcase("sema", "FunctionDef", ex);
}

TEST_CASE("SEMA_Match_Details") {}

TEST_CASE("SEMA_ClassDef_Attribute") {
    static Array<TestCase> ex = {
        {
            "class Name:\n"
            "    pass\n"
            "\n"
            "a = Name()\n"
            "a.x\n"
            "a.x = 2\n",
            {
                AE("Name", "x"),
                AE("Name", "x"),
                TE("a.x", "", "2", "i32"),
            },
        },
        {
            "class Custom:\n"
            "    def __init__(self, a: i32):\n"
            "        self.a = a\n"
            "\n"
            "a = Custom(1)\n"  // works
        },
        {
            "class Name:\n"
            "    def __init__(self, x: i32):\n"  // Resolve an attribute defined inside the ctor
            "        self.x = x\n"
            "\n"
            "a = Name(2)\n"
            "a.x\n"
            "a.x = 4\n",
        },
    };

    run_testcase("sema", "ClassDef", ex);
}

TEST_CASE("SEMA_IfStmt") {
    static Array<TestCase> ex = {
        // FIXME, sema need to detect this
        {
            // `a` might be set
            "c = True\n"
            "if c:\n"
            "    a = 2\n"
            "b = a + 1\n",
        },
        {   // `a` will NEVER be set
            "if False:\n"
            "    a = 2\n"
            "b = a + 1\n",
        },
        {   // `a` will always be set
            "if True:\n"
            "    a = 2\n"
            "else:\n"
            "    a = 3\n"
            "b = a + 1\n",
        },
    };

    run_testcase("sema", "Conditional_Assign", ex);
}

TEST_CASE("SEMA_Unpacking") {
    static Array<TestCase> ex = {
        {
            "def fun():\n"
            "    return 1, 2, 3\n"
            "a = fun()\n",
        },
        {
            "def fun():\n"
            "    return 1, 2, 3\n"
            "a, b, c = fun()\n",
        },
        {
            "def fun():\n"
            "    return 1, 2, 3\n"
            "a, *b = fun()\n",
        },
    };

    run_testcase("sema", "Unpacking", ex);
}

TEST_CASE("SEMA_ClassDef_Static_Attribute") {
    static Array<TestCase> ex = {

        {
            "class Name:\n"
            "    x: i32 = 1\n"
            "\n"
            "a = Name()\n"
            "a.x = 2\n",
        },
    };

    run_testcase("sema", "ClassDef", ex);
}

TEST_CASE("SEMA_ClassDef_Magic_BoolOperator") {
    static Array<TestCase> ex = {
        {
            "class CustomAnd:\n"  // Bool op
            "    pass\n"
            "\n"
            "a = CustomAnd()\n"
            "a and True\n",
            {
                UO("and", "CustomAnd", "bool"),
            },
        },
        {
            "class CustomAnd:\n"  // Bool op
            "    def __and__(self, b: bool) -> bool:\n"
            "        return True\n"
            "\n"
            "a = CustomAnd()\n"
            "a and True\n"  // <= lookup of __and__ to call __and__(a, b)
        },
    };

    run_testcase("sema", "ClassDef", ex);
}

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

FILE* get_fuzz_file() {
    static FILE* file = fopen("fizz.ly", "w");
    return file;
}

void write_fuzz_file(String const& name, String const& code) {
    String path = String("in/") + name + String(".ly");

    FILE* file = fopen(path.c_str(), "w");

    if (file) {
        fprintf(file, "%s\n", code.c_str());
        fclose(file);
    }
}

inline Tuple<TypeExpr*, Array<String>> sema_it(String code, Module*& mod) {

    StringBuffer reader(code);
    Lexer        lex(reader);
    Parser       parser(lex);

    kwinfo(outlog(), "{}", "Parse");
    mod = parser.parse_module();
    lyassert(mod->body.size() > 0, "Should parse more than one expression");

    kwinfo(outlog(), "{}", "Sema");

    ImportLib::instance()->add_to_path(test_modules_path());

    SemanticAnalyser sema;
    sema.exec(mod, 0);

    BindingEntry& entry = sema.bindings.bindings.back();

    Array<String> errors;
    for (auto& err: sema.errors) {
        errors.push_back(err->what());
    }

    return std::make_tuple(entry.type, errors);
}

void run_testcase(String const& folder, String const& name, Array<TestCase> cases) {
    kwinfo(outlog(), "Testing {}", name);

    Array<String> errors;
    TypeExpr*     deduced_type = nullptr;

    cases = get_test_cases(folder, name, cases);

    int i = 0;
    for (auto& c: cases) {
        StringStream ss;
        ss << name << "_" << i;
        String filename = ss.str();
        write_fuzz_file(ss.str(), c.code);

        SECTION(filename.c_str()) {
            Module* mod;

            std::tie(deduced_type, errors) = sema_it(c.code, mod);

            REQUIRE(errors == c.errors);

            if (c.expected_type != "") {
                REQUIRE(c.expected_type == str(deduced_type));
            }
            delete mod;

            kwinfo(outlog(), "<<<<<<<<<<<<<<<<<<<<<<<< DONE");

        }
        i += 1;
    }
}

TEST_CASE("Class_Attribute_Lookup") {
    // Futures tests cases
    run_testcase("sema", "ClassDef", sema_cases());
}

#define GENTEST(name)                                               \
    TEMPLATE_TEST_CASE("SEMA_" #name, #name, name) {                \
        run_testcase("parser", str(nodekind<TestType>()), name##_examples()); \
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
