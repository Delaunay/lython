#include "parser/parser.h"
#include "utilities/strings.h"

#include <algorithm>

namespace lython {

Expression Parser::parse_import(Module& m, std::size_t){
    auto tok = token();
    Expression expr = Expression::make<AST::Import>();
    AST::Import* imp = expr.ref<AST::Import>();

    auto parse_path = [&](){
        // parse: <identifier>.<identifier>. ...
        EXPECT(tok_identifier, "Expected identifier"
                               "`import <identifier?>`");
        while (token().type() == tok_identifier){
            imp->path.push_back(token().identifier());

            EAT(tok_identifier);
            if (token().type() == tok_operator){
                if (token().operator_name() == "."){
                    next_token();
                }
            }
        }
    };

    if (tok.type() == tok_import){
        EAT(tok_import);
        tok = token();

        parse_path();

        // rename import
        if (token().type() == tok_as){
            next_token();
            EXPECT(tok_identifier, "Expected identifier after as"
                                   "`import <path> as <identifier?>");
            imp->name = token().identifier();
            EAT(tok_identifier);

            m.insert(
                imp->name.str(),
                Expression::make<AST::ImportExpr>(expr, imp->name));
        } else {
            Array<String> out;

            std::transform(
                std::begin(imp->path),
                std::end(imp->path),
                std::back_inserter(out),
                [](StringRef a) { return a.str(); });

            String path = join(".", out);
            m.insert(path, Expression::make<AST::ImportExpr>(
                               expr, get_string(path)));
        }

        return expr;
    }
    else if (tok.type() == tok_from){
        EAT(tok_from);

        parse_path();

        EXPECT(tok_import, "Missing import statement"
                           "`from <path> <import?> <identifier>`");
        EAT(tok_import);
        EXPECT(tok_identifier, "Missing identifier"
                               "`from <path> import <identifier?>`");

        while (token().type() == tok_identifier){
            StringRef export_name = token().identifier();
            StringRef import_name;

            next_token();
            if (token().type() == tok_as){
                next_token();
                EXPECT(tok_identifier, "Missing identifier"
                                       "`from <path> import <identifier> as <identifier?>`");
                import_name = token().identifier();
                next_token();

                m.insert(
                    import_name.str(),
                    Expression::make<AST::ImportExpr>(expr, import_name));
            } else {
                m.insert(
                    export_name.str(),
                    Expression::make<AST::ImportExpr>(expr, export_name));
            }

            imp->imports.emplace_back(export_name, import_name);
            EAT(',');
        }

        return expr;
    }

    throw ParserException("Got {} but exepected"
                          "`import` or `from`", to_string(tok.type()));
}

}
