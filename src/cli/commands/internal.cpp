#include <iostream>
#include <sstream>

#include "cli/commands/internal.h"

#include "ast/nodes.h"
#include "ast/ops.h"
#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "logging/logging.h"
#include "parser/parser.h"
#include "sema/sema.h"
#include "vm/tree.h"

namespace lython {

bool compare(String const& a, String const& b) {
    auto size = std::min(a.size(), b.size());

    for (size_t i = 0; i < size; ++i) {
        if (a[i] != b[i]) {
            std::cout << i << " `" << a[i] << "` != `" << b[i] << "` | ";
            return false;
        }
    }

    return a.size() == b.size();
}

String strip2(String const& v) {
    int i = int(v.size()) - 1;

    while (i > 0 && v[size_t(i)] == '\n') {
        i -= 1;
    }

    return String(v.begin(), v.begin() + i + 1);
}

class InteractiveConsole: public ConsoleBuffer {
public:
    InteractiveConsole(std::function<void()> h): handler(h), ConsoleBuffer(false) {
        init();
    }

    std::function<void()> handler;

    void on_next_line() override {
        if (handler) {
            handler();
        }
    }
};

class InteractiveLexer: public Lexer {
public:
    using Super = Lexer;

    int prev = 0;
    int indent = 0;
    int newline = 0;

    InteractiveLexer(AbstractBuffer& reader):
        Lexer(reader)
    {}

    Token const& next_token() {
        
        Token const& tok = Super::next_token();

        if (tok.type() == tok_indent) {
            indent += 1;
        }

        if (tok.type() == tok_newline && indent > 0) {
            newline += 1;
        } else {
            newline = 0;
        }

        if (newline >= 2) {
            newline = 0;
            _oindent -= 1;
            _cindent -= 1;
            indent -= 1;
            return make_token(tok_desindent);
        }

        return tok;
    }
};

struct InteractiveContext {
    InteractiveConsole *reader = nullptr;
    InteractiveLexer   *lex = nullptr;
    Parser             *parser = nullptr;
    SemanticAnalyser   *sema = nullptr;
    TreeEvaluator      *eval = nullptr;

    int in_count = 0;
    int count = 0;
    int parser_finished = 0;

    std::ostream& out() {
        return std::cout;
    }

    void clear() {
        count = 0;
        parser->clear_errors();
        sema->errors.clear();
    }

    void output(Value v) {
        if (v.tag != meta::type_id<_Invalid>()) {
            out() << " [Out] " << v << "\n";
        }
        clear();
    }

    void on_next_line() {
        // parser_finished ?
        // probably need to indent if last tokens were `:\n`
        // if inside () we do not need 
        if (count == 0) 
        {
            out() << "  [In] ";
            in_count += 1;
        } else {
            out() << "   ..  ";
        }
        count += 1;
    }
};

void repl() {
    InteractiveContext ctx;

    InteractiveConsole reader([&]() { ctx.on_next_line(); });
    InteractiveLexer   lex(reader);
    Parser             parser(lex);
    SemanticAnalyser   sema;
    TreeEvaluator      eval;

    ctx.reader  = &reader;
    ctx.lex     = &lex;
    ctx.parser  = &parser;
    ctx.sema    = &sema;
    ctx.eval    = &eval;

    Module mod;
    while (true) {
        while (lex.token().type() == tok_desindent) {
            std::cout << "Eating dangling desindent\n";
            lex.next_token();
        }

        ctx.parser_finished = false;
        StmtNode* stmt = parser.parse_one(&mod, 0, true);

        if (parser.has_errors()) {
            parser.show_diagnostics(std::cout);
            ctx.clear();
            continue;
        }

        ctx.parser_finished = true;
        sema.exec(stmt, 0);

        if (sema.has_errors()) {
            sema.show_diagnostic(std::cout);
            ctx.clear();
            continue;
        }

        ctx.output(eval.eval(stmt));
    }
}

int InternalCmd::main(argparse::ArgumentParser const& args) {
    //
    repl();
    return 0;

    std::string file = "";
    if (args.is_used("--file")) {
        file = args.get<std::string>("--file");
    }

    bool dump_lexer        = args.get<bool>("--debug-lexer");
    bool lexer_format      = args.get<bool>("--lexer-format");
    bool show_alloc_layout = true;
    bool show_parsing      = args.get<bool>("--parsing");

    kwinfo("Enter");

    std::unique_ptr<AbstractBuffer> reader;
    if (file != "") {
        reader = std::make_unique<FileBuffer>(String(file.c_str()));
    } else {
        reader = std::make_unique<ConsoleBuffer>();
    }

    if (dump_lexer) {
        std::cout << std::string(80, '=') << '\n';
        std::cout << "Lexer Token Dump\n";
        {
            Lexer lex(*reader.get());
            lex.debug_print(std::cout);
            reader->reset();
        }
        std::cout << std::string(80, '-') << '\n';
        return 0;
    }

    if (lexer_format) {
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
        return 0;
    }

    Module* mod = nullptr;

    try {

        Lexer        lex(*reader.get());
        Parser       parser(lex);
        StringStream parsing;

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
            parser.show_diagnostics(std::cout);
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

        // Memory layout dump
        {
            //
            mod->dump(std::cout);
        }

        if (show_parsing) {
            return 0;
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
                std::cout << std::string(80, '-') << '\n';
                std::cout << "Sema Diagnostic dump\n";
                std::cout << std::string(80, '-') << '\n';

                sema.show_diagnostic(std::cout, &lex);
                std::cout << std::string(80, '-') << '\n';
            }

            // Memory layout dump
            {
                //
                mod->dump(std::cout);
            }

            if (has_circle(mod)) {
                kwwarn("Circle will cause infinite recursion");
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

    delete mod;

    return 0;
};
}  // namespace lython