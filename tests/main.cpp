#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include "utilities/metadata.h"
#include "utilities/names.h"

#include "lexer/lexer.h"
#include "lexer/token.h"
#include "utilities/strings.h"

int main(int argc, char* argv[]) {
    using namespace lython;

    register_globals();
    show_alloc_stats_on_destroy(true);
    show_string_stats_on_destroy(true);

    int result = Catch::Session().run(argc, argv);

    return result;
}