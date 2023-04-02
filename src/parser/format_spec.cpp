#include "format_spec.h"
#include <algorithm>

namespace lython {

String FormatSpecifier::__str__() const {
    StringStream ss;
    if (fill != '\0')
        ss << fill;
    if (align != '\0')
        ss << align;
    if (sign != '\0')
        ss << sign;
    if (alternate != '\0')
        ss << alternate;
    if (pad != '\0')
        ss << pad;
    if (width != '\0')
        ss << width;
    if (precision != '\0')
        ss << "." << precision;
    if (type != '\0')
        ss << type;
    return ss.str();
}

String FormatSpecifier::__repr__() const {
    StringStream ss;
    bool         needs_comma = false;

    auto p = [&](String name, auto val) {
        if (needs_comma) {
            ss << ", ";
        }
        ss << name << "=" << val;
        needs_comma = true;
    };

    ss << "Fmt(";

    if (fill != '\0')
        p("fill", fill);

    if (align != '\0')
        p("align", align);

    if (sign != '\0')
        p("sign", sign);
    if (alternate != '\0')
        p("alternate", alternate);
    if (pad != '\0')
        p("pad", pad);
    if (width != '\0')
        p("width", width);
    if (precision != '\0')
        p("precision", precision);
    if (type != '\0')
        p("type", type);

    ss << ")";
    return ss.str();
}

bool contains(Array<char> const& arr, char target) {
    for (auto& val: arr) {
        if (val == target) {
            return true;
        }
    }
    return false;
}

bool FormatSpecifier::is_float() const {
    return contains({'e', 'E', 'f', 'F', 'g', 'G', 'n', '%'}, type);
}
bool FormatSpecifier::is_integer() const {
    return contains({'b', 'c', 'd', 'o', 'x', 'X', 'n'}, type);
}
bool FormatSpecifier::is_undefined() const { return type == '\0'; }

struct FormatSpecParser {
    enum class ParsingStep
    {
        Fill,
        Align,
        Sign,
        Alternate,
        Pad,
        Width,
        Precision,
        Type,
    };

    ParsingStep step = ParsingStep::Type;

    inline static Array<char> valid_align     = {'<', '>', '=', '^'};
    inline static Array<char> valid_sign      = {'+', '-', ' '};
    inline static Array<char> valid_alternate = {'#'};
    inline static Array<char> valid_pad       = {'0'};
    inline static Array<char> valid_type      = {
        'b', 'c', 'd', 'o', 'x', 'X', 'n', 'e', 'E', 'f', 'F', 'g', 'G', 'n', '%'};

    FormatSpecifier parse(String const& buffer) {
        int             i = int(buffer.size()) - 1;
        FormatSpecifier spec;
        String          temp;

        while (i >= 0) {
            char c = buffer[i];

            if (step == ParsingStep::Fill) {
                spec.fill = c;
                break;
            }

            if (step == ParsingStep::Align) {
                if (contains(valid_align, c)) {
                    spec.align = c;
                    i -= 1;
                }

                step = ParsingStep::Fill;
                continue;
            }

            if (step == ParsingStep::Sign) {
                if (contains(valid_sign, c)) {
                    spec.sign = c;
                    i -= 1;
                }

                step = ParsingStep::Align;
                continue;
            }

            if (step == ParsingStep::Alternate) {
                if (contains(valid_alternate, c)) {
                    spec.alternate = c;
                    i -= 1;
                }

                step = ParsingStep::Sign;
                continue;
            }

            if (step == ParsingStep::Width) {
                temp.clear();
                while (std::isdigit(c) && i >= 0) {
                    temp.push_back(c);
                    i -= 1;

                    if (i >= 0) {
                        c = buffer[i];
                    } else {
                        break;
                    }
                }

                reverse(temp.begin(), temp.end());
                spec.width = std::strtoull(temp.c_str(), nullptr, 10);

                if (temp[0] == '0') {
                    spec.pad = '0';
                }
                step = ParsingStep::Alternate;
                continue;
            }

            if (step == ParsingStep::Precision) {
                temp.clear();
                while (std::isdigit(c)) {
                    temp.push_back(c);
                    i -= 1;

                    if (i >= 0) {
                        c = buffer[i];
                    } else {
                        break;
                    }
                }

                reverse(temp.begin(), temp.end());
                uint64_t val = std::strtoull(temp.c_str(), nullptr, 10);

                if (c == '.') {
                    i -= 1;
                    spec.precision = val;
                    step           = ParsingStep::Width;
                } else {
                    spec.width = val;
                    if (temp[0] == '0') {
                        spec.pad = '0';
                    }
                    step = ParsingStep::Alternate;
                }
                continue;
            }

            if (step == ParsingStep::Type) {
                if (contains(valid_type, c)) {
                    spec.type = c;
                    i -= 1;
                }
                step = ParsingStep::Precision;
                continue;
            }
        }

        spec.valid = true;
        if (i > 0) {
            spec.valid = false;
            spec.unparsed = buffer.substr(0, i);
        }

        return spec;
    }
};

FormatSpecifier FormatSpecifier::parse(String const& buffer) {
    return FormatSpecParser().parse(buffer);
}

}  // namespace lython