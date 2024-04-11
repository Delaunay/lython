#include "cli/commands/format.h"

#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

namespace lython {
argparse::ArgumentParser* FormatCmd::parser() {
    argparse::ArgumentParser* p = new_parser();
    p->add_description("Format lython source files");
    p->add_argument("sources")                      //
        .remaining()                                //
        .help("Directories or files to reformat");  //

    p->add_argument("--inplace")
        .default_value(false)
        .implicit_value(true)
        .help("Reformat files inplace");

    p->add_argument("--ast")   //
        .default_value(true)   //
        .implicit_value(true)  //
        .help("Use the AST to reformat the code");

    p->add_argument("--fuzz")  //
        .default_value(true)   //
        .implicit_value(true)  //
        .help("Reads from stdin for fuzzing");

    p->add_argument("--extension")                       //
        .default_value(std::vector<std::string>{".ly"})  //
        .nargs(argparse::nargs_pattern::any)
        .help("File extensions require, this is to prevent trying to reformat files that are not "
              "lython code");

    p->add_argument("--dump")  //
        .default_value(true)   //
        .implicit_value(true)  //
        .help(
            "Allways dump parsed AST even if an error occured, (--inplace gets disabled on error)");

    return p;
}

void find_regular_files(std::string const&              path,
                        std::vector<std::string> const& extensions,
                        Array<fs::path>&                out);
bool has_extension(fs::path const& file, std::vector<std::string> const& extensions);
int  ast_reformat_file(fs::path const& file, bool _ = false);
int  tok_reformat_file(fs::path const& file, bool _ = false);

int FormatCmd::main(argparse::ArgumentParser const& args) {
    //
    if (!args.is_used("sources")) {
        std::cout << "No sources provided" << std::endl;
        return -1;
    }

    auto paths = args.get<std::vector<std::string>>("sources");

    Array<fs::path>          regular_files;
    std::vector<std::string> extensions = args.get<std::vector<std::string>>("extension");

    for (std::string const& path: paths) {
        find_regular_files(path, extensions, regular_files);
    }

    kwinfo(outlog(), "Found {} files", regular_files.size());

    bool ast = args.get<bool>("--ast");
    if (args.get<bool>("fuzz")) {
        if (ast) {
            return ast_reformat_file("/dev/stdin", args.get<bool>("dump"));
        } else {
            return tok_reformat_file("/dev/stdin");
        }
    }

    int result = 0;

    for (auto const& path: regular_files) {
        if (ast) {
            result += ast_reformat_file(path, args.get<bool>("dump"));
        } else {
            result += tok_reformat_file(path);
        }
    }

    return result;
};

void find_regular_files(std::string const&              path,
                        std::vector<std::string> const& extensions,
                        Array<fs::path>&                out) {
    if (fs::is_regular_file(path)) {
        out.push_back(path);
        return;
    }

    if (fs::is_directory(path)) {
        for (fs::path const& file: fs::recursive_directory_iterator(path)) {
            if (fs::is_regular_file(file) && has_extension(file, extensions)) {

                out.push_back(file);
            }
        }
    }
}

bool has_extension(fs::path const& file, std::vector<std::string> const& extensions) {

    for (auto const& ext: extensions) {
        if (!file.has_extension())
            return false;

        if (file.extension() == ext) {
            return true;
        }
    }

    return false;
}

int tok_reformat_file(fs::path const& file, bool) {
    std::cout << "reformat: " << file << std::endl;

    String file_str = file.generic_string().c_str();

    Unique<AbstractBuffer> reader = std::make_unique<FileBuffer>(file_str);
    Lexer                  lex(*reader.get());

    StringStream ss;
    lex.print(ss);

    std::cout << ss.str() << "\n";
    return 0;
}

int ast_reformat_file(fs::path const& file, bool dump) {
    std::cout << "reformat: " << file << std::endl;

    String file_str = file.generic_string().c_str();

    Unique<AbstractBuffer> reader = std::make_unique<FileBuffer>(file_str);
    Lexer                  lex(*reader.get());
    Parser                 parser(lex);
    Module*                mod = nullptr;

    mod = parser.parse_module();
    parser.show_diagnostics(std::cout);
    int ec = 0;

    if (parser.has_errors()) {
        ec = -1;
    }

    if (parser.has_errors() && !dump) {
        return ec;
    }

    if ((parser.has_errors() && dump) || !parser.has_errors()) {
        StringStream ss;
        print(str(mod), ss);

        // We should compute a hash of the original file
        // and a hash of the formated string
        std::cout << ss.str() << "\n";
    }

    return ec;
}

}  // namespace lython