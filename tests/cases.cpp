#include "cases.h"

#include "sema/sema.h"

String NE(String const& name) { return String(NameError(nullptr, name).message().c_str()); }

String NC(std::string const& name) {
    return String(fmt::format("{} is not callable", name).c_str());
}

String TE(String const& lhs_v, String const& lhs_t, String const& rhs_v, String const& rhs_t) {
    return String(TypeError::message(lhs_v, lhs_t, rhs_v, rhs_t));
}

String AE(String const& name, String const& attr) {
    return String(AttributeError::message(name, attr));
}

String UO(String const& op, String const& lhs, String const& rhs) {
    return String(UnsupportedOperand::message(op, lhs, rhs));
}

String IE(String const& module, String const& name) {
    return String(ImportError::message(module, name));
}

String MNFE(String const& module) { return String(ModuleNotFoundError::message(module)); }

// class ClassName:
//   __match_args__ = ("f", "g")

//   def __init__(self):
//     self.f = 1
//     self.g = 2
//     self.h = 3


// def fun(a, k=0):
//   match a:
//     case [1, 3]:
//       return 'case 1'
//     case 1 | 2:
//       return 'case 2'
//     case ClassName(f, g, h=i):
//       return 'case 3'
//     case j if j > k:
//       return 'case 4'
//     case b as c if c == k:
//       return 'case 5'
//     case _:
//       return 'case 6'


// print(fun([1, 3]))
// print(fun(1))
// print(fun(ClassName()))
// print(fun(5, k=1))
// print(fun(5, 5))
// print(fun(-1))



#define MATCH_SAMPLE                  \
    "match a:\n"                      \
    "    case [1, 3]:\n"              \
    "        return \"case 1\"\n"       \
    "    case 1 | 2:\n"               \
    "        return \"case 2\"\n"       \
    "    case ClassName(f, g, h=i):\n"\
    "        return 'case 3'\n"       \
    "    case j if j > k:\n"          \
    "        return \"case 4\"\n"       \
    "    case b as c if c == k:\n"    \
    "        return \"case 5\"\n"       \
    "    case _:\n"                   \
    "        return \"case 6\"\n"
    
Array<TestCase> const& Match_examples() {
    static Array<TestCase> example = {
        // TODO: check this test case on python
        // not sure about i
        {MATCH_SAMPLE,
         TestErrors({
             NE("a"),
             NE("ClassName"),
             NE("k"),
             UO("Gt", "None", "None"),
             NE("k"),
             UO("Eq", "None", "None"),
         })},

        {"match lst:\n"
         "    case []:\n"
         "        pass\n"
         "    case [head, *tail]:\n"
         "        pass\n",
         TestErrors({
             NE("lst"),
         })},

        {"match dct:\n"
         "    case {}:\n"
         "        pass\n"
         "    case {1: value, **remainder}:\n"
         "        pass\n",
         TestErrors({
             NE("dct"),
         })},
    };
    return example;
}

Array<TestCase> const& Continue_examples() {
    static Array<TestCase> example = {
        {"continue"},
    };
    return example;
}

Array<TestCase> const& Break_examples() {
    static Array<TestCase> example = {
        {"break"},
    };
    return example;
}

Array<TestCase> const& Pass_examples() {
    static Array<TestCase> example = {
        {"pass"},
    };
    return example;
}

Array<TestCase> const& StmtNode_examples() {
    static Array<TestCase> example = {};
    return example;
}

Array<TestCase> const& Nonlocal_examples() {
    static Array<TestCase> example = {
        {"nonlocal a",
         TestErrors({
             NE("a"),
         })},
    };
    return example;
}

Array<TestCase> const& Global_examples() {
    static Array<TestCase> example = {
        {"global a",
         TestErrors({
             NE("a"),
         })},
    };
    return example;
}

Array<TestCase> const& ImportFrom_examples() {
    static Array<TestCase> example = {
        {
            "from aa.b import c as d, e.f as g",
            {
                MNFE("aa.b"),
            },
        },
        {
            // import_test should be inside the path
            "from import_test import cls as Klass, fun as Fun, ann as Ann, var as Var",
            TestErrors({}),
        }};
    return example;
}

Array<TestCase> const& Import_examples() {
    static Array<TestCase> example = {
        {
            "import aa as b, c as d, e.f as g",
            {
                MNFE("aa"),
                MNFE("c"),
                MNFE("e.f"),
            },
        },
        {
            // import_test should be inside the path
            "import import_test as imp_test",
            TestErrors({}),
        },
    };
    return example;
}

