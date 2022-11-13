#include "lexer/lexer.h"
// #include "parser/module.h"

#include "ast/nodes.h"
#include "builtin/operators.h"
#include "parser/parser.h"
#include "sema/sema.h"

#include "allocator.h"
#include "metadata.h"

namespace lython {

#ifdef __linux__
// Shared Pointer & Hash tables do not actually allocate T
// but a struct that includes T and other data
// So we rename that mangled struct name by that type T it manages
// Those details are implementation specific to GCC
template <typename T>
using SharedPtrInternal =
    std::_Sp_counted_ptr_inplace<T, lython::Allocator<T, device::CPU>, std::__default_lock_policy>;
template <typename T, bool cache>
using HashNodeInternal = std::__detail::_Hash_node<T, cache>;
#else
template <typename T>
using SharedPtrInternal = std::shared_ptr<T>;

template <typename T, bool cache>
using HashNodeInternal = std::_List_node<T, void* __ptr64>;

template <typename T, bool cache>
using ListIterator = std::_List_unchecked_iterator<std::_List_val<std::_List_simple_types<T>>>;
#endif

template <typename T>
using UniquePtrInternal = std::unique_ptr<T>;

bool _metadata_init_names() {
    meta::register_type<char>("char");
    meta::register_type<int>("int");
    meta::register_type<lython::StringRef>("StringRef");
    meta::register_type<lython::StringDatabase::StringEntry>("StringDatabase::StringEntry");

    meta::register_type<lython::Node*>("Node*");
    meta::register_type<lython::GCObject*>("GCObject*");
    meta::register_type<lython::ExprNode*>("ExprNode*");
    meta::register_type<lython::StmtNode*>("StmtNode*");
    meta::register_type<lython::ConstantValue>("ConstantValue");
    meta::register_type<lython::BinOp::NativeBinaryOp>("NativeBinaryOperation");

    meta::register_type<UniquePtrInternal<lython::SemaException>>("SemaException");
    meta::register_type<UniquePtrInternal<lython::ParsingException>>("ParsingException");

    meta::register_type<lython::Comprehension>("Comprehension");
    meta::register_type<lython::Alias>("Alias");
    meta::register_type<lython::WithItem>("WithItem");
    meta::register_type<lython::ExceptHandler>("ExceptHandler");
    meta::register_type<lython::Arg>("Arg");
    meta::register_type<lython::CmpOperator>("CmpOperator");
    meta::register_type<lython::Keyword>("Keyword");
    meta::register_type<lython::MatchCase>("MatchCase");
    meta::register_type<lython::Pattern*>("Pattern*");
    meta::register_type<lython::BindingEntry>("BindingEntry");
    meta::register_type<Array<StmtNode*>>("Array<StmtNode*>");
    meta::register_type<ExprContext>("ExprContext");
    meta::register_type<ClassDef::Attr>("ClassDef::Attr");
    meta::register_type<ParsingContext>("ParsingContext");
    meta::register_type<SemaContext>("SemaContext");
    meta::register_type<Decorator>("Decorator");
    meta::register_type<Token>("Token");
    meta::register_type<ParsingError>("ParsingError");

#define REGISTER_TYPE(type)                   \
    meta::register_type<lython::type>(#type); \
    meta::register_type<lython::type*>(#type "*");

#define X(name, _)
#define SECTION(name)
#define EXPR(name, _)  REGISTER_TYPE(name)
#define STMT(name, _)  REGISTER_TYPE(name)
#define MOD(name, _)   REGISTER_TYPE(name)
#define MATCH(name, _) REGISTER_TYPE(name)

    NODEKIND_ENUM(X, SECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

    meta::register_type<
        HashNodeInternal<std::pair<const StringRef, lython::ClassDef::Attr>, false>>(
        "Pair[Ref, Classdef::Attr]");

    meta::register_type<HashNodeInternal<std::pair<const String, lython::OpConfig>, false>>(
        "Pair[String, OpConfig]");

    meta::register_type<HashNodeInternal<std::pair<const String, lython::TokenType>, false>>(
        "Pair[String, TokenType]");

    meta::register_type<HashNodeInternal<std::pair<const String, TokenType>, true>>(
        "Pair[String, TokenType]");

    meta::register_type<HashNodeInternal<std::pair<const String, OpConfig>, true>>(
        "Pair[String, OpConfig]");

    meta::register_type<HashNodeInternal<std::pair<const StringRef, bool>, true>>(
        "Pair[String, bool]");

    meta::register_type<HashNodeInternal<std::pair<const StringRef, lython::ExprNode*>, true>>(
        "Pair[String, ExprNode*]");

    meta::register_type<
        HashNodeInternal<std::pair<const StringRef, lython::BinOp::NativeBinaryOp>, true>>(
        "Pair[String, NativeBinaryOp]");

    meta::register_type<
        HashNodeInternal<std::pair<const StringRef, lython::UnaryOp::NativeUnaryOp>, true>>(
        "Pair[String, NativeUnaryOp]");

    // String Database
    meta::register_type<Array<StringDatabase::StringEntry>*>("Array[StringEntry]*");

#if !__linux__
    // hashtable internal stuff
    // windows only
    meta::register_type<ListIterator<std::pair<const StringRef, lython::ClassDef::Attr>, false>>(
        "Iterator[Pair[Ref, Classdef::Attr]]");

    meta::register_type<ListIterator<std::pair<const int, String>, false>>(
        "Iterator[Pair[int, String]]");

    meta::register_type<ListIterator<std::pair<const std::string_view, std::size_t>, false>>(
        "Iterator[Pair[StringView, size_t]]");

    meta::register_type<ListIterator<std::pair<const String, OpConfig>, false>>(
        "Iterator[Pair[String, OpConfig]]");

    meta::register_type<ListIterator<std::pair<const String, TokenType>, false>>(
        "Iterator[Pair[String, TokenType]]");

    meta::register_type<
        ListIterator<std::pair<const StringRef, lython::BinOp::NativeBinaryOp>, false>>(
        "Iterator[Pair[StringRef, NativeBinaryOp]]");

    meta::register_type<
        ListIterator<std::pair<const StringRef, lython::UnaryOp::NativeUnaryOp>, false>>(
        "Iterator[Pair[StringRef, NativeUnaryOp]]");

    meta::register_type<ListIterator<std::pair<const StringRef, lython::ExprNode*>, false>>(
        "Iterator[Pair[StringRef, ExprNode*]]");

    meta::register_type<ListIterator<std::pair<const StringRef, bool>, false>>(
        "Iterator[Pair[StringRef, bool]]");

    meta::register_type<std::_List_node<Array<StringDatabase::StringEntry>,
                                        typename std::allocator_traits<std::allocator<
                                            Array<StringDatabase::StringEntry>>>::void_pointer>>(
        "ListNode[Array[StringEntry]]");
#endif

    // StringDatabase
    meta::register_type<HashNodeInternal<std::pair<const StringView, std::size_t>, true>>(
        "Pair[StringView, size_t]");

    meta::register_type<HashNodeInternal<std::pair<const StringRef, lython::ExprNode*>, false>>(
        "Pair[StringRef, ExprNode*]");

    meta::register_type<
        HashNodeInternal<std::pair<const StringRef, lython::BinOp::NativeBinaryOp>, false>>(
        "Pair[StringRef, NativeBinaryOp]");

    meta::register_type<
        HashNodeInternal<std::pair<const StringRef, lython::UnaryOp::NativeUnaryOp>, false>>(
        "Pair[StringRef, NativeUnaryOp]");

    meta::register_type<HashNodeInternal<std::pair<const StringRef, bool>, false>>(
        "Pair[StringRef, bool]");

    meta::register_type<std::_List_node<Array<StringDatabase::StringEntry>>>(
        "ListNode[StringEntry]");

    // module
    meta::register_type<HashNodeInternal<std::pair<const String, int>, true>>(
        "Pair[String, Index]");

    // module precedence_table
    meta::register_type<HashNodeInternal<std::pair<const String, std::tuple<int, bool>>, true>>(
        "Pair[String, Tuple[int, bool]]");

    // Keyword to string
    meta::register_type<HashNodeInternal<std::pair<const int, String>, false>>("Pair[int, String]");

    // Set Keyword
    meta::register_type<HashNodeInternal<char, false>>("Set[char]");

#define INIT_METADATA(name, typname) meta::type_name<name>();

    TYPES_METADATA(INIT_METADATA)

    return true;
}

void track_static() {
    // Record the allocation count on startup
    // so we can try to ignore static variables
    // this will only work if `metadata_init_names` is called
    // after the static variables got initialized
    auto& stat = meta::stats();
    for (auto& s: stat) {
        s.startup_count = 0;
        // s.allocated - s.deallocated;
    }
}

void metadata_init_names() { static bool _ = _metadata_init_names(); }

void register_globals() {
    {
        metadata_init_names();

        // Static globals
        {
            StringDatabase::instance();
            default_precedence();
            keywords();
            keyword_as_string();
            native_binary_operators();
            native_bool_operators();
            native_unary_operators();
            native_cmp_operators();
            operator_magic_name(BinaryOperator::Add);
            operator_magic_name(BoolOperator::And);
            operator_magic_name(UnaryOperator::Invert);
            operator_magic_name(CmpOperator::Eq);

            None();
            True();
            False();

            strip_defaults();
        }

        // --
        track_static();
    }
}

}  // namespace lython
