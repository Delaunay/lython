#include "dtypes.h"

namespace lython {

struct VMTestCase {
    VMTestCase(String const& c, String const& call, String const& t = ""):
        code(c), call(call), expected_type(t) {}

    String code;
    String call;
    String expected_type;
};

Array<VMTestCase> const& Match_vm_examples() {
    static Array<VMTestCase> ex = {
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
    return ex;
}

Array<VMTestCase> const& Pass_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    pass\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& StmtNode_vm_examples() {
    static Array<VMTestCase> ex = {};
    return ex;
}

Array<VMTestCase> const& Nonlocal_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    a: list = []"
            "    def _():\n"
            "       nonlocal a\n"
            "       a.append(1)\n"
            "    _()\n"
            "    return a\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& Global_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "a: int = 1"
            "def fun():\n"
            "    global a\n"
            "    a += 1\n"
            "    return a\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& ImportFrom_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "from aa.b import c as d, e.f as g",
            "fun()",
        },
        {
            // import_test should be inside the path
            "from import_test import cls as Klass, fun as Fun, ann as Ann, var as Var",
            "fun()",
        }};
    return ex;
}

Array<VMTestCase> const& Import_vm_examples() {
    static Array<VMTestCase> ex = {
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
    return ex;
}

Array<VMTestCase> const& Assert_vm_examples() {
    static Array<VMTestCase> ex = {
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
    return ex;
}

Array<VMTestCase> const& Try_vm_examples() {
    static Array<VMTestCase> ex = {
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
    return ex;
}

Array<VMTestCase> const& Raise_vm_examples() {
    static Array<VMTestCase> ex = {
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
    return ex;
}

Array<VMTestCase> const& With_vm_examples() {
    static Array<VMTestCase> ex = {
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
    return ex;
}

Array<VMTestCase> const& If_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun(a: int, b: int):\n"
            "    if a:\n"
            "        pass\n"
            "    elif b:\n"
            "        pass\n"
            "    else:\n"
            "        pass\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& While_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    while a:\n"
            "        pass\n"
            "    else:\n"
            "        pass\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& Continue_vm_examples() {
    static Array<VMTestCase> ex = {
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
    return ex;
}

Array<VMTestCase> const& Break_vm_examples() {
    static Array<VMTestCase> ex = {
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
    return ex;
}

Array<VMTestCase> const& For_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun(a):"
            "    for a in b:\n"
            "        a\n"
            "        b\n"
            "        c\n"
            "    else:\n"
            "        pass\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    for a, (b, c), d in b:\n"
            "        pass\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& AnnAssign_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    a: bool = True\n"
            "    return a\n",
            "fun()",
        },
        // TODO: those a lexer tests!
        // make sure "int" is not read as "in t"
        {
            "def fun():\n"
            "    a: int = 1\n"
            "    return a\n",
            "fun()",
        },
        // make sure "isnt" is not read as "is nt"
        {
            "def fun():\n"
            "    a: isnt = 1\n"
            "    return a\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    a: f32 = 2.0\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& AugAssign_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    a += b\n"
            "    return a\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    a -= b\n"
            "    return a\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& Assign_vm_examples() {
    static Array<VMTestCase> ex = {
        // Undefined variables
        {
            "def fun():\n"
            "    a = b\n"
            "    return a\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    a, b = c\n"
            "    return a\n",
            "fun()",
        },

        // Type deduction check
        {
            "def fun():\n"
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
    return ex;
}

Array<VMTestCase> const& Delete_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    del a, b\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& Return_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    return a\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    return 1, 2\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    return a + b\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    return p.x + p.y\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& ClassDef_vm_examples() {
    static Array<VMTestCase> ex = {
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
            "        self.x = 2\n",      // c8
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
    return ex;
}

Array<VMTestCase> const& Comment_vm_examples() {
    static Array<VMTestCase> ex = {};
    return ex;
}

Array<VMTestCase> const& FunctionDef_vm_examples() {
    static Array<VMTestCase> ex = {
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
    return ex;
}

Array<VMTestCase> const& Inline_vm_examples() {
    static Array<VMTestCase> ex = {
        // {
        //     "def fun(c: i32, e: i32):\n"
        //     "    a = 2; b = c; d = e\n"
        //     "    return a, b, d\n",
        //     "fun()",

        // },
    };
    return ex;
}

Array<VMTestCase> const& FunctionType_vm_examples() {
    static Array<VMTestCase> ex = {};
    return ex;
}

Array<VMTestCase> const& Expression_vm_examples() {
    static Array<VMTestCase> ex = {};
    return ex;
}

Array<VMTestCase> const& Interactive_vm_examples() {
    static Array<VMTestCase> ex = {};
    return ex;
}

Array<VMTestCase> const& Module_vm_examples() {
    static Array<VMTestCase> ex = {};
    return ex;
}

Array<VMTestCase> const& Slice_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    return a[b:c:d]\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& TupleExpr_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    return a, b, c\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    a, (b, c), d\n"
            "    return \n",
            "fun()",
        },
        {
            "def fun():\n"
            "    a, b, c = d, e, f\n"
            "    return \n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& ListExpr_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    return [a, b, c]\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& Name_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    return a\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& Starred_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    return *a\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& Subscript_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    return a[b]\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& Attribute_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    return a.b\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& Constant_vm_examples() {
    static Array<VMTestCase> ex = {
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
        {
            "def fun():\n"
            "    return \"str\"\n",
            "fun()",
        },
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
    return ex;
}

Array<VMTestCase> const& FormattedValue_vm_examples() {
    static Array<VMTestCase> ex = {};
    return ex;
}

Array<VMTestCase> const& JoinedStr_vm_examples() {
    static Array<VMTestCase> ex = {};
    return ex;
}

Array<VMTestCase> const& Call_vm_examples() {
    static Array<VMTestCase> ex = {

    };
    return ex;
}

Array<VMTestCase> const& Compare_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
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
    return ex;
}

Array<VMTestCase> const& YieldFrom_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    yield from a\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& Yield_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun(a):\n"
            "    yield a\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    yield 1, 2\n",
            "fun()",
        },
        {
            "def fun():\n"
            "    yield\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& Await_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun(a: int):\n"
            "    await a\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& DictComp_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    return {a: c for a in b if a > c}\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& SetComp_vm_examples() {
    static Array<VMTestCase> ex = {
        {"def fun():\n"
         "    return {a for a in b if a > c}\n",
         "fun()"},
    };
    return ex;
}

Array<VMTestCase> const& GeneratorExp_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    return (a for a in b if a > c)\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& ListComp_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    return [a for a in b if a > c]\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& SetExpr_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    return {a, b}\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& DictExpr_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun():\n"
            "    return {a: b, c: d}\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& IfExp_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun(c: i32, b: i32):\n"
            "    return a = c if True else d\n",
            "fun()",
        },
        // this is the real python version
        // "a = b if c else d",
    };
    return ex;
}

Array<VMTestCase> const& Lambda_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "fun = lambda a: a + 1\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& UnaryOp_vm_examples() {
    static Array<VMTestCase> ex = {
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
    return ex;
}

Array<VMTestCase> const& BinOp_vm_examples() {
    static Array<VMTestCase> ex = {
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
            "def fun(a: f64, b: f64) -> f64:\n"
            "    return a << b\n",
            "fun()",
        },
        {
            "def fun(a: f64, b: f64) -> f64:\n"
            "    return a ^ b\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& NamedExpr_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun(c: i32):\n"
            "    return a = b := c\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& BoolOp_vm_examples() {
    static Array<VMTestCase> ex = {
        {
            "def fun(a: bool, b: bool) -> bool:\n"
            "    return a and b\n",
            "fun()",
        },
        {
            "def fun(a: bool, b: bool) -> bool:\n"
            "    return a or b\n",
            "fun()",
        },
        {
            "def fun(a: bool, b: bool, c: bool) -> bool:\n"
            "    return a or b or c\n",
            "fun()",
        },
    };
    return ex;
}

Array<VMTestCase> const& DictType_vm_examples() {
    static Array<VMTestCase> ex = {};
    return ex;
}

Array<VMTestCase> const& ArrayType_vm_examples() {
    static Array<VMTestCase> ex = {};
    return ex;
}

Array<VMTestCase> const& TupleType_vm_examples() {
    static Array<VMTestCase> ex = {};
    return ex;
}

Array<VMTestCase> const& Arrow_vm_examples() {
    static Array<VMTestCase> ex = {};
    return ex;
}

Array<VMTestCase> const& ClassType_vm_examples() {
    static Array<VMTestCase> ex = {};
    return ex;
}

Array<VMTestCase> const& SetType_vm_examples() {
    static Array<VMTestCase> ex = {};
    return ex;
}

Array<VMTestCase> const& BuiltinType_vm_examples() {
    static Array<VMTestCase> ex = {};
    return ex;
}

Array<VMTestCase> const& Expr_vm_examples() {
    static Array<VMTestCase> ex = {};
    return ex;
}

Array<VMTestCase> const& InvalidStatement_vm_examples() {
    static Array<VMTestCase> ex = {};
    return ex;
}
}  // namespace lython