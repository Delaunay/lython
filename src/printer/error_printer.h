#pragma once

#include "dtypes.h"

namespace lython {

// Python traceback example:
// -------------------------

// Traceback (most recent call last):
//   File "/home/runner/ShyCalculatingGraph/main.py", line 11, in <module>
//     rec(10)
//   File "/home/runner/ShyCalculatingGraph/main.py", line 7, in rec
//     return rec(n - 1)
//   File "/home/runner/ShyCalculatingGraph/main.py", line 7, in rec
//     return rec(n - 1)
//   File "/home/runner/ShyCalculatingGraph/main.py", line 7, in rec
//     return rec(n - 1)
//   [Previous line repeated 7 more times]
//   File "/home/runner/ShyCalculatingGraph/main.py", line 5, in rec
//     raise RuntimeError()
// RuntimeError

// Lython error messages
// ---------------------
//
// Note that python stops at the first error while
// lython can keep going and print more than one error.
//
// Parsing error messages (2)                   < Error kind
//   File "<replay buffer>", line 0             < File and line
//     |self.x =                                < code line
//     |               ^                        < Underline
// SyntaxError: Expected an expression          < Error message
//
//   File "<replay buffer>", line 0             < File and line
//     |def __init__(self):                     < code line
//     |       ^                                < Underline
// SyntaxError: Expected a body                 < Error message
//

struct BaseErrorPrinter {
    BaseErrorPrinter(std::ostream& out, class AbstractLexer* lexer = nullptr):
        out(out), lexer(lexer)  //
    {}

    ~BaseErrorPrinter() {}

    virtual void print(LythonException const& err) {}

    String get_filename() const;

    virtual String        indentation();
    virtual std::ostream& firstline();
    virtual std::ostream& newline();
    virtual std::ostream& errorline();
    virtual std::ostream& codeline();
    virtual void          end();

    void underline(class Token const& tok);
    void underline(struct CommonAttributes const& attr);

    //
    int                  indent = 1;
    class AbstractLexer* lexer  = nullptr;
    std::ostream&        out;
};

struct StmtNode* get_parent_stmt(struct Node* node);

}  // namespace lython