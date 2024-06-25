#include "dtypes.h"
#include "libtest.h"

namespace lython {

Array<VMTestCase> const& Match_vm_examples() {
    static Array<VMTestCase> examples = {
        // TODO: check this test case on python
        // not sure about i
        {
            "def fun(a):\n"
            "    match a:\n"
            "        case [1, 3]:\n"
            "            pass\n"
            "        case b as c:\n"
            "            return c\n"
            "        case d | e:\n"
            "            return d\n"
            "        case ClassName(f, g, h=i):\n"
            "            return f + g + i\n"
            "        case j if k:\n"
            "            return j\n",
            "fun()",
        },
        {
            "def fun(lst):\n"
            "    match lst:\n"
            "        case []:\n"
            "            pass\n"
            "        case [head, *tail]:\n"
            "            pass\n",
            "fun()",
        },
        {
            "def fun(dct):\n"
            "    match dct:\n"
            "        case {}:\n"
            "            pass\n"
            "        case {1: value, **remainder}:\n"
            "            pass\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& Pass_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun():\n"
            "    pass\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& StmtNode_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& Nonlocal_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun():\n"
            "    a: list = []\n"
            "    def _():\n"
            "       nonlocal a\n"
            "       a.append(1)\n"
            "    _()\n"
            "    return a\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& Global_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "a: i32 = 1\n"
            "def fun():\n"
            "    global a\n"
            "    a += 1\n"
            "    return a\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& ImportFrom_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "from aa.b import c as d, e.f as g",
            "fun()",
        },
        {
            // import_test should be inside the path
            "from import_test import cls as Klass, fun as Fun, ann as Ann, var as Var",
            "fun()",
        }};
    return examples;
}

Array<VMTestCase> const& Import_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "import aa as b, c as d, e.f as g",
            "fun()",
        },
        {
            // import_test should be inside the path
            "import import_test as imp_test",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& Assert_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: int):\n"
            "    assert a\n",
            "fun()",
        },
        {
            "def fun(a: int):\n"
            "    assert a, \"b\"",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& Try_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun():\n"
            "    try:\n"
            "        pass\n"
            "    except Exception as b:\n"
            "        pass\n"
            "    else:\n"
            "        pass\n"
            "    finally:\n"
            "        pass\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& Raise_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun():\n"
            "    raise a from b\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    raise a\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& With_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: int, c: int):\n"
            "    with a as b, c as d:\n"
            "        pass\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    with a as b, c as d:\n"
            "        e = b + d\n"
            "        e = b + d\n"
            "        e = b + d\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& If_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: bool, b: bool) -> f64:\n"
            "    if a:\n"
            "        return 0.0\n"
            "    elif b:\n"
            "        return 1.0\n"
            "    else:\n"
            "        return 3.0\n"
            "    return 2.0\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& While_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(limit: i32):\n"
            "    a = 0\n"
            "    while a < limit:\n"
            "        a += 1\n"
            "    else:\n"
            "        pass\n",
            "fun(10)",
        },
    };
    return examples;
}

Array<VMTestCase> const& Continue_vm_examples() {
    static Array<VMTestCase> examples = {
        // {
        //     "def fun(b):\n"
        //     "    sum = 0\n"
        //     "    for a in b:\n"
        //     "        if a:\n"
        //     "            continue\n"
        //     "        sum +=1\n"
        //     "    return sum\n",
        //     "fun()",
        // },
    };
    return examples;
}

Array<VMTestCase> const& Break_vm_examples() {
    static Array<VMTestCase> examples = {
        // {
        //     "def fun(b):\n"
        //     "    sum = 0\n"
        //     "    for a in b:\n"
        //     "        if a:\n"
        //     "            break\n"
        //     "        sum +=1\n"
        //     "    return sum\n",
        //     "fun()",
        // },
    };
    return examples;
}

