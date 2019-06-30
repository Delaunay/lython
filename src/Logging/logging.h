#pragma once

#include <string>


namespace lython{

enum LogLevel{
    Info,
    Warm,
    Debug,
    Error,
    Fatal,
    Trace
};

void log(LogLevel level, std::string format, const char* file, int line, const char* function, ...);

void log_trace(LogLevel level, std::string format, int depth, const char* file, int line, const char* function, ...);

}

#define info(fmt, ...)  log(Info , fmt, __FILE__, __LINE__, __FUNCTION__ __VA_OPT__(,) __VA_ARGS__)
#define warm(fmt, ...)  log(Warm , fmt, __FILE__, __LINE__, __FUNCTION__ __VA_OPT__(,) __VA_ARGS__)
#define debug(fmt, ...) log(Debug, fmt, __FILE__, __LINE__, __FUNCTION__ __VA_OPT__(,) __VA_ARGS__)
#define error(fmt, ...) log(Error, fmt, __FILE__, __LINE__, __FUNCTION__ __VA_OPT__(,) __VA_ARGS__)
#define fatal(fmt, ...) log(Fatal, fmt, __FILE__, __LINE__, __FUNCTION__ __VA_OPT__(,) __VA_ARGS__)

#define trace(depth, fmt, ...) log_trace(Trace, fmt, depth, __FILE__, __LINE__, __FUNCTION__ __VA_OPT__(,) __VA_ARGS__)
