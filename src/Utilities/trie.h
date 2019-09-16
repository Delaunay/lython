#ifndef LYTHON_TRIE_H
#define LYTHON_TRIE_H

#include <cassert>
#include <array>
#include <cstdio>
#include <memory>


template<size_t size>
class Trie{
public:
    Trie(){
        for(auto& child: children){
            child = nullptr;
        }
    }

    //! Returns if the the value was inserted of not
    bool insert(std::string_view const& name){
        Trie* ptr = this;
        for(auto c: name){
            if (ptr->children[c] == nullptr){
                ptr->children[c] = std::make_unique<Trie>();
            }

            ptr = ptr->children[c].get();
        }

        assert(ptr != nullptr && "ptr cant be null");

        if (ptr->leaf()){
            return false;
        }

        ptr->_leaf = true;
        return true;
    }

    //! return the last Trie that match the given string
    //! for autocompletion for example
    Trie const* matching(std::string_view const& name) const {
        Trie const* ptr = this;

        for(auto c: name){
            ptr = matching(int(c));

            if (ptr == nullptr){
                return nullptr;
            }
        }
        return ptr;
    }

    Trie const* matching(int c) const {
        if (c >= size) {
            return nullptr;
        }
        return this->children[c].get();
    }

    bool has(std::string_view const& name) const {
        Trie const* ptr = matching(name);
        return ptr != nullptr && ptr->leaf;
    }

    int has_children(){
        int count = 0;
        for(auto child: children){
            count += child != nullptr;
        }
        return count;
    }

    bool leaf() const {
        return _leaf;
    }

private:
    std::array<std::unique_ptr<Trie>, size> children;
    bool _leaf = false;
};


#endif