Array<VMTestCase> const& For_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: i32) -> i32:"
            "    acc = 0\n"
            "    for a in range(10):\n"
            "        acc += a\n"
            "    else:\n"
            "        pass\n"
            "    return acc\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    for a, (b, c), d in b:\n"
            "        pass\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& AnnAssign_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun() -> bool:\n"
            "    a: bool = True\n"
            "    return a\n",
            "fun()",
        },
        // TODO: those a lexer tests!
        // make sure "int" is not read as "in t"
        // make sure isnot is not read as is not
        {
            "def fun() -> i32:\n"
            "    a: i32 = 1\n"
            "    return a\n",
            "fun()",
        },
        {
            "def fun() -> f64:\n"
            "    a: f64 = 2.0\n"
            "    return a\n",

            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& AugAssign_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: i32, b: i32) -> i32:\n"
            "    a += b\n"
            "    return a\n",
            "fun(2, 1)",
        },
        {
            "def fun(a: i32, b: i32) -> i32:\n"
            "    a -= b\n"
            "    return a\n",
            "fun(2, 1)",
        },
    };
    return examples;
}

Array<VMTestCase> const& Assign_vm_examples() {
    static Array<VMTestCase> examples = {
        // Undefined variables
        {
            "def fun(b: i32) -> i32:\n"
            "    a = b\n"
            "    return a\n",
            "fun(1)",
        },
        {
            "def fun(c: Tuple[i32, i32]) -> i32:\n"
            "    a, b = c\n"
            "    return a\n",
            "fun((1, 2))",
        },

        // Type deduction check
        {
            "def fun() -> i32:\n"
            "    a = 1\n"
            "    return a\n",
            "fun()",
            "i32",
        },
        {
            "def fun():\n"
            "    a = 1.0\n"
            "    return a\n",
            "fun()",
            "f64",
        },
        // {
        //     "def fun():\n"
        //     "    a = \"str\"\n"
        //     "    return a\n",
        //     "fun()",
        //     "str",
        // },
        // {
        //     "def fun():\n"
        //     "    a = [1, 2]\n"
        //     "    return a\n",
        //     "fun()",
        //     "Array[i32]",
        // },
        // {
        //     "def fun():\n"
        //     "    a = [1.0, 2.0]\n"
        //     "    return a\n",
        //     "fun()",
        //     "Array[f64]",
        // },
        // {
        //     "def fun():\n"
        //     "    a = [\"1\", \"2\"]\n"
        //     "    return a\n",
        //     "fun()",
        //     "Array[str]",
        // },
        // {
        //     "def fun():\n"
        //     "    a = {1, 2}\n"
        //     "    return a\n",
        //     "fun()",
        //     "Set[i32]",
        // },
        // {
        //     "def fun():\n"
        //     "    a = {1.0, 2.0}\n"
        //     "    return a\n",
        //     "fun()",
        //     "Set[f64]",
        // },
        // {
        //     "def fun():\n"
        //     "    a = {\"1\", \"2\"}\n"
        //     "    return a\n",
        //     "fun()",
        //     "Set[str]",
        // },
        // {
        //     "def fun():\n"
        //     "    a = {1: 1, 2: 2}\n"
        //     "    return a\n",
        //     "fun()",
        //     "Dict[i32, i32]",
        // },
        // {
        //     "def fun():\n"
        //     "    a = {1: 1.0, 2: 2.0}\n"
        //     "    return a\n",
        //     "fun()",
        //     "Dict[i32, f64]",
        // },
        // {
        //     "def fun():\n"
        //     "    a = {\"1\": 1, \"2\": 2}\n"
        //     "    return a\n",
        //     "fun()",
        //     "Dict[str, i32]",
        // },
        // {
        //     "def fun():\n"
        //     "    a = 1, 2.0, \"str\"\n"
        //     "    return a\n",
        //     "fun()",
        //     "Tuple[i32, f64, str]",
        // },
    };
    return examples;
}

Array<VMTestCase> const& Delete_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: i32, b: i32):\n"
            "    del a, b\n",
            "fun(1, 2)",
        },
    };
    return examples;
}

Array<VMTestCase> const& Return_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: i32) -> i32:\n"
            "    return a\n",
            "fun()",
        },
        {
            "def fun() -> Tuple[i32, i32]:\n"
            "    return 1, 2\n",
            "fun()",
        },
        {
            "def fun() -> i32:\n"
            "    return 1 + 2\n",
            "fun()",
        }
    };
    return examples;
}

