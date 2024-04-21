Array<VMTestCase> const& For_vm_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("FunctionDef"),
            "def fun(a: i32) -> i32:\n"
            "    return a\n",
            "fun(1)",
            "1"
        },
        {
            TestName("VM_FunctionDef_with_temporaries"),
            "def fun(a: i32, b: i32) -> i32:\n"
            "    b += 1\n"
            "    if a == 0:\n"
            "        return b\n"
            "    return fun(a - 1, b)\n",
            "fun(10, 0)",
            "11"
        },
        {
            TestName("VM_FunctionDef_recursive"),
            "def fun(a: i32) -> i32:\n"
            "    if a == 0:\n"
            "        return 0\n"
            "    return fun(a - 1) + 1\n",
            "fun(10)",
            "10"
        }
        // {
        //     TestName("FunctionDef_unpack"),
        //     "def fun():\n"
        //     "    for a, (b, c), d in b:\n"
        //     "        pass\n",
        //     "fun()",
        // },
    };
    return examples;
}


Array<VMTestCase> const& vm_assign_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("VM_assign"),
"def fun(a: i32) -> i32:\n"
                  "    b = 3\n"
                  "    b = a * b\n"
                  "    return b\n"
                  "",
                  "fun(2)",
                  "6"
        }
    };
    return examples;
}

Array<VMTestCase> const& vm_pass_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("VM_pass"),
                "def fun(a: i32) -> i32:\n"
                  "    while False:\n"
                  "        pass\n"
                  "    return 0\n",
                  "fun(0)",
                  "0"
        }
    };
    return examples;
}

Array<VMTestCase> const& vm_inlinestmt_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("VM_inline_stmt"),
                "def fun(a: i32) -> i32:\n"
                "    a += 1; return a\n",
                "fun(0)",
                "1"
        }
    };
    return examples;
}



Array<VMTestCase> const& vm_unary_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("VM_assert_True"),
                "def fun(a: i32) -> i32:\n"
                  "    return - a\n",
                  "fun(-1)",
                  "1"
        }
    };
    return examples;
}

Array<VMTestCase> const& vm_assert_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("VM_assert_True"),
            "def fun(a: i32) -> i32:\n"
            "    assert True, \"all good\"\n"
            "    return 1\n",
            "fun(0)",
            "1"
        },
        {
            TestName("VM_assert_False"),
            "def fun(a: i32) -> i32:\n"
            "    assert False, \"Very bad\"\n"
            "    return 1\n",
            "fun(0)",
            "Traceback (most recent call last):\n"
            "  File \"<input>\", line -2, in <module>\n"
            "    fun(0)\n"
            "  File \"<input>\", line 2, in fun\n"
            "    assert False, \"Very bad\"\n"
            "AssertionError: Very bad\n"
        }
    };
    return examples;
}

Array<VMTestCase> const& vm_ifstmt_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("VM_IfStmt_True"),
                "def fun(a: i32) -> i32:\n"
                "    if a > 0:\n"
                "        return 1\n"
                "    else:\n"
                "        return 2\n",
                "fun(1)",
                "1"
        },
        {
            TestName("VM_IfStmt_False"),
                  "def fun(a: i32) -> i32:\n"
                  "    if a > 0:\n"
                  "        return 1\n"
                  "    else:\n"
                  "        return 2\n",
                  "fun(0)",
                  "2"
        }
    };
    return examples;
}


Array<VMTestCase> const& vm_compare_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("VM_Compare_True"),
            "def fun() -> bool:\n"
            "    return 1 < 2 < 3 < 4 < 5\n",
            "fun()",
            "True"
        },
        {
            TestName("VM_Compare_False"),
            "def fun() -> bool:\n"
            "    return 1 > 2 > 3 > 4 > 5\n",
            "fun()",
            "False"
        }
    };
    return examples;
}

Array<VMTestCase> const& vm_bool_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("VM_BoolAnd_True"),
            "def fun() -> bool:\n"
                  "    return (1 < 2) and (2 < 3)\n",
                  "fun()",
                  "True"
        },
        {
            TestName("VM_BoolAnd_False"),
            "def fun() -> bool:\n"
            "    return (1 > 2) and (2 > 3)\n",
            "fun()",
            "False"
        }
    };
    return examples;
}


Array<VMTestCase> const& vm_binop_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("VM_BinOp_Add_i32"),
            "def fun(a: i32) -> i32:\n"
                  "    return a + 1\n",
                  "fun(1)",
                  "2"
        }
    };
    return examples;
}

Array<VMTestCase> const& vm_exception_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("VM_exception_stop_recursion"),
            "def fun(a: i32) -> i32:\n"
            "    if a == 0:\n"
            "        assert False, \"Very bad\"\n"
            "    return fun(a - 1)\n",
            "fun(2)",
            "Traceback (most recent call last):\n"
            "  File \"<input>\", line -2, in <module>\n"
            "    fun(2)\n"
            "  File \"<input>\", line 4, in fun\n"
            "    return fun(a - 1)\n"
            "  File \"<input>\", line 4, in fun\n"
            "    return fun(a - 1)\n"
            "  File \"<input>\", line 3, in fun\n"
            "    assert False, \"Very bad\"\n"
            "AssertionError: Very bad\n"
        },
        {
            TestName("VM_exception_stop_loop"),
            "def fun(a: i32) -> i32:\n"
            "    while True:\n"
            "        assert False, \"Very bad\"\n"
            "    return 1\n",
            "fun(2)",
            "Traceback (most recent call last):\n"
            "  File \"<input>\", line -2, in <module>\n"
            "    fun(2)\n"
            "  File \"<input>\", line 3, in fun\n"
            "    assert False, \"Very bad\"\n"
            "AssertionError: Very bad\n"
        }
    };
    return examples;
}