Array<TestCase> const& Assert_examples() {
    static Array<TestCase> example = {
        {"assert a",
         TestErrors({
             NE("a"),
         })},
        {"assert a, \"b\"",
         TestErrors({
             NE("a"),
         })},
    };
    return example;
}

Array<TestCase> const& Try_examples() {
    static Array<TestCase> example = {
        {"try:\n"
         "    pass\n"
         "except Exception as b:\n"
         "    pass\n"
         "else:\n"
         "    pass\n"
         "finally:\n"
         "    pass\n",
         {
             NE("Exception"),
             TE("Exception", "", "", "Type"),
         }},
    };
    return example;
}

Array<TestCase> const& Raise_examples() {
    static Array<TestCase> example = {
        {"raise a from b",
         {
             NE("a"),
             NE("b"),
         }},
        {"raise a",
         TestErrors({
             NE("a"),
         })},
    };
    return example;
}

Array<TestCase> const& CondJump_examples() {
    static Array<TestCase> example = {};
    return example;
}

Array<TestCase> const& With_examples() {
    static Array<TestCase> example = {
        {"with a as b, c as d:\n"
         "    pass\n",
         {
             NE("a"),
             NE("c"),
         }},
        {"with a as b, c as d:\n"
         "    e = b + d\n"
         "    e = b + d\n"
         "    e = b + d\n",
         {
             NE("a"),
             NE("c"),
         }},
    };
    return example;
}

Array<TestCase> const& If_examples() {
    static Array<TestCase> example = {
        {"if a:\n"
         "    pass\n"
         "elif b:\n"
         "    pass\n"
         "else:\n"
         "    pass\n",
         {
             NE("a"),
             NE("b"),
         }},
    };
    return example;
}

Array<TestCase> const& While_examples() {
    static Array<TestCase> example = {
        {"while a:\n"
         "    pass\n"
         "else:\n"
         "    pass\n",
         {
             NE("a"),
         }},
    };
    return example;
}

Array<TestCase> const& For_examples() {
    static Array<TestCase> example = {
        {"for a in b:\n"
         "    a\n"
         "    b\n"
         "    c\n"
         "else:\n"
         "    pass\n",
         {
             NE("b"),
             NE("b"),
             NE("c"),
         }},
        {"for a, (b, c), d in b:\n"
         "    pass\n",
         {
             NE("b"),
         }},
    };
    return example;
}

Array<TestCase> const& AnnAssign_examples() {
    static Array<TestCase> example = {
        {
            "a: bool = True", 
            TestErrors({})
        },
        // TODO: those a lexer tests!
        // make sure "int" is not read as "in t"
        {
            "a: int = 1",
            TestErrors({
                NE("int"),
                TE("int", "", "", "Type"),
                TE("a", "int", "1", "i32"),
            }),
        },
        // make sure "isnt" is not read as "is nt"
        {
            "a: isnt = 1",
            TestErrors({
                NE("isnt"),
                TE("isnt", "", "", "Type"),
                TE("a", "isnt", "1", "i32"),
            }),
        },
        {
            "a: f32 = 2.0", 
            TestErrors({
                TE("a", "f32", "2.0", "f64")
            })
        },
    };
    return example;
}

Array<TestCase> const& AugAssign_examples() {
    static Array<TestCase> example = {
        {"a += b",
         TestErrors({
             NE("a"),
             NE("b"),
             UO("Add", "None", "None"),
         })
         },
        {"a -= b",
         {
             NE("a"),
             NE("b"),
             UO("Sub", "None", "None"),
         }},
    };
    return example;
}

Array<TestCase> const& Assign_examples() {
    static Array<TestCase> example = {
        // Undefined variables
        {"a = b",
         {
             NE("b"),
         }},
        {"a, b = c",
         {
             NE("c"),
         }},

        // Type deduction check
        {"a = 1", TestErrors({}), "i32"},
        {"a = 1.0", TestErrors({}), "f64"},
        {"a = \"str\"", TestErrors({}), "str"},

        {"a = [1, 2]", TestErrors({}), "Array[i32]"},
        {"a = [1.0, 2.0]", TestErrors({}), "Array[f64]"},
        {"a = [\"1\", \"2\"]", TestErrors({}), "Array[str]"},

        {"a = {1, 2}", TestErrors({}), "Set[i32]"},
        {"a = {1.0, 2.0}", TestErrors({}), "Set[f64]"},
        {"a = {\"1\", \"2\"}", TestErrors({}), "Set[str]"},

        {"a = {1: 1, 2: 2}", TestErrors({}), "Dict[i32, i32]"},
        {"a = {1: 1.0, 2: 2.0}", TestErrors({}), "Dict[i32, f64]"},
        {"a = {\"1\": 1, \"2\": 2}", TestErrors({}), "Dict[str, i32]"},

        {"a = 1, 2.0, \"str\"", TestErrors({}), "Tuple[i32, f64, str]"},
    };
    return example;
}

