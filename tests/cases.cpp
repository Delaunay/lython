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

Array<TestCase> const& Match_examples() {
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
         {
             NE("a"),
             NE("ClassName"),
             NE("k"),
         }},

        {"match lst:\n"
         "    case []:\n"
         "        pass\n"
         "    case [head, *tail]:\n"
         "        pass\n",
         {
             NE("lst"),
         }},

        {"match dct:\n"
         "    case {}:\n"
         "        pass\n"
         "    case {1: value, **remainder}:\n"
         "        pass\n",
         {
             NE("dct"),
         }},
    };
    return ex;
}

Array<TestCase> const& Continue_examples() {
    static Array<TestCase> ex = {
        {"continue"},
    };
    return ex;
}

Array<TestCase> const& Break_examples() {
    static Array<TestCase> ex = {
        {"break"},
    };
    return ex;
}

Array<TestCase> const& Pass_examples() {
    static Array<TestCase> ex = {
        {"pass"},
    };
    return ex;
}

Array<TestCase> const& StmtNode_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Nonlocal_examples() {
    static Array<TestCase> ex = {
        {"nonlocal a",
         {
             NE("a"),
         }},
    };
    return ex;
}

Array<TestCase> const& Global_examples() {
    static Array<TestCase> ex = {
        {"global a",
         {
             NE("a"),
         }},
    };
    return ex;
}

Array<TestCase> const& ImportFrom_examples() {
    static Array<TestCase> ex = {
        {
            "from aa.b import c as d, e.f as g",
            {
                MNFE("aa.b"),
            },
        },
        {
            // import_test should be inside the path
            "from import_test import cls as Klass, fun as Fun, ann as Ann, var as Var",
            {},
        }};
    return ex;
}

Array<TestCase> const& Import_examples() {
    static Array<TestCase> ex = {
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
            {},
        },
    };
    return ex;
}

Array<TestCase> const& Assert_examples() {
    static Array<TestCase> ex = {
        {"assert a",
         {
             NE("a"),
         }},
        {"assert a, \"b\"",
         {
             NE("a"),
         }},
    };
    return ex;
}

