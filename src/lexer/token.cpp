#include "token.h"
#include "utilities/strings.h"
#include <spdlog/fmt/bundled/core.h>

namespace lython {

String to_string(int8 t) {
    switch (t) {
#define X(name, nb) \
    case nb: return String(#name);

        LYTHON_TOKEN(X)
#undef X
    default:
        String s = "' '";
        s[1]     = t;
        return s;
    }
}

String to_human_name(int8 t) {
    static String eof = String("EOF (End Of File)");

    String        n   = to_string(t);
    Array<String> arr = split('_', n);

    if (arr.size() == 2) {
        return arr[1];
    }

    switch (t) {
    case EOF: return eof;
    case '\0': return eof;
    }

    return n;
}

// this should be computed at compile time
// this is used for pretty printing
int8 tok_name_size() {
    std::vector<String> v = {
#define X(name, nb) #name,
        LYTHON_TOKEN(X)
#undef X
    };

    int8 max = 0;

    for (auto& i: v)
        max = std::max(int8(i.size()), max);

    return max;
}

std::ostream& Token::debug_print(std::ostream& out) const {
    out << fmt::format("{:>20}", to_string(_type));

    out << " =>"
        << " [l:" << fmt::format("{:4}", _line) << ", c:" << fmt::format("{:4}", _col) << "] `"
        << _identifier << "`";
    return out;
}

std::ostream& Token::print(std::ostream& out) const {

    if (type() > 0) {
        return out << type();
    }

    if (type() == tok_identifier) {
        return out << identifier();
    } else if (type() == tok_docstring) {
        return out << "\"\"\"" << identifier() << "\"\"\"";
    } else if (type() == tok_int || type() == tok_float) {
        return out << identifier();
    }

    String const& str = keyword_as_string()[type()];

    if (str.size() > 0) {
        return out << str;
    }

    // is this possible ?
    return out << identifier();
}

ReservedKeyword& keywords() {
    static ReservedKeyword _keywords = {
#define X(str, tok) {str, tok},
        LYTHON_KEYWORDS(X)
#undef X
    };
    return _keywords;
}

KeywordToString& keyword_as_string() {
    static KeywordToString _keywords = {
#define X(str, tok) {int(tok), String(str)},
        LYTHON_KEYWORDS(X)
#undef X
    };
    return _keywords;
}

}  // namespace lython
