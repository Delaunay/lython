#ifndef LYTHON_INTERPRETER_HEADER
#define LYTHON_INTERPRETER_HEADER

#include "parser/module.h"
#include "logging/logging.h"

#include <cmath>

namespace lython{

NEW_EXCEPTION(InterpreterError);

struct InterpreterImpl;

// We do not want to overload the header with useless definitions
// use pimpl to hide the implementation
class Interpreter{
public:
    Interpreter(Module& m);

    Value eval(Expression const expr);

    ~Interpreter();

private:
    InterpreterImpl* ptr;
};

} // namespace lython

#endif
