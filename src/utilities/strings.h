#ifndef LYTHON_UTILITIES_STRINGS_HEADER
#define LYTHON_UTILITIES_STRINGS_HEADER

#include "dtypes.h"
#include <algorithm>

namespace lython {

String join(String const& sep, Array<String> const& strs);

Set<char> const& strip_defaults();

String strip(String const& v);

String join(String const& sep, Array<struct ExprNode*> const& exprs);

template <typename T>
String join(String const& sep, Array<T> const& exprs) {
    Array<String> strs;
    strs.reserve(exprs.size());

    ::std::transform(::std::begin(exprs),
                     ::std::end(exprs),
                     std::back_inserter(strs),
                     [](T const& e) -> String { return str(e); });

    return join(sep, strs);
}

Array<String> split(char sep, String const& text);

// template <typename Container, typename S, typename... Args,
//           FMT_ENABLE_IF(
//               is_contiguous<Container>::value&& internal::is_string<S>::value)>

// Replace a by b in t
template <typename T>
T replace(T const& t, char a, T const& b) {
    int n = int(t.size());
    // auto iter = t.rbegin();
    // while (iter != t.rend() && *iter == a){
    //    n -= 1;
    //    iter -= 1;
    //}

    int count = 0;
    for (int i = 0; i < n; ++i) {
        if (t[i] == a) {
            count += 1;
        }
    }

    auto str = T(n + b.size() * size_t(count), ' ');
    int  k   = 0;
    for (int i = 0; i < n; ++i) {
        if (t[i] != a) {
            str[k] = t[i];
            k += 1;
        } else {
            for (int j = 0; j < b.size(); ++j) {
                str[k] = b[j];
                k += 1;
            }
        }
    }

    return str;
}

template <typename T>
inline void print(Array<T> const& obj, std::ostream& out) {
    out << '[' << join(", ", obj) << ']';
}

}  // namespace lython

#endif
