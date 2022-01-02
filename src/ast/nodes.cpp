#include "nodes.h"

namespace lython {

ExprNode *None();
ExprNode *True();
ExprNode *False();

// --------------------------------------------------------------------
// to-string

String str(NodeKind k) {
#define CASEGEN(name)    \
    case NodeKind::name: \
        return #name;

#define X(name, _)     CASEGEN(name)
#define SECTION(name)  CASEGEN(name)
#define EXPR(name, _)  CASEGEN(name)
#define STMT(name, _)  CASEGEN(name)
#define MOD(name, _)   CASEGEN(name)
#define MATCH(name, _) CASEGEN(name)

    switch (k) { NODEKIND_ENUM(X, SECTION, EXPR, STMT, MOD, MATCH) }

#undef X
#undef SECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH
    return "<invalid>";
}

void print(BoolOperator const &v, std::ostream &out) {
    switch (v) {
#define OP(name, kw)           \
    case BoolOperator::name: { \
        out << #kw;            \
        return;                \
    }
        BOOL_OPERATORS(OP)

#undef OP
    }
}

// ------------------------------------------
} // namespace lython