Array<VMTestCase> const& ClassDef_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "@e(g, h, i=j)\n"
            "@f\n"
            "class a(b, c=d):\n"
            "    \"\"\"docstring\"\"\"\n"
            "    pass",
            "fun()",
        },

        {
            "class Name:\n"              // c1
            "    x: i32 = 0\n"           // c2
            "    y: i32 = 1\n"           // c3
            "    z = 1.2\n"              // c4
            "\n"                         // c5
            "    def __init__(self):\n"  // c7
            "        self.x = 2\n"       // c8
            "def fun():\n"
            "    return Name().x\n",
            "fun()",
            //"\n"
            //"    class Nested:\n"
            //"        xx: i32 = 0\n"
            //"\n"
        }
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
    return examples;
}

Array<VMTestCase> const& Comment_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& FunctionDef_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "@j\n"
            "def a(b, c=d, *e, f=g, **h) -> bool:\n"
            "    \"\"\"docstring\"\"\"\n"
            "    return True\n",
            "fun()",
        },

        {
            "@j(l, m, c=n)\n"
            "@k\n"
            "def a(b: bool, d: bool = True):\n"
            "    pass\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& Inline_vm_examples() {
    static Array<VMTestCase> examples = {
        // {
        //     "def fun(c: i32, e: i32):\n"
        //     "    a = 2; b = c; d = e\n"
        //     "    return a, b, d\n",
        //     "fun()",

        // },
    };
    return examples;
}

Array<VMTestCase> const& FunctionType_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& Expression_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& Interactive_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& Module_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& Slice_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: list[i32], s: i32, step: i32, e: i32):\n"
            "    return a[s:step:e]\n",
            "fun([1, 2, 3, 4, 5, 6], 0, 2 ,4)",
        },
    };
    return examples;
}

Array<VMTestCase> const& TupleExpr_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: i32) -> Tuple[i32, i32, i32]:\n"
            "    return a, a, a\n",
            "fun(1)",
        },
        {
            "def fun(v: Tuple[i32, Tuple[i32, i32], i32]):\n"
            "    a, (b, c), d = v\n"
            "    return b\n",
            "fun((1, (2, 3), 4))",
        },
        // Unpacking
        {
            "def fun(v: Tuple[i32, i32]) -> i32:\n"
            "    d, e, f = v\n"
            "    a, b, c = d, e, f\n"
            "    return b\n",
            "fun((1, 2, 3))",
        },
    };
    return examples;
}

Array<VMTestCase> const& ListExpr_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: i32) -> list[i32]:\n"
            "    return [a, a, a]\n",
            "fun(1)",
        },
    };
    return examples;
}

Array<VMTestCase> const& Name_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: i32) -> i32:\n"
            "    return a\n",
            "fun(1)",
        },
    };
    return examples;
}

Array<VMTestCase> const& Starred_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: list[i32]) -> list[i32]:\n"
            "    return *a\n",
            "fun([1, 2])",
        },
    };
    return examples;
}

Array<VMTestCase> const& Subscript_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: list[i32]) -> i32:\n"
            "    return a[0]\n",
            "fun([1, 2])",
        },
    };
    return examples;
}

Array<VMTestCase> const& Attribute_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "class Point:\n"
            "    x: i32\n"
            "    y: i32\n"
            "\n"
            "def fun(a: Point) -> i32:\n"
            "    return a.x\n",
            "fun(Point())",
        },
    };
    return examples;
}

Array<VMTestCase> const& Constant_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun():\n"
            "    return 1\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    return 2.1\n",
            "fun()",
        },
        // "'str'",
        // {
        //     "def fun():\n"
        //     "    return \"str\"\n",
        //     "fun()",
        // },
        {
            "def fun():\n"
            "    return None\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    return True\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    return False\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& FormattedValue_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& JoinedStr_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& Call_vm_examples() {
    static Array<VMTestCase> examples = {{
        "def myfunction(a: f64, b: f64) -> f64:\n"
        "    return a + b\n"
        "\n"
        "def fun():\n"
        "    return myfunction(1.0, 2.0)\n",
        "fun()",
    }

    };
    return examples;
}

Array<VMTestCase> const& Compare_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: i32, b: i32, c: i32, d: i32) -> bool:\n"
            "    return a < b > c != d\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    return a not in b\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    return a in b\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    return a is b\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    return a is not b\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& YieldFrom_vm_examples() {
    static Array<VMTestCase> examples = {
        // {
        //     "def fun():\n"
        //     "    yield from a\n",
        //     "fun()",
        // },
    };
    return examples;
}