Array<TestCase> const& Try_examples() {
    static Array<TestCase> ex = {
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
    return ex;
}

Array<TestCase> const& Raise_examples() {
    static Array<TestCase> ex = {
        {"raise a from b",
         {
             NE("a"),
             NE("b"),
         }},
        {"raise a",
         {
             NE("a"),
         }},
    };
    return ex;
}

Array<TestCase> const& With_examples() {
    static Array<TestCase> ex = {
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
    return ex;
}

Array<TestCase> const& If_examples() {
    static Array<TestCase> ex = {
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
    return ex;
}

Array<TestCase> const& While_examples() {
    static Array<TestCase> ex = {
        {"while a:\n"
         "    pass\n"
         "else:\n"
         "    pass\n",
         {
             NE("a"),
         }},
    };
    return ex;
}

Array<TestCase> const& For_examples() {
    static Array<TestCase> ex = {
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
    return ex;
}

Array<TestCase> const& AnnAssign_examples() {
    static Array<TestCase> ex = {
        {"a: bool = True", {}},
        // TODO: those a lexer tests!
        // make sure "int" is not read as "in t"
        {
            "a: int = 1",
            {
                NE("int"),
                TE("int", "", "", "Type"),
                TE("a", "int", "1", "i32"),
            },
        },
        // make sure "isnt" is not read as "is nt"
        {"a: isnt = 1",
         {
             NE("isnt"),
             TE("isnt", "", "", "Type"),
             TE("a", "isnt", "1", "i32"),
         },
         ""},
        {"a: f32 = 2.0", {TE("a", "f32", "2.0", "f64")}, ""},
    };
    return ex;
}

Array<TestCase> const& AugAssign_examples() {
    static Array<TestCase> ex = {
        {"a += b",
         {
             NE("a"),
             NE("b"),
             UO("Add", "None", "None"),
         }},
        {"a -= b",
         {
             NE("a"),
             NE("b"),
             UO("Sub", "None", "None"),
         }},
    };
    return ex;
}

Array<TestCase> const& Assign_examples() {
    static Array<TestCase> ex = {
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
        {"a = 1", {}, "i32"},
        {"a = 1.0", {}, "f64"},
        {"a = \"str\"", {}, "str"},

        {"a = [1, 2]", {}, "Array[i32]"},
        {"a = [1.0, 2.0]", {}, "Array[f64]"},
        {"a = [\"1\", \"2\"]", {}, "Array[str]"},

        {"a = {1, 2}", {}, "Set[i32]"},
        {"a = {1.0, 2.0}", {}, "Set[f64]"},
        {"a = {\"1\", \"2\"}", {}, "Set[str]"},

        {"a = {1: 1, 2: 2}", {}, "Dict[i32, i32]"},
        {"a = {1: 1.0, 2: 2.0}", {}, "Dict[i32, f64]"},
        {"a = {\"1\": 1, \"2\": 2}", {}, "Dict[str, i32]"},

        {"a = 1, 2.0, \"str\"", {}, "Tuple[i32, f64, str]"},
    };
    return ex;
}

Array<TestCase> const& Delete_examples() {
    static Array<TestCase> ex = {
        {"del a, b",
         {
             NE("a"),
             NE("b"),
         }},
    };
    return ex;
}

Array<TestCase> const& Return_examples() {
    static Array<TestCase> ex = {
        {
            "return a",
            {
                NE("a"),
            },
        },
        {
            "return 1, 2",
            {},
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
    return ex;
}

Array<TestCase> const& ClassDef_examples() {
    static Array<TestCase> ex = {
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
    return ex;
}

Array<TestCase> const& Comment_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& FunctionDef_examples() {
    static Array<TestCase> ex = {
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
    return ex;
}

Array<TestCase> const& Inline_examples() {
    static Array<TestCase> ex = {
        {
            "a = 2; b = c; d = e",
            {
                NE("c"),
                NE("e"),
            },

        },
    };
    return ex;
}

Array<TestCase> const& FunctionType_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Expression_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Interactive_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Module_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Slice_examples() {
    static Array<TestCase> ex = {
        {"a[b:c:d]",
         {
             NE("a"),
             NE("b"),
             NE("c"),
             NE("d"),
         }},
    };
    return ex;
}

Array<TestCase> const& TupleExpr_examples() {
    static Array<TestCase> ex = {
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
    return ex;
}

Array<TestCase> const& ListExpr_examples() {
    static Array<TestCase> ex = {
        {"[a, b, c]",
         {
             NE("a"),
             NE("b"),
             NE("c"),
         }},
    };
    return ex;
}

Array<TestCase> const& Name_examples() {
    static Array<TestCase> ex = {
        {"a",
         {
             NE("a"),
         }},
    };
    return ex;
}

Array<TestCase> const& Starred_examples() {
    static Array<TestCase> ex = {
        {"*a",
         {
             NE("a"),
         }},
    };
    return ex;
}

Array<TestCase> const& Subscript_examples() {
    static Array<TestCase> ex = {
        {"a[b]",
         {
             NE("a"),
             NE("b"),
         }},
    };
    return ex;
}

Array<TestCase> const& Attribute_examples() {
    static Array<TestCase> ex = {
        {"a.b",
         {
             NE("a"),
             // NE("a"),
         }},
    };
    return ex;
}

Array<TestCase> const& Constant_examples() {
    static Array<TestCase> ex = {
        {"1", {}},
        {"2.1", {}},
        // "'str'",
        {"\"str\"", {}},
        {"None"},
        {"True", {}},
        {"False", {}},
    };
    return ex;
}

Array<TestCase> const& FormattedValue_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& JoinedStr_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Call_examples() {
    static Array<TestCase> ex = {
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
    return ex;
}

Array<TestCase> const& Compare_examples() {
    static Array<TestCase> ex = {
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
    return ex;
}

Array<TestCase> const& YieldFrom_examples() {
    static Array<TestCase> ex = {
        {
            "yield from a",
            {
                NE("a"),
            },

        },
    };
    return ex;
}

Array<TestCase> const& Yield_examples() {
    static Array<TestCase> ex = {
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
    return ex;
}

Array<TestCase> const& Await_examples() {
    static Array<TestCase> ex = {
        {"await a",
         {
             NE("a"),
         }},
    };
    return ex;
}

Array<TestCase> const& DictComp_examples() {
    static Array<TestCase> ex = {
        {"{a: c for a in b if a > c}",
         {
             NE("b"),
             NE("c"),
             UO("Gt", "None", "None"),
             NE("c"),
         }},
    };
    return ex;
}

Array<TestCase> const& SetComp_examples() {
    static Array<TestCase> ex = {
        {"{a for a in b if a > c}",
         {
             NE("b"),
             NE("c"),
             UO("Gt", "None", "None"),
         }},
    };
    return ex;
}

Array<TestCase> const& GeneratorExp_examples() {
    static Array<TestCase> ex = {
        {"(a for a in b if a > c)",
         {
             NE("b"),
             NE("c"),
             UO("Gt", "None", "None"),
         }},
    };
    return ex;
}

Array<TestCase> const& ListComp_examples() {
    static Array<TestCase> ex = {
        {"[a for a in b if a > c]",
         {
             NE("b"),
             NE("c"),
             UO("Gt", "None", "None"),
         }},
    };
    return ex;
}

Array<TestCase> const& SetExpr_examples() {
    static Array<TestCase> ex = {
        {"{a, b}",
         {
             NE("a"),
             NE("b"),
         }},
    };
    return ex;
}

Array<TestCase> const& DictExpr_examples() {
    static Array<TestCase> ex = {
        {"{a: b, c: d}",
         {
             NE("a"),
             NE("b"),
             NE("c"),
             NE("d"),
         }},
    };
    return ex;
}

Array<TestCase> const& IfExp_examples() {
    static Array<TestCase> ex = {
        {"a = c if True else d",
         {
             NE("c"),
             NE("d"),
         }},
        // this is the real python version
        // "a = b if c else d",
    };
    return ex;
}

Array<TestCase> const& Lambda_examples() {
    static Array<TestCase> ex = {
        {"lambda a: b",
         {
             NE("b"),
         }},
    };
    return ex;
}

Array<TestCase> const& UnaryOp_examples() {
    static Array<TestCase> ex = {
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
    return ex;
}

Array<TestCase> const& BinOp_examples() {
    static Array<TestCase> ex = {
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
    return ex;
}

Array<TestCase> const& NamedExpr_examples() {
    static Array<TestCase> ex = {
        {"a = b := c",
         {
             NE("c"),
         }},
    };
    return ex;
}

Array<TestCase> const& BoolOp_examples() {
    static Array<TestCase> ex = {
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
    return ex;
}

Array<TestCase> const& DictType_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& ArrayType_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& TupleType_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Arrow_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& ClassType_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& SetType_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& BuiltinType_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Expr_examples() {
    static Array<TestCase> ex = {};
    return ex;
}