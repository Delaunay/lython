#include "sexpression.h"

namespace lython {

// This is essentially compile time lookup
// no-need for the function to actually exist at runtime
#define SPECGEN(name)           \
    template <>                 \
    NodeKind nodekind<name>() { \
        return NodeKind::name;  \
    }

#define X(name, _)
#define SECTION(name)
#define EXPR(name, _)  SPECGEN(name)
#define STMT(name, _)  SPECGEN(name)
#define MOD(name, _)   SPECGEN(name)
#define MATCH(name, _) SPECGEN(name)

NODEKIND_ENUM(X, SECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef SPECGEN

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
// ------------------------------------------
} // namespace lython
