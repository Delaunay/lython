#ifndef LYTHON_LOGGING_HEADER
#    define LYTHON_LOGGING_HEADER

#    include <cassert>
#    include <spdlog/fmt/bundled/core.h>
#    include <string>
#    include <vector>

#    include "revision_data.h"

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

    std::string repr() const { return fmt::format("{}:{} {}", filename, line, function_name); }
};

#    ifdef __linux__
#        define LOC lython::CodeLocation(__FILE__, __FUNCTION__, __LINE__, __PRETTY_FUNCTION__)
#    else
#        define LOC lython::CodeLocation(__FILE__, __FUNCTION__, __LINE__, __func__)
#    endif

enum LogLevel
{
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
    void
    _log_message(LogLevel level, CodeLocation const& loc, const char* fmt, const Args&... args) {}

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
void log(LogLevel level, CodeLocation const& loc, const char* fmt, const Args&... args) {
    if (!is_log_enabled(level)) {
        return;
    }

    auto msg = fmt::format("{}:{} {} - {}",
                           loc.filename,
                           loc.line,
                           format_function(loc.function_name),
                           fmt::format(fmt, args...));

    spdlog_log(level, msg);
}

template <typename... Args>
void log_trace(LogLevel            level,
               size_t              depth,
               bool                end,
               CodeLocation const& loc,
               const char*         fmt,
               const Args&... args) {
    if (!is_log_enabled(level)) {
        return;
    }

    const char* msg_fmt = "{:>30}:{:4} {}+-> {}";
    if (end) {
        msg_fmt = "{:>30}:{:4} {}+-< {}";
    }

    std::string str(depth, ' ');
    for (size_t i = 0; i < depth; ++i) {
        str[i] = i % 2 ? '|' : ':';
    }

    auto msg = fmt::format(
        msg_fmt, format_function(loc.function_name), loc.line, str, fmt::format(fmt, args...));

    spdlog_log(level, msg);
}

// Exception that shows the backtrace when .what() is called
class Exception: public std::exception {
    public:
    template <typename... Args>
    Exception(const char* fmt, std::string const& name, const Args&... args):
        message(fmt::format(fmt, name, args...)) {}

    const char* what() const noexcept final;

    private:
    std::string message;
};

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

#endif

// Make a simple exception
#define NEW_EXCEPTION(name)                                                                        \
    class name: public Exception {                                                                 \
        public:                                                                                    \
        template <typename... Args>                                                                \
        name(const char* fmt, const Args&... args): Exception(fmt, std::string(#name), args...) {} \
    };

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

#ifdef assert
#    undef assert
#endif

#define assert(expr, message) assert_true(((bool)(expr)), (message), #expr, LOC)