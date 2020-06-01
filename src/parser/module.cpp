#include <spdlog/fmt/bundled/core.h>

#include "utilities/strings.h"
#include "parser/module.h"
#include "ast/nodes.h"

namespace lython{
    std::string const none_name = "<none>";

    Expression Module::type_type() {
        static auto type = Expression::make<AST::Type>("Type");
        return type;
    }

    Expression Module::float_type() {
        static auto type =
            Expression::make<AST::Builtin>("Float", type_type(), 1);
        return type;
    }

    Expression Module::reference(String const &view) const{
        int tsize = size();
        int idx = _find_index(view);
        return Expression::make<AST::Reference>(view, tsize - idx, tsize, Expression());
    }

    void Module::insert_builtin(){
        if (!_parent){
            insert("Type", type_type());
            insert("Float", float_type());

            auto make_binary = [&](){
                auto f_f_f = Expression::make<AST::Arrow>();
                auto binary_type = f_f_f.ref<AST::Arrow>();
                binary_type->params.push_back(
                    AST::Parameter("a", reference("Float")));
                binary_type->params.push_back(
                    AST::Parameter("b", reference("Float")));
                return f_f_f;
            };

            auto make_unary = [&](){
                auto f_f = Expression::make<AST::Arrow>();
                auto unary_type = f_f.ref<AST::Arrow>();
                unary_type->params.push_back(
                    AST::Parameter("a", reference("Float")));
                return f_f;
            };

            auto min_fun = Expression::make<AST::Builtin>("min", make_binary(), 2);
            insert("min", Expression(min_fun));

            auto max_fun = Expression::make<AST::Builtin>("max", make_binary(), 2);
            insert("max", Expression(max_fun));

            auto sin_fun = Expression::make<AST::Builtin>("sin", make_unary(), 1);
            insert("sin", Expression(sin_fun));

            auto pi = Expression::make<AST::Value>(3.14, reference("Float"));
            insert("pi", Expression(pi));
        }
    }

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

    std::ostream& Module::print(std::ostream& out, int depth) const {
        auto line = String(80, '-');

        if (depth == 0){
            out << "Module dump print:\n";
        }

        if (!_parent){
            out << line << "\n";
            out << fmt::format("{:4}", "id") << "   ";
            out << fmt::format("{:30}", "name") << "   type\n";
            out << line << "\n";
        }
        else{
            _parent->print(out, depth + 1);
            out << String(40, '-') << "\n";
        }
        for(int i = 0; i < int(_scope.size()); ++i){
            auto name = _idx_name[size_t(i)];
            auto expr = _scope[size_t(i)];

            out << fmt::format("{:4}", int(i)) << "   ";
            out << fmt::format("{:30}", name) << "   ";

            std::stringstream ss;
            expr.print(ss, 0, true, true) << "\n";

            auto str = ss.str();

            out << replace(str, '\n', "\n" + std::string(40, ' ')) << '\n';
        }

        if (depth == 0){
            out << line << "\n";
        }
        return out;
    }
} // namespace lython
