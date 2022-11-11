

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