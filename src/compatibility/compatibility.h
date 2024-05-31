#pragma once

// Each supported platforms gets their own block
// even if most of it is duplicated

#if BUILD_WEBASSEMBLY
#    define LY_ALIGN(X) __attribute__((aligned(X)))
#    define LY_PACKED   __attribute__((packed))
#    define LY_NOEXCEPT _NOEXCEPT
#    define LY_INLINE
#    define XXH_VECTOR XXH_SCALAR

#elif BUILD_UNIX
#    define LY_ALIGN(X) __attribute__((aligned(X)))
#    define LY_PACKED   __attribute__((packed))
#    define LY_NOEXCEPT noexcept
#    define LY_INLINE
#    define XXH_VECTOR XXH_AVX2

#elif BUILD_WINDOWS
#    define LY_ALIGN(X)
#    define LY_PACKED
#    define LY_NOEXCEPT
#    define LY_INLINE  __forceinline
#    define XXH_VECTOR XXH_AVX2

#else
#    define LY_ALIGN(X)
#    define LY_PACKED
#    define LY_NOEXCEPT
#    define LY_INLINE
#    define XXH_VECTOR XXH_SCALAR

#endif

// Fixes for zigg
#if BUILD_WEBASSEMBLY
#    define LY_NOEXCEPT _NOEXCEPT
#elif __clang__
#    undef LY_NOEXCEPT
#    define LY_NOEXCEPT _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW
#elif __GNUC__
#    undef LY_NOEXCEPT
#    define LY_NOEXCEPT _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW
#endif