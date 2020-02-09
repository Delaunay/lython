#include "logging.h"

#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <unordered_map>
namespace lython{

const char* log_level_str[] = {
    "[T] TRACE",
    "[I]  INFO",
    "/!\\  WARN",
    "[D] DEBUG",
    "[E] ERROR",
    "[!] FATAL"
};

std::string format_code_loc(const char*, const char* function, int line){
    return fmt::format(
        "{}:{}", function, line);
}


std::string format_code_loc_trace(const char*, const char* function, int line){
    return fmt::format(
        "{:25>}:{:4d}", function, line);
}


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

const char* trace_start = "{} {} {}+-> {}\n";
const char* trace_end = "{} {} {}+-< {}\n";

}