TEST_CASE("VM_raise") {
    // This fails because of SEMA, we need a real exception
    // run_test_case(
    //     "def fun(a: i32) -> i32:\n"
    //     "    raise 'Ohoh'\n"
    //     "    return a\n",
    //     "fun(-1)",
    //     "1"
    // );
}

TEST_CASE("VM_Try_AllGood") {
    // Increase once in try
    // another inside the else (no exception)
    // and final one inside the finally
    /*
    run_test_case(
        "def fun(a: i32) -> i32:\n"
        "    try:\n"
        "        a += 1\n"
        "    except:\n"
        "        a += 1\n"
        "    else:\n"
        "        a += 1\n"
        "    finally:\n"
        "        a += 1\n",
        "    return a\n"
        "fun(0)",
        "3"
    );
    //*/
}


Array<VMTestCase> const& vm_augassign_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("VM_Aug_Add_i32"),
                "def fun(a: i32) -> i32:\n"
                  "    a += 1\n"
                  "    return a\n",
                  "fun(0)",
                  "1"
        }
    };
    return examples;
}

Array<VMTestCase> const& vm_whilestmt_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("VM_While"),
            "def fun(a: i32) -> i32:\n"
                  "    b = 0\n"
                  "    while a > 0:\n"
                  "        a -= 1\n"
                  "        b += 2\n"
                  "    return b\n"
                  "",

                  "fun(2)",
                  "4"
        },
        {
            TestName("VM_While_continue"),
                "def fun(a: i32) -> i32:\n"
                  "    b = 0\n"
                  "    while a > 0:\n"
                  "        a -= 1\n"
                  "        b += 1\n"
                  "        continue\n"
                  "        b += 1\n"
                  "    return b\n"
                  "",

                  "fun(10)",
                  "10"
        },
        {
            TestName("VM_While_break"),
            "def fun(a: i32) -> i32:\n"
                  "    b = 0\n"
                  "    while a > 0:\n"
                  "        a -= 1\n"
                  "        b += 1\n"
                  "        break\n"
                  "        b += 1\n"
                  "    return b\n"
                  "",

                  "fun(10)",
                  "1"
        }
    };
    return examples;
}


Array<VMTestCase> const& vm_annassign_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("VM_AnnAssign"),
            "def fun(a: i32) -> i32:\n"
            "    b: i32 = 3\n"
            "    b: i32 = a * b\n"
            "    return b\n"
            "",
            "fun(2)",
            "6"
        }
    };
    return examples;
}

Array<VMTestCase> const& vm_ifexpr_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("VM_ifexp_True"),
                "def fun(a: i32) -> i32:\n"
                  "    return 1 if a > 0 else 0\n"
                  "",
                  "fun(2)",
                  "1"
        }, 
        {  
            TestName("VM_ifexp_False"),
            "def fun(a: i32) -> i32:\n"
                  "    return 1 if a > 0 else 0\n"
                  "",
                  "fun(-2)",
                  "0"
        }
    };
    return examples;
}


// TEST_CASE("VM_ifexp_ext_True") {
//     // FIXME: wrong syntax
//     run_test_case("def fun(a: i32) -> i32:\n"
//                   "    return if a > 0: 1 else 0\n"
//                   "",
//                   "fun(2)",
//                   "1");
// }

// TEST_CASE("VM_ifexp_ext_False") {
//     // FIXME: wrong syntax
//     run_test_case("def fun(a: i32) -> i32:\n"
//                   "    return if a > 0: 1 else 0\n"
//                   "",
//                   "fun(-2)",
//                   "0");
// }

Array<VMTestCase> const& vm_namedexpr_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("VM_NamedExpr"),
            "def fun(a: i32) -> i32:\n"
            "    if (b := a + 1) > 0:\n"
            "        return b\n"
            "    return 0\n"
            "",
            "fun(0)",
            "1"
        }
    };
    return examples;
}

Array<VMTestCase> const& vm_classdef_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("VM_ClassDef"),
            "class Point:\n"
                  "    def __init__(self, x: f64, y: f64):\n"
                  "        self.x = x\n"
                  "        self.y = y\n"
                  "\n",
                  "Point(1.0, 2.0)",
                  "(x=1.0, y=2.0)"
        },
        {
            TestName("VM_ClassDef_2"),
            "class Point:\n"
                  "    def __init__(self, x: f64, y: f64):\n"
                  "        self.x = x\n"
                  "        self.y = y\n"
                  "\n"
                  "def fun(p: Point) -> f64:\n"
                  "    return p.x + p.y\n"
                  "",
                  "fun(Point(1.0, 2.0))",
                  "3.0"
        }
    };
    return examples;
}

Array<VMTestCase> const& vm_generator_ext_examples() {
    static Array<VMTestCase> examples = {
        {
            TestName("VM_Generator"),
            "def range(n: i32) -> i32:\n"
            "    c: i32 = 0\n"
            "    while c < n:\n"
            "        yield c\n"
            "        c += 1\n"
            "\n"
            "def fun() -> i32:\n"
            "    acc: i32 = 0\n"
            "    for i in range(10):\n"
            "        acc += i\n"
            "    return acc\n"
            ,
            "fun()",
            "45"
        }
    };
    return examples;
}