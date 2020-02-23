#include "module.h"

namespace lython{
    std::string const none_name = "<none>";

    std::string_view get_name(const Expression& v){
        switch (v.kind()){
        case AST::NodeKind::KParameter:
            return v.ref<AST::Parameter>()->name;

        case AST::NodeKind::KFunction:
            return v.ref<AST::Function>()->name;
            assert(false, "This expression is not hashable");

        default:
            return none_name;
        }
    }

    std::size_t expr_hash::operator() (const Expression& v) const noexcept{
        auto n = get_name(v);
        std::string tmp(std::begin(n), std::end(n));
        return _h(tmp);
    }

    bool expr_equal::operator() (const Expression& a, const Expression& b) const noexcept{
        return get_name(a) == get_name(b);
    }
} // namespace lython
