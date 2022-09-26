#ifndef LYTHON_TRIE_H
#define LYTHON_TRIE_H

#include <array>
#include <cstdio>
#include <memory>

#include "logging/logging.h"

namespace lython {

// This use a flat array to store children
// it use more memory than a simple node trie
//
// Depth is usually 2-3 tries since it is only used for operators
//
//
template <size_t size>
class Trie {
    public:
    Trie() {
        for (auto& child: children) {
            child = nullptr;
        }
    }

    Trie(Trie const& trie) {
        for (size_t i = 0; i < size; i++) {
            auto& child = trie.children[i];

            if (child != nullptr) {
                // call copy constructor recursively
                children[i] = ::std::make_unique<Trie>(*child.get());
            }
        }
    }

    Trie operator=(Trie const& trie) {
        for (size_t i = 0; i < size; i++) {
            auto& child = trie.children[i];

            if (child != nullptr) {
                // call copy constructor recursively
                children[i] = std::make_unique<Trie>(*child.get());
            }
        }

        return *this;
    }

    //! Returns if the the value was inserted of not
    bool insert(std::string_view const& name) {
        Trie* ptr = this;
        for (auto c: name) {
            if (ptr->children[c] == nullptr) {
                ptr->children[c] = std::make_unique<Trie>();
            }

            ptr = ptr->children[c].get();
        }

        assert(ptr != nullptr, "ptr cant be null");

        if (ptr->leaf()) {
            return false;
        }

        ptr->_leaf = true;
        return true;
    }

    //! return the last Trie that match the given string
    //! for autocompletion for example
    Trie const* matching(std::string_view const& name) const {
        Trie const* ptr = this;

        for (auto c: name) {
            ptr = ptr->matching(int(c));

            if (ptr == nullptr) {
                return nullptr;
            }
        }
        return ptr;
    }

    Trie const* matching(int c) const {
        if (c >= size) {
            return nullptr;
        }

        return children.at(c).get();
    }

    bool has(std::string_view const& name) const {
        Trie const* ptr = matching(name);
        return ptr != nullptr && ptr->leaf();
    }

    int has_children() const {
        int count = 0;
        for (auto& child: children) {
            count += child != nullptr;
        }
        return count;
    }

    bool leaf() const { return _leaf; }

    private:
    std::array<std::unique_ptr<Trie>, size> children;
    bool                                    _leaf = false;
};

// naive CoWTrie
// this should not happen often as the top level module should be the one
// handling the addition of new operators. Although users can define
// local operators which does require to copy the entrie Trie
template <size_t size>
class CoWTrie {
    public:
    CoWTrie(Trie<size> const* original): original(original) {}

    // if no original is provided just use the copy
    CoWTrie(): was_copied(true), original(nullptr) {}

    bool insert(std::string_view const& name) { return trie().insert(name); }

    bool leaf() const { return trie().leaf(); }

    int has_children() const { return trie().has_children(); }

    bool has(std::string_view const& name) const { return trie().has(name); }

    Trie<size> const* matching(std::string_view const& name) const { return trie().matching(name); }

    Trie<size> const* matching(int c) const { return trie().matching(c); }

    Trie<size> const& trie() const { return was_copied ? copy : *original; }

    private:
    Trie<size>& trie() {
        if (!was_copied) {
            copy = Trie<size>(*original);
        }
        return copy;
    }

    bool              was_copied = false;
    Trie<size> const* original;
    Trie<size>        copy;
};
}  // namespace lython

#endif
