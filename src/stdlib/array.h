#pragma once

// #include "ast/values/value.h"
// #include "dtypes.h"
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <optional>
#include <variant>
#include <vector>

#define KIWI_STANDALONE
#ifdef KIWI_STANDALONE
namespace lython {
//template <typename V>
//using Array = std::vector<V>;
}
#endif


namespace kiwi {


struct StopIteration {};
struct ValueError {};
struct IndexError {};


struct Slice {
    int start;
    int step;
    int end;
};

//
// Adapt C++ iterators for python
//
template <typename It>
struct PyIter {
    using value_type = typename It::value_type;

    It begin;
    It end;
    int step;

    std::variant<value_type, StopIteration> __next__() {
        if (begin != end) {
            auto v = *begin;
            begin += step;
            return v;
        }
        return StopIteration();
    }
};

struct error {};
template <typename V>  //
struct ArrayNative {

    using UnderlyingIterator = typename lython::Array<V>::iterator;
    using ArrayPyIter = PyIter<UnderlyingIterator>;

    std::size_t __sizeof__() const { return sizeof(this) + sizeof(V) * values.size(); }

    template <typename T>  //
    static V* _find(T* self, V const& key) {
        for (V& value: self->values) {
            if (value == key)
                return &value;
        }
        return nullptr;
    }

    bool __contains__(V const& key) const {
        for (auto const& v: values) {
            if (v == key) {
                return true;
            }
        }
        return false;
    }

    int __len__() const { return values.size(); }

    void append(V const& val) { values.push_back(val); }

    void extend(ArrayNative<V> const& other) {
        values.reserve(other.size() + values.size());
        for (auto const& v: other.values) {
            values.push_back(v);
        }
    }
    void insert(int i, V const& val) { values.insert(values.begin() + i, val); }
    std::optional<ValueError> remove(V const& val) {
        for (int i = 0; i < values.size(); i++) {
            if (values[i] == val) {
                values.erase(values.begin() + i);
                return {};
            }
        }
        return ValueError();
    }

    std::variant<V, IndexError> pop(int i = -1) {
        if (values.empty()) {
            return IndexError();
        }

        if (i < 0) {
            i += int(values.size());
        }

        V val = values[i];
        values.erase(values.begin() + i);
        return val;
    }
    void clear() { values.clear(); }

    std::variant<int, ValueError> index(V const& val, int start = 0, int end = -1) {
        if (start < 0) {
            start += int(values.size());
        }
        if (end < 0) {
            end += int(values.size());
        }
        for (int i = start; i < end; i++) {
            if (values[i] == val) {
                return i;
            }
        }
        return ValueError();
    }

    int count(V const& val) {
        int acc = 0;
        for (V const& v: values) {
            if (v == val) {
                acc += 1;
            }
        }
        return acc;
    }

    void sort() { std::sort(values.begin(), values.end()); }

    struct Reverse {
        // C++20
        // using iterator_concept  = std::contiguous_iterator_tag;
        using iterator_category = std::random_access_iterator_tag;

        using difference_type = std::ptrdiff_t;
        using value_type      = V;
        using pointer         = V*;
        using reference       = V&;

        // clang-format: off
                        operator bool() const { return i > 0; }
        reference       operator*() const { return self.values[i]; }
        pointer         operator->() const { return &self.values[i]; }
        bool            operator<(Reverse const& other) const { return i > other.i; }
        bool            operator>(Reverse const& other) const { return i < other.i; }
        bool            operator<=(Reverse const& other) const { return i >= other.i; }
        bool            operator>=(Reverse const& other) const { return i <= other.i; }
        bool            operator==(Reverse const& other) const { return i == other.i; }
        bool            operator!=(Reverse const& other) const { return i != other.i; }
        difference_type operator-(Reverse const& other) const { return i + other.i; }
        Reverse         operator+(difference_type n) const { return Reverse{self, i - n}; }
        Reverse         operator-(difference_type n) const { return Reverse{self, i + n}; }
        Reverse&        operator+=(difference_type n) {
            i -= n;
            return *this;
        }
        Reverse& operator-=(difference_type n) {
            i += n;
            return *this;
        }
        Reverse& operator++() {
            i -= 1;
            return *this;
        }
        Reverse& operator--() {
            i += 1;
            return *this;
        }
        Reverse operator++(int) {
            Reverse temp = *this;
            i -= 1;
            return temp;
        }
        Reverse operator--(int) {
            Reverse temp = *this;
            i += 1;
            return temp;
        }
        // clang-format: on

        std::variant<V, StopIteration> __next__() {
            int c = i;
            i -= 1;

            if (c < 0) {
                return StopIteration{};
            }
            return self.values[c];
        }

        ArrayNative<V>& self;
        int             i;
    };

    Reverse rbegin() { return Reverse{*this, int(values.size()) - 1}; }
    Reverse rend() { return Reverse{*this, -1}; }
    Reverse reverse() { return Reverse{this, int(values.size()) - 1}; }

    struct Iterator {
        // C++20
        // using iterator_concept  = std::contiguous_iterator_tag;
        using iterator_category = std::random_access_iterator_tag;

        using difference_type = std::ptrdiff_t;
        using value_type      = V;
        using pointer         = V*;
        using reference       = V&;

