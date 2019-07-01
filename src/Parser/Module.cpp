#include "Module.h"
#include <cassert>

namespace lython{

    std::string get_name(const ST::Expr& v){
        AST::Parameter* p = dynamic_cast<AST::Parameter*>(v.get());
        AST::Function* f = dynamic_cast<AST::Function*>(v.get());

        switch (v->kind()){
        case AST::Expression::KindParameter:
            return *(p->name().get());

        case AST::Expression::KindFunction:
            return *(f->name().get());

            assert("This expression is not hashable");
        }
    }

    std::size_t expr_hash::operator() (const ST::Expr& v) const noexcept{
        return _h(get_name(v));
    }

    bool expr_equal::operator() (const ST::Expr& a, const ST::Expr& b) const noexcept{
        return get_name(a) == get_name(b);
    }
}
