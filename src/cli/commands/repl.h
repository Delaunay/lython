
#pragma once

#include "cli/command.h"

namespace lython {
struct ReplCmd: public Command {
    // Read Eval Print Loop
    ReplCmd(): Command("repl") {}

    virtual argparse::ArgumentParser* parser() {
        argparse::ArgumentParser* p = new_parser();
        return p;
    }

    virtual int main(argparse::ArgumentParser const& args);
};

}  // namespace lython