Lython
======

Simple compiler without pretention.


Features
--------

* Borrows syntax & AST from Python
* Extensive logging for debugging
* Re-usable
* `Unit-Tested <https://delaunay.github.io/lython>`_
* Staticly typed
* Type-deduction


Language Goals (i.e maybe one day)
----------------------------------

* Simple language for fast prototyping
* Able to reuse Python tools (sphinx)
* Testing & documentation as a first class citizen
* Code Rendering (Text, Graph & Blocks)
* Code visualization (Call graphs)
* Code metrics (Complexity)
* Heterogeneous computing (GP-GPU)
* Concurrentcy (async)
* Parallelism (multi-threading)
* Distributed computing (multi-node)
* Optional GC with multiple kind of GC per application
* Reflection through code generation
  meta programming & extended compile-time execution


With Batteries
^^^^^^^^^^^^^^

* Matrix
* Visualization
* Database
* Webserver
* Dyanmic Webpage
* Networking
* Threadpool (parallelism)
* Event Manager (concurrency)
* Message queue (multi-node)

Compiler Goals (i.e maybe one day)
----------------------------------

* Type-inference maybe
* LLVM-Code Generation
* Multi-language inputs (Python, Javascript)
* Multi-language outputs (Python, Javascript/Webassembly, C++ or C, LLVM-IR)
* Multi Context execution (Standard CLI main, Library, DOM, UI/3D with Vulkan)
* AST aware diff instead of text diff
* Serialization to database for primtive versionning,
  i.e language primitives are versionned instead of text
* AST hashing
* Coverage Tools
* Documentation Generation


Compiling
---------

.. code-block:: bash

   GCOV=gcov-8 CC=gcc-8 CXX=g++-8 cmake -DCMAKE_BUILD_TYPE=Debug .. && make coverage -j 8


Libraries
---------

* fpm: fixed math for financial use
* lmath: effecient vector for 2d/3d applications
* breakpad: crash reporting
* Eigen: matrix
* ICU: internationalization
* Godot: (SDL2 + vulkan/OpenGL/OpenES)
* re2: regex


* GPU Compute ?
* SDL2: basic 2D Library ?
* vulkan  ?
* rpclib ?

Compiler
--------

* Tokenizer
* Parser
* Sema
* Optimization
* Code Gen

Interpreter
-----------


* Tokenizer
* Parser
* Sema
* TreeEvaluator

Linter
------

* Tokenizer
* Parser
* Sema
* Linter


Format
------

* Tokenizer
* Parser
* Format

or

* Tokenizer
* Format

Modularity
----------

* Language
  * Tokenizer
  * Parser

* Codegen
  * lisp
  * C
  * C++


Debug
-----

Sema
^^^^

Show sema log & errors, finish by dumping the processed bindings.

