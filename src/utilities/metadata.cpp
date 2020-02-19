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

template<typename T>
using HashNodeInternal = std::__detail::_Hash_node<T, false>;


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

    register_type<HashNodeInternal<
        std::pair<const StringRef, Value>>>("Pair[StringRef, Value]");

    register_type<HashNodeInternal<
        std::pair<const String, Index>>>("Pair[String, Index]");

    register_type<HashNodeInternal<
        std::pair<const String, std::function<Value(Array<Value>)>>>>("Pair[String, Value(Value[])]");

    register_type<HashNodeInternal<
        std::pair<const String, TokenType>>>("Pair[String, TokenType]");


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
