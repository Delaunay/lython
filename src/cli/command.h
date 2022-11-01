
#pragma once

#include "../dtypes.h"

#include <argparse/argparse.hpp>
#include <memory>

namespace lython {

using ArgumentParser = Unique<argparse::ArgumentParser>;

struct Command {
    const String   name;
    ArgumentParser args;

    Command(String name_): name(name_) {}

    virtual ~Command() {}

    argparse::ArgumentParser* new_parser() {
        args = std::make_unique<argparse::ArgumentParser>(
            name.c_str(), "", argparse::default_arguments::help);
        return args.get();
    }

    virtual argparse::ArgumentParser* parser() { return nullptr; }

    int exec() {
        if (!args) {
            return -1;
        }
        return main(*args.get());
    }

    virtual int main(argparse::ArgumentParser const& args) { return 0; };
};

}  // namespace lython