#ifndef LYTHON_UTILITIES_STRINGS_HEADER
#define LYTHON_UTILITIES_STRINGS_HEADER

#include "dtypes.h"

namespace lython {

String join(String const& sep, Array<String> const& strs);

String strip(String const& v);

// Replace a by b in t
template<typename T>
T replace(T const& t, char a, T const& b) {
    int n = t.size();
    auto iter = t.rbegin();
    while (*iter == '\n'){
        n -= 1;
        iter -= 1;
    }

    int count = 0;
    for (int i = 0; i < n; ++i){
        if (t[i] == a){
            count += 1;
        }
    }

    auto str = T(n + b.size() * size_t(count), ' ');
    int k = 0;
    for(int i = 0; i < n; ++i){
        if (t[i] != a){
            str[k] = t[i];
            k += 1;
        } else {
            for(int j = 0; j < b.size(); ++j){
                str[k] = b[j];
                k += 1;
            }
        }
    }

    return str;
}
}

#endif
