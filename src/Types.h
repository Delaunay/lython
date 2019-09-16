#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace lython
{

typedef std::uint32_t uint32;
typedef std::int32_t int32;

typedef std::uint16_t uint15;
typedef std::int16_t int16;

typedef std::uint8_t uint8;
typedef std::int8_t int8;

typedef float float32;
typedef double float64;

typedef unsigned char uchar;

template<typename K, typename V>
using Dict = std::unordered_map<K, V>;

template<typename V>
using Array = std::vector<V>;

template<typename V>
using Set = std::unordered_set<V>;

using String = std::string;

}
