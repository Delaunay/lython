

#include "logging/_logging.h"

#ifdef assert
#    undef assert
#endif

#if WITH_LOG
#    define assert(expr, message) assert_true(((bool)(expr)), (message), #    expr, LOC)
#else
#    define assert(expr, message)
#endif