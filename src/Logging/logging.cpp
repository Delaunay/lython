#include "logging.h"

#include <cstdio>
#include <cstdarg>

namespace lython{

const char* log_level_str[] = {
    "[I]  INFO",
    "/!\\  WARM",
    "[D] DEBUG",
    "[E] ERROR",
    "[!] FATAL",
    "[T] TRACE"
};

void log(LogLevel level, std::string format, const char* file, int line, const char* function, ...){
    static const int BUFFER_SIZE = 120;
    char buffer [BUFFER_SIZE];

    snprintf(buffer, BUFFER_SIZE, "%s %s:%d %s - %s\n", log_level_str[level], file, line, function, format.c_str());

    va_list arglist;
    va_start(arglist, buffer);
    vprintf(buffer, arglist);
    va_end(arglist);
}


void log_trace(LogLevel level, std::string format, int depth, const char* file, int line, const char* function, ...){
    static const int BUFFER_SIZE = 120;
    char buffer [BUFFER_SIZE];

    std::string str(depth, ' ');
    snprintf(buffer, BUFFER_SIZE, "%s %25s:%4d %s %s\n",
             log_level_str[level], function, line, str.c_str(), format.c_str());

    va_list arglist;
    va_start(arglist, buffer);
    vprintf(buffer, arglist);
    va_end(arglist);
}
}
