#include "lexer/lexer.h"
#include "parser/module.h"

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
    meta::register_type<AST::Parameter>("Parameter");
    meta::register_type<lython::Expression>("Expression");
    meta::register_type<lython::Value>("Value");
    meta::register_type<lython::State::Entry>("State::Entry");
    meta::register_type<lython::AST::Import::DeclarationImport>("DeclImport");

    meta::register_type<SharedPtrInternal<lython::value::Closure>>("Closure");
    meta::register_type<SharedPtrInternal<lython::value::Class>>("Class");
    meta::register_type<SharedPtrInternal<lython::AST::Call>>("AST::Call");
    meta::register_type<SharedPtrInternal<lython::AST::Arrow>>("AST::Arrow");
    meta::register_type<SharedPtrInternal<lython::AST::Value>>("AST::Value");
    meta::register_type<SharedPtrInternal<lython::AST::Struct>>("AST::Struct");
    meta::register_type<SharedPtrInternal<lython::AST::Ref>>("AST::Reference");
    meta::register_type<SharedPtrInternal<lython::AST::Builtin>>("AST::Builtin");
    meta::register_type<SharedPtrInternal<lython::AST::Type>>("AST::Type");
    meta::register_type<SharedPtrInternal<lython::AST::Function>>("AST::Function");
    meta::register_type<SharedPtrInternal<lython::AST::SeqBlock>>("AST::SeqBlock");
    meta::register_type<SharedPtrInternal<lython::AST::Statement>>("AST::Statement");
    meta::register_type<SharedPtrInternal<lython::AST::Parameter>>("AST::Parameter");
    meta::register_type<SharedPtrInternal<lython::AST::BinaryOperator>>("AST::BinaryOperator");
    meta::register_type<SharedPtrInternal<lython::AST::Import>>("AST::Import");
    meta::register_type<SharedPtrInternal<lython::AST::ImportedExpr>>("AST::ImportedExpr");
    meta::register_type<SharedPtrInternal<lython::AST::UnparsedBlock>>("AST::UnparsedBlock");
    meta::register_type<SharedPtrInternal<lython::AST::Match>>("AST::Match");

    // value::Struct
    meta::register_type<HashNodeInternal<
        std::pair<const StringRef, Value>, false>>(
        "Pair[StringRef, Value]");

    // Module dict
    meta::register_type<HashNodeInternal<
        std::pair<const String, State>, true>>(
        "Pair[String, State]");

    // AST::Struct
    meta::register_type<HashNodeInternal<
        std::pair<const StringRef, int>, false>>(
        "Pair[StringRef, int]");


    meta::register_type<HashNodeInternal<
        std::pair<const String, lython::OpConfig>, true>>(
        "Pair[String, OpConfig]");

    meta::register_type<HashNodeInternal<
        std::pair<const String, lython::TokenType>, true>>(
        "Pair[String, TokenType]");

    // AST::KwArguments
    meta::register_type<HashNodeInternal<
        std::pair<const StringRef, Expression>, false>>(
        "Pair[StringRef, Expression]");

    // AST::Variables
    meta::register_type<HashNodeInternal<
        std::pair<const AST::Parameter, Expression>, false>>(
        "Pair[Parameter, Expression]");

    // ParameterDict
    meta::register_type<HashNodeInternal<
        std::pair<const String, AST::Parameter>, false>>(
        "Pair[String, Parameter]");

    // interpreter
    meta::register_type<HashNodeInternal<
        std::pair<const String, std::function<Value(State&)>>, true>>(
        "Pair[String, Value(State)]");

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


    // std::unordered_map


    //using PairStringSize_t = Dict<String, Index>::value_type;
    //register_type<PairStringSize_t>("Pair[String, Index]");

    //using PairStringTupleIntBool = Dict<String, std::tuple<int, bool>>::value_type;
    //register_type<PairStringTupleIntBool>("Pair[String, Tuple[Int, Bool]]");

    meta::register_type<Attributes::value_type>("Attribute");

    // Token
    //using RKeyword =  Dict<String, TokenType>::value_type;
    //register_type<RKeyword>("RKeyword");

    //using Keyword =  Dict<int, String>::value_type;
    //register_type<Keyword>("Keyword");
    //register_type<Token>("Token");
    //register_type<AST::MathNode>("MathNode");

    #define INIT_METADATA(name, typname)\
        meta::type_name<name>();

    TYPES_METADATA(INIT_METADATA)
    return true;
}


void metadata_init_names(){
    static bool _ = _metadata_init_names();
}

} // namespace lython
