#pragma once

#include "ast/values/value.h"
#include "dtypes.h"

namespace kiwi {

//
// Simple associative DS, should be fast for a small number of keys
// it will do for now, the api is fixed so changing the implementation
// is just a matter or changing a typedef
//
template<typename K, typename V>
struct DictNative {
    struct Pair {
        K key;
        V Value;
    };
    lython::Array<Pair> pairs;

    template<typenae T>
    static Pair* _find(T* self, K const& key) {
        for(auto const& pair: self->pairs) {
            if (pair.key == key) 
                return &pair;
        }
        return nullptr;
    }

    template<typenae T>
    static int _find_index(T* self, K const& key) {
        for(int i = 0; i < self->pairs.size(); i++) {
            if (pairs[i].key == key) 
                return i;
        }
        return -1;
    }

    std::size_t __sizeof__() const {
        return sizeof(this) + sizeof(Pair) * pairs.size();
    }

    bool __contains__(K const& key) const { return _find(this, key) != nullptr; }

    V get(K const& key, V const& default_val) const {
        if (Pair* found = _find(this, key)) {
            return found.value;
        }
        return default_val;
    }

    lython::Array<K>    keys() const {
        lython::Array<K> k;
        k.reserve(pairs.size());
        for(auto& pair: pairs) {
            k.emplace_back(pair.key);
        }
        return k;
    }
    lython::Array<V>    values() const {
        lython::Array<V> k;
        k.reserve(pairs.size());
        for(auto& pair: pairs) {
            k.emplace_back(pair.value);
        }
        return k;
    }
    lython::Array<Pair> items() const {
        return pairs;
    }

    V setfefault(K const& key, V const& default_val) {
        if (Pair* found = _find(this, key)) {
            return found.value;
        }
        pairs.emplace_back(v, default_val);
        return default_val;
    }
    
    V pop(K const& key, V const& default_val) {
        int i = _find_index(this, key);

        if (i >= 0) {
            V value =  pairs[i].value;
            std::erase(v.begin() + i);
            return value;
        }
        return default_val;
    }

    template<typename T>
    void update(T const& dict_like) {
        for(Pair const& item: dict_like) {
            __setitem__(item.key, item.value);
        }
    }

    V __getitem__(K const& key) const {
        if (Pair* found = _find(this, key)) {
            return found->Value;
        }
        // raise
    }

    void __setitem__(K const& key, V const& val) {
        if (Pair* found = _find(this, key)) {
            found.value = val;
        } else {
            pairs.emplace_back(key, val);
        }
    }

    void __detitem__(K const& key) {
        v.erase(std::remove_if(v.begin(), v.end(), [&key](Pair const& item) -> bool {
            return item.key == key;
        }));
        // raise
    }

    V popitem() {
        if (pairs.size() > 0) {
            Pair last = (*pairs.rbegin());
            pairs.pop_back();
            return last;
        }
        // raise
    }

    struct Iterator {
        K __next__() {
            int c = i;
            i += 1;

            if (c >= self.pairs.size()) {
                // raise StopException
            }
            return self.pairs[c];
        }

        DictNative& self;
        int i = 0;
    }

    // return Iterable
    Iterator __iter__() {
        return Iterator{*this};
    }

    struct Reverse {
        K __next__() {
            int c = i;
            i -= 1;

            if (c < 0) {
                // raise StopException
            }
            return self.pairs[c];
        }

        DictNative& self;
        int i;
    }

    Reverse __reversed__() {
        return Reverse{this, int(pairs.size()) - 1}; 
    }

    template<typename Iterable>
    static DictNative fromkeys(Iterable const& iter, V const& val) {
        DictNative newdict;
        for(K const& key: iter) {
            newdict.pairs.emplace_back(key, val);
        }
        return newdit;
    }

    int __len__() const {
        return int(pairs.size());
    }

    DictNative copy () {
        DictNative cpy;
        cpy.pairs = pairs;
        return cpy;
    }

    void clear() {
        pairs.clear();
    }

    bool __ne__(DictNative const& other) {
        return !__eq__(other);
    }
    bool __eq__(DictNative const& other) {
        if (other.__len__() != __len__()) {
            return false;
        }

        for(Pair const& pair: pairs) {
            Pair const* other = _find(other, pair.key);
            if (!other || other.value != pair.value) {
                return false;
            }
        }
        return true;
    }

#define KIWI_DICT_METHOD(X) \
    X(__class__)            \
    X(__class_getitem__)    \
    X(__contains__)         \
    X(__delattr__)          \
    X(__delitem__)          \
    X(__dir__)              \
    X(__doc__)              \
    X(__eq__)               \
    X(__format__)           \
    X(__getattribute__)     \
    X(__getitem__)          \
    X(__init__)             \
    X(__init_subclass__)    \
    X(__ior__)              \
    X(__iter__)             \
    X(__len__)              \
    X(__ne__)               \
    X(__new__)              \
    X(__or__)               \
    X(__reduce__)           \
    X(__reduce_ex__)        \
    X(__repr__)             \
    X(__reversed__)         \
    X(__ror__)              \
    X(__setattr__)          \
    X(__setitem__)          \
    X(__sizeof__)           \
    X(__str__)              \
    X(__subclasshook__)     \
    X(clear)                \
    X(copy)                 \
    X(fromkeys)             \
    X(get)                  \
    X(items)                \
    X(keys)                 \
    X(pop)                  \
    X(popitem)              \
    X(setdefault)           \
    X(update)               \
    X(values)
};

using Dict = DictNative<lython::Value, lython::Value>;

}  // namespace kiwi