#include <iostream>
#include <sstream>

#include "utilities/metadata.h"

// #include "ast/expressions.h"
#include "ast/nodes.h"
#include "ast/ops.h"
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

struct Args {
    std::string file              = "";
    bool        dump_lexer        = true;
    bool        lexer_format      = true;
    bool        dump_string_db    = true;
    bool        show_alloc_stats  = true;
    bool        show_alloc_layout = true;
};

template <typename T>
struct ArgumentsParser {
    using Handler = std::function<void(T &args, std::string const &value)>;

    void add_argument(std::string const &name, Handler fun) { parser[name] = fun; }

    T parse_args(int argc, const char *argv[]) {
        T args;

        for (int i = 1; i < argc;) {
            auto name  = std::string(argv[i]);
            auto value = std::string(argv[i + 1]);

            auto handler_result = parser.find(name);
            if (handler_result != parser.end()) {
                handler_result->second(args, value);
            }

            i += 2;
        }

        return args;
    }

    private:
    std::unordered_map<std::string, Handler> parser;
};

int main(int argc, const char *argv[]) {
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

    ArgumentsParser<Args> argparser;
    argparser.add_argument("--file", [](Args &arg, std::string const &value) { arg.file = value; });
    argparser.add_argument("--debug-lexer", [](Args &arg, std::string const &value) {
        std::stringstream ss(value);
        ss >> arg.dump_lexer;
    });
    argparser.add_argument("--lexer-format", [](Args &arg, std::string const &value) {
        std::stringstream ss(value);
        ss >> arg.lexer_format;
    });
    Args args = argparser.parse_args(argc, argv);

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

        std::unique_ptr<AbstractBuffer> reader;
        if (args.file != "") {
            reader = std::make_unique<FileBuffer>(String(args.file.c_str()));
        } else {
            reader = std::make_unique<ConsoleBuffer>();
        }

        if (args.dump_lexer) {
            std::cout << std::string(80, '=') << '\n';
            std::cout << "Lexer Token Dump\n";
            {
                Lexer lex(*reader.get());
                lex.debug_print(std::cout);
                reader->reset();
            }
            std::cout << std::string(80, '-') << '\n';
        }

        if (args.lexer_format) {
            std::cout << std::string(80, '=') << '\n';
            std::cout << "Lexing Round-trip\n";
            String lexer_string;
            {
                Lexer        lex(*reader.get());
                StringStream ss;
                lex.print(ss);
                lexer_string = ss.str();
                reader->reset();
            }

            std::cout << std::string(80, '-') << '\n';
            std::cout << strip2(lexer_string) << std::endl;
            std::cout << std::string(80, '-') << '\n';
        }

        Module *mod = nullptr;

        try {

            Lexer  lex(*reader.get());
            Parser parser(lex);

            // Parsing Logs
            // ------------
            {
                std::cout << std::string(80, '=') << '\n';
                std::cout << "Parsing Trace\n";
                std::cout << std::string(80, '-') << '\n';
                mod = parser.parse_module();
            }

            // Parsing diagnostics
            // -------------------
            {
                std::cout << std::string(80, '-') << '\n';
                std::cout << "Parsing Diag\n";
                std::cout << std::string(80, '-') << '\n';
                for (auto &diag: parser.get_errors()) {
                    diag.print(std::cout);
                }
                std::cout << std::string(80, '-') << '\n';
            }

            // Parsing dump
            // ------------
            {
                std::stringstream ss;
                for (auto stmt: mod->body) {
                    print(str(stmt), ss);
                    ss << "\n";
                }
                std::cout << std::string(80, '-') << '\n';
                std::cout << "Parsed Module dump\n";
                std::cout << std::string(80, '-') << '\n';
                std::cout << ss.str();
                std::cout << std::string(80, '-') << '\n';
            }

            // Sema
            // ----
            {
                SemanticAnalyser sema;

                std::cout << std::string(80, '-') << '\n';
                std::cout << "Sema Logs\n";
                std::cout << std::string(80, '-') << '\n';
                sema.exec(mod, 0);
                std::cout << std::string(80, '-') << '\n';

                // Sema Diagnostic
                // ---------------
                {
                    std::stringstream ss;
                    for (auto &diag: sema.errors) {
                        ss << "  - " << diag->what() << "\n";
                    }

                    std::cout << std::string(80, '-') << '\n';
                    std::cout << "Sema Diagnostic dump\n";
                    std::cout << std::string(80, '-') << '\n';
                    std::cout << ss.str();
                    std::cout << std::string(80, '-') << '\n';
                }

                // Bindings Dump
                // -------------
                {
                    std::stringstream ss;
                    sema.bindings.dump(ss);

                    std::cout << std::string(80, '-') << '\n';
                    std::cout << "Sema bindings dump\n";
                    std::cout << std::string(80, '-') << '\n';
                    std::cout << ss.str();
                    std::cout << std::string(80, '-') << '\n';
                }
            }

        } catch (lython::Exception e) {
            std::cout << "Error Occured:" << std::endl;
            std::cout << "\t" << e.what() << std::endl;
        }

        if (args.show_alloc_layout) {
            std::cout << std::string(80, '=') << '\n';
            std::cout << "Alloc Layout\n";
            std::cout << std::string(80, '-') << '\n';
            std::stringstream ss;
            mod->dump(ss);
            std::cout << ss.str();
            std::cout << std::string(80, '-') << '\n';
        }

        delete mod;
    }

    if (args.show_alloc_stats) {
        std::cout << std::string(80, '=') << '\n';
        std::cout << "Alloc\n";
        show_alloc_stats();
    }

    if (args.dump_string_db) {
        StringDatabase::instance().report(std::cout);
    }
    return 0;
}
