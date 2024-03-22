
#include "utilities/trie.h"
#include <catch2/catch_all.hpp>

using namespace lython;

TEST_CASE("Trie") {
    using TestTrie = lython::Trie<128>;

    TestTrie base;

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
    REQUIRE(base.matching("abc")->leaf() == false);
    REQUIRE(base.matching("abcd")->leaf() == true);

    REQUIRE(base.matching("a")->has_children() >= 1);
    REQUIRE(base.matching("ab")->has_children() >= 1);
    REQUIRE(base.matching("abcd")->has_children() == 0);

    base.insert("abce");
    auto lookup = base.matching("abc");
    REQUIRE(lookup != nullptr);
    REQUIRE(lookup->retrieve() == Array<String>{"d", "e"});

    auto leaf = lookup->matching("d");
    REQUIRE(leaf->leaf() == true);

    REQUIRE(base.complete("abc") == Array<String>{"abcd", "abce"});
    REQUIRE(base.retrieve() == Array<String>{"abcd", "abce"});

    REQUIRE(base.remove("abce") == true);
    REQUIRE(base.retrieve() == Array<String>{"abcd"});

    auto newbase = base;
    REQUIRE(newbase.retrieve() == Array<String>{"abcd"});
}
