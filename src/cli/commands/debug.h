#pragma once

#include "cli/command.h"

namespace lython {

struct DebugCmd: public Command {
    DebugCmd(): Command("debug") {}

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