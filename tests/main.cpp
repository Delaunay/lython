#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include "utilities/metadata.h"
#include "utilities/names.h"

#include "lexer/lexer.h"
#include "lexer/token.h"
#include "utilities/strings.h"

int main(int argc, char *argv[]) {
    using namespace lython;

    {
        metadata_init_names();
        // Static globals
        {
            StringDatabase::instance();
            default_precedence();
            keywords();
            keyword_as_string();
            strip_defaults();
        }
        // --
        track_static();
    }

    int result = Catch::Session().run(argc, argv);

    lython::StringDatabase::instance().report(std::cout);
    lython::show_alloc_stats();

    return result;
}