Array<TestCase> const& Delete_examples() {
    static Array<TestCase> example = {
        {"del a, b",
         {
             NE("a"),
             NE("b"),
         }},
    };
    return example;
}

Array<TestCase> const& Return_examples() {
    static Array<TestCase> example = {
        {
            "return a",
            {
                NE("a"),
            },
        },
        {
            "return 1, 2",
            TestErrors({}),
        },
        {
            "return a + b",
            {
                NE("a"),
                NE("b"),
            },
        },
        {
            "return p.x + p.y",
            {
                NE("p"),
                NE("p"),
            },
        },
    };
    return example;
}

Array<TestCase> const& ClassDef_examples() {
    static Array<TestCase> example = {
        {"@e(g, h, i=j)\n"
         "@f\n"
         "class a(b, c=d):\n"
         "    \"\"\"docstring\"\"\"\n"
         "    pass",
         {
             NE("b"),
             NE("d"),
             NE("e"),
             NC("e"),
             NE("g"),
             NE("h"),
             NE("j"),
             NE("f"),
         }},

        {
            "class Name:\n"              // c1
            "    x: i32 = 0\n"           // c2
            "    y: i32 = 1\n"           // c3
            "    z = 1.2\n"              // c4
            "\n"                         // c5
            "    def __init__(self):\n"  // c7
            "        self.x = 2\n"       // c8
            //"\n"
            //"    class Nested:\n"
            //"        xx: i32 = 0\n"
            //"\n"
        },
        // {
        //     "class Name:\n"
        //     "    x: i32 = 0\n"
        //     "    y: i32 = 1\n"
        //     "    z = 1.2\n"
        //     "\n"
        //     "    def __init__(self):\n"
        //     "        self.x = 2\n"
        //     "\n"
        //     "    def method(self):\n"
        //     "        return self.x\n"
        //     "\n"
        //     //
        // },
    };
    return example;
}

Array<TestCase> const& Comment_examples() {
    static Array<TestCase> example = {};
    return example;
}

// def function(a=1, /, b=2, c=3, *, d=3,  **kwargs):
//   return a + b + c

Array<TestCase> const& FunctionDef_examples() {
    static Array<TestCase> example = {
        // FunctionDef(
        //   name='function',
        //   args=arguments(
        //     posonlyargs=[
        //       arg(arg='b'),
        //       arg(arg='c')],
        //     args=[
        //       arg(arg='e')],
        //     vararg=arg(arg='g'),
        //     kwonlyargs=[
        //       arg(arg='h')],
        //     kw_defaults=[
        //       Constant(value=3)],
        //     kwarg=arg(arg='j'),
        //     defaults=[
        //       Constant(value=1),
        //       Constant(value=2)]),
        //   body=[
        //     Return(
        {"@j\n"
         "def a(b, c=d, /, e=f, *g, h=i, **j) -> bool:\n"
         "    \"\"\"docstring\"\"\"\n"
         "    return True",
         {
             NE("d"),
             NE("g"),
             NE("j"),
         }},

        {"@j\n"
         "def a(b, c=d, *e, f=g, **h) -> bool:\n"
         "    \"\"\"docstring\"\"\"\n"
         "    return True",
         {
             NE("d"),
             NE("g"),
             NE("j"),
         }},

        {"@j(l, m, c=n)\n"
         "@k\n"
         "def a(b: bool, d: bool = True):\n"
         "    pass",
         {
             NE("j"),
             NC("j"),
             NE("l"),
             NE("m"),
             NE("n"),
             NE("k"),
         }},
    };
    return example;
}

Array<TestCase> const& Inline_examples() {
    static Array<TestCase> example = {
        {
            "a = 2; b = c; d = e",
            {
                NE("c"),
                NE("e"),
            },

        },
    };
    return example;
}

Array<TestCase> const& FunctionType_examples() {
    static Array<TestCase> example = {};
    return example;
}

Array<TestCase> const& Expression_examples() {
    static Array<TestCase> example = {};
    return example;
}

Array<TestCase> const& Interactive_examples() {
    static Array<TestCase> example = {};
    return example;
}

