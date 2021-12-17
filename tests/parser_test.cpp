#include "cases.h"
#include "samples.h"

#include <catch2/catch.hpp>
#include <sstream>

#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "logging/logging.h"
#include "parser/parser.h"
#include "utilities/strings.cpp"

using namespace lython;

template <typename T>
Array<String> const &examples() {
    static Array<String> ex = {};
    return ex;
}

inline String parse_it(String code) {
    StringBuffer reader(code);
    Module       module;

    Lexer  lex(reader);
    Parser parser(lex);

    Module *mod = parser.parse_module();
    assert(mod->body.size() > 0, "Should parse more than one expression");

    auto data = str(mod);

    delete mod;
    return data;
}

#define TEST_PARSING(code) \
    SECTION(#code) { REQUIRE(strip(parse_it(code())) == strip(code())); }

TEST_CASE("Parser") {
    CODE_SAMPLES(TEST_PARSING)
    IMPORT_TEST(TEST_PARSING)
}

void run_testcase(String const &name, Array<TestCase> cases) {
    info("Testing {}", name);
    for (auto &c: cases) {
        REQUIRE(strip(parse_it(c.code)) == strip(c.code));
        info("<<<<<<<<<<<<<<<<<<<<<<<< DONE");
    }
}

#define GENTEST(name)                                               \
    TEMPLATE_TEST_CASE("PARSE_" #name, #name, name) {               \
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

template <>
Array<String> const &examples<Match>() {
    static Array<String> ex = {
        "match a:\n"
        "    case [1, 3]:\n"
        "        pass\n"
        "    case p as c:\n"
        "        pass\n"
        "    case a | c:\n"
        "        pass\n"
        "    case ClassName(a, b, c=d):\n"
        "        pass\n"
        "    case d if b:\n"
        "        pass\n",

        "match lst:\n"
        "    case []:\n"
        "        pass\n"
        "    case [head, *tail]:\n"
        "        pass\n",

        "match dct:\n"
        "    case {}:\n"
        "        pass\n"
        "    case {1: value, **remainder}:\n"
        "        pass\n",
    };
    return ex;
}

template <>
Array<String> const &examples<Continue>() {
    static Array<String> ex = {
        "continue",
    };
    return ex;
}

template <>
Array<String> const &examples<Break>() {
    static Array<String> ex = {
        "break",
    };
    return ex;
}

template <>
Array<String> const &examples<Pass>() {
    static Array<String> ex = {
        "pass",
    };
    return ex;
}

template <>
Array<String> const &examples<StmtNode>() {
    static Array<String> ex = {};
    return ex;
}

template <>
Array<String> const &examples<Nonlocal>() {
    static Array<String> ex = {
        "nonlocal a",
    };
    return ex;
}

template <>
Array<String> const &examples<Global>() {
    static Array<String> ex = {
        "global a",
    };
    return ex;
}

template <>
Array<String> const &examples<ImportFrom>() {
    static Array<String> ex = {
        "from a.b import c as d, e.f as g",
    };
    return ex;
}

template <>
Array<String> const &examples<Import>() {
    static Array<String> ex = {
        "import a as b, c as d, e.f as g",
    };
    return ex;
}

template <>
Array<String> const &examples<Assert>() {
    static Array<String> ex = {
        "assert a",
        "assert a, \"b\"",
    };
    return ex;
}

template <>
Array<String> const &examples<Try>() {
    static Array<String> ex = {
        "try:\n"
        "    pass\n"
        "except err as b:\n"
        "    pass\n"
        "else:\n"
        "    pass\n"
        "finally:\n"
        "    pass\n",
    };
    return ex;
}

template <>
Array<String> const &examples<Raise>() {
    static Array<String> ex = {
        "raise a from b",
    };
    return ex;
}

template <>
Array<String> const &examples<With>() {
    static Array<String> ex = {
        "with a as b, c as d:\n"
        "    pass\n",
    };
    return ex;
}

template <>
Array<String> const &examples<If>() {
    static Array<String> ex = {
        "if a:\n"
        "    pass\n"
        "elif b:\n"
        "    pass\n"
        "else:\n"
        "    pass\n",
    };
    ;
    return ex;
}

template <>
Array<String> const &examples<While>() {
    static Array<String> ex = {
        "while a:\n"
        "    pass\n"
        "else:\n"
        "    pass\n",
    };
    return ex;
}

template <>
Array<String> const &examples<For>() {
    static Array<String> ex = {
        "for a in b:\n"
        "    pass\n"
        "else:\n"
        "    pass\n",
        "for a, (b, c), d in b:\n"
        "    pass\n",
    };
    return ex;
}

template <>
Array<String> const &examples<AnnAssign>() {
    static Array<String> ex = {
        "a: b = c",
    };
    return ex;
}

template <>
Array<String> const &examples<AugAssign>() {
    static Array<String> ex = {
        "a += b",
        "a -= b",
    };
    return ex;
}

template <>
Array<String> const &examples<Assign>() {
    static Array<String> ex = {
        "a = b",
        "a, b = c",
    };
    return ex;
}

template <>
Array<String> const &examples<Delete>() {
    static Array<String> ex = {
        "del a, b",
    };
    return ex;
}

template <>
Array<String> const &examples<Return>() {
    static Array<String> ex = {
        "return a",
    };
    return ex;
}

template <>
Array<String> const &examples<ClassDef>() {
    static Array<String> ex = {
        "@decorator1(a, b, c=d)\n"
        "@decorator2\n"
        "class a(a, b=c):\n"
        "    \"\"\"docstring\"\"\"\n"
        "    pass",
    };
    return ex;
}

template <>
Array<String> const &examples<FunctionDef>() {
    static Array<String> ex = {
        "@decorator\n"
        "def a(b, c=d, *e, f=g, **h) -> i:\n"
        "    \"\"\"docstring\"\"\"\n"
        "    pass",

        "@decorator1(a, b, c=d)\n"
        "@decorator2\n"
        "def a(b: c, d: e = f):\n"
        "    pass",
    };
    return ex;
}

template <>
Array<String> const &examples<Inline>() {
    static Array<String> ex = {
        "a = 2; b = c; c = d",
    };
    return ex;
}

template <>
Array<String> const &examples<FunctionType>() {
    static Array<String> ex = {};
    return ex;
}

template <>
Array<String> const &examples<Expression>() {
    static Array<String> ex = {};
    return ex;
}

template <>
Array<String> const &examples<Interactive>() {
    static Array<String> ex = {};
    return ex;
}

template <>
Array<String> const &examples<Module>() {
    static Array<String> ex = {};
    return ex;
}

template <>
Array<String> const &examples<Slice>() {
    static Array<String> ex = {
        "a[b:c:d]",
    };
    return ex;
}

template <>
Array<String> const &examples<TupleExpr>() {
    static Array<String> ex = {
        "a, b, c",
        "a, (b, c), d",
        "a, b, c = d, e, f",
    };
    return ex;
}

template <>
Array<String> const &examples<ListExpr>() {
    static Array<String> ex = {
        "[a, b, c]",
    };
    return ex;
}

template <>
Array<String> const &examples<Name>() {
    static Array<String> ex = {
        "a",
    };
    return ex;
}

template <>
Array<String> const &examples<Starred>() {
    static Array<String> ex = {
        "*a",
    };
    return ex;
}

template <>
Array<String> const &examples<Subscript>() {
    static Array<String> ex = {
        "a[b];",
    };
    return ex;
}

template <>
Array<String> const &examples<Attribute>() {
    static Array<String> ex = {
        "a.b",
    };
    return ex;
}

template <>
Array<String> const &examples<Constant>() {
    static Array<String> ex = {
        "1",
        "2.1",
        // "'str'",
        "\"str\"",
        "None",
        "True",
        "False",
    };
    return ex;
}

template <>
Array<String> const &examples<FormattedValue>() {
    static Array<String> ex = {};
    return ex;
}

template <>
Array<String> const &examples<JoinedStr>() {
    static Array<String> ex = {};
    return ex;
}
template <>
Array<String> const &examples<Call>() {
    static Array<String> ex = {
        "fun(a, b, c=d)",
    };
    return ex;
}
template <>
Array<String> const &examples<Compare>() {
    static Array<String> ex = {
        "a < b > c != d", "a not in b", "a in b", "a is b", "a is not b",
    };
    return ex;
}
template <>
Array<String> const &examples<YieldFrom>() {
    static Array<String> ex = {
        "yield from a",
    };
    return ex;
}
template <>
Array<String> const &examples<Yield>() {
    static Array<String> ex = {
        "yield a",
        "yield",
    };
    return ex;
}
template <>
Array<String> const &examples<Await>() {
    static Array<String> ex = {
        "await a",
    };
    return ex;
}
template <>
Array<String> const &examples<DictComp>() {
    static Array<String> ex = {
        "{a: c for a in b if a > c}",
    };
    return ex;
}
template <>
Array<String> const &examples<SetComp>() {
    static Array<String> ex = {
        "{a for a in b if a > c}",
    };
    return ex;
}
template <>
Array<String> const &examples<GeneratorExp>() {
    static Array<String> ex = {
        "(a for a in b if a > c)",
    };
    return ex;
}
template <>
Array<String> const &examples<ListComp>() {
    static Array<String> ex = {
        "[a for a in b if a > c]",
    };
    return ex;
}
template <>
Array<String> const &examples<SetExpr>() {
    static Array<String> ex = {
        "{a, b}",
    };
    return ex;
}
template <>
Array<String> const &examples<DictExpr>() {
    static Array<String> ex = {
        "{a: b, c: d}",
    };
    return ex;
}
template <>
Array<String> const &examples<IfExp>() {
    static Array<String> ex = {
        "a = if b: c else d",
        // this is the real python version
        // "a = b if c else d",
    };
    return ex;
}
template <>
Array<String> const &examples<Lambda>() {
    static Array<String> ex = {
        "lambda a: b",
    };
    return ex;
}
template <>
Array<String> const &examples<UnaryOp>() {
    static Array<String> ex = {
        "+ a",
        "- a",
        "~ a",
        "! a",
    };
    return ex;
}
template <>
Array<String> const &examples<BinOp>() {
    static Array<String> ex = {
        "a + b", "a - b", "a * b", "a << b", "a ^ b",
    };
    return ex;
}

template <>
Array<String> const &examples<NamedExpr>() {
    static Array<String> ex = {
        "a = a := b",
    };
    return ex;
}

template <>
Array<String> const &examples<BoolOp>() {
    static Array<String> ex = {
        "a and b",
        "a or b",
    };
    return ex;
}