.. code-block:: Python

   $ lython internal --file ../code/python_test.ly
    --------------------------------------------------------------------------------
    Sema Logs
    --------------------------------------------------------------------------------
    [T] [17960]                         module: 245 :+-> Module
    [T] [17960]                    functiondef: 245 :|+-> FunctionDef
    [T] [17960]                       exprstmt: 245 :|:+-> Expr
    [T] [17960]                           call: 245 :|:|+-> Call
    [T] [17960]                           name: 245 :|:|:+-> Name
    [D] [17960] src\sema\sema.cpp:705 name - Value print not found
    [E] [17960] src\sema\sema.cpp:706 name - NameError: name 'print' is not defined
    [D] [17960] src\sema\sema.cpp:714 name - Value print does not have a type
    [E] [17960] src\sema\sema.cpp:534 call - print is not callable
    [T] [17960]                         assign: 245 :|:+-> Assign
    [T] [17960]                       constant: 245 :|:|+-> Constant
    [T] [17960]                       exprstmt: 245 :|:+-> Expr
    [T] [17960]                           call: 245 :|:|+-> Call
    [T] [17960]                           name: 245 :|:|:+-> Name
    [D] [17960] src\sema\sema.cpp:705 name - Value print not found
    [E] [17960] src\sema\sema.cpp:706 name - NameError: name 'print' is not defined
    [D] [17960] src\sema\sema.cpp:714 name - Value print does not have a type
    [E] [17960] src\sema\sema.cpp:534 call - print is not callable
    [T] [17960]                      subscript: 245 :|:|:+-> Subscript
    [T] [17960]                           name: 245 :|:|:|+-> Name
    [D] [17960] src\sema\sema.cpp:716 name - Loading value a: 20 of type i32
    [T] [17960]                       constant: 245 :|:|:|+-> Constant
    [T] [17960]                        forstmt: 245 :|:+-> For
    [T] [17960]                           call: 245 :|:|+-> Call
    [T] [17960]                           name: 245 :|:|:+-> Name
    [D] [17960] src\sema\sema.cpp:705 name - Value range not found
    [E] [17960] src\sema\sema.cpp:706 name - NameError: name 'range' is not defined
    [D] [17960] src\sema\sema.cpp:714 name - Value range does not have a type
    [E] [17960] src\sema\sema.cpp:534 call - range is not callable
    [T] [17960]                       constant: 245 :|:|:+-> Constant
    [T] [17960]                       constant: 245 :|:|:+-> Constant
    [T] [17960]                           name: 245 :|:|+-> Name
    [D] [17960] src\sema\sema.cpp:696 name - Storing value for i (21)
    [D] [17960] src\sema\sema.cpp:714 name - Value i does not have a type
    [T] [17960]                       exprstmt: 245 :|:|+-> Expr
    [T] [17960]                           call: 245 :|:|:+-> Call
    [T] [17960]                           name: 245 :|:|:|+-> Name
    [D] [17960] src\sema\sema.cpp:705 name - Value test_f not found
    [E] [17960] src\sema\sema.cpp:706 name - NameError: name 'test_f' is not defined
    [D] [17960] src\sema\sema.cpp:714 name - Value test_f does not have a type
    [E] [17960] src\sema\sema.cpp:534 call - test_f is not callable
    [T] [17960]                     returnstmt: 245 :|:+-> Return
    [T] [17960]                       constant: 245 :|:|+-> Constant
    [T] [17960]                       classdef: 245 :|+-> ClassDef
    [T] [17960]                    functiondef: 245 :|:+-> FunctionDef
    [D] [17960] src\sema\sema.cpp:795 add_arguments - Insert class type
    [T] [17960]                           pass: 245 :|:|+-> Pass
    --------------------------------------------------------------------------------
    --------------------------------------------------------------------------------
    Sema Diagnostic dump
    --------------------------------------------------------------------------------
      - NameError: name 'print' is not defined
      - print is not callable
      - NameError: name 'print' is not defined
      - print is not callable
      - NameError: name 'range' is not defined
      - range is not callable
      - NameError: name 'test_f' is not defined
      - test_f is not callable
    --------------------------------------------------------------------------------
    --------------------------------------------------------------------------------
    Sema bindings dump
    --------------------------------------------------------------------------------
        -----------------------------------------+----------------------+---------------------
        name                                     | type                 | value
        -----------------------------------------+----------------------+---------------------
      0                                     Type |                 Type | Type
      1                                     None |                 Type | None
      2                                       i8 |                 Type | i8
      3                                      i16 |                 Type | i16
      4                                      i32 |                 Type | i32
      5                                      i64 |                 Type | i64
      6                                      f32 |                 Type | f32
      7                                      f64 |                 Type | f64
      8                                       u8 |                 Type | u8
      9                                      u16 |                 Type | u16
     10                                      u32 |                 Type | u32
     11                                      u64 |                 Type | u64
     12                                      str |                 Type | str
     13                                     bool |                 Type | bool
     14                                   Module |                 Type | Module
     15                                     None |                 None | None
     16                                     True |                 bool | True
     17                                    False |                 bool | False
     18                             function_def |       (None) -> None | def function_def(a):
                                                 |                      |     print()
                                                 |                      |     a = 2
                                                 |                      |     print(a[2])
                                                 |                      |     for i in range(0, 10):
                                                 |                      |         test_f()
                                                 |                      |     return 0
     19                                     Name |                 Type | class Name:
                                                 |                      |     def add(self):
                                                 |                      |         pass
     20                                 Name.add |       (Name) -> None | def add(self):
                                                 |                      |     pass
        -----------------------------------------+----------------------+---------------------


Parsing
^^^^^^^

Stop after parsing the file, show log trace and dump the parsed module

