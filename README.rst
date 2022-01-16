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

