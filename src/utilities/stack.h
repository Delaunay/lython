#ifndef LYTHON_STACK_H
#define LYTHON_STACK_H

#include "dtypes.h"

namespace lython {

template <typename V>
class Stack {
    public:
    using ReverseIterator      = typename Array<V>::iterator;
    using Iterator             = typename Array<V>::reverse_iterator;
    using ConstReverseIterator = typename Array<V>::const_iterator;
    using ConstIterator        = typename Array<V>::const_reverse_iterator;

    Stack() {
        meta::type_id<V>();
        meta::type_name<V>();
    }

    void push(V const& value) {
        stack.push_back(value);
        _size += 1;
    }

    V pop() {
        V v = stack[_size];
        stack.pop_back();
        _size -= 1;
        return v;
    }

    V const& peek() const { return stack[_size]; }

    int size() const { return _size + 1; }

    Iterator        begin() { return stack.rbegin(); }
    Iterator        end() { return stack.rend(); }
    ReverseIterator rbegin() { return stack.begin(); }
    ReverseIterator rend() { return stack.end(); }

    ConstIterator        begin() const { return stack.rbegin(); }
    ConstIterator        end() const { return stack.rend(); }
    ConstReverseIterator rbegin() const { return stack.begin(); }
    ConstReverseIterator rend() const { return stack.end(); }

    private:
    Array<V> stack;
    int      _size = -1;
};

}  // namespace lython

#endif
