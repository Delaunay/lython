#pragma once

#include <unordered_set>

#include "AbstractSyntaxTree/Expressions.h"


namespace lython{
    //  Module
    // -------------------------------------

    // Holds Currently defined entities

    // I want AST::Expr to be hashable
struct expr_hash{
    std::size_t operator() (const ST::Expr& v) const noexcept;
    std::hash<std::string> _h;
};

struct expr_equal{
    bool operator() (const ST::Expr& a, const ST::Expr& b) const noexcept;
};
    typedef std::unordered_set<ST::Expr, expr_hash, expr_equal> BaseScope;
}
