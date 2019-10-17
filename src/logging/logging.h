#pragma once

#include <string>


namespace lython{

enum LogLevel{
    Trace,
    Info,
    Warn,
    Debug,  // force flush
    Error,
    Fatal
};

void set_log_level(LogLevel level, bool enabled);

bool is_log_enabled(LogLevel level);

void log(LogLevel level, const char* file, int line, const char* function, std::string format, ...);

void log_trace(LogLevel level, bool end, size_t depth, const char* file, int line, const char* function, std::string format, ...);

} // namespace lython

#define LOG_HELPER(level, ...)\
    log(level, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#define LOG_TRACE_HELPER(level, end, depth, ...)\
    log_trace(level, end, depth, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

// info("test %s", "const char*")
#define info(...) LOG_HELPER(lython::Info , __VA_ARGS__)
#define warn(...) LOG_HELPER(lython::Warn , __VA_ARGS__)
#define debug(...) LOG_HELPER(lython::Debug, __VA_ARGS__)
#define error(...) LOG_HELPER(lython::Error, __VA_ARGS__)
#define fatal(...) LOG_HELPER(lython::Fatal, __VA_ARGS__)

#define trace(depth, ...)\
    LOG_TRACE_HELPER(lython::Trace, false, depth, __VA_ARGS__)

#define trace_start(depth, ...)\
    LOG_TRACE_HELPER(lython::Trace, false, depth, __VA_ARGS__)

#define trace_end(depth, ...)\
    LOG_TRACE_HELPER(lython::Trace, true, depth, __VA_ARGS__)