.. code-block:: Python

   $ lython internal --file ../code/python_test.ly --parsing

  [I] [33856] src\cli\commands\internal.cpp:52 main - Enter
  ================================================================================
  Parsing Trace
  --------------------------------------------------------------------------------
  [T] [33856]                     parse_body: 148 +-> tok_newline: -5 - ``
  [T] [33856]                parse_statement:1917 :+-> tok_def: -10 - ``
  [T] [33856]        parse_statement_primary:1972 :|+-> tok_def: -10 - ``
  [T] [33856]             parse_function_def: 193 :|+-> tok_def: -10 - ``
  [T] [33856]                parse_arguments:1373 :|:+-> tok_identifier: -1 - `a`
  [T] [33856]                     parse_body: 148 :|:+-> tok_identifier: -1 - `print`
  [T] [33856]                parse_statement:1917 :|:|+-> tok_identifier: -1 - `print`
  [T] [33856]        parse_statement_primary:1972 :|:|:+-> tok_identifier: -1 - `print`
  [T] [33856]                     parse_name:1257 :|:|:+-> tok_identifier: -1 - `print`
  [T] [33856]                     parse_call:1790 :|:|:+-> tok_parens: 40 - ``
  [T] [33856]                parse_call_args:1750 :|:|:|+-> ')': 41 - ``
  [T] [33856]                     parse_call:1803 :|:|:+-< tok_newline: -5
  [T] [33856]        parse_statement_primary:2063 :|:|:+-> tok_newline: -5 - ``
  [T] [33856]        parse_statement_primary:2085 :|:|:+-< tok_newline: -5
  [T] [33856]                parse_statement:1934 :|:|+-< tok_newline: -5
  [T] [33856]                parse_statement:1917 :|:|+-> tok_identifier: -1 - `a`
  [T] [33856]        parse_statement_primary:1972 :|:|:+-> tok_identifier: -1 - `a`
  [T] [33856]                     parse_name:1257 :|:|:+-> tok_identifier: -1 - `a`
  [T] [33856]        parse_statement_primary:2063 :|:|:+-> tok_assign: 61 - `=`
  [T] [33856]                   parse_assign:1199 :|:|:+-> tok_assign: 61 - `=`
  [T] [33856]                 parse_constant:1293 :|:|:|+-> tok_int: -4 - `2`
  [T] [33856]                parse_statement:1934 :|:|+-< tok_newline: -5
  [T] [33856]                parse_statement:1917 :|:|+-> tok_identifier: -1 - `print`
  [T] [33856]        parse_statement_primary:1972 :|:|:+-> tok_identifier: -1 - `print`
  [T] [33856]                     parse_name:1257 :|:|:+-> tok_identifier: -1 - `print`
  [T] [33856]                     parse_call:1790 :|:|:+-> tok_parens: 40 - ``
  [T] [33856]                parse_call_args:1750 :|:|:|+-> tok_identifier: -1 - `a`
  [T] [33856]                     parse_name:1257 :|:|:|:+-> tok_identifier: -1 - `a`
  [T] [33856]                parse_subscript:1827 :|:|:|:+-> tok_square: 91 - ``
  [T] [33856]                 parse_constant:1293 :|:|:|:|+-> tok_int: -4 - `2`
  [T] [33856]                     parse_call:1803 :|:|:+-< tok_newline: -5
  [T] [33856]        parse_statement_primary:2063 :|:|:+-> tok_newline: -5 - ``
  [T] [33856]        parse_statement_primary:2085 :|:|:+-< tok_newline: -5
  [T] [33856]                parse_statement:1934 :|:|+-< tok_newline: -5
  [T] [33856]                parse_statement:1917 :|:|+-> tok_for: -29 - ``
  [T] [33856]        parse_statement_primary:1972 :|:|:+-> tok_for: -29 - ``
  [T] [33856]                      parse_for: 345 :|:|:+-> tok_for: -29 - ``
  [T] [33856]                     parse_name:1257 :|:|:|:+-> tok_identifier: -1 - `i`
  [T] [33856]                     parse_name:1257 :|:|:|+-> tok_identifier: -1 - `range`
  [T] [33856]                     parse_call:1790 :|:|:|+-> tok_parens: 40 - ``
  [T] [33856]                parse_call_args:1750 :|:|:|:+-> tok_int: -4 - `0`
  [T] [33856]                 parse_constant:1293 :|:|:|:|+-> tok_int: -4 - `0`
  [T] [33856]                 parse_constant:1293 :|:|:|:|+-> tok_int: -4 - `10`
  [T] [33856]                     parse_call:1803 :|:|:|+-< ':': 58
  [T] [33856]                     parse_body: 148 :|:|:|+-> tok_identifier: -1 - `test_f`
  [T] [33856]                parse_statement:1917 :|:|:|:+-> tok_identifier: -1 - `test_f`
  [T] [33856]        parse_statement_primary:1972 :|:|:|:|+-> tok_identifier: -1 - `test_f`
  [T] [33856]                     parse_name:1257 :|:|:|:|+-> tok_identifier: -1 - `test_f`
  [T] [33856]                     parse_call:1790 :|:|:|:|+-> tok_parens: 40 - ``
  [T] [33856]                parse_call_args:1750 :|:|:|:|:+-> ')': 41 - ``
  [T] [33856]                     parse_call:1803 :|:|:|:|+-< tok_newline: -5
  [T] [33856]        parse_statement_primary:2063 :|:|:|:|+-> tok_newline: -5 - ``
  [T] [33856]        parse_statement_primary:2085 :|:|:|:|+-< tok_newline: -5
  [T] [33856]                parse_statement:1934 :|:|:|:+-< tok_newline: -5
  [T] [33856]                parse_statement:1934 :|:|+-< tok_return: -14
  [T] [33856]                parse_statement:1917 :|:|+-> tok_return: -14 - ``
  [T] [33856]        parse_statement_primary:1972 :|:|:+-> tok_return: -14 - ``
  [T] [33856]                   parse_return:1117 :|:|:+-> tok_return: -14 - ``
  [T] [33856]                 parse_constant:1293 :|:|:|+-> tok_int: -4 - `0`
  [T] [33856]                   parse_return:1131 :|:|:+-< tok_newline: -5
  [T] [33856]                parse_statement:1934 :|:|+-< tok_newline: -5
  [T] [33856]             parse_function_def: 235 :|+-< tok_class: -32
  [T] [33856]                parse_statement:1934 :+-< tok_class: -32
  [T] [33856]                parse_statement:1917 :+-> tok_class: -32 - ``
  [T] [33856]        parse_statement_primary:1972 :|+-> tok_class: -32 - ``
  [T] [33856]                parse_class_def: 240 :|+-> tok_class: -32 - ``
  [T] [33856]                     parse_body: 148 :|:+-> tok_def: -10 - ``
  [T] [33856]                parse_statement:1917 :|:|+-> tok_def: -10 - ``
  [T] [33856]        parse_statement_primary:1972 :|:|:+-> tok_def: -10 - ``
  [T] [33856]             parse_function_def: 193 :|:|:+-> tok_def: -10 - ``
  [T] [33856]                parse_arguments:1373 :|:|:|+-> tok_identifier: -1 - `self`
  [T] [33856]                     parse_body: 148 :|:|:|+-> tok_pass: -36 - ``
  [T] [33856]                parse_statement:1917 :|:|:|:+-> tok_pass: -36 - ``
  [T] [33856]        parse_statement_primary:1972 :|:|:|:|+-> tok_pass: -36 - ``
  [T] [33856]                     parse_pass:1158 :|:|:|:|+-> tok_pass: -36 - ``
  [T] [33856]                parse_statement:1934 :|:|:|:+-< tok_newline: -5
  [T] [33856]             parse_function_def: 235 :|:|:+-< tok_eof: -9
  [T] [33856]                parse_statement:1934 :|:|+-< tok_eof: -9
  [T] [33856]                parse_statement:1934 :+-< tok_eof: -9
  --------------------------------------------------------------------------------
  Parsing Diag
  --------------------------------------------------------------------------------
  --------------------------------------------------------------------------------
  --------------------------------------------------------------------------------
  Parsed Module dump
  --------------------------------------------------------------------------------
  def function_def(a):
      print()
      a = 2
      print(a[2])
      for i in range(0, 10):
          test_f()
      return 0


  class Name:
      def add(self):
          pass



  --------------------------------------------------------------------------------

