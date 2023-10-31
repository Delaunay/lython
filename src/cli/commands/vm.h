#pragma once

#include "cli/command.h"

namespace lython {

struct VMCmd: public Command {
    VMCmd(): Command("vm") {}

    virtual argparse::ArgumentParser* parser();
    virtual int main(argparse::ArgumentParser const& args);
};

}  // namespace lython