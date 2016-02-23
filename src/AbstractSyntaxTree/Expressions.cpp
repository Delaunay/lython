#include "Expressions.h"

namespace lython{
namespace AbstractSyntaxTree{
    std::size_t pl_hash::operator() (Placeholder& v) const noexcept{
        return _h(*(v.name().get()));
    }
}
}
