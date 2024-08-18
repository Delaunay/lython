#pragma once

#include "ast/values/value.h"
#include "dtypes.h"

namespace kiwi {

template <typename V>
struct SetNative {

    std::size_t __sizeof__() const {
        return sizeof(this) + sizeof(V) * values.size();
    }

    template<typename T>
    static V* _find(T* self, T const& key) {
        for(V& value: self->values) {
            if (value == key) 
                return &value;
        }
        return nullptr;
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

    bool isdisjoint(SetNative<V>const& other) {}
    bool issubset(SetNative<V>const& other) {}
    bool issuperset(SetNative<V>const& other) {}
    bool __lte__(SetNative<V>const& other) {}
    bool __lt__(SetNative<V>const& other) {}
    bool __gte__(SetNative<V>const& other) {}
    bool __gt__(SetNative<V>const& other) {}

    SetNative<V> intersection(SetNative<V>const& other) {}
    SetNative<V> __and__(SetNative<V>const& other) {}

    SetNative<V> union_(SetNative<V>const& other) {}
    SetNative<V> __or__(SetNative<V>const& other) {}

    SetNative<V> difference(SetNative<V>const& other) {}
    SetNative<V> __sub__(SetNative<V>const& other) {}
    
    SetNative<V> symmetric_difference(SetNative<V>const& other) {}
    SetNative<V> __xor__(SetNative<V>const& other) {}
    
    void update(SetNative<V>const& other) {}
    void intersection_update(SetNative<V>const& other) {}
    void difference_update(SetNative<V>const& other) {}
    void symmetric_difference_update(SetNative<V>const& other) {}
    
    void add(V const& val){  values.push_back(val);  }
    void discard(V const& val) {}
    void remove(V const& val){
        for(int i = 0; i < values.size(); i++) {
            if (values[i] == val) {
                values.erase(values.begin() + i);
                return;
            }
        }
        // raise KeyError
    }

    V pop() {
        V val = *values.rbegin();
        values.pop_back();
        return val;
        // raise KeyError
    }
    void clear(){ values.clear(); }

    void index(V const& val, int start=0, int end=-1){
        if (start < 0) {
            start += int(values.size());
        }
        if (end < 0) {
            end += int(values.size());
        }
        for(int i = start; i < end; i++) {
            if (values[i] == val) {
                return i;
            }
        }
        // raise ValueError
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
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        reference operator*() const { return *ptr_; }
         pointer operator->() const { return ptr_; }

        V __next__() {
            int c = i;
            i += 1;

            if (c >= self.values.size()) {
                // raise StopException
            }
            return self.values[c];
        }

        bool operator==(Iterator const& other) const {
            return i == other.i;
        }

        SetNative<T>& values;
        int i = 0;
    }

    Iterator begin() { return Iterator{0, self.values.size()}; }
    Iterator end() { return Iterator{self.values.size()}; }

    // return Iterable
    Iterator __iter__() {
        return Iterator{*this};
    }
    
    ArrayNative<V> copy(){ return this; }

    void __detitem__(V const& key) {
        remove(key);
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

using Set = SetNative<lython::Value>;

}  // namespace kiwi