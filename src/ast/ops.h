#ifndef LYTHON_OPS_HEADER
#define LYTHON_OPS_HEADER

#include "nodes.h"

namespace lython {

void set_context(Node *n, ExprContext ctx);

// Typecheck/Equality
bool equal(Node *a, Node *b);
bool equal(ExprNode *a, ExprNode *b);
bool equal(Pattern *a, Pattern *b);
bool equal(StmtNode *a, StmtNode *b);
bool equal(ModNode *a, ModNode *b);

} // namespace lython

#endif