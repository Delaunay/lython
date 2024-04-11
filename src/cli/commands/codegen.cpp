// Include
#include "cli/commands/codegen.h"

// Kiwi
#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "sema/sema.h"

#include "codegen/clang/clang_gen.h"
#include "codegen/llvm/llvm_gen.h"


namespace lython {

argparse::ArgumentParser* CodegenCmd::parser() {
    argparse::ArgumentParser* p = new_parser();

    p->add_argument("--file")  //
        .help("file to process");

    return p;
}

int CodegenCmd::main(argparse::ArgumentParser const& args) {
    
    std::string file;
    if (args.is_used("--file")) {
        file = args.get<std::string>("--file");
    }

    std::unique_ptr<AbstractBuffer> reader = std::make_unique<FileBuffer>(String(file.c_str()));
    Module* mod = nullptr;

    Lexer        lex(*reader.get());
    Parser       parser(lex);
    mod = parser.parse_module();

    parser.show_diagnostics(std::cout);

    SemanticAnalyser sema;
    sema.exec(mod, 0);
    sema.show_diagnostic(std::cout, &lex);

#if WITH_CLANG_CODEGEN
    std::cout << "CLANG_CODE_GEN\n";
    ClangGen generator;
    generator.exec(mod, 0);
    generator.dump();
#elif WITH_LLVM && WITH_LLVM_CODEGEN
    std::cout << "LLVM_CODE_GEN\n";
    LLVMGen generator;
    generator.exec(mod, 0);
    generator.dump();
#endif

    return 0;
};



}