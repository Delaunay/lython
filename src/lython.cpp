#include <iostream>
#include <sstream>

#include "utilities/metadata.h"

// #include "ast/expressions.h"
#include "ast/nodes.h"
#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "sema/sema.h"

// #include "interpreter/interpreter.h"
#include "logging/logging.h"

#include "../tests/samples.h"
#include "revision_data.h"

using namespace lython;

bool compare(String const &a, String const &b) {
    auto size = std::min(a.size(), b.size());

    for (size_t i = 0; i < size; ++i) {
        if (a[i] != b[i]) {
            std::cout << i << " `" << a[i] << "` != `" << b[i] << "` | ";
            return false;
        }
    }

    return a.size() == b.size();
}

String strip2(String const &v) {
    int i = int(v.size()) - 1;

    while (i > 0 && v[size_t(i)] == '\n') {
        i -= 1;
    }

    return String(v.begin(), v.begin() + i + 1);
}

/*
Expression make_point(Module &mod);
Expression make_point_check(Module &mod);
Expression make_import_call_check(Module &mod);
*/

int main() {
    {
        metadata_init_names();
        // Static globals
        {
            StringDatabase::instance();
            default_precedence();
            keywords();
            keyword_as_string();
            strip_defaults();
        }
        // --
        track_static();
    }

    {
        info("Enter");
        //*
        // debug info
        std::cout << "\n"
                     "[0] Lython Interpreter \n"
                     "[0]   Compiler: " COMPILER_ID " " COMPILER_VERSION "\n"
                     "[0]     Branch: " _BRANCH "\n"
                     "[0]    Version: " _HASH "\n"
                     "[0]       Date: " _DATE "\n\n";

        // ConsoleBuffer reader;

        String code = "def simple_function(a: b, c: d) -> e:\n"
                      "    return a + c\n"
                      "\n";

        "def test1(p: Float, b):\n"
        "    return sin(1)\n\n"

            ;
        "import a.b.c\n"
        "import a.b.c as e\n"
        "from a.b.c import f, k\n"
        "from a.b.c import g as h, i as j\n\n"

        "struct Point:\n"
        "    x: Float\n"
        "    y: Float\n"
        "\n"

        "def test1(p: Float) -> Float:\n"
        "    return sin(1)\n\n"

        "def test3(p: Float) -> Float:\n"
        "    return sin(max(sin(p * 2), sin(p + 1)))\n\n"

        "def test2(p: Float) -> Float:\n"
        "    return sin(max(2, 3) / 3 * p)\n\n"

        "def get_x(p: Point) -> Float:\n"
        "    return p.x\n\n"

        "def set_x(p: Point, x: Float) -> Point:\n"
        "    p.x = x\n"
        "    return p\n\n"

        "def struct_set_get(v: Float) -> Float:\n"
        "    p = Point(1.0, 2.0)\n"
        "    set_x(p, v)\n"
        "    a = get_x(p)\n"
        "    return a\n\n"

        "def call_import() -> Float:\n"
        "    return k(1.0, 2.0)\n\n";

        "def function2(test: double, test) -> double:\n"
        "    \"\"\"This is a docstring\"\"\"\n"
        "    return add(1, 1)\n\n"

        "def function3(test: int, test) -> e:\n"
        "    return add(1, 1)\n\n"

        "struct Object:\n"
        "    \"\"\"This is a docstring\"\"\"\n"
        "    a: Type\n";

        StringBuffer reader(code);

        std::cout << std::string(80, '=') << '\n';
        std::cout << "Lexer Token Dump\n";
        {
            Lexer lex(reader);
            lex.debug_print(std::cout);
            reader.reset();
        }
        std::cout << std::string(80, '-') << '\n';

        std::cout << std::string(80, '=') << '\n';
        std::cout << "Lexing Round-trip\n";
        String lexer_string;
        {
            Lexer        lex(reader);
            StringStream ss;
            lex.print(ss);
            lexer_string = ss.str();
            reader.reset();
        }
        std::cout << std::string(80, '-') << '\n';
        std::cout << strip2(lexer_string) << std::endl;
        std::cout << std::string(80, '-') << '\n';

        std::cout << std::string(80, '=') << '\n';
        std::cout << "Parsing Trace\n";
        std::cout << std::string(80, '-') << '\n';
        Module *mod = nullptr;

        try {
            Lexer lex(reader);

            Parser parser(lex);

            mod = parser.parse_module();

            std::cout << std::string(80, '-') << '\n';
            std::cout << "Parsing Diag\n";
            std::cout << std::string(80, '-') << '\n';
            for (auto &diag: parser.get_errors()) {
                diag.print(std::cout);
            }
            std::cout << std::string(80, '-') << '\n';

            std::cout << std::string(80, '-') << '\n';
            std::cout << "Parsed Module dump\n";
            std::cout << std::string(80, '-') << '\n';
            for (auto stmt: mod->body) {
                stmt->print(std::cout, 0);
                std::cout << "\n";
            }
            std::cout << std::string(80, '-') << '\n';

            SemanticAnalyser sema;
            sema.exec(mod, 0);

            sema.bindings.dump(std::cout);

        } catch (lython::Exception e) {
            std::cout << "Error Occured:" << std::endl;
            std::cout << "\t" << e.what() << std::endl;
        }

        std::cout << std::string(80, '=') << '\n';
        std::cout << "Alloc Layout\n";
        std::cout << std::string(80, '-') << '\n';
        mod->dump(std::cout);
        std::cout << std::string(80, '-') << '\n';

        delete mod;
    }

    std::cout << std::string(80, '=') << '\n';
    std::cout << "Alloc\n";
    show_alloc_stats();
    // StringDatabase::instance().report(std::cout);
    return 0;
}

/*
Expression make_point(Module& mod){
    auto expr = Expression::make<AST::Call>();
    AST::Call* call = expr.ref<AST::Call>();
    call->function = mod.reference("Point");
    call->arguments.emplace_back(
        Expression::make<AST::Value>(1.0, Expression()));
    call->arguments.emplace_back(
        Expression::make<AST::Value>(2.0, Expression()));
    return expr;
}


Expression make_point_check(Module& mod){
    auto expr = Expression::make<AST::Call>();
    AST::Call* call = expr.ref<AST::Call>();
    call->function = mod.reference("struct_set_get");
    call->arguments.emplace_back(
        Expression::make<AST::Value>(2.0, Expression()));
    return expr;
}

Expression make_import_call_check(Module& mod){
    auto expr = Expression::make<AST::Call>();
    AST::Call* call = expr.ref<AST::Call>();
    call->function = mod.reference("call_import");
    return expr;
}


Expression make_max(Module& mod){
    auto expr = Expression::make<AST::Call>();
    AST::Call* call = expr.ref<AST::Call>();
    call->function = mod.reference("max_alias");
    call->arguments.emplace_back(Expression::make<AST::Value>(1.0, Expression()));
    call->arguments.emplace_back(Expression::make<AST::Value>(2.0, Expression()));
    return expr;
}
*/
