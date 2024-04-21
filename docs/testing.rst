Testing
=======


Tests have 3 sections, the setup code, the expression usally a function call and finally the expected result
Each section starts with a line ``# >>> <section>`` and finishes with ``# <<<``.

.. code-block:: text

   # >>> case: VM_AnnAssign
   # >>> code
   def fun(a: i32) -> i32:
      b: i32 = 3
      b: i32 = a * b
      return b
   # <<<


   # >>> call
   fun(2)# <<<


   # >>> expected
   6# <<<


Round Trips
-----------

* Lexer  -> Unlex   -> Lexer  -> Unlex
* Parser -> Unparse -> Parser -> Unparse
* (Parser -> Sema) -> Unparse -> (Parser -> Sema) -> Unparse


Pyramid
-------

* Lexer
* Parser
* Sema
* Tree
* ByteCode
   * LLVM
   * Custom
* JIT
   * LLVM
   * Custom
* Native Gen