        // clang-format: off
                        operator bool() const { return i < self.values.size(); }
        reference       operator*() const { return self.values[i]; }
        pointer         operator->() const { return &self.values[i]; }
        bool            operator<(const Iterator& other) const { return i < other.i; }
        bool            operator>(const Iterator& other) const { return i > other.i; }
        bool            operator<=(const Iterator& other) const { return i <= other.i; }
        bool            operator>=(const Iterator& other) const { return i >= other.i; }
        bool            operator==(Iterator const& other) const { return i == other.i; }
        bool            operator!=(Iterator const& other) const { return i != other.i; }
        difference_type operator-(const Iterator& other) const { return i - other.i; }
        Iterator        operator+(difference_type n) const { return Iterator{self, i + n}; }
        Iterator        operator-(difference_type n) const { return Iterator{self, i - n}; }
        Iterator&       operator+=(difference_type n) {
            i += n;
            return *this;
        }
        Iterator& operator-=(difference_type n) {
            i -= n;
            return *this;
        }
        Iterator& operator++() {
            i += 1;
            return *this;
        }
        Iterator& operator--() {
            i -= 1;
            return *this;
        }
        Iterator operator++(int) {
            Iterator temp = *this;
            i += 1;
            return temp;
        }
        Iterator operator--(int) {
            Iterator temp = *this;
            i -= 1;
            return temp;
        }
        // clang-format: on

        // we would like to return Optional<V>
        std::variant<V, StopIteration> __next__() {
            int c = i;
            i += 1;

            if (c >= self.values.size()) {
                // raise StopException
                return StopIteration{};
            }
            return self.values[c];
        }

        ArrayNative<V>& self;
        int             i = 0;
    };

    // return Iterable
    Iterator begin() { return Iterator{*this, 0}; }
    Iterator end() { return Iterator{*this, int(values.size())}; }
    Iterator __iter__() { return Iterator{*this}; }

    ArrayNative<V> copy() { return this; }

    ArrayNative<V> __add__(ArrayNative<V> const& other) const {
        ArrayNative<V> result;
        result.values.reserve(other.size() + values.size());
        for (auto const& v: values) {
            result.values.push_back(v);
        }
        for (auto const& v: other.values) {
            result.values.push_back(v);
        }
        return result;
    }

    bool normalize_index(int& idx) {
        if (idx < 0) {
            idx += int(values.size());
        }
        if (idx >= values.size()) {
            return true;
        }
        return false;
    }

    bool normalize_slice(Slice& slice) {
        if (slice.start < 0) {
            slice.start += int(values.size());
        }
        if (slice.start >= values.size()) {
            return true;
        }
        if (slice.end < 0) {
            slice.end += int(values.size());
        }
        if (slice.end >= values.size()) {
            return true;
        }
        return false;
    };

    std::variant<ArrayPyIter, IndexError> __getitem__(Slice const& key) const {
        if (normalize_slice(key)) {
            return IndexError();
        }
        return ArrayPyIter{values.begin() + key.start, values.begin() + key.end, key.step};
    }

    std::variant<V, IndexError> __getitem__(int const& key) const {
        if (normalize_index(key)) {
            return IndexError();
        }
        return values[key];
    }

    std::optional<IndexError> __setitem__(int const& key, V const& val) const {
        if (normalize_index(key)) {
            return IndexError();
        }
        values[key] = val;
    }

    template<typename It>
    std::optional<IndexError> __setitem__(Slice const& key, It& values) const {
        if (normalize_slice(key)) {
            return IndexError();
        }

        auto destiter = ArrayPyIter{
            values.begin() + key.start, 
            values.begin() + key.end, 
            key.step
        };

        // TODO: check the error when destiter is not filled
        // TODO: check the error when values size is bigger
        while (destiter.begin != destiter.end) {
            std::variant<V, StopIteration> val = values.__next__();
            
            if (val.index == 0) {
                (*destiter.begin) = std::get<0>(val);
                destiter.begin += key.step;
            } else {
                break;
            }
        } 
    }

    std::optional<IndexError> __detitem__(int key) {
        if (key < 0) {
            key += int(values.size());
        }
        if (key >= values.size()) {
            return IndexError();
        }
        values.erase(values.begin() + key);
    }

    lython::Array<V> values;
};

#define KIWI_ARRAY_METHOD(X) \
    X(__add__)               \
    X(__class__)             \
    X(__class_getitem__)     \
    X(__contains__)          \
    X(__delattr__)           \
    X(__delitem__)           \
    X(__dir__)               \
    X(__doc__)               \
    X(__eq__)                \
    X(__format__)            \
    X(__ge__)                \
    X(__getattribute__)      \
    X(__getitem__)           \
    X(__gt__)                \
    X(__hash__)              \
    X(__iadd__)              \
    X(__imul__)              \
    X(__init__)              \
    X(__init_subclass__)     \
    X(__iter__)              \
    X(__le__)                \
    X(__len__)               \
    X(__lt__)                \
    X(__mul__)               \
    X(__ne__)                \
    X(__new__)               \
    X(__reduce__)            \
    X(__reduce_ex__)         \
    X(__repr__)              \
    X(__reversed__)          \
    X(__rmul__)              \
    X(__setattr__)           \
    X(__setitem__)           \
    X(__sizeof__)            \
    X(__str__)               \
    X(__subclasshook__)      \
    X(append)                \
    X(clear)                 \
    X(copy)                  \
    X(count)                 \
    X(extend)                \
    X(index)                 \
    X(insert)                \
    X(pop)                   \
    X(remove)                \
    X(reverse)               \
    X(sort)

// using Array = ArrayNative<lython::Value>;

}  // namespace kiwi