#pragma once

#ifdef __linux__
#    define LY_ALIGN(X) __attribute__((aligned(X)))
#    define LY_PACKED   __attribute__((packed))
#endif