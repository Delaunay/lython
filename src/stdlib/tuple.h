#pragma once

#include "ast/values/value.h"
#include "dtypes.h"

namespace kiwi {

template <typename V>
struct TupleNative {

    std::size_t __sizeof__() const {
        return sizeof(this) + sizeof(V) * values.size();
    }

    bool __contains__(V const& key) const { 
        for(auto const& v: values) {
            if (v == key) {
                return true;
            }
        }
        return false;
    }

    int __len__() const {
        return values.size();
    }

    struct Reverse {
        V __next__() {
            int c = i;
            i -= 1;

            if (c < 0) {
                // raise StopException
            }
            return self.values[c];
        }

        ArrayNative<V>& self;
        int i;
    }

    Reverse reverse(){
        return Reverse{this, int(values.size()) - 1}; 
    }

    struct Iterator {
        V __next__() {
            int c = i;
            i += 1;

            if (c >= self.values.size()) {
                // raise StopException
            }
            return self.values[c];
        }

        ArrayNative<V>& self;
        int i = 0;
    };

    // return Iterable
    Iterator __iter__() {
        return Iterator{*this};
    }
    
    TupleNative<V> copy(){ return this; }

    TupleNative<V> __add__(TupleNative<V> const& other) const {
        ArrayNative<V> result;
        result.values.reserve(other.size() + values.size());
        for(auto const& v: values) {
            result.values.push_back(v);
        }
        for(auto const& v: other.values) {
            result.values.push_back(v);
        }
        return result;
    }

    V __getitem__(int const& key) const {
        return result.values[key];
        // raise
    }

    void __setitem__(int const& key, V const& val) const {
        // raise TypeError
    }

    void __detitem__(int const& key) {
        // raise TypeError
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

using Tuple = TupleNative<lython::Value>;

}  // namespace kiwi