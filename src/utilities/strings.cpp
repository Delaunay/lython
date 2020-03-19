#include "utilities/strings.h"

namespace lython {

String join(String const& sep, Array<String> const& strs){
    if (strs.size() == 1)
        return strs.at(0);

    if (strs.size() == 0)
        return "";

    std::ptrdiff_t size = 0;
    for(auto& str: strs)
        size += str.size();

    size += sep.size() * (strs.size() - 1);
    String result(size_t(size), ' ');

    size = 0;
    for(auto& str: strs){
        std::copy(
            std::begin(str),
            std::end(str),
            result.begin() + size);
        size += str.size();

        if (size_t(size) < result.size()){
            std::copy(
                std::begin(sep),
                std::end(sep),
                result.begin() + size);
            size += sep.size();
        }
    }

    return result;
}

String strip(String const& v){
    static Set<char> char_set = {'\n', ' ', '\t'};

    // remove trailing from the left
    auto k = 0ul;
    for(auto i = v.begin(); i != v.end(); ++i){
        if (char_set.count(*i) == 0){
            break;
        }
        k += 1;
    }

    // remove trailing from the right
    auto p = 0ul;
    for(auto i = v.rbegin(); i != v.rend(); ++i){
        if (char_set.count(*i) == 0){
            break;
        }
        p += 1;
    }

    String result;
    result.reserve(v.size() - (k + p));

    std::copy(std::begin(v) + k, std::end(v) - p, std::back_inserter(result));
    return result;
}


}
