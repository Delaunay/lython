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

void log(LogLevel level, std::string format, const char* file, int line, const char* function, ...);

void log_trace(LogLevel level, std::string format, int depth, const char* file, int line, const char* function, ...);

} // namespace lython

#define LOG_HELPER(level, fmt, ...)\
    log(level, fmt, __FILE__, __LINE__, __FUNCTION__ __VA_OPT__(,) __VA_ARGS__)\

#define LOG_TRACE_HELPER(level, depth, fmt, ...)\
    log(level, fmt, __FILE__, __LINE__, __FUNCTION__ __VA_OPT__(,) __VA_ARGS__)

#define info(fmt , ...) LOG_HELPER(lython::Info , fmt __VA_OPT__(,) __VA_ARGS__)
#define warn(fmt , ...) LOG_HELPER(lython::Warn , fmt __VA_OPT__(,) __VA_ARGS__)
#define debug(fmt, ...) LOG_HELPER(lython::Debug, fmt __VA_OPT__(,) __VA_ARGS__)
#define error(fmt, ...) LOG_HELPER(lython::Error, fmt __VA_OPT__(,) __VA_ARGS__)
#define fatal(fmt, ...) LOG_HELPER(lython::Fatal, fmt __VA_OPT__(,) __VA_ARGS__)

#define trace(depth, fmt, ...)\
    LOG_TRACE_HELPER(lython::Trace, depth, fmt __VA_OPT__(,) __VA_ARGS__)
