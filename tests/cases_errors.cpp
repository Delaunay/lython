#include "cases.h"

Array<TestCase> const& Match_error_examples() {
    static Array<TestCase> ex = {

    };
    return ex;
}

Array<TestCase> const& Continue_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Break_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& InvalidStatement_examples() {
    static Array<TestCase> ex = {};
    return ex;
}
Array<TestCase> const& InvalidStatement_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Pass_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& StmtNode_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Nonlocal_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Global_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& ImportFrom_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Import_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Assert_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Try_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Raise_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& With_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& If_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& While_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& For_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& AnnAssign_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& AugAssign_error_examples() {
    static Array<TestCase> ex = {
        // invalid Aug use
        {"a > b %= 1"},
    };
    return ex;
}

Array<TestCase> const& Assign_error_examples() {
    static Array<TestCase> ex = {
        // Invalid assign
        {"a * int = 3"},
    };
    return ex;
}

Array<TestCase> const& Delete_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Return_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& ClassDef_error_examples() {
    static Array<TestCase> ex = {// In python this does a recurcive call until infinity
                                 {"class Name:\n"
                                  "    def __init__(self):\n"
                                  "        self.a = Name()\n"
                                  "\n"}};
    return ex;
}

Array<TestCase> const& FunctionDef_error_examples() {
    static Array<TestCase> ex = {
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
    return ex;
}

Array<TestCase> const& Inline_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& FunctionType_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Expression_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Interactive_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Module_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Slice_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& TupleExpr_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& ListExpr_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Name_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Starred_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Subscript_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Attribute_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Constant_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& FormattedValue_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& JoinedStr_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Call_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Compare_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& YieldFrom_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Yield_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Await_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& DictComp_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& SetComp_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& GeneratorExp_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& ListComp_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& SetExpr_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& DictExpr_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& IfExp_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Lambda_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& UnaryOp_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& BinOp_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& NamedExpr_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& BoolOp_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& DictType_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& ArrayType_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& TupleType_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Arrow_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& ClassType_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& SetType_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& BuiltinType_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}

Array<TestCase> const& Expr_error_examples() {
    static Array<TestCase> ex = {};
    return ex;
}