#include "logging.h"

#include <cstdio>
#include <cstdarg>

const char* log_level_str[] = {
    "[I]  INFO",
    "/!\\  WARM",
    "[D] DEBUG",
    "[E] ERROR",
    "[!] FATAL"
};

void log(LogLevel level, std::string format, const char* file, int line, const char* function, ...){
    static const int BUFFER_SIZE = 100;
    char buffer [BUFFER_SIZE];

    snprintf(buffer, BUFFER_SIZE, "%s %s:%d %s \n  - %s\n", log_level_str[level], file, line, function, format.c_str());

    va_list arglist;
    va_start(arglist, buffer);
    vprintf(buffer, arglist);
    va_end(arglist);
}