Array<VMTestCase> const& Yield_vm_examples() {
    static Array<VMTestCase> examples = {
        // {
        //     "def fun(a):\n"
        //     "    yield a\n",
        //     "fun()",
        // },
        // {
        //     "def fun():\n"
        //     "    yield 1, 2\n",
        //     "fun()",
        // },
        // {
        //     "def fun():\n"
        //     "    yield\n",
        //     "fun()",
        // },
    };
    return examples;
}

Array<VMTestCase> const& Await_vm_examples() {
    static Array<VMTestCase> examples = {
        // {
        //     "def fun(a: int):\n"
        //     "    await a\n",
        //     "fun()",
        // },
    };
    return examples;
}

Array<VMTestCase> const& DictComp_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(b: i32):\n"
            "    return {a: a for a in range(10) if a > b}\n",
            "fun(2)",
        },
    };
    return examples;
}

Array<VMTestCase> const& SetComp_vm_examples() {
    static Array<VMTestCase> examples = {
        {"def fun(b: i32):\n"
         "    return {a for a in range(10) if a > b}\n",
         "fun(2)"},
    };
    return examples;
}

Array<VMTestCase> const& GeneratorExp_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(b: i32) -> list[i32]:\n"
            "    return (a for a in range(10) if a > b)\n",
            "fun(2)",
        },
    };
    return examples;
}

Array<VMTestCase> const& ListComp_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(b: i32) -> list[i32]:\n"
            "    return [a for a in range(10) if a > b]\n",
            "fun(2)",
        },
    };
    return examples;
}

Array<VMTestCase> const& SetExpr_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun():\n"
            "    return {1, 2}\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& DictExpr_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun():\n"
            "    return {1: 2, 3: 4}\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& IfExp_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: i32, b: i32) -> i32:\n"
            "    return a if True else b\n",
            "fun(1, 2)",
        },
        // this is the real python version
        // "a = b if c else d",
    };
    return examples;
}

Array<VMTestCase> const& Lambda_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "fun = lambda a: a + 1\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& UnaryOp_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: i32) -> i32:\n"
            "    return + a\n",
            "fun()",
        },
        {
            "def fun(a: i32) -> i32:\n"
            "    return - a\n",
            "fun()",
        },
        {
            "def fun(a: i32) -> i32:\n"
            "    return ~ a\n",
            "fun()",
        },
        {
            "def fun(a: i32) -> i32:\n"
            "    return ! a\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& BinOp_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: f64, b: f64) -> f64:\n"
            "    return a + b\n",
            "fun()",
        },
        {
            "def fun(a: f64, b: f64) -> f64:\n"
            "    return a - b\n",
            "fun()",
        },
        {
            "def fun(a: f64, b: f64) -> f64:\n"
            "    return a * b\n",
            "fun()",
        },
        {
            "def fun(a: i32, b: i32) -> i32:\n"
            "    return a << b\n",
            "fun()",
        },
        {
            "def fun(a: i32, b: i32) -> i32:\n"
            "    return a ^ b\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& NamedExpr_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(c: i32):\n"
            "    return a := c\n",
            "fun()",
        },
    };
    return examples;
}

Array<VMTestCase> const& BoolOp_vm_examples() {
    static Array<VMTestCase> examples = {
        {
            "def fun(a: bool, b: bool) -> bool:\n"
            "    return a and b\n",
            "fun(True, False)",
        },
        {
            "def fun(a: bool, b: bool) -> bool:\n"
            "    return a or b\n",
            "fun(True, False)",
        },
        {
            "def fun(a: bool, b: bool, c: bool) -> bool:\n"
            "    return a or b or c\n",
            "fun(True, False, False)",
        },
    };
    return examples;
}

Array<VMTestCase> const& DictType_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& ArrayType_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& TupleType_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& Arrow_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& ClassType_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& SetType_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& BuiltinType_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& Expr_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& InvalidStatement_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& Placeholder_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& Exported_vm_examples() {
    static Array<VMTestCase> examples = {};
    return examples;
}

Array<VMTestCase> const& CondJump_vm_examples() {
    static Array<TestCase> example = {};
    return example;
}


}  // namespace lython