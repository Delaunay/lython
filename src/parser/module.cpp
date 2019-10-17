#include "module.h"
#include <cassert>

namespace lython{
    std::string const none_name = "<none>";

    std::string_view get_name(const ST::Expr& v){
        auto p = dynamic_cast<AST::Parameter*>(v.get());
        auto f = dynamic_cast<AST::Function*>(v.get());

        switch (v->kind()){
        case AST::Expression::KindParameter:
            return p->name();

        case AST::Expression::KindFunction:
            return f->name();
            assert("This expression is not hashable");

        default:
            return none_name;
        }
    }

    std::size_t expr_hash::operator() (const ST::Expr& v) const noexcept{
        auto n = get_name(v);
        std::string tmp(std::begin(n), std::end(n));
        return _h(tmp);
    }

    bool expr_equal::operator() (const ST::Expr& a, const ST::Expr& b) const noexcept{
        return get_name(a) == get_name(b);
    }
} // namespace lython
