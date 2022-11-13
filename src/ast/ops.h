#ifndef LYTHON_OPS_HEADER
#define LYTHON_OPS_HEADER

#include "nodes.h"
#include <iostream>

namespace lython {

void set_context(Node* n, ExprContext ctx);

// Typecheck/Equality
bool equal(Node* a, Node* b);
bool equal(ExprNode* a, ExprNode* b);
bool equal(Pattern* a, Pattern* b);
bool equal(StmtNode* a, StmtNode* b);
bool equal(ModNode* a, ModNode* b);

void print(Node const*& obj, std::ostream& out);
void print(ExprNode const*& obj, std::ostream& out);
void print(Pattern const*& obj, std::ostream& out);
void print(StmtNode const*& obj, std::ostream& out);
void print(ModNode const*& obj, std::ostream& out);

String str(ExprNode const* obj);

bool has_circle(ExprNode const* obj);
bool has_circle(Pattern const* obj);
bool has_circle(StmtNode const* obj);
bool has_circle(ModNode const* obj);

StmtNode* getattr(StmtNode* obj, String const& attr, ExprNode*& type);
bool      hasattr(StmtNode* obj, String const& attr);

}  // namespace lython

#endif