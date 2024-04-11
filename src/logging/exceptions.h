#pragma once

#include <exception>

//
#include "logging/logger.h"
#include "compatibility/compatibility.h"
#include "dependencies/fmt.h"

namespace lython {

// Exception that shows the backtrace when .what() is called
class Exception: public std::exception {
    public:
    template <typename... Args>
    Exception(FmtStr fmt, String const& name, const Args&... args):
        message(fmtstr(fmt, name, args...)) {}

    const char* what() const LY_NOEXCEPT final {
        outlog().err(LOC, "Exception raised: {}", message);
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