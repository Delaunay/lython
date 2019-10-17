#include "logging.h"

#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <unordered_map>

namespace lython{

const int BUFFER_SIZE = 120;

static const char* log_level_str[] = {
    "[T] TRACE",
    "[I]  INFO",
    "/!\\  WARN",
    "[D] DEBUG",
    "[E] ERROR",
    "[!] FATAL"
};

// instead of setting a single log level for the entire program allow to cherry pick
// which level is enabled
std::unordered_map<LogLevel, bool>& log_levels(){
    static std::unordered_map<LogLevel, bool> levels{
        {Info,  true},
        {Warn,  true},
        {Debug, true},
        {Error, true},
        {Fatal, true},
        {Trace, true}
    };
    return levels;
}

void set_log_level(LogLevel level, bool enabled){
    log_levels()[level] = enabled;
}

bool is_log_enabled(LogLevel level){
    return log_levels()[level];
}


void log(LogLevel level, const char* file, int line, const char* function, std::string format, ...){
    if (! is_log_enabled(level)){
        return ;
    }

    char buffer [BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE,
             "%s %s:%d %s - %s\n", log_level_str[level], file, line, function, format.c_str());

    va_list arglist;
    va_start(arglist, buffer);
    vprintf(buffer, arglist);
    va_end(arglist);

    if (level > Debug){
        fflush(stdout);
    }
}

static const char* trace_start = "%s %25s:%4d %s+-> %s\n";
static const char* trace_end = "%s %25s:%4d %s+-< %s\n";

void log_trace(LogLevel level, bool end, size_t depth, const char*, int line, const char* function, std::string format, ...){
    if (! is_log_enabled(level)){
        return ;
    }

    const char* fmt = trace_start;
    if (end){
        fmt = trace_end;
    }

    char buffer [BUFFER_SIZE];
    std::string str(depth, ' ');
    for (size_t i = 0; i < depth; ++i){
        str[i] = i % 2 ? '|' : ':';
    }

    snprintf(buffer, BUFFER_SIZE, fmt, log_level_str[level], function, line, str.c_str(), format.c_str());

    va_list arglist;
    va_start(arglist, buffer);
    vprintf(buffer, arglist);
    va_end(arglist);
}
}
