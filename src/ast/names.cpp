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
}
