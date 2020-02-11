#include "parser/module.h"
#include "allocator.h"
#include "metadata.h"

namespace lython{

void metadata_init_names(){

    type_id<AST::Parameter>();
    _insert_typename<AST::Parameter>("Parameter");

    using PairStringSize_t = Dict<String, Index>::value_type;
    type_id<PairStringSize_t>();
    _insert_typename<PairStringSize_t>("Pair[String, Index]");

    using PairStringTupleIntBool = Dict<String, std::tuple<int, bool>>::value_type;
    type_id<PairStringTupleIntBool>();
    _insert_typename<PairStringTupleIntBool>("Pair[String, Tuple[Int, Bool]]");

//    type_id<Expression>();
//    _insert_typename<Expression>("Expr");

    type_id<Attributes::value_type>();
    _insert_typename<Attributes::value_type>("Attribute");

    // Token
    using RKeyword =  Dict<String, TokenType>::value_type;
    type_id<RKeyword>();
    _insert_typename<RKeyword>("RKeyword");

    using Keyword =  Dict<int, String>::value_type;
    type_id<Keyword>();
    _insert_typename<Keyword>("Keyword");

    type_id<Token>();
    _insert_typename<Token>("Token");

    type_id<AST::MathNode>();
    _insert_typename<AST::MathNode>("MathNode");

    #define INIT_METADATA(name, typname)\
        type_name<name>();

    TYPES_METADATA(INIT_METADATA)
}

} // namespace lython
