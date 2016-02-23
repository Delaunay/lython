#pragma once


namespace lython
{

typedef std::uint32_t uint32;
typedef std::int32_t int32;

typedef std::uint16_t uint15;
typedef std::int16_t int16;

typedef std::uint8_t uint8;
typedef std::int8_t int8;

typedef float float32;
typedef double float64;

typedef unsigned char uchar;

}

// Simple way to get colors on linux
#if WINNT
#   define MRED
#   define MGREEN
#   define MYELLOW
#   define MBLUE
#   define MMAGENTA
#   define MCYAN
#   define MRESET
#else
#   define MRED     "\x1b[31m"
#   define MGREEN   "\x1b[32m"
#   define MYELLOW  "\x1b[33m"
#   define MBLUE    "\x1b[34m"
#   define MMAGENTA "\x1b[35m"
#   define MCYAN    "\x1b[36m"
#   define MRESET   "\x1b[0m"
#endif
