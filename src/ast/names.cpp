#include "names.h"

namespace lython{
StringRef get_string(String const& str){
    return StringDatabase::instance().string(str);
}

std::ostream& operator<< (std::ostream& out, StringRef ref){
    return out << StringDatabase::instance()[ref.ref];
}

String StringRef::str() const {
    auto view = StringDatabase::instance()[ref];
    return String(view);
}

StringRef::operator StringView() const {
    return StringDatabase::instance()[ref];
}

std::ostream& StringDatabase::report(std::ostream& out) const{
    std::size_t saved = 0;
    out << fmt::format("| {:30} | {:4} | {:4} |\n", "str", "#", "svd");

    for(auto i = 0u; i < count.size(); ++i){
        auto s = strings[i].size() * count[i];
        out << fmt::format("| {:30} | {:4} | {:4} |\n", strings[i], count[i], s);
        saved += s;
    }

    out << fmt::format("Saved: {} bytes\n", saved);
    return out;
}
}
