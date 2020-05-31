#include "parser/parser.h"
#include "utilities/strings.h"

#include <algorithm>
#include <filesystem>

namespace lython {

String to_string(std::filesystem::path const& p){
    // Want to check how versatile the STL is ?
    //  - Use custom Allocator !
    // auto alloc = AllocatorCPU<char>();
    // return p.generic_string<char, std::char_traits<char>, AllocatorCPU<char>>(alloc);
    auto str = p.string();
    return String(std::begin(str), std::end(str));
}

String find_module_file(AST::Import* const imp){
    std::filesystem::path search_path = "/home/setepenre/work/lython/code";
    String path = imp->file_path();

    auto p = search_path.append(path).append("__init__.py");

    if (std::filesystem::exists(p)){
        debug("loading module {}", p);
        return to_string(p);
    }

    debug("could not find module {}", p);
    return "";
}

void process_module(AST::Import* imp){
    auto path = find_module_file(imp);

    if (path.size() <= 0)
        return;

    // StringBuffer reader(read_file(path), path);
    FileBuffer reader(path);
    Lexer lex(reader);
    Parser par(reader, &imp->module);

    try {
        Expression expr;
        do {
            expr = par.parse_one(imp->module);
        } while(expr);

    } catch (lython::Exception e) {
        error("Error Occured: {}", e.what());
    }

    imp->module.print(std::cout);
}

Expression Parser::parse_import(Module& m, std::size_t){
    auto tok = token();
    Expression module_expr = Expression::make<AST::Import>();
    AST::Import* imp = module_expr.ref<AST::Import>();

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

        // parse the imported module
        process_module(imp);
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

            auto p = Expression::make<AST::ImportedExpr>(module_expr, imp->name);
            m.insert(imp->name.str(), p);
        } else {
            auto path = imp->module_path();
            auto p = Expression::make<AST::ImportedExpr>(module_expr, get_string(path));
            m.insert(path, p);
        }

        return module_expr;
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
            StringRef import_name = export_name;
            bool renamed = false;

            next_token();
            if (token().type() == tok_as){
                next_token();
                EXPECT(tok_identifier, "Missing identifier"
                                       "`from <path> import <identifier> as <identifier?>`");
                import_name = token().identifier();
                next_token();
                renamed = true;
            }

            auto ref = imp->module.reference(export_name.str());
            auto p = Expression::make<AST::ImportedExpr>(module_expr, ref);
            m.insert(import_name.str(), p);

            if (!renamed)
                import_name = StringRef();

            imp->imports.emplace_back(export_name, import_name);
            EAT(',');
        }

        return module_expr;
    }

    throw ParserException("{}: Got {} but exepected"
                          "`import` or `from`", to_string(tok.type()));
}

}
