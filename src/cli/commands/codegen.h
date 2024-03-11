#pragma once

#include "cli/command.h"

namespace lython {
struct CodegenCmd: public Command {
    CodegenCmd(): Command("codegen") {}

    virtual argparse::ArgumentParser* parser() override;

    virtual int main(argparse::ArgumentParser const& args) override;
};

}  // namespace lython