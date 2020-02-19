#include "parser/module.h"
#include "allocator.h"
#include "metadata.h"

namespace lython{

template<typename T>
void register_type(const char* name){
    type_id<T>();
    _insert_typename<T>(name);
}

template<typename T>
using SharedPtrInternal = std::_Sp_counted_ptr_inplace<
    T, lython::Allocator<T, device::CPU>, std::__default_lock_policy>;

template<typename T, bool cache>
using HashNodeInternal = std::__detail::_Hash_node<T, cache>;


void metadata_init_names(){
    register_type<int>("int");
    register_type<AST::Parameter>("Parameter");
    register_type<lython::Expression>("Expression");
    register_type<lython::Value>("Value");

    register_type<SharedPtrInternal<lython::value::Closure>>("Closure");
    register_type<SharedPtrInternal<lython::value::Class>>("Class");
    register_type<SharedPtrInternal<lython::AST::Call>>("AST::Call");
    register_type<SharedPtrInternal<lython::AST::Arrow>>("AST::Arrow");
    register_type<SharedPtrInternal<lython::AST::Value>>("AST::Value");
    register_type<SharedPtrInternal<lython::AST::Struct>>("AST::Struct");
    register_type<SharedPtrInternal<lython::AST::Ref>>("AST::Reference");
    register_type<SharedPtrInternal<lython::AST::Builtin>>("AST::Builtin");
    register_type<SharedPtrInternal<lython::AST::Type>>("AST::Type");

    // value::Struct
    register_type<HashNodeInternal<
        std::pair<const StringRef, Value>, false>>(
        "Pair[StringRef, Value]");

    // AST::Struct
    register_type<HashNodeInternal<
        std::pair<const StringRef, int>, false>>(
        "Pair[StringRef, int]");

    // AST::KwArguments
    register_type<HashNodeInternal<
        std::pair<const StringRef, Expression>, false>>(
        "Pair[StringRef, Expression]");

    // AST::Variables
    register_type<HashNodeInternal<
        std::pair<const AST::Parameter, Expression>, false>>(
        "Pair[Parameter, Expression]");

    // ParameterDict
    register_type<HashNodeInternal<
        std::pair<const String, AST::Parameter>, false>>(
        "Pair[String, Parameter]");

    // interpreter
    register_type<HashNodeInternal<
        std::pair<const String, std::function<Value(Array<Value>&)>>, true>>(
        "Pair[String, Value(Value[])]");

    register_type<HashNodeInternal<
        std::pair<const String, TokenType>, true>>(
        "Pair[String, TokenType]");

    // StringDatabase
    register_type<HashNodeInternal<
        std::pair<const StringView, std::size_t>, true>>(
        "Pair[StringView, size_t]");

    // module
    register_type<HashNodeInternal<
        std::pair<const String, Index>, true>>(
        "Pair[String, Index]");

    // module precedence_table
    register_type<HashNodeInternal<
        std::pair<const String, std::tuple<int, bool>>, true>>(
        "Pair[String, Tuple[int, bool]]");

    // Keyword to string
    register_type<HashNodeInternal<
        std::pair<const int, String>, false>>(
        "Pair[int, String]");

    // Set Keyword
    register_type<HashNodeInternal<
        char, false>>(
        "Set char");


    // std::unordered_map


    //using PairStringSize_t = Dict<String, Index>::value_type;
    //register_type<PairStringSize_t>("Pair[String, Index]");

    //using PairStringTupleIntBool = Dict<String, std::tuple<int, bool>>::value_type;
    //register_type<PairStringTupleIntBool>("Pair[String, Tuple[Int, Bool]]");

    register_type<Attributes::value_type>("Attribute");

    // Token
    //using RKeyword =  Dict<String, TokenType>::value_type;
    //register_type<RKeyword>("RKeyword");

    //using Keyword =  Dict<int, String>::value_type;
    //register_type<Keyword>("Keyword");
    //register_type<Token>("Token");
    //register_type<AST::MathNode>("MathNode");

    #define INIT_METADATA(name, typname)\
        type_name<name>();

    TYPES_METADATA(INIT_METADATA)
}

} // namespace lython
