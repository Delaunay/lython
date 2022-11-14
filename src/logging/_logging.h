#ifndef LYTHON_LOGGING_H
#define LYTHON_LOGGING_H

#include <cassert>
#include <string>
#include <vector>

#include "revision_data.h"

#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/ostream.h>

// This file should not include spdlog
// spdlog file is compile time cancer so it is only included inside the .cpp

namespace lython {

// Path to repository on current system
constexpr char __source_dir[] = _SOURCE_DIRECTORY;
// Length of the path so we can cut unimportant folders
constexpr int __size_src_dir = sizeof(_SOURCE_DIRECTORY) / sizeof(char);

struct CodeLocation {
    CodeLocation(std::string const& file,
                 std::string const& fun,
                 int                line,
                 std::string const& fun_long):
        filename(file.substr(__size_src_dir)),
        function_name(fun), line(line), function_long(fun_long) {}

    std::string filename;
    std::string function_name;
    int         line;
    std::string function_long;

    static CodeLocation const& noloc() {
        static CodeLocation loc(__FILE__, "", 0, "");
        return loc;
    }

    std::string repr() const { return fmt::format("{}:{} {}", filename, line, function_name); }
};

// Avoid spending time creating the same codeloc multiple time
// clang-format off
using CodeLocationConstRef = CodeLocation const&;

#define MAKE_LOC(fun, funp) lython::CodeLocation(__FILE__, fun, __LINE__, funp)
#define LAMBDA_LOC(fun, funp) (([](const char* afun, const char* afunp) -> lython::CodeLocationConstRef { static lython::CodeLocation loc = lython::CodeLocation(__FILE__, afun, __LINE__, afunp); return loc; })(fun, funp))
// clang-format on

#if WITH_LOG
#    ifdef __linux__
#        define LOC LAMBDA_LOC(__FUNCTION__, __PRETTY_FUNCTION__)
#    else
#        define LOC LAMBDA_LOC(__FUNCTION__, __func__)
#    endif
#else
#    define LOC lython::CodeLocation::noloc()
#endif

enum LogLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Fatal,
    Off
};

struct Handle {};
typedef Handle* LoggerHandle;

LoggerHandle new_log(const char* name, std::ostream& out);

class LoggingScope {

    void set_log_level();

    void logerr();
    void logwarn();
    void logdebug();
    void loginfo();
    void logtrace();
    void logfatal();

    template <typename... Args>
    void _log_message(LogLevel            level,
                      CodeLocation const& loc,
                      fmt::string_view    fmtstr,
                      const Args&... args) {}

    LoggerHandle logger;
};

void spdlog_log(LogLevel level, const std::string& msg);

extern const char* log_level_str[];
extern const char* trace_start;
extern const char* trace_end;

void set_log_level(LogLevel level, bool enabled);

bool is_log_enabled(LogLevel level);

// Show backtrace using spdlog
void show_log_backtrace();

// Show backtrace using execinfo
void show_backtrace();

std::string demangle(std::string const& original_str);

// retrieve backtrace using execinfo
std::vector<std::string> get_backtrace(size_t size);

// remove namespace info
std::string format_function(std::string const&);

template <typename... Args>
void log(LogLevel level, CodeLocation const& loc, fmt::string_view fmtstr, const Args&... args) {

#if WITH_LOG
    if (!is_log_enabled(level)) {
        return;
    }

    auto msg = fmt::format("{}:{} {} - {}",
                           loc.filename,
                           loc.line,
                           format_function(loc.function_name),
                           fmt::format(fmtstr, args...));

    spdlog_log(level, msg);
#endif
}

template <typename... Args>
void log_trace(LogLevel            level,
               size_t              depth,
               bool                end,
               CodeLocation const& loc,
               fmt::string_view    fmtstr,
               const Args&... args) {

#if WITH_LOG
    if (!is_log_enabled(level)) {
        return;
    }

    fmt::string_view msg_fmt = "{:>30}:{:4} {}+-> {}";
    if (end) {
        msg_fmt = "{:>30}:{:4} {}+-< {}";
    }
    auto log_message = fmt::format(fmtstr, args...);

    std::string str(depth, ' ');
    for (size_t i = 0; i < depth; ++i) {
        str[i] = i % 2 ? '|' : ':';
    }

    auto msg = fmt::format(msg_fmt, format_function(loc.function_name), loc.line, str, log_message);

    spdlog_log(level, msg);
#endif
}

inline void assert_true(bool                        cond,
                        char const*                 message,
                        char const*                 assert_expr,
                        lython::CodeLocation const& loc) {
    if (!cond) {
        lython::log(lython::LogLevel::Error,
                    loc,
                    "Assertion errror: {}\n"
                    "  - expr: {}",
                    message,
                    assert_expr);

        // lython::show_backtrace();
        assert(cond);
    }
}
}  // namespace lython

#define SYM_LOG_HELPER(level, ...) lython::log(level, LOC, __VA_ARGS__)
#define info(...)                  SYM_LOG_HELPER(lython::LogLevel::Info, __VA_ARGS__)
#define warn(...)                  SYM_LOG_HELPER(lython::LogLevel::Warn, __VA_ARGS__)
#define debug(...)                 SYM_LOG_HELPER(lython::LogLevel::Debug, __VA_ARGS__)
#define error(...)                 SYM_LOG_HELPER(lython::LogLevel::Error, __VA_ARGS__)
#define fatal(...)                 SYM_LOG_HELPER(lython::LogLevel::Fatal, __VA_ARGS__)

#define SYM_LOG_TRACE_HELPER(level, depth, end, ...) \
    lython::log_trace(level, depth, end, LOC, __VA_ARGS__)

#define trace(depth, ...) SYM_LOG_TRACE_HELPER(lython::LogLevel::Trace, depth, false, __VA_ARGS__)
#define trace_start(depth, ...) \
    SYM_LOG_TRACE_HELPER(lython::LogLevel::Trace, depth, false, __VA_ARGS__)
#define trace_end(depth, ...) \
    SYM_LOG_TRACE_HELPER(lython::LogLevel::Trace, depth, true, __VA_ARGS__)
#endif