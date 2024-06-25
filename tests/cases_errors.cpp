// #include "cases.h"

#include "ast/nodes.h"
#include "dtypes.h"
#include "parser/parsing_error.h"

#include "libtest.h"

using namespace lython;

Array<TestCase> const& Match_error_examples() {
    static Array<TestCase> examples = {

    };
    return examples;
}

Array<TestCase> const& Continue_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Break_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& InvalidStatement_examples() {
    static Array<TestCase> examples = {};
    return examples;
}
Array<TestCase> const& InvalidStatement_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Pass_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& StmtNode_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Nonlocal_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Global_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& ImportFrom_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Import_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Assert_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Try_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Raise_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& With_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& If_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& While_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& For_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& AnnAssign_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& AugAssign_error_examples() {
    static Array<TestCase> examples = {
        // invalid Aug use
        {"a > b %= 1"},
    };
    return examples;
}

Array<TestCase> const& Assign_error_examples() {
    static Array<TestCase> examples = {
        // Invalid assign
        {"a * int = 3"},
    };
    return examples;
}

Array<TestCase> const& Delete_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Return_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& ClassDef_error_examples() {
    static Array<TestCase> examples = {// In python this does a recurcive call until infinity
                                 {"class Name:\n"
                                  "    def __init__(self):\n"
                                  "        self.a = Name()\n"
                                  "\n"}};
    return examples;
}

Array<TestCase> const& FunctionDef_error_examples() {
    static Array<TestCase> examples = {
        {"def "},
        {"def name"},
        {"def name("},
        {"def name(arg"},
        {"def name(arg, "},
        {"def name(arg, a"},
        {"def name(arg, a="},
        {"def name(arg, a=b"},
        {"def name(arg, a=b)"},
        {"def name(arg, a=b) ->"},
        {"def name(arg, a=b):"},
        {"def name(arg, a=2):\n"},
        {"def name(arg, a=2):\n"
         "    "},
        // name is not a type
        {"def name(arg: name):\n"
         "    pass\n"},
    };
    return examples;
}

Array<TestCase> const& Inline_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& FunctionType_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Expression_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Interactive_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Module_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Slice_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& TupleExpr_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& ListExpr_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Name_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Starred_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Subscript_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Attribute_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Constant_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& FormattedValue_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& JoinedStr_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Call_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Compare_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& YieldFrom_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Yield_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Await_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& DictComp_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& SetComp_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& GeneratorExp_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& ListComp_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& SetExpr_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& DictExpr_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& IfExp_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Lambda_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& UnaryOp_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& BinOp_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& NamedExpr_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& BoolOp_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& DictType_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& ArrayType_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& TupleType_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Arrow_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& ClassType_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& SetType_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& BuiltinType_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}

Array<TestCase> const& Expr_error_examples() {
    static Array<TestCase> examples = {};
    return examples;
}