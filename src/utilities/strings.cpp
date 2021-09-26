#include "utilities/names.h"
#include "utilities/strings.h"

#include "ast/magic.h"
#include "ast/sexpression.h"

namespace lython {

template <typename Iterator>
String join(String const &sep, Iterator const &start, Iterator const &end) {
    int count = int(end - start);

    if (count == 1)
        return String(*start);

    if (count == 0)
        return "";

    std::ptrdiff_t size = 0;
    for (auto i = start; i != end; ++i)
        size += StringView(*i).size();

    size += int(sep.size()) * (count - 1);
    String result(size_t(size), ' ');

    size = 0;
    for (auto i = start; i != end; ++i) {
        StringView const &str = *i;
        std::copy(std::begin(str), std::end(str), result.begin() + size);
        size += str.size();

        if (size_t(size) < result.size()) {
            std::copy(std::begin(sep), std::end(sep), result.begin() + size);
            size += sep.size();
        }
    }

    return result;
}

String join(String const &sep, Array<String> const &strs) {
    return join(sep, std::begin(strs), std::end(strs));
}

String join(String const &sep, Array<StringView> const &strs) {
    return join(sep, std::begin(strs), std::end(strs));
}

String join(String const &sep, Array<StringRef> const &strs) {
    return join(sep, std::begin(strs), std::end(strs));
}

template String join(String const &sep, Array<ExprNode *> const &exprs);
template String join(String const &sep, Array<Pattern *> const &exprs);

String strip(String const &v) {
    static Set<char> char_set = {'\n', ' ', '\t'};

    // remove trailing from the left
    auto k = 0ul;
    for (auto i = v.begin(); i != v.end(); ++i) {
        if (char_set.count(*i) == 0) {
            break;
        }
        k += 1;
    }

    // remove trailing from the right
    auto p = 0ul;
    for (auto i = v.rbegin(); i != v.rend(); ++i) {
        if (char_set.count(*i) == 0) {
            break;
        }
        p += 1;
    }

    String result;
    result.reserve(v.size() - (k + p));

    std::copy(std::begin(v) + k, std::end(v) - p, std::back_inserter(result));
    return result;
}

} // namespace lython
