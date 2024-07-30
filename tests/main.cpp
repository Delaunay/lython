#define CATCH_CONFIG_RUNNER
#include <catch2/catch_all.hpp>

#include "utilities/metadata.h"
#include "utilities/names.h"

#include "lexer/lexer.h"
#include "lexer/token.h"
#include "utilities/strings.h"

int main(int argc, char* argv[]) {
    using namespace lython;

    // set_log_level(LogLevel::Trace, true);
    // set_log_level(LogLevel::Debug, true);
    // set_log_level(LogLevel::Info, true);
    // set_log_level(LogLevel::Warn, true);
    // set_log_level(LogLevel::Error, true);
    // set_log_level(LogLevel::Fatal, true);

    bool show = false;

    register_globals();
    show_alloc_stats_on_destroy(show);
    show_string_stats_on_destroy(show);

    int result = Catch::Session().run(argc, argv);

    return result;
}