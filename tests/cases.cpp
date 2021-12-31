#include "cases.h"

#include "sema/sema.h"

Array<TestCase> const &Match_examples() {
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

Array<TestCase> const &Continue_examples() {
    static Array<TestCase> ex = {
        {"continue"},
    };
    return ex;
}

Array<TestCase> const &Break_examples() {
    static Array<TestCase> ex = {
        {"break"},
    };
    return ex;
}

Array<TestCase> const &Pass_examples() {
    static Array<TestCase> ex = {
        {"pass"},
    };
    return ex;
}

Array<TestCase> const &StmtNode_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const &Nonlocal_examples() {
    static Array<TestCase> ex = {
        {"nonlocal a", {"a"}},
    };
    return ex;
}

Array<TestCase> const &Global_examples() {
    static Array<TestCase> ex = {
        {"global a", {"a"}},
    };
    return ex;
}

Array<TestCase> const &ImportFrom_examples() {
    static Array<TestCase> ex = {
        {"from a.b import c as d, e.f as g"},
    };
    return ex;
}

Array<TestCase> const &Import_examples() {
    static Array<TestCase> ex = {
        {"import a as b, c as d, e.f as g"},
    };
    return ex;
}

Array<TestCase> const &Assert_examples() {
    static Array<TestCase> ex = {
        {"assert a", {"a"}},
        {"assert a, \"b\"", {"a"}},
    };
    return ex;
}

Array<TestCase> const &Try_examples() {
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

Array<TestCase> const &Raise_examples() {
    static Array<TestCase> ex = {
        {"raise a from b", {"a", "b"}},
        {"raise a", {"a"}},
    };
    return ex;
}

Array<TestCase> const &With_examples() {
    static Array<TestCase> ex = {
        {"with a as b, c as d:\n"
         "    pass\n",
         {"a", "c"}},
    };
    return ex;
}

Array<TestCase> const &If_examples() {
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

Array<TestCase> const &While_examples() {
    static Array<TestCase> ex = {
        {"while a:\n"
         "    pass\n"
         "else:\n"
         "    pass\n",
         {"a"}},
    };
    return ex;
}

Array<TestCase> const &For_examples() {
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

Array<TestCase> const &AnnAssign_examples() {
    static Array<TestCase> ex = {
        {"a: bool = True", {}},
        // TODO: those a lexer tests!
        {"a: int = 1", {}, "", ""},  // make sure "int" is not read as "in t"
        {"a: isnt = 1", {}, "", ""}, // make sure "isnt" is not read as "is nt"
        {"a: f32 = 2.0", {}, "", "Type 2.0: f64 is not compatible with a: f32"},
    };
    return ex;
}

Array<TestCase> const &AugAssign_examples() {
    static Array<TestCase> ex = {
        {"a += b", {"a", "b"}},
        {"a -= b", {"a", "b"}},
    };
    return ex;
}

Array<TestCase> const &Assign_examples() {
    static Array<TestCase> ex = {
        // Undefined variables
        {"a = b", {"b"}},
        {"a, b = c", {"c"}},

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

Array<TestCase> const &Delete_examples() {
    static Array<TestCase> ex = {
        {"del a, b", {"a", "b"}},
    };
    return ex;
}

Array<TestCase> const &Return_examples() {
    static Array<TestCase> ex = {
        {"return a", {"a"}},
    };
    return ex;
}

Array<TestCase> const &ClassDef_examples() {
    static Array<TestCase> ex = {
        {"@e(g, h, i=j)\n"
         "@f\n"
         "class a(b, c=d):\n"
         "    \"\"\"docstring\"\"\"\n"
         "    pass",
         {"b", "d", "e", "g", "h", "j", "f"}},

        {
            "class Name:\n"
            "    x: i32 = 0\n"
            "    y: i32 = 1\n"
            "    z = 1.2\n"
            "\n"
            "    def __init__(self):\n"
            "        self.a = 2\n"
            //"\n"
            //"    class Nested:\n"
            //"        xx: i32 = 0\n"
            //"\n"
        },
    };
    return ex;
}

Array<TestCase> const &FunctionDef_examples() {
    static Array<TestCase> ex = {
        {"@j\n"
         "def a(b, c=d, *e, f=g, **h) -> bool:\n"
         "    \"\"\"docstring\"\"\"\n"
         "    return True",
         {"d", "g", "j"}},

        {"@j(l, m, c=n)\n"
         "@k\n"
         "def a(b: bool, d: bool = True):\n"
         "    pass",
         {"j", "l", "m", "n", "k"}},
    };
    return ex;
}

Array<TestCase> const &Inline_examples() {
    static Array<TestCase> ex = {
        {"a = 2; b = c; d = e", {"c", "e"}},
    };
    return ex;
}

Array<TestCase> const &FunctionType_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const &Expression_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const &Interactive_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const &Module_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const &Slice_examples() {
    static Array<TestCase> ex = {
        {"a[b:c:d]", {"a", "b", "c", "d"}},
    };
    return ex;
}

Array<TestCase> const &TupleExpr_examples() {
    static Array<TestCase> ex = {
        {"a, b, c", {"a", "b", "c"}},
        {"a, (b, c), d", {"a", "b", "c", "d"}},
        {"a, b, c = d, e, f", {"d", "e", "f"}},
    };
    return ex;
}

Array<TestCase> const &ListExpr_examples() {
    static Array<TestCase> ex = {
        {"[a, b, c]", {"a", "b", "c"}},
    };
    return ex;
}

Array<TestCase> const &Name_examples() {
    static Array<TestCase> ex = {
        {"a", {"a"}},
    };
    return ex;
}

Array<TestCase> const &Starred_examples() {
    static Array<TestCase> ex = {
        {"*a", {"a"}},
    };
    return ex;
}

Array<TestCase> const &Subscript_examples() {
    static Array<TestCase> ex = {
        {"a[b]", {"a", "b"}},
    };
    return ex;
}

Array<TestCase> const &Attribute_examples() {
    static Array<TestCase> ex = {
        {"a.b", {"a"}},
    };
    return ex;
}

Array<TestCase> const &Constant_examples() {
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

Array<TestCase> const &FormattedValue_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const &JoinedStr_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const &Call_examples() {
    static Array<TestCase> ex = {
        {"fun(a, b, c=d)", {"fun", "a", "b", "d"}},
    };
    return ex;
}

Array<TestCase> const &Compare_examples() {
    static Array<TestCase> ex = {
        {"a < b > c != d", {"a", "b", "c", "d"}},
        {"a not in b", {"a", "b"}},
        {"a in b", {"a", "b"}},
        {"a is b", {"a", "b"}},
        {"a is not b", {"a", "b"}},
    };
    return ex;
}

Array<TestCase> const &YieldFrom_examples() {
    static Array<TestCase> ex = {
        {"yield from a", {"a"}},
    };
    return ex;
}

Array<TestCase> const &Yield_examples() {
    static Array<TestCase> ex = {
        {"yield a", {"a"}},
        {"yield"},
    };
    return ex;
}

Array<TestCase> const &Await_examples() {
    static Array<TestCase> ex = {
        {"await a", {"a"}},
    };
    return ex;
}

Array<TestCase> const &DictComp_examples() {
    static Array<TestCase> ex = {
        {"{a: c for a in b if a > c}", {"b", "c", "c"}},
    };
    return ex;
}

Array<TestCase> const &SetComp_examples() {
    static Array<TestCase> ex = {
        {"{a for a in b if a > c}", {"b", "c"}},
    };
    return ex;
}

Array<TestCase> const &GeneratorExp_examples() {
    static Array<TestCase> ex = {
        {"(a for a in b if a > c)", {"b", "c"}},
    };
    return ex;
}

Array<TestCase> const &ListComp_examples() {
    static Array<TestCase> ex = {
        {"[a for a in b if a > c]", {"b", "c"}},
    };
    return ex;
}

Array<TestCase> const &SetExpr_examples() {
    static Array<TestCase> ex = {
        {"{a, b}", {"a", "b"}},
    };
    return ex;
}

Array<TestCase> const &DictExpr_examples() {
    static Array<TestCase> ex = {
        {"{a: b, c: d}", {"a", "b", "c", "d"}},
    };
    return ex;
}

Array<TestCase> const &IfExp_examples() {
    static Array<TestCase> ex = {
        {"a = if True: c else d", {"c", "d"}},
        // this is the real python version
        // "a = b if c else d",
    };
    return ex;
}

Array<TestCase> const &Lambda_examples() {
    static Array<TestCase> ex = {
        {"lambda a: b", {"b"}},
    };
    return ex;
}

Array<TestCase> const &UnaryOp_examples() {
    static Array<TestCase> ex = {
        {"+ a", {"a"}},
        {"- a", {"a"}},
        {"~ a", {"a"}},
        {"! a", {"a"}},
    };
    return ex;
}

Array<TestCase> const &BinOp_examples() {
    static Array<TestCase> ex = {
        {"a + b", {"a", "b"}},  {"a - b", {"a", "b"}}, {"a * b", {"a", "b"}},
        {"a << b", {"a", "b"}}, {"a ^ b", {"a", "b"}},
    };
    return ex;
}

Array<TestCase> const &NamedExpr_examples() {
    static Array<TestCase> ex = {
        {"a = b := c", {"c"}},
    };
    return ex;
}

Array<TestCase> const &BoolOp_examples() {
    static Array<TestCase> ex = {
        {"a and b", {"a", "b"}},
        {"a or b", {"a", "b"}},
    };
    return ex;
}

Array<TestCase> const &DictType_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const &ArrayType_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const &TupleType_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const &Arrow_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const &ClassType_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const &SetType_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const &BuiltinType_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const &Expr_examples() {
    static Array<TestCase> ex = {};
    return ex;
}