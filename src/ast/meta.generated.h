#include "dtypes.h"
#include "ast/nodes.h"
#include "utilities/names.h"

namespace lython {
struct CommonAttributesType {
    static CommonAttributesType& cls() { static CommonAttributesType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("lineno"), offsetof(CommonAttributes, lineno), sizeof(CommonAttributes::lineno), StringRef("int")),
            Field(StringRef("col_offset"), offsetof(CommonAttributes, col_offset), sizeof(CommonAttributes::col_offset), StringRef("int")),
            Field(StringRef("end_lineno"), offsetof(CommonAttributes, end_lineno), sizeof(CommonAttributes::end_lineno), StringRef("Optional<int>")),
            Field(StringRef("end_col_offset"), offsetof(CommonAttributes, end_col_offset), sizeof(CommonAttributes::end_col_offset), StringRef("Optional<int>")),
        };
        return fields;
    }
};
struct NodeType {
    static NodeType& cls() { static NodeType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("kind"), offsetof(Node, kind), sizeof(Node::kind), StringRef("const NodeKind")),
        };
        return fields;
    }
};
struct StmtNodeType {
    static StmtNodeType& cls() { static StmtNodeType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("comment"), offsetof(StmtNode, comment), sizeof(StmtNode::comment), StringRef("Comment *")),
        };
        return fields;
    }
};
struct ComprehensionType {
    static ComprehensionType& cls() { static ComprehensionType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("target"), offsetof(Comprehension, target), sizeof(Comprehension::target), StringRef("ExprNode *")),
            Field(StringRef("iter"), offsetof(Comprehension, iter), sizeof(Comprehension::iter), StringRef("ExprNode *")),
            Field(StringRef("ifs"), offsetof(Comprehension, ifs), sizeof(Comprehension::ifs), StringRef("int")),
            Field(StringRef("is_async"), offsetof(Comprehension, is_async), sizeof(Comprehension::is_async), StringRef("int")),
        };
        return fields;
    }
};
struct ExceptHandlerType {
    static ExceptHandlerType& cls() { static ExceptHandlerType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("type"), offsetof(ExceptHandler, type), sizeof(ExceptHandler::type), StringRef("Optional<ExprNode *>")),
            Field(StringRef("name"), offsetof(ExceptHandler, name), sizeof(ExceptHandler::name), StringRef("Optional<Identifier>")),
            Field(StringRef("body"), offsetof(ExceptHandler, body), sizeof(ExceptHandler::body), StringRef("int")),
            Field(StringRef("comment"), offsetof(ExceptHandler, comment), sizeof(ExceptHandler::comment), StringRef("Comment *")),
        };
        return fields;
    }
};
struct ArgType {
    static ArgType& cls() { static ArgType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("arg"), offsetof(Arg, arg), sizeof(Arg::arg), StringRef("Identifier")),
            Field(StringRef("annotation"), offsetof(Arg, annotation), sizeof(Arg::annotation), StringRef("Optional<ExprNode *>")),
            Field(StringRef("type_comment"), offsetof(Arg, type_comment), sizeof(Arg::type_comment), StringRef("int")),
        };
        return fields;
    }
};
struct ArgumentsType {
    static ArgumentsType& cls() { static ArgumentsType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("posonlyargs"), offsetof(Arguments, posonlyargs), sizeof(Arguments::posonlyargs), StringRef("int")),
            Field(StringRef("args"), offsetof(Arguments, args), sizeof(Arguments::args), StringRef("int")),
            Field(StringRef("vararg"), offsetof(Arguments, vararg), sizeof(Arguments::vararg), StringRef("Optional<Arg>")),
            Field(StringRef("kwonlyargs"), offsetof(Arguments, kwonlyargs), sizeof(Arguments::kwonlyargs), StringRef("int")),
            Field(StringRef("kw_defaults"), offsetof(Arguments, kw_defaults), sizeof(Arguments::kw_defaults), StringRef("int")),
            Field(StringRef("kwarg"), offsetof(Arguments, kwarg), sizeof(Arguments::kwarg), StringRef("Optional<Arg>")),
            Field(StringRef("defaults"), offsetof(Arguments, defaults), sizeof(Arguments::defaults), StringRef("int")),
        };
        return fields;
    }
};
struct KeywordType {
    static KeywordType& cls() { static KeywordType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("arg"), offsetof(Keyword, arg), sizeof(Keyword::arg), StringRef("Identifier")),
            Field(StringRef("value"), offsetof(Keyword, value), sizeof(Keyword::value), StringRef("ExprNode *")),
        };
        return fields;
    }
};
struct AliasType {
    static AliasType& cls() { static AliasType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("name"), offsetof(Alias, name), sizeof(Alias::name), StringRef("Identifier")),
            Field(StringRef("asname"), offsetof(Alias, asname), sizeof(Alias::asname), StringRef("Optional<Identifier>")),
        };
        return fields;
    }
};
struct WithItemType {
    static WithItemType& cls() { static WithItemType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("context_expr"), offsetof(WithItem, context_expr), sizeof(WithItem::context_expr), StringRef("ExprNode *")),
            Field(StringRef("optional_vars"), offsetof(WithItem, optional_vars), sizeof(WithItem::optional_vars), StringRef("Optional<ExprNode *>")),
        };
        return fields;
    }
};
struct TypeIgnoreType {
    static TypeIgnoreType& cls() { static TypeIgnoreType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("lineno"), offsetof(TypeIgnore, lineno), sizeof(TypeIgnore::lineno), StringRef("int")),
            Field(StringRef("tag"), offsetof(TypeIgnore, tag), sizeof(TypeIgnore::tag), StringRef("int")),
        };
        return fields;
    }
};
struct MatchValueType {
    static MatchValueType& cls() { static MatchValueType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("value"), offsetof(MatchValue, value), sizeof(MatchValue::value), StringRef("ExprNode *")),
        };
        return fields;
    }
};
struct MatchSingletonType {
    static MatchSingletonType& cls() { static MatchSingletonType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("value"), offsetof(MatchSingleton, value), sizeof(MatchSingleton::value), StringRef("Value")),
            Field(StringRef("deleter"), offsetof(MatchSingleton, deleter), sizeof(MatchSingleton::deleter), StringRef("ValueDeleter")),
        };
        return fields;
    }
};
struct MatchSequenceType {
    static MatchSequenceType& cls() { static MatchSequenceType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("patterns"), offsetof(MatchSequence, patterns), sizeof(MatchSequence::patterns), StringRef("int")),
        };
        return fields;
    }
};
struct MatchMappingType {
    static MatchMappingType& cls() { static MatchMappingType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("keys"), offsetof(MatchMapping, keys), sizeof(MatchMapping::keys), StringRef("int")),
            Field(StringRef("patterns"), offsetof(MatchMapping, patterns), sizeof(MatchMapping::patterns), StringRef("int")),
            Field(StringRef("rest"), offsetof(MatchMapping, rest), sizeof(MatchMapping::rest), StringRef("Optional<Identifier>")),
        };
        return fields;
    }
};
struct MatchClassType {
    static MatchClassType& cls() { static MatchClassType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("cls"), offsetof(MatchClass, cls), sizeof(MatchClass::cls), StringRef("ExprNode *")),
            Field(StringRef("patterns"), offsetof(MatchClass, patterns), sizeof(MatchClass::patterns), StringRef("int")),
            Field(StringRef("kwd_attrs"), offsetof(MatchClass, kwd_attrs), sizeof(MatchClass::kwd_attrs), StringRef("int")),
            Field(StringRef("kwd_patterns"), offsetof(MatchClass, kwd_patterns), sizeof(MatchClass::kwd_patterns), StringRef("int")),
        };
        return fields;
    }
};
struct MatchStarType {
    static MatchStarType& cls() { static MatchStarType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("name"), offsetof(MatchStar, name), sizeof(MatchStar::name), StringRef("Optional<Identifier>")),
        };
        return fields;
    }
};
struct MatchAsType {
    static MatchAsType& cls() { static MatchAsType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("pattern"), offsetof(MatchAs, pattern), sizeof(MatchAs::pattern), StringRef("Optional<Pattern *>")),
            Field(StringRef("name"), offsetof(MatchAs, name), sizeof(MatchAs::name), StringRef("Optional<Identifier>")),
        };
        return fields;
    }
};
struct MatchOrType {
    static MatchOrType& cls() { static MatchOrType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("patterns"), offsetof(MatchOr, patterns), sizeof(MatchOr::patterns), StringRef("int")),
        };
        return fields;
    }
};
struct MatchCaseType {
    static MatchCaseType& cls() { static MatchCaseType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("pattern"), offsetof(MatchCase, pattern), sizeof(MatchCase::pattern), StringRef("Pattern *")),
            Field(StringRef("guard"), offsetof(MatchCase, guard), sizeof(MatchCase::guard), StringRef("Optional<ExprNode *>")),
            Field(StringRef("body"), offsetof(MatchCase, body), sizeof(MatchCase::body), StringRef("int")),
            Field(StringRef("comment"), offsetof(MatchCase, comment), sizeof(MatchCase::comment), StringRef("Comment *")),
        };
        return fields;
    }
};
struct CommentType {
    static CommentType& cls() { static CommentType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("comment"), offsetof(Comment, comment), sizeof(Comment::comment), StringRef("int")),
        };
        return fields;
    }
};
struct ConstantType {
    static ConstantType& cls() { static ConstantType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("value"), offsetof(Constant, value), sizeof(Constant::value), StringRef("Value")),
            Field(StringRef("deleter"), offsetof(Constant, deleter), sizeof(Constant::deleter), StringRef("ValueDeleter")),
            Field(StringRef("kind"), offsetof(Constant, kind), sizeof(Constant::kind), StringRef("int")),
        };
        return fields;
    }
};
struct PlaceholderType {
    static PlaceholderType& cls() { static PlaceholderType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("expr"), offsetof(Placeholder, expr), sizeof(Placeholder::expr), StringRef("ExprNode *")),
        };
        return fields;
    }
};
struct BoolOpType {
    static BoolOpType& cls() { static BoolOpType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("op"), offsetof(BoolOp, op), sizeof(BoolOp::op), StringRef("BoolOperator")),
            Field(StringRef("values"), offsetof(BoolOp, values), sizeof(BoolOp::values), StringRef("int")),
            Field(StringRef("opcount"), offsetof(BoolOp, opcount), sizeof(BoolOp::opcount), StringRef("int")),
            Field(StringRef("resolved_operator"), offsetof(BoolOp, resolved_operator), sizeof(BoolOp::resolved_operator), StringRef("StmtNode *")),
            Field(StringRef("native_operator"), offsetof(BoolOp, native_operator), sizeof(BoolOp::native_operator), StringRef("Function")),
            Field(StringRef("varid"), offsetof(BoolOp, varid), sizeof(BoolOp::varid), StringRef("int")),
        };
        return fields;
    }
};
struct CompareType {
    static CompareType& cls() { static CompareType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("left"), offsetof(Compare, left), sizeof(Compare::left), StringRef("ExprNode *")),
            Field(StringRef("ops"), offsetof(Compare, ops), sizeof(Compare::ops), StringRef("int")),
            Field(StringRef("comparators"), offsetof(Compare, comparators), sizeof(Compare::comparators), StringRef("int")),
            Field(StringRef("resolved_operator"), offsetof(Compare, resolved_operator), sizeof(Compare::resolved_operator), StringRef("int")),
            Field(StringRef("native_operator"), offsetof(Compare, native_operator), sizeof(Compare::native_operator), StringRef("int")),
        };
        return fields;
    }
};
struct NamedExprType {
    static NamedExprType& cls() { static NamedExprType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("target"), offsetof(NamedExpr, target), sizeof(NamedExpr::target), StringRef("ExprNode *")),
            Field(StringRef("value"), offsetof(NamedExpr, value), sizeof(NamedExpr::value), StringRef("ExprNode *")),
        };
        return fields;
    }
};
struct BinOpType {
    static BinOpType& cls() { static BinOpType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("left"), offsetof(BinOp, left), sizeof(BinOp::left), StringRef("ExprNode *")),
            Field(StringRef("op"), offsetof(BinOp, op), sizeof(BinOp::op), StringRef("BinaryOperator")),
            Field(StringRef("right"), offsetof(BinOp, right), sizeof(BinOp::right), StringRef("ExprNode *")),
            Field(StringRef("resolved_operator"), offsetof(BinOp, resolved_operator), sizeof(BinOp::resolved_operator), StringRef("StmtNode *")),
            Field(StringRef("native_operator"), offsetof(BinOp, native_operator), sizeof(BinOp::native_operator), StringRef("Function")),
            Field(StringRef("varid"), offsetof(BinOp, varid), sizeof(BinOp::varid), StringRef("int")),
        };
        return fields;
    }
};
struct UnaryOpType {
    static UnaryOpType& cls() { static UnaryOpType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("op"), offsetof(UnaryOp, op), sizeof(UnaryOp::op), StringRef("UnaryOperator")),
            Field(StringRef("operand"), offsetof(UnaryOp, operand), sizeof(UnaryOp::operand), StringRef("ExprNode *")),
            Field(StringRef("resolved_operator"), offsetof(UnaryOp, resolved_operator), sizeof(UnaryOp::resolved_operator), StringRef("StmtNode *")),
            Field(StringRef("native_operator"), offsetof(UnaryOp, native_operator), sizeof(UnaryOp::native_operator), StringRef("Function")),
        };
        return fields;
    }
};
struct LambdaType {
    static LambdaType& cls() { static LambdaType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("args"), offsetof(Lambda, args), sizeof(Lambda::args), StringRef("Arguments")),
            Field(StringRef("body"), offsetof(Lambda, body), sizeof(Lambda::body), StringRef("ExprNode *")),
        };
        return fields;
    }
};
struct IfExpType {
    static IfExpType& cls() { static IfExpType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("test"), offsetof(IfExp, test), sizeof(IfExp::test), StringRef("ExprNode *")),
            Field(StringRef("body"), offsetof(IfExp, body), sizeof(IfExp::body), StringRef("ExprNode *")),
            Field(StringRef("orelse"), offsetof(IfExp, orelse), sizeof(IfExp::orelse), StringRef("ExprNode *")),
        };
        return fields;
    }
};
struct DictExprType {
    static DictExprType& cls() { static DictExprType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("keys"), offsetof(DictExpr, keys), sizeof(DictExpr::keys), StringRef("int")),
            Field(StringRef("values"), offsetof(DictExpr, values), sizeof(DictExpr::values), StringRef("int")),
        };
        return fields;
    }
};
struct SetExprType {
    static SetExprType& cls() { static SetExprType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("elts"), offsetof(SetExpr, elts), sizeof(SetExpr::elts), StringRef("int")),
        };
        return fields;
    }
};
struct ListCompType {
    static ListCompType& cls() { static ListCompType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("elt"), offsetof(ListComp, elt), sizeof(ListComp::elt), StringRef("ExprNode *")),
            Field(StringRef("generators"), offsetof(ListComp, generators), sizeof(ListComp::generators), StringRef("int")),
        };
        return fields;
    }
};
struct GeneratorExpType {
    static GeneratorExpType& cls() { static GeneratorExpType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("elt"), offsetof(GeneratorExp, elt), sizeof(GeneratorExp::elt), StringRef("ExprNode *")),
            Field(StringRef("generators"), offsetof(GeneratorExp, generators), sizeof(GeneratorExp::generators), StringRef("int")),
        };
        return fields;
    }
};
struct SetCompType {
    static SetCompType& cls() { static SetCompType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("elt"), offsetof(SetComp, elt), sizeof(SetComp::elt), StringRef("ExprNode *")),
            Field(StringRef("generators"), offsetof(SetComp, generators), sizeof(SetComp::generators), StringRef("int")),
        };
        return fields;
    }
};
struct DictCompType {
    static DictCompType& cls() { static DictCompType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("key"), offsetof(DictComp, key), sizeof(DictComp::key), StringRef("ExprNode *")),
            Field(StringRef("value"), offsetof(DictComp, value), sizeof(DictComp::value), StringRef("ExprNode *")),
            Field(StringRef("generators"), offsetof(DictComp, generators), sizeof(DictComp::generators), StringRef("int")),
        };
        return fields;
    }
};
struct AwaitType {
    static AwaitType& cls() { static AwaitType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("value"), offsetof(Await, value), sizeof(Await::value), StringRef("ExprNode *")),
        };
        return fields;
    }
};
struct YieldType {
    static YieldType& cls() { static YieldType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("value"), offsetof(Yield, value), sizeof(Yield::value), StringRef("Optional<ExprNode *>")),
        };
        return fields;
    }
};
struct YieldFromType {
    static YieldFromType& cls() { static YieldFromType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("value"), offsetof(YieldFrom, value), sizeof(YieldFrom::value), StringRef("ExprNode *")),
        };
        return fields;
    }
};
struct CallType {
    static CallType& cls() { static CallType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("func"), offsetof(Call, func), sizeof(Call::func), StringRef("ExprNode *")),
            Field(StringRef("args"), offsetof(Call, args), sizeof(Call::args), StringRef("int")),
            Field(StringRef("keywords"), offsetof(Call, keywords), sizeof(Call::keywords), StringRef("int")),
            Field(StringRef("varargs"), offsetof(Call, varargs), sizeof(Call::varargs), StringRef("int")),
            Field(StringRef("jump_id"), offsetof(Call, jump_id), sizeof(Call::jump_id), StringRef("int")),
        };
        return fields;
    }
};
struct JoinedStrType {
    static JoinedStrType& cls() { static JoinedStrType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("values"), offsetof(JoinedStr, values), sizeof(JoinedStr::values), StringRef("int")),
        };
        return fields;
    }
};
struct FormattedValueType {
    static FormattedValueType& cls() { static FormattedValueType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("value"), offsetof(FormattedValue, value), sizeof(FormattedValue::value), StringRef("ExprNode *")),
            Field(StringRef("conversion"), offsetof(FormattedValue, conversion), sizeof(FormattedValue::conversion), StringRef("Optional<ConversionKind>")),
            Field(StringRef("format_spec"), offsetof(FormattedValue, format_spec), sizeof(FormattedValue::format_spec), StringRef("JoinedStr *")),
        };
        return fields;
    }
};
struct SubscriptType {
    static SubscriptType& cls() { static SubscriptType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("value"), offsetof(Subscript, value), sizeof(Subscript::value), StringRef("ExprNode *")),
            Field(StringRef("slice"), offsetof(Subscript, slice), sizeof(Subscript::slice), StringRef("ExprNode *")),
            Field(StringRef("ctx"), offsetof(Subscript, ctx), sizeof(Subscript::ctx), StringRef("ExprContext")),
        };
        return fields;
    }
};
struct StarredType {
    static StarredType& cls() { static StarredType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("value"), offsetof(Starred, value), sizeof(Starred::value), StringRef("ExprNode *")),
            Field(StringRef("ctx"), offsetof(Starred, ctx), sizeof(Starred::ctx), StringRef("ExprContext")),
        };
        return fields;
    }
};
struct NameType {
    static NameType& cls() { static NameType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("id"), offsetof(Name, id), sizeof(Name::id), StringRef("Identifier")),
            Field(StringRef("ctx"), offsetof(Name, ctx), sizeof(Name::ctx), StringRef("ExprContext")),
            Field(StringRef("type"), offsetof(Name, type), sizeof(Name::type), StringRef("ExprNode *")),
            Field(StringRef("store_id"), offsetof(Name, store_id), sizeof(Name::store_id), StringRef("int")),
            Field(StringRef("load_id"), offsetof(Name, load_id), sizeof(Name::load_id), StringRef("int")),
        };
        return fields;
    }
};
struct ListExprType {
    static ListExprType& cls() { static ListExprType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("elts"), offsetof(ListExpr, elts), sizeof(ListExpr::elts), StringRef("int")),
            Field(StringRef("ctx"), offsetof(ListExpr, ctx), sizeof(ListExpr::ctx), StringRef("ExprContext")),
        };
        return fields;
    }
};
struct TupleExprType {
    static TupleExprType& cls() { static TupleExprType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("elts"), offsetof(TupleExpr, elts), sizeof(TupleExpr::elts), StringRef("int")),
            Field(StringRef("ctx"), offsetof(TupleExpr, ctx), sizeof(TupleExpr::ctx), StringRef("ExprContext")),
            Field(StringRef("type"), offsetof(TupleExpr, type), sizeof(TupleExpr::type), StringRef("struct TupleType *")),
        };
        return fields;
    }
};
struct SliceType {
    static SliceType& cls() { static SliceType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("lower"), offsetof(Slice, lower), sizeof(Slice::lower), StringRef("Optional<ExprNode *>")),
            Field(StringRef("upper"), offsetof(Slice, upper), sizeof(Slice::upper), StringRef("Optional<ExprNode *>")),
            Field(StringRef("step"), offsetof(Slice, step), sizeof(Slice::step), StringRef("Optional<ExprNode *>")),
        };
        return fields;
    }
};
struct ModuleType {
    static ModuleType& cls() { static ModuleType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("body"), offsetof(Module, body), sizeof(Module::body), StringRef("int")),
            Field(StringRef("docstring"), offsetof(Module, docstring), sizeof(Module::docstring), StringRef("int")),
            Field(StringRef("__init__"), offsetof(Module, __init__), sizeof(Module::__init__), StringRef("struct FunctionDef *")),
        };
        return fields;
    }
};
struct InteractiveType {
    static InteractiveType& cls() { static InteractiveType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("body"), offsetof(Interactive, body), sizeof(Interactive::body), StringRef("int")),
        };
        return fields;
    }
};
struct ExpressionType {
    static ExpressionType& cls() { static ExpressionType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("body"), offsetof(Expression, body), sizeof(Expression::body), StringRef("ExprNode *")),
        };
        return fields;
    }
};
struct FunctionTypeType {
    static FunctionTypeType& cls() { static FunctionTypeType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("argtypes"), offsetof(FunctionType, argtypes), sizeof(FunctionType::argtypes), StringRef("int")),
            Field(StringRef("returns"), offsetof(FunctionType, returns), sizeof(FunctionType::returns), StringRef("ExprNode *")),
        };
        return fields;
    }
};
struct InvalidStatementType {
    static InvalidStatementType& cls() { static InvalidStatementType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("tokens"), offsetof(InvalidStatement, tokens), sizeof(InvalidStatement::tokens), StringRef("int")),
        };
        return fields;
    }
};
struct InlineType {
    static InlineType& cls() { static InlineType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("body"), offsetof(Inline, body), sizeof(Inline::body), StringRef("int")),
        };
        return fields;
    }
};
struct DecoratorType {
    static DecoratorType& cls() { static DecoratorType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("expr"), offsetof(Decorator, expr), sizeof(Decorator::expr), StringRef("ExprNode *")),
            Field(StringRef("comment"), offsetof(Decorator, comment), sizeof(Decorator::comment), StringRef("Comment *")),
        };
        return fields;
    }
};
struct DocstringType {
    static DocstringType& cls() { static DocstringType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("docstring"), offsetof(Docstring, docstring), sizeof(Docstring::docstring), StringRef("int")),
            Field(StringRef("comment"), offsetof(Docstring, comment), sizeof(Docstring::comment), StringRef("Comment *")),
        };
        return fields;
    }
};
struct FunctionDefType {
    static FunctionDefType& cls() { static FunctionDefType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("name"), offsetof(FunctionDef, name), sizeof(FunctionDef::name), StringRef("Identifier")),
            Field(StringRef("args"), offsetof(FunctionDef, args), sizeof(FunctionDef::args), StringRef("Arguments")),
            Field(StringRef("body"), offsetof(FunctionDef, body), sizeof(FunctionDef::body), StringRef("int")),
            Field(StringRef("decorator_list"), offsetof(FunctionDef, decorator_list), sizeof(FunctionDef::decorator_list), StringRef("int")),
            Field(StringRef("returns"), offsetof(FunctionDef, returns), sizeof(FunctionDef::returns), StringRef("Optional<ExprNode *>")),
            Field(StringRef("type_comment"), offsetof(FunctionDef, type_comment), sizeof(FunctionDef::type_comment), StringRef("int")),
            Field(StringRef("docstring"), offsetof(FunctionDef, docstring), sizeof(FunctionDef::docstring), StringRef("Optional<Docstring>")),
            Field(StringRef("async"), offsetof(FunctionDef, async), sizeof(FunctionDef::async), StringRef("bool")),
            Field(StringRef("generator"), offsetof(FunctionDef, generator), sizeof(FunctionDef::generator), StringRef("bool")),
            Field(StringRef("type"), offsetof(FunctionDef, type), sizeof(FunctionDef::type), StringRef("struct Arrow *")),
            Field(StringRef("native"), offsetof(FunctionDef, native), sizeof(FunctionDef::native), StringRef("Function")),
        };
        return fields;
    }
};
struct AttrType {
    static AttrType& cls() { static AttrType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("name"), offsetof(ClassDef::Attr, name), sizeof(ClassDef::Attr::name), StringRef("StringRef")),
            Field(StringRef("offset"), offsetof(ClassDef::Attr, offset), sizeof(ClassDef::Attr::offset), StringRef("int")),
            Field(StringRef("stmt"), offsetof(ClassDef::Attr, stmt), sizeof(ClassDef::Attr::stmt), StringRef("StmtNode *")),
            Field(StringRef("type"), offsetof(ClassDef::Attr, type), sizeof(ClassDef::Attr::type), StringRef("ExprNode *")),
        };
        return fields;
    }
};
struct ClassDefType {
    static ClassDefType& cls() { static ClassDefType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("name"), offsetof(ClassDef, name), sizeof(ClassDef::name), StringRef("Identifier")),
            Field(StringRef("bases"), offsetof(ClassDef, bases), sizeof(ClassDef::bases), StringRef("int")),
            Field(StringRef("keywords"), offsetof(ClassDef, keywords), sizeof(ClassDef::keywords), StringRef("int")),
            Field(StringRef("body"), offsetof(ClassDef, body), sizeof(ClassDef::body), StringRef("int")),
            Field(StringRef("decorator_list"), offsetof(ClassDef, decorator_list), sizeof(ClassDef::decorator_list), StringRef("int")),
            Field(StringRef("docstring"), offsetof(ClassDef, docstring), sizeof(ClassDef::docstring), StringRef("Optional<Docstring>")),
            Field(StringRef("type_id"), offsetof(ClassDef, type_id), sizeof(ClassDef::type_id), StringRef("int")),
            Field(StringRef("ctor_t"), offsetof(ClassDef, ctor_t), sizeof(ClassDef::ctor_t), StringRef("Arrow *")),
            Field(StringRef("cls_namespace"), offsetof(ClassDef, cls_namespace), sizeof(ClassDef::cls_namespace), StringRef("int")),
            Field(StringRef("attributes"), offsetof(ClassDef, attributes), sizeof(ClassDef::attributes), StringRef("int")),
        };
        return fields;
    }
};
struct AttributeType {
    static AttributeType& cls() { static AttributeType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("value"), offsetof(Attribute, value), sizeof(Attribute::value), StringRef("ExprNode *")),
            Field(StringRef("attr"), offsetof(Attribute, attr), sizeof(Attribute::attr), StringRef("Identifier")),
            Field(StringRef("ctx"), offsetof(Attribute, ctx), sizeof(Attribute::ctx), StringRef("ExprContext")),
            Field(StringRef("resolved"), offsetof(Attribute, resolved), sizeof(Attribute::resolved), StringRef("ClassDef::Attr *")),
            Field(StringRef("attrid"), offsetof(Attribute, attrid), sizeof(Attribute::attrid), StringRef("int")),
        };
        return fields;
    }
};
struct ExportedType {
    static ExportedType& cls() { static ExportedType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("source"), offsetof(Exported, source), sizeof(Exported::source), StringRef("struct Bindings *")),
            Field(StringRef("dest"), offsetof(Exported, dest), sizeof(Exported::dest), StringRef("struct Bindings *")),
            Field(StringRef("node"), offsetof(Exported, node), sizeof(Exported::node), StringRef("Node *")),
        };
        return fields;
    }
};
struct ReturnType {
    static ReturnType& cls() { static ReturnType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("value"), offsetof(Return, value), sizeof(Return::value), StringRef("Optional<ExprNode *>")),
        };
        return fields;
    }
};
struct DeleteType {
    static DeleteType& cls() { static DeleteType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("targets"), offsetof(Delete, targets), sizeof(Delete::targets), StringRef("int")),
        };
        return fields;
    }
};
struct AssignType {
    static AssignType& cls() { static AssignType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("targets"), offsetof(Assign, targets), sizeof(Assign::targets), StringRef("int")),
            Field(StringRef("value"), offsetof(Assign, value), sizeof(Assign::value), StringRef("ExprNode *")),
            Field(StringRef("type_comment"), offsetof(Assign, type_comment), sizeof(Assign::type_comment), StringRef("int")),
        };
        return fields;
    }
};
struct AugAssignType {
    static AugAssignType& cls() { static AugAssignType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("target"), offsetof(AugAssign, target), sizeof(AugAssign::target), StringRef("ExprNode *")),
            Field(StringRef("op"), offsetof(AugAssign, op), sizeof(AugAssign::op), StringRef("BinaryOperator")),
            Field(StringRef("value"), offsetof(AugAssign, value), sizeof(AugAssign::value), StringRef("ExprNode *")),
            Field(StringRef("resolved_operator"), offsetof(AugAssign, resolved_operator), sizeof(AugAssign::resolved_operator), StringRef("StmtNode *")),
            Field(StringRef("native_operator"), offsetof(AugAssign, native_operator), sizeof(AugAssign::native_operator), StringRef("Function")),
        };
        return fields;
    }
};
struct AnnAssignType {
    static AnnAssignType& cls() { static AnnAssignType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("target"), offsetof(AnnAssign, target), sizeof(AnnAssign::target), StringRef("ExprNode *")),
            Field(StringRef("annotation"), offsetof(AnnAssign, annotation), sizeof(AnnAssign::annotation), StringRef("ExprNode *")),
            Field(StringRef("value"), offsetof(AnnAssign, value), sizeof(AnnAssign::value), StringRef("Optional<ExprNode *>")),
            Field(StringRef("simple"), offsetof(AnnAssign, simple), sizeof(AnnAssign::simple), StringRef("int")),
        };
        return fields;
    }
};
struct ForType {
    static ForType& cls() { static ForType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("target"), offsetof(For, target), sizeof(For::target), StringRef("ExprNode *")),
            Field(StringRef("iter"), offsetof(For, iter), sizeof(For::iter), StringRef("ExprNode *")),
            Field(StringRef("body"), offsetof(For, body), sizeof(For::body), StringRef("int")),
            Field(StringRef("orelse"), offsetof(For, orelse), sizeof(For::orelse), StringRef("int")),
            Field(StringRef("type_comment"), offsetof(For, type_comment), sizeof(For::type_comment), StringRef("int")),
            Field(StringRef("async"), offsetof(For, async), sizeof(For::async), StringRef("bool")),
            Field(StringRef("else_comment"), offsetof(For, else_comment), sizeof(For::else_comment), StringRef("Comment *")),
        };
        return fields;
    }
};
struct WhileType {
    static WhileType& cls() { static WhileType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("test"), offsetof(While, test), sizeof(While::test), StringRef("ExprNode *")),
            Field(StringRef("body"), offsetof(While, body), sizeof(While::body), StringRef("int")),
            Field(StringRef("orelse"), offsetof(While, orelse), sizeof(While::orelse), StringRef("int")),
            Field(StringRef("else_comment"), offsetof(While, else_comment), sizeof(While::else_comment), StringRef("Comment *")),
        };
        return fields;
    }
};
struct IfType {
    static IfType& cls() { static IfType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("test"), offsetof(If, test), sizeof(If::test), StringRef("ExprNode *")),
            Field(StringRef("body"), offsetof(If, body), sizeof(If::body), StringRef("int")),
            Field(StringRef("orelse"), offsetof(If, orelse), sizeof(If::orelse), StringRef("int")),
            Field(StringRef("tests"), offsetof(If, tests), sizeof(If::tests), StringRef("int")),
            Field(StringRef("bodies"), offsetof(If, bodies), sizeof(If::bodies), StringRef("int")),
            Field(StringRef("tests_comment"), offsetof(If, tests_comment), sizeof(If::tests_comment), StringRef("int")),
            Field(StringRef("else_comment"), offsetof(If, else_comment), sizeof(If::else_comment), StringRef("Comment *")),
        };
        return fields;
    }
};
struct WithType {
    static WithType& cls() { static WithType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("items"), offsetof(With, items), sizeof(With::items), StringRef("int")),
            Field(StringRef("body"), offsetof(With, body), sizeof(With::body), StringRef("int")),
            Field(StringRef("type_comment"), offsetof(With, type_comment), sizeof(With::type_comment), StringRef("int")),
            Field(StringRef("async"), offsetof(With, async), sizeof(With::async), StringRef("bool")),
        };
        return fields;
    }
};
struct RaiseType {
    static RaiseType& cls() { static RaiseType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("exc"), offsetof(Raise, exc), sizeof(Raise::exc), StringRef("Optional<ExprNode *>")),
            Field(StringRef("cause"), offsetof(Raise, cause), sizeof(Raise::cause), StringRef("Optional<ExprNode *>")),
        };
        return fields;
    }
};
struct TryType {
    static TryType& cls() { static TryType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("body"), offsetof(Try, body), sizeof(Try::body), StringRef("int")),
            Field(StringRef("handlers"), offsetof(Try, handlers), sizeof(Try::handlers), StringRef("int")),
            Field(StringRef("orelse"), offsetof(Try, orelse), sizeof(Try::orelse), StringRef("int")),
            Field(StringRef("finalbody"), offsetof(Try, finalbody), sizeof(Try::finalbody), StringRef("int")),
            Field(StringRef("else_comment"), offsetof(Try, else_comment), sizeof(Try::else_comment), StringRef("Comment *")),
            Field(StringRef("finally_comment"), offsetof(Try, finally_comment), sizeof(Try::finally_comment), StringRef("Comment *")),
        };
        return fields;
    }
};
struct AssertType {
    static AssertType& cls() { static AssertType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("test"), offsetof(Assert, test), sizeof(Assert::test), StringRef("ExprNode *")),
            Field(StringRef("msg"), offsetof(Assert, msg), sizeof(Assert::msg), StringRef("Optional<ExprNode *>")),
        };
        return fields;
    }
};
struct ImportType {
    static ImportType& cls() { static ImportType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("names"), offsetof(Import, names), sizeof(Import::names), StringRef("int")),
        };
        return fields;
    }
};
struct ImportFromType {
    static ImportFromType& cls() { static ImportFromType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("module"), offsetof(ImportFrom, module), sizeof(ImportFrom::module), StringRef("Optional<Identifier>")),
            Field(StringRef("names"), offsetof(ImportFrom, names), sizeof(ImportFrom::names), StringRef("int")),
            Field(StringRef("level"), offsetof(ImportFrom, level), sizeof(ImportFrom::level), StringRef("Optional<int>")),
        };
        return fields;
    }
};
struct GlobalType {
    static GlobalType& cls() { static GlobalType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("names"), offsetof(Global, names), sizeof(Global::names), StringRef("int")),
        };
        return fields;
    }
};
struct NonlocalType {
    static NonlocalType& cls() { static NonlocalType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("names"), offsetof(Nonlocal, names), sizeof(Nonlocal::names), StringRef("int")),
        };
        return fields;
    }
};
struct ExprType {
    static ExprType& cls() { static ExprType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("value"), offsetof(Expr, value), sizeof(Expr::value), StringRef("ExprNode *")),
        };
        return fields;
    }
};
struct MatchType {
    static MatchType& cls() { static MatchType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("subject"), offsetof(Match, subject), sizeof(Match::subject), StringRef("ExprNode *")),
            Field(StringRef("cases"), offsetof(Match, cases), sizeof(Match::cases), StringRef("int")),
        };
        return fields;
    }
};
struct NotAllowedEpxrType {
    static NotAllowedEpxrType& cls() { static NotAllowedEpxrType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("msg"), offsetof(NotAllowedEpxr, msg), sizeof(NotAllowedEpxr::msg), StringRef("int")),
        };
        return fields;
    }
};
struct ArrowType {
    static ArrowType& cls() { static ArrowType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("names"), offsetof(Arrow, names), sizeof(Arrow::names), StringRef("int")),
            Field(StringRef("defaults"), offsetof(Arrow, defaults), sizeof(Arrow::defaults), StringRef("int")),
            Field(StringRef("returns"), offsetof(Arrow, returns), sizeof(Arrow::returns), StringRef("ExprNode *")),
            Field(StringRef("args"), offsetof(Arrow, args), sizeof(Arrow::args), StringRef("int")),
        };
        return fields;
    }
};
struct DictTypeType {
    static DictTypeType& cls() { static DictTypeType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("key"), offsetof(DictType, key), sizeof(DictType::key), StringRef("ExprNode *")),
            Field(StringRef("value"), offsetof(DictType, value), sizeof(DictType::value), StringRef("ExprNode *")),
        };
        return fields;
    }
};
struct SetTypeType {
    static SetTypeType& cls() { static SetTypeType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("value"), offsetof(SetType, value), sizeof(SetType::value), StringRef("ExprNode *")),
        };
        return fields;
    }
};
struct ArrayTypeType {
    static ArrayTypeType& cls() { static ArrayTypeType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("value"), offsetof(ArrayType, value), sizeof(ArrayType::value), StringRef("ExprNode *")),
        };
        return fields;
    }
};
struct TupleTypeType {
    static TupleTypeType& cls() { static TupleTypeType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("types"), offsetof(TupleType, types), sizeof(TupleType::types), StringRef("int")),
        };
        return fields;
    }
};
struct BuiltinTypeType {
    static BuiltinTypeType& cls() { static BuiltinTypeType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("name"), offsetof(BuiltinType, name), sizeof(BuiltinType::name), StringRef("StringRef")),
            Field(StringRef("native_macro"), offsetof(BuiltinType, native_macro), sizeof(BuiltinType::native_macro), StringRef("NativeMacro")),
        };
        return fields;
    }
};
struct VMStmtType {
    static VMStmtType& cls() { static VMStmtType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("stmt"), offsetof(VMStmt, stmt), sizeof(VMStmt::stmt), StringRef("StmtNode *")),
        };
        return fields;
    }
};
struct CondJumpType {
    static CondJumpType& cls() { static CondJumpType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("condition"), offsetof(CondJump, condition), sizeof(CondJump::condition), StringRef("ExprNode *")),
            Field(StringRef("then_jmp"), offsetof(CondJump, then_jmp), sizeof(CondJump::then_jmp), StringRef("int")),
            Field(StringRef("else_jmp"), offsetof(CondJump, else_jmp), sizeof(CondJump::else_jmp), StringRef("int")),
        };
        return fields;
    }
};
struct JumpType {
    static JumpType& cls() { static JumpType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("destination"), offsetof(Jump, destination), sizeof(Jump::destination), StringRef("int")),
        };
        return fields;
    }
};
struct VMNativeFunctionType {
    static VMNativeFunctionType& cls() { static VMNativeFunctionType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("fun"), offsetof(VMNativeFunction, fun), sizeof(VMNativeFunction::fun), StringRef("Function")),
        };
        return fields;
    }
};
struct ClassTypeType {
    static ClassTypeType& cls() { static ClassTypeType _; return _; }
    static Array<Field> const& get_fields() {
        static Array<Field> fields = {
            Field(StringRef("def"), offsetof(ClassType, def), sizeof(ClassType::def), StringRef("ClassDef *")),
        };
        return fields;
    }
};
}