Array<TestCase> const& Module_examples() {
    static Array<TestCase> example = {};
    return example;
}

Array<TestCase> const& Slice_examples() {
    static Array<TestCase> example = {
        {"a[b:c:d]",
         {
             NE("a"),
             NE("b"),
             NE("c"),
             NE("d"),
         }},
    };
    return example;
}

Array<TestCase> const& TupleExpr_examples() {
    static Array<TestCase> example = {
        {"a, b, c",
         {
             NE("a"),
             NE("b"),
             NE("c"),
         }},
        {"a, (b, c), d",
         {
             NE("a"),
             NE("b"),
             NE("c"),
             NE("d"),
         }},
        {"a, b, c = d, e, f",
         {
             NE("d"),
             NE("e"),
             NE("f"),
         }},
    };
    return example;
}

Array<TestCase> const& ListExpr_examples() {
    static Array<TestCase> example = {
        {"[a, b, c]",
         {
             NE("a"),
             NE("b"),
             NE("c"),
         }},
    };
    return example;
}

Array<TestCase> const& Name_examples() {
    static Array<TestCase> example = {
        {"a",
         {
             NE("a"),
         }},
    };
    return example;
}

Array<TestCase> const& Starred_examples() {
    static Array<TestCase> example = {
        {"*a",
         {
             NE("a"),
         }},
    };
    return example;
}

Array<TestCase> const& Subscript_examples() {
    static Array<TestCase> example = {
        {"a[b]",
         {
             NE("a"),
             NE("b"),
         }},
    };
    return example;
}

Array<TestCase> const& Attribute_examples() {
    static Array<TestCase> example = {
        {"a.b",
         {
             NE("a"),
             // NE("a"),
         }},
    };
    return example;
}

Array<TestCase> const& Constant_examples() {
    static Array<TestCase> example = {
        {"1", TestErrors({})},
        {"2.1",TestErrors({})},
        // "'str'",
        {"\"str\"", TestErrors({})},
        {"None"},
        {"True", TestErrors({})},
        {"False", TestErrors({})},
    };
    return example;
}

Array<TestCase> const& FormattedValue_examples() {
    static Array<TestCase> example = {};
    return example;
}

Array<TestCase> const& JoinedStr_examples() {
    static Array<TestCase> example = {
        //
        {
            "a = f\"str1 {b:4d}\"",
            {
                NE("b"),
            },
        },
        {
            "a = f\"str1 {b:{s}d}\"",
            {
                NE("b"),
                NE("s"),
            },
        },
        {
            "a = f\"str1 {{ {b:{s}d} }}\"",
            {
                NE("b"),
                NE("s"),
            },
        },
        // Add bad example for sema checking
    };
    return example;
}

Array<TestCase> const& Call_examples() {
    static Array<TestCase> example = {
        {
            "fun(a, b, c=d)",
            {
                NE("fun"),
                NC("fun"),
                NE("a"),
                NE("b"),
                NE("d"),
            },
            "",
        },
    };
    return example;
}

Array<TestCase> const& Compare_examples() {
    static Array<TestCase> example = {
        {"a < b > c != d",
         {
             NE("a"),
             NE("b"),
             UO("Lt", "None", "None"),
             NE("c"),
             UO("Gt", "None", "None"),
             NE("d"),
             UO("NotEq", "None", "None"),
         }},
        {"a not in b",
         {
             NE("a"),
             NE("b"),
             UO("NotIn", "None", "None"),
         }},
        {"a in b",
         {
             NE("a"),
             NE("b"),
             UO("In", "None", "None"),
         }},
        {"a is b",
         {
             NE("a"),
             NE("b"),
             UO("Is", "None", "None"),
         }},
        {"a is not b",
         {
             NE("a"),
             NE("b"),
             UO("IsNot", "None", "None"),
         }},
    };
    return example;
}

Array<TestCase> const& YieldFrom_examples() {
    static Array<TestCase> example = {
        {
            "yield from a",
            {
                NE("a"),
            },

        },
    };
    return example;
}

Array<TestCase> const& Yield_examples() {
    static Array<TestCase> example = {
        {
            "yield a",
            {
                NE("a"),
            },
        },
        {
            "yield 1, 2",
        },
        {
            "yield",
        },
    };
    return example;
}

Array<TestCase> const& Await_examples() {
    static Array<TestCase> example = {
        {"await a",
         {
             NE("a"),
         }},
    };
    return example;
}

