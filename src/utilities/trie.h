#ifndef LYTHON_TRIE_H
#define LYTHON_TRIE_H

#include <array>
#include <cstdio>
#include <memory>
#include <iostream>

#include "logging/logging.h"
#include "dtypes.h"

namespace lython {




struct _dummy {};

// This use a flat array to store children
// it use more memory than a simple node trie
//
// Depth is usually 2-3 tries since it is only used for operators
//
//
template <size_t size, typename ValueT = _dummy>
class Trie {
    struct TrieNode{
        using Children = std::array<Trie*, size>;

        TrieNode() {
            children.fill(nullptr);
        }

        Children children;
        bool     is_end = false;
        ValueT   value = ValueT();
    };
    Array<TrieNode> nodes;

public:
    Trie() {
        nodes.emplace_back();
    }

    bool remove(String const& name) {
        TrieNode* parent = nullptr;
        TrieNode* current = this;

        Array<std::tuple<TrieNode*, int>> path;

        for (auto c: name) {
            // we do not have the value in the trie
            if (current->children[c] == nullptr) {
                return false;
            }
            
            if (current->children[c]->has_children() <= 1) {
                // queue this char for deletion
                path.push_back(std::make_tuple(current, c));
            }

            else {
                // this path has children we cannot delete it
                path.clear();
            }

            parent = current;
            current = current->children[c].get();
        }

        if (path.size() > 0) {
            for(auto item: path) {
                TrieNode* rm = item.first->children[item.second];
                item.first->children[item.second] = nullptr;
                nodes.remove(rm);
            }
            // we only need to delete the first one
            // destructor will take care of the rest
            Trie* begin = nullptr;
            int c = '\0';
            std::tie(begin, c) = path[0];
            begin->children[c] = nullptr;
            return true;
        }
        return false;
    }

    Array<String> retrieve() const {
        Array<String> results;
        _retrieve(results, "");
        return results;
    }

    void _retrieve(Array<String>& results, String const& prev) const {
        for (int i = 0; i < size; i++) {
            TrieNode const* ptr = children[i];

            if (ptr != nullptr) {
                String newprev = prev + char(i);
                if (ptr->is_end) {
                    results.push_back(newprev);
                }
                ptr->_retrieve(results, newprev);
            }
        }
    }

    Array<String> complete(String const& name) const {
        TrieNode const* ptr = search(nodes[0], name);
        Array<String> suggestions;

        if (ptr != nullptr) {
            ptr->_retrieve(suggestions, name);
        }
        return suggestions;
    }

    //! Returns if the the value was inserted of not
    bool insert(String const& name, ValueT value = ValueT(), bool override = true) {
        TrieNode* ptr = &nodes[0];
        for (auto c: name) {
            if (ptr->children[c] == nullptr) {
                ptr->children[c] = &nodes.emplace_back();
            }
            ptr = ptr->children[c];
        }
        lyassert(ptr != nullptr, "ptr cant be null");
        if (ptr->is_end) {
            if (override) {
                ptr->value = value;
            }
            return false;
        }
        ptr->value = value;
        ptr->is_end = true;
        return true;
    }

    int children_count(TrieNode const* node) const {
        int count = 0;
        for(TrieNode const* child: node->children) {
            count += (child != nullptr);
        }
        return count;
    } 

    TrieNode const* search(TrieNode const* node, String const& path) const {
        for(auto p: path) {
            if (p >= size) {
                return nullptr;
            } 

            node = node->children[p];

            if (node == nullptr) {
                return nullptr;
            }
        }
        return node;
    }

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
