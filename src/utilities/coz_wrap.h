

#if WITH_COZ
#    include "coz.h"
#else
// clang-format off
#    define STR2(x) #x
#    define STR(x)  STR2(x)

#    define COZ_PROGRESS_NAMED(name)
#    define COZ_PROGRESS
#    define COZ_BEGIN(name)
#    define COZ_END(name)
// clang-format on
#endif

// the latency mode does not seem that useful
#ifdef BENCH_LATENCY
#    undef COZ_PROGRESS_NAMED
#    define COZ_PROGRESS_NAMED(name)
#else
#    undef COZ_BEGIN
#    define COZ_BEGIN(name)
#    undef COZ_END
#    define COZ_END(name)
#endif