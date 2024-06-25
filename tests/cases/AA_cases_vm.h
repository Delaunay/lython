#include "dtypes.h"
#include "libtest.h"

#include "ast/nodes.h"

namespace lython {
#define GENCASES(name, _) Array<TestCase>const& name##_vm_examples();
#define X(name, _)
#define SSECTION(name)

NODEKIND_ENUM(X, SSECTION, GENCASES, GENCASES, X, X, X)

#undef X
#undef SSECTION
#undef GENCASES
}