Array<TestCase> const& DictComp_examples() {
    static Array<TestCase> example = {
        {"{a: c for a in b if a > c}",
         {
             NE("b"),
             NE("c"),
             UO("Gt", "None", "None"),
             NE("c"),
         }},
    };
    return example;
}

Array<TestCase> const& SetComp_examples() {
    static Array<TestCase> example = {
        {"{a for a in b if a > c}",
         {
             NE("b"),
             NE("c"),
             UO("Gt", "None", "None"),
         }},
    };
    return example;
}

Array<TestCase> const& GeneratorExp_examples() {
    static Array<TestCase> example = {
        {"(a for a in b if a > c)",
         {
             NE("b"),
             NE("c"),
             UO("Gt", "None", "None"),
         }},
    };
    return example;
}

Array<TestCase> const& ListComp_examples() {
    static Array<TestCase> example = {
        {"[a for a in b if a > c]",
         {
             NE("b"),
             NE("c"),
             UO("Gt", "None", "None"),
         }},
    };
    return example;
}

Array<TestCase> const& SetExpr_examples() {
    static Array<TestCase> example = {
        {"{a, b}",
         {
             NE("a"),
             NE("b"),
         }},
    };
    return example;
}

Array<TestCase> const& DictExpr_examples() {
    static Array<TestCase> example = {
        {"{a: b, c: d}",
         {
             NE("a"),
             NE("b"),
             NE("c"),
             NE("d"),
         }},
    };
    return example;
}

Array<TestCase> const& IfExp_examples() {
    static Array<TestCase> example = {
        {"a = c if True else d",
         {
             NE("c"),
             NE("d"),
         }},
        // this is the real python version
        // "a = b if c else d",
    };
    return example;
}

Array<TestCase> const& Lambda_examples() {
    static Array<TestCase> example = {
        {"lambda a: b",
         {
             NE("b"),
         }},
    };
    return example;
}

Array<TestCase> const& UnaryOp_examples() {
    static Array<TestCase> example = {
        {"+ a",
         {
             NE("a"),
             UO("UAdd", "None", "None"),
         }},
        {"- a",
         {
             NE("a"),
             UO("USub", "None", "None"),
         }},
        {"~ a",
         {
             NE("a"),
             UO("Invert", "None", "None"),
         }},
        {"! a",
         {
             NE("a"),
             UO("Not", "None", "None"),
         }},
    };
    return example;
}

Array<TestCase> const& BinOp_examples() {
    static Array<TestCase> example = {
        {"a + b",
         {
             NE("a"),
             NE("b"),
         }},
        {"a - b",
         {
             NE("a"),
             NE("b"),
         }},
        {"a * b",
         {
             NE("a"),
             NE("b"),
         }},
        {"a << b",
         {
             NE("a"),
             NE("b"),
         }},
        {"a ^ b",
         {
             NE("a"),
             NE("b"),
         }},
    };
    return example;
}

Array<TestCase> const& NamedExpr_examples() {
    static Array<TestCase> example = {
        {"a = b := c",
         {
             NE("c"),
         }},
    };
    return example;
}

Array<TestCase> const& BoolOp_examples() {
    static Array<TestCase> example = {
        {"a and b",
         {
             NE("a"),
             NE("b"),
             UO("and", "None", "None"),
         }},
        {"a or b",
         {
             NE("a"),
             NE("b"),
             UO("or", "None", "None"),
         }},
        {"a or b or c",
         {
             NE("a"),
             NE("b"),
             UO("or", "None", "None"),
             NE("c"),
             UO("or", "None", "None"),
         }},
    };
    return example;
}

Array<TestCase> const& DictType_examples() {
    static Array<TestCase> example = {};
    return example;
}

Array<TestCase> const& ArrayType_examples() {
    static Array<TestCase> example = {};
    return example;
}

Array<TestCase> const& TupleType_examples() {
    static Array<TestCase> example = {};
    return example;
}

Array<TestCase> const& Arrow_examples() {
    static Array<TestCase> example = {};
    return example;
}

Array<TestCase> const& ClassType_examples() {
    static Array<TestCase> example = {};
    return example;
}

Array<TestCase> const& SetType_examples() {
    static Array<TestCase> example = {};
    return example;
}

Array<TestCase> const& BuiltinType_examples() {
    static Array<TestCase> example = {};
    return example;
}

Array<TestCase> const& Expr_examples() {
    static Array<TestCase> example = {};
    return example;
}

Array<TestCase> const& Placeholder_examples() {
    static Array<TestCase> example = {};
    return example;
}

Array<TestCase> const& Exported_examples() {
    static Array<TestCase> example = {};
    return example;
}