#include "utilities/strings.h"
#include "utilities/names.h"

#include "ast/magic.h"
#include "ast/nodes.h"

namespace lython {

template <typename Iterator>
String join(String const& sep, Iterator const& start, Iterator const& end) {
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
        StringView const& str = *i;
        std::copy(std::begin(str), std::end(str), result.begin() + size);
        size += str.size();

        if (size_t(size) < result.size()) {
            std::copy(std::begin(sep), std::end(sep), result.begin() + size);
            size += sep.size();
        }
    }

    return result;
}

String join(String const& sep, Array<String> const& strs) {
    return join(sep, std::begin(strs), std::end(strs));
}

String join(String const& sep, Array<StringView> const& strs) {
    return join(sep, std::begin(strs), std::end(strs));
}

String join(String const& sep, Array<StringRef> const& strs) {
    return join(sep, std::begin(strs), std::end(strs));
}

template String join(String const& sep, Array<ExprNode*> const& exprs);
template String join(String const& sep, Array<Pattern*> const& exprs);

Set<char> const& strip_defaults() {
    static Set<char> char_set = {'\n', ' ', '\t'};
    return char_set;
}

String strip(String const& v) {
    Set<char> const& char_set = strip_defaults();

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
    int    new_size = int(v.size()) - (k + p);

    if (new_size <= 0) {
        return result;
    }

    result.reserve(new_size);

    std::copy(std::begin(v) + k, std::end(v) - p, std::back_inserter(result));
    return result;
}

Array<String> split(char sep, String const& text) {
    int count = 1;
    for (auto c: text) {
        if (c == sep) {
            count += 1;
        }
    }

    Array<String> frags;
    frags.reserve(count);
    String buffer;

    for (int i = 0; i < text.size(); i++) {
        char c = text[i];

        if (c == sep) {
            frags.push_back(buffer);
            buffer.resize(0);
        } else {
            buffer.push_back(c);
        }
    }

    frags.push_back(buffer);
    return frags;
}

}  // namespace lython
