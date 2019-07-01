#pragma once

#include <string>


namespace lython{

enum LogLevel{
    Info,
    Warn,
    Debug,
    Error,
    Fatal,
    Trace
};

void log(LogLevel level, const char* file, int line, const char* function, std::string format, ...);

void log_trace(LogLevel level, int depth, const char* file, int line, const char* function, std::string format, ...);

} // namespace lython

#define LOG_HELPER(level, ...)\
    log(level, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#define LOG_TRACE_HELPER(level, depth, ...)\
    log_trace(level, depth, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#define info(...) LOG_HELPER(lython::Info , __VA_ARGS__)
#define warn(...) LOG_HELPER(lython::Warn , __VA_ARGS__)
#define debug(...) LOG_HELPER(lython::Debug, __VA_ARGS__)
#define error(...) LOG_HELPER(lython::Error, __VA_ARGS__)
#define fatal(...) LOG_HELPER(lython::Fatal, __VA_ARGS__)

#define trace(depth, ...)\
    LOG_TRACE_HELPER(lython::Trace, depth, __VA_ARGS__)
