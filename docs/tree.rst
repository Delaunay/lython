Tree Evaluator
==============

The tree evaluator interprets the AST as is.

Implementing a tree evaluator is challenging when it comes to complex control flow handling
such as `yield`. Indeed the flow needs to be paused and later resumed.

In the case of a byte code VM, one could simply save the instruction pointer and the local state.
But because we are executing the AST we need to remember each body and at which point we are.

One way to think about it is to see the tree evaluator as a tree iterator, iterating through the instructions 
to executes. When execution is paused, we need to store the iterator state through the AST so we can restore it later on.