Lexer Round trip
^^^^^^^^^^^^^^^^

Reformat the code from the token alone

.. code-block:: Python

   $ lython internal --file ../code/python_test.ly --lexer-format

    ================================================================================
    Lexing Round-trip
    --------------------------------------------------------------------------------

    def function_def(a):

        print()
        a = 2
        print(a[2])

        for i in range(0, 10):
            test_f()

        return 0


    class Name:
        def add(self):
            pass
    --------------------------------------------------------------------------------

Lexer Token Dump
^^^^^^^^^^^^^^^^

DUmps the sequence of tokens created by the lexer

.. code-block:: Python

   $ lython internal --file ../code/python_test.ly --debug-lexer

    ================================================================================
    Lexer Token Dump
      1           tok_newline => [l:   2, c:   0] ``
      2               tok_def => [l:   2, c:   3] ``
      3        tok_identifier => [l:   2, c:  16] `function_def`
      4            tok_parens => [l:   2, c:  17] ``
      5        tok_identifier => [l:   2, c:  18] `a`
      6                   ')' => [l:   2, c:  19] ``
      7                   ':' => [l:   2, c:  20] `:`
      8           tok_newline => [l:   3, c:   0] ``
      9           tok_newline => [l:   4, c:   0] ``
      10            tok_indent => [l:   4, c:   4] ``
      11        tok_identifier => [l:   4, c:   9] `print`
      12            tok_parens => [l:   4, c:  10] ``
      13                   ')' => [l:   4, c:  11] ``
      14           tok_newline => [l:   5, c:   0] ``
      15        tok_identifier => [l:   5, c:   5] `a`
      16            tok_assign => [l:   5, c:   7] `=`
      17               tok_int => [l:   5, c:   9] `2`
      18           tok_newline => [l:   6, c:   0] ``
      19        tok_identifier => [l:   6, c:   9] `print`
      20            tok_parens => [l:   6, c:  10] ``
      21        tok_identifier => [l:   6, c:  11] `a`
      22            tok_square => [l:   6, c:  12] ``
      23               tok_int => [l:   6, c:  13] `2`
      24                   ']' => [l:   6, c:  14] ``
      25                   ')' => [l:   6, c:  15] ``
      26           tok_newline => [l:   7, c:   0] ``
      27           tok_newline => [l:   8, c:   0] ``
      28               tok_for => [l:   8, c:   7] ``
      29        tok_identifier => [l:   8, c:   9] `i`
      30                tok_in => [l:   8, c:  12] `in`
      31        tok_identifier => [l:   8, c:  18] `range`
      32            tok_parens => [l:   8, c:  19] ``
      33               tok_int => [l:   8, c:  20] `0`
      34             tok_comma => [l:   8, c:  21] ``
      35               tok_int => [l:   8, c:  24] `10`
      36                   ')' => [l:   8, c:  25] ``
      37                   ':' => [l:   8, c:  26] `:`
      38           tok_newline => [l:   9, c:   0] ``
      39            tok_indent => [l:   9, c:   8] ``
      40        tok_identifier => [l:   9, c:  14] `test_f`
      41            tok_parens => [l:   9, c:  15] ``
      42                   ')' => [l:   9, c:  16] ``
      43           tok_newline => [l:  10, c:   0] ``
      44           tok_newline => [l:  11, c:   0] ``
      45         tok_desindent => [l:  11, c:   4] ``
      46            tok_return => [l:  11, c:  10] ``
      47               tok_int => [l:  11, c:  12] `0`
      48           tok_newline => [l:  12, c:   0] ``
      49           tok_newline => [l:  13, c:   0] ``
      50           tok_newline => [l:  14, c:   0] ``
      51         tok_desindent => [l:  14, c:   0] ``
      52             tok_class => [l:  14, c:   5] ``
      53        tok_identifier => [l:  14, c:  10] `Name`
      54                   ':' => [l:  14, c:  11] `:`
      55           tok_newline => [l:  15, c:   0] ``
      56            tok_indent => [l:  15, c:   4] ``
      57               tok_def => [l:  15, c:   7] ``
      58        tok_identifier => [l:  15, c:  11] `add`
      59            tok_parens => [l:  15, c:  12] ``
      60        tok_identifier => [l:  15, c:  16] `self`
      61                   ')' => [l:  15, c:  17] ``
      62                   ':' => [l:  15, c:  18] `:`
      63           tok_newline => [l:  16, c:   0] ``
      64            tok_indent => [l:  16, c:   8] ``
      65              tok_pass => [l:  16, c:  12] ``
      66           tok_newline => [l:  17, c:   0] ``
      67               tok_eof => [l:  17, c:   0] ``
    --------------------------------------------------------------------------------



