#include "nodes.h"
#include "../logging/logging.h"
#include "../parser/module.h"

#define TRACE_START() trace_start(0, "");
#define TRACE_END() trace_end(0, "");

namespace lython {
namespace AST {

std::size_t pl_hash::operator()(Parameter &v) const noexcept {
    return _h(v.name.ref);
}

} // namespace AbstractSyntaxTree
} // namespace lython
