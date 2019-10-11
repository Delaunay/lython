#ifndef LYTHON_TYPES_HEADER
#define LYTHON_TYPES_HEADER

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Utilities/allocator.h"

// ---------------
namespace lython {

using uint32 = std::uint32_t;
using int32 = std::int32_t;

using uint15 = std::uint16_t;
using int16 = std::int16_t;

using uint8 = std::uint8_t;
using int8 = std::int8_t;

using float32 = float;
using float64 = double ;

using uchar = unsigned char;

template<typename V>
using Array = std::vector<V, Allocator<V>>;

using String = std::basic_string<char, std::char_traits<char>, Allocator<char>>;

} // namespace lython



// ------------
namespace std {

template <typename Char, typename Allocator>
struct hash<std::basic_string<Char, std::char_traits<Char>, Allocator>>{
    using key = std::basic_string<Char, std::char_traits<Char>, Allocator>;

    std::size_t operator()(key const& k) const {
        auto a = std::hash<std::string>();
        return a(k.c_str());
    }
};


} // namespace std

// ---------------
namespace lython {


//template<typename _Key, typename _Tp,
//     typename _Hash = hash<_Key>,
//     typename _Pred = equal_to<_Key>,
//     typename _Alloc = allocator<std::pair<const _Key, _Tp>>>
//  class unordered_map

template<typename K, typename V>
using Dict = std::unordered_map<K, V, std::hash<K>, std::equal_to<K>, Allocator<std::pair<K const, V>>>;

//template<typename _Value,
//     typename _Hash = hash<_Value>,
//     typename _Pred = equal_to<_Value>,
//     typename _Alloc = allocator<_Value>>
//  class unordered_set

template<typename V>
using Set = std::unordered_set<V, std::hash<V>, std::equal_to<V>, Allocator<V>>;

#define TYPES_METADATA(X)\
    X(String, String)\
    X(int, Int)\
    X(char, Char)

#define DEFINE_METADATA(type, tname)\
    template <>\
    inline const char* type_name<type>() {\
        static const char* name = _insert_typename<type>(#tname);\
        return name;\
    }

TYPES_METADATA(DEFINE_METADATA)

} // namespace lython


#endif
