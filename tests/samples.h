#ifndef LYTHON_TESTS_SAMPLES_HEADER
#define LYTHON_TESTS_SAMPLES_HEADER

#include "dtypes.h"

#define CODE_SAMPLES(X)            \
    X(simple_function)             \
    X(simple_function_noargs)      \
    X(simple_function_docstring)   \
    X(misc_code)                   \
    X(function_call)               \
    X(simple_function_global)      \
    X(simple_function_max)         \
    X(simple_function_return_args) \
    X(simple_function_rpe)         \
    X(simple_match)                \
    X(simple_while_loop)           \
    X(simple_for_loop)

#define STRUCT_TEST(X)         \
    X(simple_struct)           \
    X(simple_struct_docstring) \
    X(struct_set_get)

#define IMPORT_TEST(X)  \
    X(import_code)      \
    X(import_as_code)   \
    X(from_import_code) \
    X(from_import_as_code)

#define SAMPLE_PROTO(name) lython::String const &name();

CODE_SAMPLES(SAMPLE_PROTO)
STRUCT_TEST(SAMPLE_PROTO)
IMPORT_TEST(SAMPLE_PROTO)

#undef SAMPLE_PROTO

#endif
