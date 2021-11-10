#include "ast/magic.h"
#include "lexer/buffer.h"
#include "parser/parser.h"
#include "sema/sema.h"
#include "utilities/strings.h"

#include <catch2/catch.hpp>
#include <sstream>

#include "logging/logging.h"

using namespace lython;

struct TestCase {
    TestCase(String const &c, Array<String> const &u = Array<String>()): code(c), undefined(u) {}

    String        code;
    Array<String> undefined;
    TypeExpr *    expected_type = nullptr;
};

template <typename T>
Array<TestCase> const &examples() {
    static Array<TestCase> ex = {};
    return ex;
}

inline Tuple<TypeExpr *, Array<String>> sema_it(String code) {
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

    BindingEntry &entry = *(sema.bindings.bindings.rbegin());

    Array<String> errors;
    for (auto &err: sema.errors) {
        errors.push_back(err.message);
    }

    delete mod;
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

#define GENTEST(name)                                         \
    TEMPLATE_TEST_CASE("SEMA_" #name, #name, name) {          \
        info("Testing {}", str(nodekind<TestType>()));        \
        Array<TestCase> const &cases = examples<TestType>();  \
        Array<String>          errors;                        \
        TypeExpr *             deduced_type = nullptr;        \
        for (auto &c: cases) {                                \
            std::tie(deduced_type, errors) = sema_it(c.code); \
            REQUIRE(errors == expected_errors(c));            \
            info("<<<<<<<<<<<<<<<<<<<<<<<< DONE");            \
        }                                                     \
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
Array<TestCase> const &examples<Match>() {
    static Array<TestCase> ex = {
        // TODO: check this test case on python
        // not sure about i
        {"match a:\n"
         "    case [1, 3]:\n"
         "        pass\n"
         "    case b as c:\n"
         "        return c\n"
         "    case d | e:\n"
         "        return d\n"
         "    case ClassName(f, g, h=i):\n"
         "        return f + g + i\n"
         "    case j if k:\n"
         "        return j\n",
         {"a", "ClassName", "k"}},

        {"match lst:\n"
         "    case []:\n"
         "        pass\n"
         "    case [head, *tail]:\n"
         "        pass\n",
         {"lst"}},

        {"match dct:\n"
         "    case {}:\n"
         "        pass\n"
         "    case {1: value, **remainder}:\n"
         "        pass\n",
         {"dct"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<Continue>() {
    static Array<TestCase> ex = {
        {"continue"},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<Break>() {
    static Array<TestCase> ex = {
        {"break"},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<Pass>() {
    static Array<TestCase> ex = {
        {"pass"},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<StmtNode>() {
    static Array<TestCase> ex = {};
    return ex;
}

template <>
Array<TestCase> const &examples<Nonlocal>() {
    static Array<TestCase> ex = {
        {"nonlocal a", {"a"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<Global>() {
    static Array<TestCase> ex = {
        {"global a", {"a"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<ImportFrom>() {
    static Array<TestCase> ex = {
        {"from a.b import c as d, e.f as g"},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<Import>() {
    static Array<TestCase> ex = {
        {"import a as b, c as d, e.f as g"},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<Assert>() {
    static Array<TestCase> ex = {
        {"assert a", {"a"}},
        {"assert a, \"b\"", {"a"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<Try>() {
    static Array<TestCase> ex = {
        {"try:\n"
         "    pass\n"
         "except Exception as b:\n"
         "    pass\n"
         "else:\n"
         "    pass\n"
         "finally:\n"
         "    pass\n"},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<Raise>() {
    static Array<TestCase> ex = {
        {"raise a from b", {"a", "b"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<With>() {
    static Array<TestCase> ex = {
        {"with a as b, c as d:\n"
         "    pass\n",
         {"a", "c"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<If>() {
    static Array<TestCase> ex = {
        {"if a:\n"
         "    pass\n"
         "elif b:\n"
         "    pass\n"
         "else:\n"
         "    pass\n",
         {"a", "b"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<While>() {
    static Array<TestCase> ex = {
        {"while a:\n"
         "    pass\n"
         "else:\n"
         "    pass\n",
         {"a"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<For>() {
    static Array<TestCase> ex = {
        {"for a in b:\n"
         "    pass\n"
         "else:\n"
         "    pass\n",
         {"b"}},
        {"for a, (b, c), d in b:\n"
         "    pass\n",
         {"b"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<AnnAssign>() {
    static Array<TestCase> ex = {
        {"a: b = c", {"b", "c"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<AugAssign>() {
    static Array<TestCase> ex = {
        {"a += b", {"a", "b"}},
        {"a -= b", {"a", "b"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<Assign>() {
    static Array<TestCase> ex = {
        {"a = b", {"b"}},
        {"a, b = c", {"c"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<Delete>() {
    static Array<TestCase> ex = {
        {"del a, b", {"a", "b"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<Return>() {
    static Array<TestCase> ex = {
        {"return a", {"a"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<ClassDef>() {
    static Array<TestCase> ex = {
        {"@e(g, h, i=j)\n"
         "@f\n"
         "class a(b, c=d):\n"
         "    \"\"\"docstring\"\"\"\n"
         "    pass",
         {"b", "d", "e", "g", "h", "j", "f"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<FunctionDef>() {
    static Array<TestCase> ex = {
        {"@j\n"
         "def a(b, c=d, *e, f=g, **h) -> i:\n"
         "    \"\"\"docstring\"\"\"\n"
         "    pass",
         {"d", "g", "i", "j"}},

        {"@j(l, m, c=n)\n"
         "@k\n"
         "def a(b: c, d: e = f):\n"
         "    pass",
         {"c", "f", "e", "j", "l", "m", "n", "k"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<Inline>() {
    static Array<TestCase> ex = {
        {"a = 2; b = c; d = e", {"c", "e"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<FunctionType>() {
    static Array<TestCase> ex = {};
    return ex;
}

template <>
Array<TestCase> const &examples<Expression>() {
    static Array<TestCase> ex = {};
    return ex;
}

template <>
Array<TestCase> const &examples<Interactive>() {
    static Array<TestCase> ex = {};
    return ex;
}

template <>
Array<TestCase> const &examples<Module>() {
    static Array<TestCase> ex = {};
    return ex;
}

template <>
Array<TestCase> const &examples<Slice>() {
    static Array<TestCase> ex = {
        {"a[b:c:d]", {"a", "b", "c", "d"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<TupleExpr>() {
    static Array<TestCase> ex = {
        {"a, b, c", {"a", "b", "c"}},
        {"a, (b, c), d", {"a", "b", "c", "d"}},
        {"a, b, c = d, e, f", {"d", "e", "f"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<ListExpr>() {
    static Array<TestCase> ex = {
        {"[a, b, c]", {"a", "b", "c"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<Name>() {
    static Array<TestCase> ex = {
        {"a", {"a"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<Starred>() {
    static Array<TestCase> ex = {
        {"*a", {"a"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<Subscript>() {
    static Array<TestCase> ex = {
        {"a[b]", {"a", "b"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<Attribute>() {
    static Array<TestCase> ex = {
        {"a.b", {"a"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<Constant>() {
    static Array<TestCase> ex = {
        {"1"},
        {"2.1"},
        // "'str'",
        {"\"str\""},
        {"None"},
        {"True"},
        {"False"},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<FormattedValue>() {
    static Array<TestCase> ex = {};
    return ex;
}

template <>
Array<TestCase> const &examples<JoinedStr>() {
    static Array<TestCase> ex = {};
    return ex;
}
template <>
Array<TestCase> const &examples<Call>() {
    static Array<TestCase> ex = {
        {"fun(a, b, c=d)", {"fun", "a", "b", "d"}},
    };
    return ex;
}
template <>
Array<TestCase> const &examples<Compare>() {
    static Array<TestCase> ex = {
        {"a < b > c != d", {"a", "b", "c", "d"}},
        {"a not in b", {"a", "b"}},
        {"a in b", {"a", "b"}},
        {"a is b", {"a", "b"}},
        {"a is not b", {"a", "b"}},
    };
    return ex;
}
template <>
Array<TestCase> const &examples<YieldFrom>() {
    static Array<TestCase> ex = {
        {"yield from a", {"a"}},
    };
    return ex;
}
template <>
Array<TestCase> const &examples<Yield>() {
    static Array<TestCase> ex = {
        {"yield a", {"a"}},
        {"yield"},
    };
    return ex;
}
template <>
Array<TestCase> const &examples<Await>() {
    static Array<TestCase> ex = {
        {"await a", {"a"}},
    };
    return ex;
}
template <>
Array<TestCase> const &examples<DictComp>() {
    static Array<TestCase> ex = {
        {"{a: c for a in b if a > c}", {"b", "c", "c"}},
    };
    return ex;
}
template <>
Array<TestCase> const &examples<SetComp>() {
    static Array<TestCase> ex = {
        {"{a for a in b if a > c}", {"b", "c"}},
    };
    return ex;
}
template <>
Array<TestCase> const &examples<GeneratorExp>() {
    static Array<TestCase> ex = {
        {"(a for a in b if a > c)", {"b", "c"}},
    };
    return ex;
}
template <>
Array<TestCase> const &examples<ListComp>() {
    static Array<TestCase> ex = {
        {"[a for a in b if a > c]", {"b", "c"}},
    };
    return ex;
}
template <>
Array<TestCase> const &examples<SetExpr>() {
    static Array<TestCase> ex = {
        {"{a, b}", {"a", "b"}},
    };
    return ex;
}
template <>
Array<TestCase> const &examples<DictExpr>() {
    static Array<TestCase> ex = {
        {"{a: b, c: d}", {"a", "b", "c", "d"}},
    };
    return ex;
}
template <>
Array<TestCase> const &examples<IfExp>() {
    static Array<TestCase> ex = {
        {"a = if b: c else d", {"b", "c", "d"}},
        // this is the real python version
        // "a = b if c else d",
    };
    return ex;
}
template <>
Array<TestCase> const &examples<Lambda>() {
    static Array<TestCase> ex = {
        {"lambda a: b", {"b"}},
    };
    return ex;
}
template <>
Array<TestCase> const &examples<UnaryOp>() {
    static Array<TestCase> ex = {
        {"+ a", {"a"}},
        {"- a", {"a"}},
        {"~ a", {"a"}},
        {"! a", {"a"}},
    };
    return ex;
}
template <>
Array<TestCase> const &examples<BinOp>() {
    static Array<TestCase> ex = {
        {"a + b", {"a", "b"}},  {"a - b", {"a", "b"}}, {"a * b", {"a", "b"}},
        {"a << b", {"a", "b"}}, {"a ^ b", {"a", "b"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<NamedExpr>() {
    static Array<TestCase> ex = {
        {"a = b := c", {"c"}},
    };
    return ex;
}

template <>
Array<TestCase> const &examples<BoolOp>() {
    static Array<TestCase> ex = {
        {"a and b", {"a", "b"}},
        {"a or b", {"a", "b"}},
    };
    return ex;
}
