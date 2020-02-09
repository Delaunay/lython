#pragma once

#include <vector>
#include <string>
#include <fmt/core.h>

// This file should not include spdlog
// spdlog file is compile time cancer so it is only included inside the .cpp


namespace lython{

enum LogLevel{
    Trace,
    Info,
    Warn,
    Debug,  // force flush
    Error,
    Fatal
};

void spdlog_log(LogLevel level, const std::string& msg);

extern const char* log_level_str[];
extern const char* trace_start;
extern const char* trace_end;

std::string format_code_loc(const char* file, const char* function, int line);
std::string format_code_loc_trace(const char* file, const char* function, int line);

void set_log_level(LogLevel level, bool enabled);

bool is_log_enabled(LogLevel level);

// Show backtrace using spdlog
void show_log_backtrace();

// Show backtrace using execinfo
void show_backtrace();

// retrieve backtrace using execinfo
std::vector<std::string> get_backtrace(size_t size);

template<typename ... Args>
void log(LogLevel level, std::string loc, const char* fmt, const Args& ... args){
    if (! is_log_enabled(level)){
        return ;
    }

    auto msg = fmt::format("{} - {}", /*log_level_str[level],*/ loc, fmt::format(fmt, args...));
    spdlog_log(level, msg);
}

template<typename ... Args>
void log_trace(LogLevel level, size_t depth, bool end, std::string loc,  const char* fmt, const Args& ... args){
    if (! is_log_enabled(level)){
        return ;
    }

    const char* msg_fmt = trace_start;
    if (end){
        msg_fmt = trace_end;
    }

    std::string str(depth, ' ');
    for (size_t i = 0; i < depth; ++i){
        str[i] = i % 2 ? '|' : ':';
    }

    auto msg = fmt::format(msg_fmt, /*log_level_str[level],*/ loc, str, fmt::format(fmt, args...));
    spdlog_log(level, msg);
}

} // namespace lython

#define CODE_LOC\
    lython::format_code_loc(__FILE__, __FUNCTION__, __LINE__)

#define CODE_LOC_TRACE\
    lython::format_code_loc_trace(__FILE__, __FUNCTION__, __LINE__)

#define LOG_HELPER(level, ...)\
    lython::log(level, CODE_LOC __VA_OPT__(,) __VA_ARGS__)

#define LOG_TRACE_HELPER(level, depth, end, fmt, ...)\
    lython::log_trace(level, depth, end, CODE_LOC_TRACE, fmt __VA_OPT__(,) __VA_ARGS__)

// info("test %s", "const char*")
#define info(...) LOG_HELPER(lython::Info , __VA_ARGS__)
#define warn(...) LOG_HELPER(lython::Warn , __VA_ARGS__)
#define debug(...) LOG_HELPER(lython::Debug, __VA_ARGS__)
#define error(...) LOG_HELPER(lython::Error, __VA_ARGS__)
#define fatal(...) LOG_HELPER(lython::Fatal, __VA_ARGS__)

#define trace(depth, ...)\
    LOG_TRACE_HELPER(lython::Trace, depth, false, __VA_ARGS__)

#define trace_start(depth, ...)\
    LOG_TRACE_HELPER(lython::Trace, depth, false, __VA_ARGS__)

#define trace_end(depth, ...)\
    LOG_TRACE_HELPER(lython::Trace, depth, true, __VA_ARGS__)

inline void assert_true(bool cond, char const* message,  char const* assert_expr, char const* file, int line, char const* function){
    if (!cond){
        error("Assertion errror: {}\n"
              "  - expr: {}\n"
              "  - function: {}\n"
              "  - file: {}:{}", message, assert_expr, function, file, line);
    }
}

#ifdef assert
#undef assert
#endif

#define assert(expr, message)\
    assert_true(static_cast <bool> (expr), message, __FILE__, __LINE__, __FUNCTION__)
