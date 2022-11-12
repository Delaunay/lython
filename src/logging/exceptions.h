#pragma once

#include "dependencies/fmt.h"

#include <exception>

#ifdef __linux__
#    define LY_NOEXCEPT _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW
#endif

namespace lython {

// Exception that shows the backtrace when .what() is called
class Exception: public std::exception {
    public:
    template <typename... Args>
    Exception(FmtStr fmt, String const& name, const Args&... args):
        message(fmtstr(fmt, name, args...)) {}

    const char* what() const LY_NOEXCEPT final {
        spdlog_log(LogLevel::Error, fmt::format("Exception raised: {}", message));
        show_backtrace();
        return message.c_str();
    }

    private:
    String message;
};

// Make a simple exception
#define NEW_EXCEPTION(name)                                                                    \
    class name: public Exception {                                                             \
        public:                                                                                \
        template <typename... Args>                                                            \
        name(FmtStr fmtstr, const Args&... args): Exception(fmtstr, String(#name), args...) {} \
    };

}  // namespace lython