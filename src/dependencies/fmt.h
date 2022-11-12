#pragma once

#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/ostream.h>

#include "dtypes.h"

namespace lython {
using FmtStr = fmt::string_view;

template <typename... Args>
String fmtstr(FmtStr fmtstr, Args&&... args) {
    StringStream ss;
    fmt::print(ss, fmtstr, args...);
    return ss.str();
}
}  // namespace lython