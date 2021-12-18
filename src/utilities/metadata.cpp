#include "lexer/lexer.h"
// #include "parser/module.h"

#include "ast/nodes.h"
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
using HashNodeInternal = T;
#endif

bool _metadata_init_names() {
    meta::register_type<int>("int");
    meta::register_type<lython::StringRef>("StringRef");
    meta::register_type<lython::StringDatabase::StringEntry>("StringDatabase::StringEntry");

    meta::register_type<lython::GCObject *>("GCObject*");
    meta::register_type<lython::ExprNode *>("ExprNode*");
    meta::register_type<lython::StmtNode *>("StmtNode*");

    meta::register_type<lython::Comprehension>("Comprehension");
    meta::register_type<lython::Alias>("Alias");
    meta::register_type<lython::WithItem>("WithItem");
    meta::register_type<lython::ExceptHandler>("ExceptHandler");
    meta::register_type<lython::Arg>("Arg");
    meta::register_type<lython::CmpOperator>("CmpOperator");
    meta::register_type<lython::Keyword>("Keyword");
    meta::register_type<lython::MatchCase>("MatchCase");
    meta::register_type<lython::Pattern *>("Pattern*");
    meta::register_type<lython::BindingEntry>("BindingEntry");
    meta::register_type<Array<StmtNode *>>("Array<StmtNode*>");

#define REGISTER_TYPE(type)                   \
    meta::register_type<lython::type>(#type); \
    meta::register_type<lython::type *>(#type "*");

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

    meta::register_type<HashNodeInternal<std::pair<const String, lython::OpConfig>, false>>(
        "Pair[String, OpConfig]");

    meta::register_type<HashNodeInternal<std::pair<const String, lython::TokenType>, false>>(
        "Pair[String, TokenType]");

    meta::register_type<HashNodeInternal<std::pair<const String, TokenType>, true>>(
        "Pair[String, TokenType]");

    meta::register_type<HashNodeInternal<std::pair<const String, OpConfig>, true>>(
        "Pair[String, OpConfig]");

    // StringDatabase
    meta::register_type<HashNodeInternal<std::pair<const StringView, std::size_t>, true>>(
        "Pair[StringView, size_t]");

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
    auto &stat = meta::stats();
    for (auto &s: stat) {
        s.startup_count = s.allocated - s.deallocated;
    }
}

void metadata_init_names() { static bool _ = _metadata_init_names(); }

} // namespace lython
