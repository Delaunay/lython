
#include "utilities/trie.h"
#include <catch2/catch.hpp>

TEST_CASE("Trie") {
    using Trie = lython::Trie<128>;

    Trie base;

    base.insert("abcd");
    // Trie cp;
    // cp = base;
    // REQUIRE(cp.has("abcd") == true);

    REQUIRE(base.has("a") == false);
    REQUIRE(base.has("ab") == false);
    REQUIRE(base.has("abcd") == true);
    REQUIRE(base.has("abcde") == false);
    REQUIRE(base.has("vc") == false);

    REQUIRE(base.matching("a")->leaf() == false);
    REQUIRE(base.matching("ab")->leaf() == false);
    REQUIRE(base.matching("abcd")->leaf() == true);

    REQUIRE(base.matching("a")->has_children() >= 1);
    REQUIRE(base.matching("ab")->has_children() >= 1);
    REQUIRE(base.matching("abcd")->has_children() == 0);

    base.insert("abce");
    auto lookup = base.matching("abc");
    REQUIRE(lookup != nullptr);

    auto leaf = lookup->matching("d");
    REQUIRE(leaf->leaf() == true);
}
