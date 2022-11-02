#pragma once

#include "cli/command.h"

namespace lython {
struct FormatCmd: public Command {
    FormatCmd(): Command("fmt") {}

    virtual argparse::ArgumentParser* parser();

    virtual int main(argparse::ArgumentParser const& args);
};

}  // namespace lython