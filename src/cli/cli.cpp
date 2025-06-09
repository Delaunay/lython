
#include "revision_data.h"

#include "cli/commands/code.h"
#include "cli/commands/codegen.h"
#include "cli/commands/debug.h"
#include "cli/commands/doc.h"
#include "cli/commands/format.h"
#include "cli/commands/install.h"
#include "cli/commands/internal.h"
#include "cli/commands/linter.h"
#include "cli/commands/profile.h"
#include "cli/commands/tests.h"
#include "cli/commands/vm.h"
#include "cli/commands/repl.h"

#include "utilities/metadata.h"
#include "utilities/names.h"
#include "utilities/strings.h"

using namespace lython;

const char* VERSION_STRING = "\n"
                             "[0] Lython Interpreter \n"
                             "[0]   Compiler: " COMPILER_ID " " COMPILER_VERSION "\n"
                             "[0]     Branch: " _BRANCH "\n"
                             "[0]    Version: " _HASH "\n"
                             "[0]       Date: " _DATE "\n\n";




#ifndef BUILD_WEBASSEMBLY
#define BUILD_WEBASSEMBLY 1
#endif

#if BUILD_WEBASSEMBLY

#else
int main(int argc, const char* argv[]) {

    auto linter   = std::make_unique<LinterCmd>();
    auto install  = std::make_unique<InstallCmd>();
    auto codegen  = std::make_unique<CodegenCmd>();
    auto fmtcmd   = std::make_unique<FormatCmd>();
    auto debug    = std::make_unique<DebugCmd>();
    auto doc      = std::make_unique<DocCmd>();
    auto profile  = std::make_unique<ProfileCmd>();
    auto tests    = std::make_unique<TestsCmd>();
    auto internal = std::make_unique<InternalCmd>();
    auto vm       = std::make_unique<VMCmd>();
    auto repl       = std::make_unique<ReplCmd>();

    // There is a problem when putting unique ptr inside the array :/
    Array<Command*> commands = {
        linter.get(),
        install.get(),
        codegen.get(),
        fmtcmd.get(),
        debug.get(),
        doc.get(),
        profile.get(),
        tests.get(),
        internal.get(),
        vm.get(),
        repl.get(),
    };

    // Main Parser
    // --------------------------------------------
    argparse::ArgumentParser lython_args("lython", "", argparse::default_arguments::help);
    lython_args.add_description("");
    lython_args.add_argument("--show-alloc-stats")
        .help("Print memory allocation statistics")
        .default_value(false)
        .implicit_value(true);

    lython_args.add_argument("--show-string-stats")
        .help("Print string database allocations")
        .default_value(false)
        .implicit_value(true);

    lython_args.add_argument("-v", "--version")
        .action([&](const auto& /*unused*/) {
            std::cout << VERSION_STRING;
            std::exit(0);
        })
        .default_value(false)
        .help("prints version information and exits")
        .implicit_value(true)
        .nargs(0);

    for (Command* cmd: commands) {
        lython_args.add_subparser(*cmd->parser());
    }

    try {
        lython_args.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << lython_args;
        return -1;
    }

    // -----------------
    register_globals();
    show_alloc_stats_on_destroy(lython_args.get<bool>("--show-alloc-stats"));
    show_string_stats_on_destroy(lython_args.get<bool>("--show-string-stats"));

    // Execute command
    for (Command* cmd: commands) {
        if (lython_args.is_subcommand_used(cmd->name)) {
            return cmd->exec();
        }
    }

    // At least one command should have been selected
    std::cerr << lython_args;
    return -1;
}
#endif