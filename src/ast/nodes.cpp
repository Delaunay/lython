#include "nodes.h"
#include "../logging/logging.h"
#include "../parser/module.h"

#define TRACE_START() trace_start(0, "");
#define TRACE_END() trace_end(0, "");

namespace lython {
namespace AST {
std::size_t pl_hash::operator()(Parameter &v) const noexcept {
    auto n = v.name;
    String tmp(std::begin(n), std::end(n));
    return _h(tmp);
}

} // namespace AbstractSyntaxTree
} // namespace lython
