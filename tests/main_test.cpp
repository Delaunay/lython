#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include "utilities/metadata.h"

int main(int argc, char *argv[]) {
    lython::metadata_init_names();

    int result = Catch::Session().run(argc, argv);

    lython::show_alloc_stats();
    return result;
}