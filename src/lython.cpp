#include <iostream>
#include <sstream>

#include "utilities/metadata.h"

#include "ast/expressions.h"
#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

#include "interpreter/interpreter.h"
#include "logging/logging.h"

// #include "Lexer/Prelexer.h"

#include "revision_data.h"

using namespace lython;

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
            "def my_function1() -> e:\n"
            "    return 3 + x * 2 / (1 - 5) ^ 2 ^ 3\n"
            "\n" // 3 4 2 × 1 5 − 2 3 ^ ^ ÷ +

            "def my_function3() -> e:\n"
            "    return 2\n"
            "\n"

            "def my_function1() -> e:\n"
            "    return sin(max (2, 3) / 3 * pi)\n"
            "\n" // 2 3 max 3 ÷ π × sin

            "def my_max(a: Double, b: Double) -> Double:\n"
            "    return max(a, b)\n"
            "\n"

            "struct Object:\n"
            "    \"\"\"This is a docstring\"\"\"\n"
            "    a: Float\n"
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

        {
            Lexer lex(reader);
            lex.print(std::cout);
            // lex.debug_print(std::cout);
            std::cout << std::endl;
        }

        reader.reset();
        Module module;

        try {
            Parser par(reader, &module);
            ST::Expr expr = nullptr;
            do {
                expr = par.parse_one(module);

                if (expr){
                    std::cout << "--\n\n";
                    expr->print(std::cout) << "\n";
                }

            } while(expr != nullptr);

        } catch (lython::Exception e) {
            std::cout << "Error Occured:" << std::endl;
            std::cout << "\t" << e.what() << std::endl;
        }

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
            expr.second->print(std::cout) << "\n\n";
        }

        // -------------------------------------------------
        module.print(std::cout);

//        for(Index i = 0; i < module.size(); ++i){
//            std::cout << int(i) << " " << std::endl;
//            module.get_item(i)->print(std::cout) << std::endl;
//        }
        std::cout << std::string(80, '-') << '\n';

        Interpreter vm(&module);
        AST::Call* call = new AST::Call();
        call->function() = module.find("my_function3");

        Value v = vm.eval(ST::Expr(call));

        v.print(std::cout);
        std::cout << std::endl;
    }
    show_alloc_stats();

    return 0;
}
