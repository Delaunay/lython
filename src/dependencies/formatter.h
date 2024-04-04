#pragma once

#include <fmt/core.h>


template <>
struct fmt::formatter<lython::String>: formatter<string_view> {
    auto format(lython::String const& c, format_context& ctx) const {
        string_view view = lython::StringView(c);
        return formatter<string_view>::format(view, ctx);
    }
};

template <>
struct fmt::formatter<lython::StringRef>: formatter<string_view> {
    auto format(lython::StringRef c, format_context& ctx) const {
        string_view view = lython::StringView(c);
        return formatter<string_view>::format(view, ctx);
    }
};


template <>
struct fmt::formatter<lython::NodeKind>: formatter<string_view> {
    auto format(lython::NodeKind c, format_context& ctx) const {
        string_view view = str(c);
        return formatter<string_view>::format(view, ctx);
    }
}; 

