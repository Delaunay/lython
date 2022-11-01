#pragma once

#include "cli/command.h"

namespace lython {
struct FormatCmd: public Command {
    FormatCmd(): Command("fmt") {}

    virtual argparse::ArgumentParser* parser() {
        argparse::ArgumentParser* p = new_parser();
        return p;
    }

    virtual int main(argparse::ArgumentParser const& args) {
        //
        return 0;
    };
};

}  // namespace lython