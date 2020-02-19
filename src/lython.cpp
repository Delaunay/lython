#include <iostream>
#include <sstream>

#include "utilities/metadata.h"

#include "ast/expressions.h"
#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

#include "interpreter/interpreter.h"
#include "logging/logging.h"

#include "../tests/samples.h"
#include "revision_data.h"

using namespace lython;

bool compare(String const& a, String const& b){
    auto size = std::min(a.size(), b.size());

    for(size_t i = 0; i < size; ++i){
        if (a[i] != b[i]){
            std::cout << i << " `" << a[i] << "` != `" << b[i] << "` | ";
            return false;
        }
    }

    return a.size() == b.size();
}

String strip(String const& v){
    int i = int(v.size()) - 1;

    while (i > 0 && v[size_t(i)] == '\n'){
        i -= 1;
    }

    return String(v.begin(), v.begin() + i + 1);
}

Expression make_point(Module& mod);


int main() {
    {
        info("Enter");

        // auto cst = AST::Constant<int>(10, "int");
        // auto pl1 = AST::Placeholder("name", "double");
        // auto pl2 = AST::Placeholder("name", "int");

        //*
        // debug info
        std::cout << "\n"
                     "[0] Lython Interpreter \n"
                     "[0]   Compiler: " COMPILER_ID " " COMPILER_VERSION "\n"
                     "[0]     Branch: " _BRANCH "\n"
                     "[0]    Version: " _HASH "\n"
                     "[0]       Date: " _DATE "\n\n";

        // ConsoleBuffer reader;

        String code =
        "struct Point:\n"
        "    x: Float\n"
        "    y: Float\n"
        "\n"

        "a = 1\n"

        "def get_x(p: Point) -> Float:\n"
        "    return p.x\n\n"

        "def set_x(p: Point, x: Float):\n"
        "    p.x = x\n\n"
        ;

        "def function2(test: double, test) -> double:\n"
        "    \"\"\"This is a docstring\"\"\"\n"
        "    return add(1, 1)\n\n"

        "def function3(test: int, test) -> e:\n"
        "    return add(1, 1)\n\n"

        "struct Object:\n"
        "    \"\"\"This is a docstring\"\"\"\n"
        "    a: Type\n";

        StringBuffer reader(code);

        String lexer_string;
        {
            Lexer lex(reader);

            StringStream ss;
            lex.print(ss);
            lexer_string = ss.str();
        }

        reader.reset();
        Module module;
        String parser_string;

        try {
            Parser par(reader, &module);
            Expression expr;
            do {
                expr = par.parse_one(module);

                if (expr){
                    StringStream ss;
                    expr.print(ss);

                    parser_string = ss.str();
                }

            } while(expr);

        } catch (lython::Exception e) {
            std::cout << "Error Occured:" << std::endl;
            std::cout << "\t" << e.what() << std::endl;
        }

        std::cout << std::string(80, '-') << '\n';

        std::cout << strip(lexer_string) << std::endl;
        std::cout << strip(parser_string) << std::endl;
        std::cout << strip(code) << std::endl;

        std::cout << std::string(80, '-') << '\n';

        for (auto expr : module) {
            if (expr.first == "sin")
                continue;

            if (expr.first == "min")
                continue;

            if (expr.first == "max")
                continue;

            if (expr.first == "Type")
                continue;

            if (expr.first == "Float")
                continue;

            std::cout << expr.first << ":" << std::endl;
            expr.second.print(std::cout) << "\n\n";
        }

        // -------------------------------------------------
        module.print(std::cout);

//        for(Index i = 0; i < module.size(); ++i){
//            std::cout << int(i) << " " << std::endl;
//            module.get_item(i)->print(std::cout) << std::endl;
//        }
        std::cout << std::string(80, '-') << '\n';

        Interpreter vm(module);

//        "pp = Point(1.0, 2.0)\n"

//        "get_x(pp)\n"

//        "set_x(pp, 3)\n"

//        "get_x(pp)\n";

        auto expr = make_point(module);
        Value v = vm.eval(expr);

        v.print(std::cout) << std::endl;
        std::cout << code.size() << std::endl;
        std::cout << std::endl;
    }
    show_alloc_stats();
    // StringDatabase::instance().report(std::cout);
    return 0;
}

Expression make_point(Module& mod){
    auto expr = Expression::make<AST::Call>();
    AST::Call* call = expr.ref<AST::Call>();
    call->function = mod.find("Point");
    call->arguments.emplace_back(Expression::make<AST::Value>(1.0, Expression()));
    call->arguments.emplace_back(Expression::make<AST::Value>(2.0, Expression()));
    return expr;
}


Expression make_max(Module& mod){
    auto expr = Expression::make<AST::Call>();
    AST::Call* call = expr.ref<AST::Call>();
    call->function = mod.find("max_alias");
    call->arguments.emplace_back(Expression::make<AST::Value>(1.0, Expression()));
    call->arguments.emplace_back(Expression::make<AST::Value>(2.0, Expression()));
    return expr;
}
