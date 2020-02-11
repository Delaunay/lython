#ifndef LYTHON_FOLD_HEADER
#define LYTHON_FOLD_HEADER

#include "expressions.h"

#include <functional>

namespace lython {
void foreach(std::function<void(Expression)> fun, Expression node);

template <typename T>
T fold(std::function<T(T, Expression)> fun, T result, Expression node){
    std::function<void(Expression)> foreach_fun = [&](Expression a){
        result = fun(result, a);
    };
    foreach(foreach_fun, node);
    return result;
}

}

#endif
