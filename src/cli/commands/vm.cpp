#include "cli/commands/vm.h"


#include "ast/nodes.h"
#include "ast/ops.h"
#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "logging/logging.h"
#include "parser/parser.h"
#include "sema/sema.h"
#include "vm/tree.h"
#include "vm/vm.h"

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


    Program p = compile(mod);

    std::cout << "\nVM\n";
    std::cout << "====\n";


    Label* label = nullptr;
    int count = 0;

    for(int i = 0; i < p.instructions.size(); i++) {
        Instruction& inst = p.instructions[i];
        
        count += 1;
        for(Label& l: p.labels) {
            if (l.index == i) {
                label = &l;
                count = 0;
                break;
            }
        }

        int idt = 2;
        if (label)  {
            idt *= label->depth;

            if (count == 0) {
                std::cout << fmt::format("{:4d}", i)  << "|" << String(idt, ' ') << "." << label->name << "\n";
            }
        }
        std::cout << "    |" << String(idt + 2, ' ') << str(inst.stmt) << "\n"; 
    }


    Value result = eval(p);

    //
    // std::cout << "\nExec\n";
    // std::cout << "====\n";
    // TreeEvaluator eval;
    // eval.module(mod, 0);
    return result.tag != meta::type_id<_Invalid>();
};  

}