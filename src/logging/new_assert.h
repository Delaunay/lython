

#include "logging/_logging.h"

#ifdef assert
#    undef assert
#endif

// clang-format off
#if WITH_LOG
#    define assert(expr, message) assert_true(((bool)(expr)), (message), #expr, LOC)
#    define kwassert(expr, message) assert_true(((bool)(expr)), (message), #expr, LOC)
#else
#    define assert(expr, message)
#    define kwassert(expr, message)
#endif
// clang-format on