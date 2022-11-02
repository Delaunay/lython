#include "samples.h"

#define DEFINE_SAMPLE_CODE(name, code)   \
    lython::String const& name() {       \
        static lython::String cc = code; \
        return cc;                       \
    }

// Functions
// ---------
DEFINE_SAMPLE_CODE(simple_function,
                   "def simple_function(a: b, c: d) -> e:\n"
                   "    return 1\n"
                   "\n")

DEFINE_SAMPLE_CODE(simple_function_noargs,
                   "def simple_function_noargs() -> e:\n"
                   "    return 1\n"
                   "\n")

DEFINE_SAMPLE_CODE(simple_function_docstring,
                   "def simple_function_docstring(a: b, c: d) -> e:\n"
                   "    \"\"\"This is a docstring\"\"\"\n"
                   "    return 1\n"
                   "\n")
DEFINE_SAMPLE_CODE(simple_function_rpe,
                   "def simple_function_rpe() -> e:\n"
                   "    return sin(3) + x * 2 / (1 - 5) ^ 2 ^ 3\n"
                   "\n")
DEFINE_SAMPLE_CODE(simple_function_return_args,
                   "def simple_function_return_args(a) -> e:\n"
                   "    return a\n"
                   "\n")
DEFINE_SAMPLE_CODE(simple_function_max,
                   "def simple_function_max(a: Float, b: Float) -> Float:\n"
                   "    return max(a, b)\n"
                   "\n")
DEFINE_SAMPLE_CODE(simple_function_global,
                   "def simple_function_global(a: Float) -> Float:\n"
                   "    return a + pi\n"
                   "\n")
DEFINE_SAMPLE_CODE(function_call,
                   "def add(a: Float, b: Float) -> Float:\n"
                   "    return a + b\n"
                   "\n\n"
                   "def function_call() -> Float:\n"
                   "    return add(1, 2)\n")

DEFINE_SAMPLE_CODE(max_alias,
                   "def max_alias(a: Float, b: Float) -> Float:\n"
                   "   return max(a, b)\n"
                   "\n"
                   // "max_alias(1.0, 2.0)"
)

DEFINE_SAMPLE_CODE(simple_match,
                   "def simple_match(a: bool) -> Float:\n"
                   "    match a:\n"
                   "        case true:\n"
                   "            return false\n"
                   "        case false:\n"
                   "            return true\n"
                   "\n"
                   "\n"
                   "simple_match(true)\n\n")

DEFINE_SAMPLE_CODE(simple_while_loop,
                   "def simple_while_loop():\n"
                   "    i = 10\n"
                   "    while i > 0:\n"
                   "        i -= 1\n"
                   "\n\n"
                   "simple_while_loop()\n\n")

DEFINE_SAMPLE_CODE(simple_for_loop,
                   "def simple_for_loop():\n"
                   "    for i in range(10):\n"
                   "        d = 2\n"
                   "\n"
                   "\n"
                   "simple_for_loop()\n\n")

// Struct
// ------
DEFINE_SAMPLE_CODE(simple_struct,
                   "struct a:\n"
                   "    b: c\n"
                   "\n")
DEFINE_SAMPLE_CODE(simple_struct_docstring,
                   "struct a:\n"
                   "    \"\"\"This is a docstring\"\"\"\n"
                   "    b: c\n"
                   "\n")

DEFINE_SAMPLE_CODE(struct_set_get,
                   "struct Point:\n"
                   "    x: Float\n"
                   "    y: Float\n\n"

                   "def get_x(p: Point) -> Float:\n"
                   "    return p.x\n\n"

                   "def set_x(p: Point, x: Float) -> Point:\n"
                   "    p.x = x\n"
                   "    return p\n\n"

                   "def struct_set_get(v: Float) -> Float:\n"
                   "    p = Point(1.0, 2.0)\n"
                   "    set_x(p, v)\n"
                   "    a = get_x(p)\n"
                   "    return a\n\n")

// Impor
DEFINE_SAMPLE_CODE(import_code, "import a.b.c")

DEFINE_SAMPLE_CODE(import_as_code, "import a.b.c as e")

DEFINE_SAMPLE_CODE(from_import_code, "from a.b.c import f, k")

DEFINE_SAMPLE_CODE(from_import_as_code, "from a.b.c import g as h, i as j")

DEFINE_SAMPLE_CODE(from_import_call_code,
                   "from a.b.c import g as h\n"

                   "def fun(a, b):\n"
                   "    return h(a, b)\n\n")

DEFINE_SAMPLE_CODE(misc_code,
                   "def my_function() -> e:\n"  // correct indent management
                   "    return 1.1\n"           // tok_float
)

DEFINE_SAMPLE_CODE(simple_assignment,
                   "a = \"2 + 2\"\n"  // tok_identifier '=' tok_string
)

DEFINE_SAMPLE_CODE(edge_case_incorrect_tok,
                   "b = 1yy\n"  // tok_identifier '=' tok_incorrect
)

DEFINE_SAMPLE_CODE(edge_case_incorrect_num,
                   "c = 1.1.1\n";  // tok_identifier '=' tok_incorrect
)
