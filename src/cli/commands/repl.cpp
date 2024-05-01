#include <iostream>
#include <sstream>

#include "cli/commands/repl.h"

#include "ast/nodes.h"
#include "ast/ops.h"
#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "logging/logging.h"
#include "parser/parser.h"
#include "sema/sema.h"
#include "vm/tree.h"

#include "dependencies/formatter.h"

namespace lython {

class InteractiveConsole: public ConsoleBuffer {
    public:
    InteractiveConsole(std::function<void()> h, std::function<void(String const&)> command):
        handler(h), command(command), ConsoleBuffer(false) {
        init();
    }

    std::function<void()>              handler;
    std::function<void(String const&)> command;

    bool filter(String const& str) override {
        if (str[0] == '%') {
            command(str);
            return true;
        }
        return false;
    }

    void on_next_line() override {
        if (handler) {
            handler();
        }
    }
};

class InteractiveLexer: public Lexer {
    public:
    using Super = Lexer;

    int prev    = 0;
    int indent  = 0;
    int newline = 0;

    InteractiveLexer(AbstractBuffer& reader): Lexer(reader) {}

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

        // Convert 2 newlines to desindent
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
    InteractiveConsole* reader = nullptr;
    InteractiveLexer*   lex    = nullptr;
    Parser*             parser = nullptr;
    SemanticAnalyser*   sema   = nullptr;
    TreeEvaluator*      eval   = nullptr;
    InMemory            past_logs;

    using Command = std::function<void(InteractiveContext* self, Array<String> const&)>;

    Dict<String, Command> commands = {
        {"globals", [](InteractiveContext* self, Array<String> const& args){
            self->out() << "\n";
            self->out() << format("      {:>20} | {:>20} | {}\n", "name", "value", "type");
            self->out() << format("      {:>20} | {:>20} | {}\n", String(20, '-'), String(20, '-'), String(20, '-'));
            for(ValuePair& var: self->eval->variables) {
                Value val = var.value;
                String strval = str(val);

                auto& registry = meta::TypeRegistry::instance();
                auto& meta = registry.id_to_meta[val.tag];

                self->out() << format("      {:>20} | {:>20} | {}\n", var.name, strval, meta.name);
            }
            self->out() << "\n";
        }},
        {
            "log", [](InteractiveContext* self, Array<String> const& args){
                self->out() << "\n";
                String logname = args[1];

                LogSystem& logsys = LogSystem::system();
                Output& logstdout = *logsys.outputs[0].get();
                self->past_logs.flush_to(logstdout);

                // for(auto& log: LogSystem::system().loggers) {
                //     if (log->name == std::string(logname.c_str())) {
                //         auto* logger = log.get();

                //         logsys.outputs[logger->outputs[0]]->flush_to(logstdout);
                //         return;
                //     }
                // }
            }
        }
    };

    int in_count        = 0;
    int count           = 0;
    int parser_finished = 0;

    std::ostream& out() { return std::cout; }

    void command(String const& cmd) {
        Array<String> args = split(' ', strip(cmd.substr(1)));

        Command fun = commands[args[0]];
        if (fun) {
            fun(this, args);
        } else {
            out() << args[0] << " not found\n";
        }
        count = 0;
    }

    void clear() {
        count = 0;
        parser->clear_errors();
        sema->errors.clear();
        eval->clear_exceptions();
    }

    void savelogs() {
        // Save old logs and clear current buffer
        past_logs.clear();
        for(auto& output: LogSystem::system().outputs) {
            output->flush_to(past_logs);
            output->clear();
        }
    }

    void output(Value v) {
        if (v.tag == meta::type_id<_LyException*>()) {
            out() << v << "\n";
        } else if (v.tag != meta::type_id<_Invalid>()) {
            out() << " [Out] " << v << "\n";
        }
        clear();
    }

    void on_next_line() {
        // parser_finished ?
        // probably need to indent if last tokens were `:\n`
        // if inside () we do not need
        if (count == 0) {
            out() << "  [In] ";
            in_count += 1;
        } else {
            out() << "   ..  ";
        }
        count += 1;
    }
};

void repl() {
    LogSystem::system();

    outlog();
    errlog();

    for(auto& log: LogSystem::system().loggers) {
        log->outputs.clear();
        auto out = new_output<InMemory>();
        log->add_output(out);
    }

    InteractiveContext ctx;
    SemanticAnalyser   sema;
    TreeEvaluator      eval;
    ctx.sema   = &sema;
    ctx.eval   = &eval;

    InteractiveConsole reader([&]() { ctx.on_next_line(); },
                              [&](String const& name) { ctx.command(name); });
    InteractiveLexer   lex(reader);
    Parser             parser(lex);


    ctx.reader = &reader;
    ctx.lex    = &lex;
    ctx.parser = &parser;


    Module mod;
    while (true) {
        ctx.savelogs();
        ctx.clear();

        while (lex.token().type() == tok_desindent) {
            std::cout << "Eating dangling desindent\n";
            lex.next_token();
        }

        // Read
        ctx.parser_finished = false;
        StmtNode* stmt      = parser.parse_one(&mod, 0, true);

        if (parser.has_errors()) {
            parser.show_diagnostics(std::cout);
            continue;
        }

        // Semantic Analysis
        ctx.parser_finished = true;
        sema.exec(stmt, 0);

        if (sema.has_errors()) {
            sema.show_diagnostic(std::cout);
            continue;
        }

        // Run
        Value result = eval.eval(stmt);

        // Print
        ctx.output(result);
    }
}

int ReplCmd::main(argparse::ArgumentParser const& args) {
    repl();
    return 0;
}

}  // namespace lython