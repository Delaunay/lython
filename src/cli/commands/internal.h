#pragma once

#include "cli/command.h"

namespace lython {
struct InternalCmd: public Command {
    InternalCmd(): Command("internal") {}

    virtual argparse::ArgumentParser* parser() {
        argparse::ArgumentParser* p = new_parser();
        p->add_description("Utilities to debug lython internal components");

        p->add_argument("--file")  //
            .help("file to process");

        p->add_argument("--debug-lexer")
            .help("Dump lexer tokens")
            .default_value(false)
            .implicit_value(true);

        p->add_argument("--lexer-format")
            .help("Format lexer tokens back to the original code")
            .default_value(false)
            .implicit_value(true);

        p->add_argument("--parsing")
            .help("Stop after parsing")
            .default_value(false)
            .implicit_value(true);

        return p;
    }

    virtual int main(argparse::ArgumentParser const& args);
};

}  // namespace lython