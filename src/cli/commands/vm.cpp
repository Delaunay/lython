#include "cli/commands/vm.h"


#include "ast/nodes.h"
#include "ast/ops.h"
#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "logging/logging.h"
#include "parser/parser.h"
#include "sema/sema.h"
#include "vm/tree.h"

namespace lython {

VMCmd::VMCmd(): Command("vm") {}

argparse::ArgumentParser* VMCmd::parser() {
    argparse::ArgumentParser* p = new_parser();
        
    p->add_argument("--file")  //
        .help("file to process");

    return p;
}

int VMCmd::main(argparse::ArgumentParser const& args)
{    
    std::string file = "";
    if (args.is_used("--file")) {
        file = args.get<std::string>("--file");
    }

    std::unique_ptr<AbstractBuffer> reader;
    reader = std::make_unique<FileBuffer>(  //
        String(file.c_str())                //
    );

    //
    Lexer            lex(*reader.get());
    Parser           parser(lex);
    SemanticAnalyser sema;

    //
    std::cout << "Parsing\n";
    std::cout << "=======\n";
    Module* mod = parser.parse_module();
    parser.show_diagnostics(std::cout);

    //
    std::cout << "\nSema\n";
    std::cout << "====\n";
    sema.exec(mod, 0);
    sema.show_diagnostic(std::cout, &lex);

    //
    std::cout << "\nExec\n";
    std::cout << "====\n";
    TreeEvaluator eval(sema.bindings);
    
    return eval.eval();
};  

}