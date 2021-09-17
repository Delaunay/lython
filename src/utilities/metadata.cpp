#include "lexer/lexer.h"
// #include "parser/module.h"

#include "ast/sexpression.h"
#include "parser/parser.h"

#include "allocator.h"
#include "metadata.h"

namespace lython{

#ifdef __linux__
// Shared Pointer & Hash tables do not actually allocate T
// but a struct that includes T and other data
// So we rename that mangled struct name by that type T it manages
// Those details are implementation specific to GCC
template<typename T>
using SharedPtrInternal = std::_Sp_counted_ptr_inplace<
    T, lython::Allocator<T, device::CPU>, std::__default_lock_policy>;

template<typename T, bool cache>
using HashNodeInternal = std::__detail::_Hash_node<T, cache>;
#else
template<typename T>
using SharedPtrInternal = std::shared_ptr<T>;

template<typename T, bool cache>
using HashNodeInternal = T;
#endif

bool _metadata_init_names(){
    meta::register_type<int>("int");

		meta::register_type<lython::GCObject*>("GCObject*");
		meta::register_type<lython::ExprNode*>("ExprNode*");
		meta::register_type<lython::StmtNode*>("StmtNode*");

		meta::register_type<lython::FunctionDef>("FunctionDef");
		meta::register_type<lython::Name>("Name");
		meta::register_type<lython::Arg>("Arg");
		meta::register_type<lython::ParsingError>("ParsingError");
		meta::register_type<lython::Return>("Return");
		meta::register_type<lython::Call>("Call");
		meta::register_type<lython::Constant>("Constant");

    meta::register_type<HashNodeInternal<
				std::pair<const String, lython::OpConfig>, false>>(
        "Pair[String, OpConfig]");

    meta::register_type<HashNodeInternal<
				std::pair<const String, lython::TokenType>, false>>(
        "Pair[String, TokenType]");

    meta::register_type<HashNodeInternal<
        std::pair<const String, TokenType>, true>>(
        "Pair[String, TokenType]");

    meta::register_type<HashNodeInternal<
        std::pair<const String, OpConfig>, true>>(
        "Pair[String, OpConfig]");

    // StringDatabase
    meta::register_type<HashNodeInternal<
        std::pair<const StringView, std::size_t>, true>>(
        "Pair[StringView, size_t]");

    // module
    meta::register_type<HashNodeInternal<
        std::pair<const String, int>, true>>(
        "Pair[String, Index]");

    // module precedence_table
    meta::register_type<HashNodeInternal<
        std::pair<const String, std::tuple<int, bool>>, true>>(
        "Pair[String, Tuple[int, bool]]");

    // Keyword to string
    meta::register_type<HashNodeInternal<
        std::pair<const int, String>, false>>(
        "Pair[int, String]");

    // Set Keyword
    meta::register_type<HashNodeInternal<
        char, false>>(
        "Set char");

    #define INIT_METADATA(name, typname)\
        meta::type_name<name>();

    TYPES_METADATA(INIT_METADATA)
    return true;
}


void metadata_init_names(){
    static bool _ = _metadata_init_names();
}

} // namespace lython
