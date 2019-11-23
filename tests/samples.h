#ifndef LYTHON_TESTS_SAMPLES_HEADER
#define LYTHON_TESTS_SAMPLES_HEADER

#include <catch2/catch.hpp>

#include "Types.h"

using namespace lython;

#define DEFINE_SAMPLE_CODE(name, code)\
    inline String name(){\
        return code;\
    }

#define DEFINE_SAMPLE_CODE2(name, code) DEFINE_SAMPLE_CODE(name, #code)
// Functions
// ---------
DEFINE_SAMPLE_CODE(
    simple_function,
    "def simple_function(a: b, c: d) -> e:\n"
    "    return 1\n"
    "\n"
    )
DEFINE_SAMPLE_CODE(
    simple_function_noargs,
    "def simple_function_noargs() -> e:\n"
    "    return 1\n"
    "\n"
    )
DEFINE_SAMPLE_CODE(
    simple_function_docstring,
    "def simple_function_docstring(a: b, c: d) -> e:\n"
    "    \"\"\"This is a docstring\"\"\"\n"
    "    return 1\n"
    "\n"
    )
DEFINE_SAMPLE_CODE(
    simple_function_rpe,
    "def simple_function_rpe() -> e:\n"
    "    return 3 + x * 2 / (1 - 5) ^ 2 ^ 3\n"
    "\n"
    )
DEFINE_SAMPLE_CODE(
    simple_function_return_args,
    "def simple_function_return_args(a) -> e:\n"
    "    return a\n"
    "\n"
    )
DEFINE_SAMPLE_CODE(
    simple_function_max,
    "def simple_function_max(a: Double, b: Double) -> Double:\n"
    "    return max(a, b)\n"
    "\n"
    )
DEFINE_SAMPLE_CODE(
    simple_function_global,
    "def simple_function_global(a: Double) -> Double:\n"
    "    return a + pi\n"
    "\n"
    )
DEFINE_SAMPLE_CODE(
    function_call,
    "def add(a: Double, b: Double) -> Double:\n"
    "    return a + b\n"
    "\n"
    "def function_call() -> Double:\n"
    "    return add(1, 2)\n"
    )

// Struct
// ------
DEFINE_SAMPLE_CODE(
    simple_struct,
    "struct a:\n"
    "    b: c\n"
    "\n"
    )
DEFINE_SAMPLE_CODE(
    simple_struct_docstring,
    "struct a:\n"
    "    \"\"\"This is a docstring\"\"\"\n"
    "    b: c\n"
    "\n"
    )
DEFINE_SAMPLE_CODE(
    misc_code,
    "def my_function():\n"       // correct indent management
    "    return 1.1\n"           // tok_float

    "a = \"2 + 2\"\n"            // tok_identifier '=' tok_string
    "b = 1yy\n"                  // tok_identifier '=' tok_incorrect
    "c = 1.1.1\n";               // tok_identifier '=' tok_incorrect
    )


#define CODE_SAMPLES(X)\
    X(simple_function)\
    X(simple_function_noargs)\
    X(simple_function_docstring)\
    X(simple_struct)\
    X(simple_struct_docstring)\
    X(misc_code)\
    X(function_call)\
    X(simple_function_global)\
    X(simple_function_max)\
    X(simple_function_return_args)\
    X(simple_function_rpe)

#endif
