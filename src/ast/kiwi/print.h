#ifndef LYTHON_AST_PRINT_HEADER
#define LYTHON_AST_PRINT_HEADER

#include "ast/visitor.h"

#include <ostream>

namespace lython{

std::ostream& print(std::ostream& out, Expression const expr, int indent, bool debug=false, bool colors=false);

}

#endif
