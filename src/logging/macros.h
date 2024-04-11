

#include "logging/logger.h"

// clang-format off
#if WITH_LOG
#   define lyassert(expr, message) assert_true(((bool)(expr)), (message), #expr, LOC)
#   define kwassert(expr, message) assert_true(((bool)(expr)), (message), #expr, LOC)

#   define kwinfo(ly_logref, ...)  (info(ly_logref, LOC, __VA_ARGS__))
#   define kwwarn(ly_logref, ...)  (warn(ly_logref, LOC, __VA_ARGS__))
#   define kwdebug(ly_logref, ...) (debug(ly_logref, LOC, __VA_ARGS__))
#   define kwerror(ly_logref, ...) (err(ly_logref, LOC, __VA_ARGS__))
#   define kwfatal(ly_logref, ...) (fatal(ly_logref, LOC, __VA_ARGS__))

#   define kwtrace(ly_logref, depth, ...)       (trace_trace<false>(ly_logref, LOC, depth, __VA_ARGS__))
#   define kwtrace_start(ly_logref, depth, ...) (trace_trace<false>(ly_logref, LOC, depth, __VA_ARGS__))
#   define kwtrace_end(ly_logref, depth, ...)   (trace_trace<true>(ly_logref, LOC, depth, __VA_ARGS__))

#else
#   define lyassert(expr, message)
#   define kwassert(expr, message)
#   define SYM_LOG_HELPER(...)
#   define SYM_LOG_TRACE_HELPER(...)

#   define kwinfo(log, ...)  
#   define kwwarn(log, ...)  
#   define kwdebug(log, ...) 
#   define kwerror(log, ...) 
#   define kwfatal(log, ...)

#   define kwtrace(log, depth, ...)       
#   define kwtrace_start(log, depth, ...) 
#   define kwtrace_end(log, depth, ...)  
#endif
// clang-format on


#define KW_NEWLOG(name)                                                \
    inline                                                             \
    lython::Logger& name ## _log() {                                   \
        static lython::Logger* log = &lython::new_stdout_log(#name);    \
        return *log;                                                    \
    }

