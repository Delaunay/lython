#ifndef PROGRAM_CONFIG
#define PROGRAM_CONFIG

#define LLVM_CODEGEN 0
#define LLVM_JIT 0

#define LIBNAMESPACE lython

#define TAB_SIZE 4

#ifdef _MSC_VER
#   define _CRT_SECURE_NO_WARNINGS
#endif

#if 0
#   define MRED     "\x1b[31m"
#   define MGREEN   "\x1b[32m"
#   define MYELLOW  "\x1b[33m"
#   define MBLUE    "\x1b[34m"
#   define MMAGENTA "\x1b[35m"
#   define MCYAN    "\x1b[36m"
#   define MRESET   "\x1b[0m"
#else
#   define MRED     ""
#   define MGREEN   ""
#   define MYELLOW  ""
#   define MBLUE    ""
#   define MMAGENTA ""
#   define MCYAN    ""
#   define MRESET   ""
#endif

#endif
