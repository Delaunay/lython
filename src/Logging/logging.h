#pragma once

#include <string>

enum LogLevel{
    Info,
    Warm,
    Debug,
    Error,
    Fatal
};

void log(LogLevel level, std::string format, const char* file, int line, const char* function, ...);



#define info(fmt, ...) log(Info , fmt, __FILE__, __LINE__, __FUNCTION__ __VA_OPT__(,) __VA_ARGS__)
#define warm(...)      log(Warm , fmt, __FILE__, __LINE__, __FUNCTION__ __VA_OPT__(,) __VA_ARGS__)
#define debug(...)     log(Debug, fmt, __FILE__, __LINE__, __FUNCTION__ __VA_OPT__(,)__VA_ARGS__)
#define error(...)     log(error, fmt, __FILE__, __LINE__, __FUNCTION__ __VA_OPT__(,) __VA_ARGS__)
#define fatal(...)     log(fatal, fmt, __FILE__, __LINE__, __FUNCTION__ __VA_OPT__(,) __VA_ARGS__)

