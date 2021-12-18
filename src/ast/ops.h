#ifndef LYTHON_OPS_HEADER
#define LYTHON_OPS_HEADER

#include "nodes.h"
#include <iostream>

namespace lython {

void set_context(Node *n, ExprContext ctx);

// template <>
void print(Node *const &obj, std::ostream &out);

} // namespace lython

#endif