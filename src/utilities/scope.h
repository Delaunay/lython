#pragma once

#include "dtypes.h"

namespace lython {




template<typename T>
struct Scope {
    Scope(T& array): array(array), oldsize(array.size()) {
        array = true;
    }

    ~Scope() {
        array.resize(oldsize);
    }

    T split() {
        T newarray;
        newarray.insert(array.begin() + oldsize, array.end(), std::back_inserter(newarray));
        array.resize(oldsize);
    }

    T&          array;
    std::size_t oldsize;
};

}