Fuzzing
-------

Linux only

.. code-block::

   cd fuzzing
   make build
   make run


Roadmap
-------
* [X] function call
* [X] basic classes
* [X] basic binary function 
* [X] base import
* [ ] unicode / byte
* [ ] array
* [ ] native function expose
* [ ] import system
  * [ ] cache imports
* [ ] VM raise
* [ ] VM catch
* [ ] VM yield (new UV maybe)
* [ ] magic methods
* [ ] kwargs/args
* [ ] args reordering
* [ ] context manager
* [ ] decorator
* [ ] fmt





mkdir build-emcc
cd build-emcc
export TOOLCHAIN_FILE=../dependencies/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
CMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE cmake ..


export FREETYPE_LIBRARY=/home/newton/work/lython/dependencies/freetype/build
export FREETYPE_INCLUDE_DIRS=/home/newton/work/lython/dependencies/freetype/build/include  
export CMAKE_TOOLCHAIN_FILE=/home/newton/work/lython/dependencies/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake 
cmake -DSDL2TTF_VENDORED=1 -DNO_LLVM=1 ..



cd build-emac
conan install ../conan/ --profile emacscripten --build missing -of ../build-emac
cmake .. -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=./conan_toolchain.cmake -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DSDL2TTF_VENDORED=1 -DNO_LLVM=1 -DCMAKE_BUILD_TYPE=Release
cmake --build .


cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../binaries
cmake --build .
cmake --install .