#include <iostream>
#include <vector>
#include <functional>
#include <random>
#include <string>
#include <cassert>

namespace lython {

struct Branch;

//
Branch* mod();

// Match
Branch* pattern();
Branch* match_value() ;
Branch* match_singleton();
Branch* match_sequence();
Branch* match_mapping();
Branch* match_class();
Branch* match_star() ;
Branch* match_as() ;
Branch* match_or() ;

// Statement
Branch* statement();
Branch* functiondef();
Branch* classdef();
Branch* return_();
Branch* del();
Branch* assign();
Branch* typealias();
Branch* aug_assign();
Branch* ann_assign();
Branch* for_();
Branch* while_();
Branch* if_();
Branch* with();
Branch* match();
Branch* raise();
Branch* try_();
Branch* try_star();
Branch* assert_();
Branch* import();
Branch* import_from();
Branch* global();
Branch* nonlocal();
Branch* expression();
Branch* pass();
Branch* break_();
Branch* continue_();

// Expressions
Branch* expr();
Branch* bool_();
Branch* namedexpr();
Branch* binary();
Branch* unary();
Branch* compare();
Branch* lambda();
Branch* ifexp();
Branch* await();
Branch* yield();
Branch* yield_from();
Branch* call();
Branch* formatted_value();
Branch* joined_str();
Branch* constant();
Branch* attribute();
Branch* subscript();
Branch* starred();
Branch* name();
Branch* list();
Branch* slice();
Branch* tuple();
Branch* dict();
Branch* set();
Branch* listcomp();
Branch* setcomp();
Branch* dictcomp();
Branch* generatorexp();


// Helpers
Branch* body();
Branch* compop();
Branch* unaryop();
Branch* binop();
Branch* boolop();
Branch* callargs();
Branch* number();

//
//
//
using Str = std::string;

template<typename T>
using Array = std::vector<T>;


//
// Base GNode
//
struct Generator {
    Generator():
        gen(std::random_device()())
    {}

    virtual void write(std::string const& str) {
        if (newline) {
            std::cout << Str(4 * indent, ' ');
            newline = false;
        }
        std::cout << str;
    }

    bool newline = false;
    int indent = 0;

    // generate a random number
    template<typename T>
    int next(T& distribution) {
        if (replay) {
            int value = (*replay_path.rbegin());
            replay_path.pop_back();
            return value;
        }
        int value = distribution(prng());
        path.push_back(value);
        return value;
    }

    void set_replay(std::vector<int> const& replay_vector) {
        replay_path = replay_vector;
        replay = true;
    }

    static int& fetch_depth() {
        static int depth = 0;
        return depth;
    }

    bool done() {
        return fetch_depth() > 20;
    }


private:
    std::mt19937& prng() {  return gen; }

    bool replay = false;
    std::vector<int> replay_path;
    std::vector<int> path;
    std::mt19937 gen;
};

struct GNode {
    virtual ~GNode() {}

    virtual void generate(Generator& generator, int depth) = 0;
};

struct Branch: public GNode {
    Branch()
    {
    }

    ~Branch() {
        for(GNode* node: children) {
            delete node;
        }
    }

    virtual void generate(Generator& generator, int depth) {
        for(GNode* node: children) {
            node->generate(generator, depth + 1);
        }
    }

    Array<GNode*> children;
};


struct Leaf: public GNode {};

//
// Leaf
//

struct Identifier: public Leaf {
    virtual void generate(Generator& generator, int depth) {
        generator.write("identifier");
    }
};

struct Newline: public Leaf {
    virtual void generate(Generator& generator, int depth) {
        generator.write("\n");
        generator.newline = true;
        // generator.write(Str(' ', generator.indent * 4));
    }
};

struct Keyword: public Leaf {
    Keyword(Str const& keyword):
        keyword(keyword)
    {}
    Str keyword;

    virtual void generate(Generator& generator, int depth) {
        generator.write(keyword);
        generator.write(" ");
    }
};

struct String: public Leaf {
    virtual void generate(Generator& generator, int depth) {
        generator.write("\"");
        generator.write("random string");
        generator.write("\"");
    }
};

//
// Branch
//


struct Optional: public Branch {
    virtual void generate(Generator& generator, int depth) {
        std::uniform_int_distribution<> distrib(0, 1);
        int idx = generator.next(distrib);

        if (idx > 0 && !generator.done()) {
            Branch::generate(generator, depth + 1);
        }
    }
};

struct Type: public Identifier {};

struct Multiple: public Branch {
    Multiple(int e = 2):
        s(0), e(e)
    {}

    Multiple(int s, int e):
        s(s), e(e)
    {}

    virtual void generate(Generator& generator, int depth) {
        std::uniform_int_distribution<> distrib(s, e);
        int idx = generator.next(distrib);

        for(int i = s; i < idx; i++) {
            for(GNode* node: children) {
                node->generate(generator, depth + 1);
            }

            if (generator.done()) {
                return;
            }
        }
    }

    int s;
    int e;
};

struct Either: public Branch {
    Either() { }

    virtual void generate(Generator& generator, int depth) {
        std::uniform_int_distribution<> distrib(0, int(children.size()) - 1);
        int idx = generator.next(distrib);

        if (children.size() > 0) {
            children[idx]->generate(generator, depth + 1);
        }
    } 
};

struct Group: public Branch {
    Group(Str const& group):
        group(group)
    {}

    Str group;
};

//
struct Indent: public Branch {
    virtual void generate(Generator& generator, int depth) {
        generator.indent += 1;
        for(GNode* node: children) {
            node->generate(generator, depth + 1);
        }
        generator.indent -= 1;
    }
};

struct Atom: public Leaf {
    Atom(char ch)
    {
        c += ch;
    }

    Atom(Str c):
        c(c)
    {}

    virtual void generate(Generator& generator, int depth) {
        generator.write(c);
    }

    Str c;
};


struct Join: public Branch {
    Join(Str const& sep, int e=2):
        sep(sep), s(0), e(e)
    {}
    Str sep;
    int s;
    int e;

    virtual void generate(Generator& generator, int depth) {
        std::uniform_int_distribution<> distrib(s, e);
        int idx = generator.next(distrib);

        int k = 0;
        for (int i = s; i < e; i++) {
            for(GNode* node: children) {
                if (k != 0) {
                    generator.write(sep);
                }
                node->generate(generator, depth + 1);
                k += 1;
            }
        }
    }
};

struct Docstring: public Leaf {
    Docstring() {}
    Docstring(Str const& str):
        docstring(str)
    {}

    virtual void generate(Generator& generator, int depth) {
        generator.write("\"\"\"");
        generator.write(docstring);
        generator.write("\"\"\"");
        generator.write("\n");
        generator.newline = true;
    }

    Str docstring;
};


//
//
//
struct Pair {
    Pair(Str const& name, Branch* tree):
        name(name), tree(tree)
    {}

    Str name;
    Branch* tree;
};

struct Forest {
    static Forest& forest() {
        static Forest f;
        return f;
    }

    template<typename T, typename... Args>
    Branch* get(Str const& name, std::function<void(struct Builder&)> lazy, Args... args);

    Array<Pair> trees;
};

struct Builder {
    static Branch* make(Str const& name, std::function<void(Builder&)> lazy) {
        Forest& forest = Forest::forest();
        Branch* b = forest.get<Group>(name, lazy, name);
        return b;
    }

    Builder(Branch* parent) {
        stack.push_back(parent);
    }

    ~Builder() {
    }
    
    template<typename T, typename...Args>
    Builder& leaf(Args... args) {
        Leaf* l = new T(args...);

        Branch* node = *stack.rbegin();
        node->children.push_back(l);
    
        return *this;
    }

    template<typename T, typename...Args>
    Builder& branch(Args... args) {
        Branch* b = new T(args...);

        assert(stack.size() > 0);
        Branch* node = *stack.rbegin();
        node->children.push_back(b);

        stack.push_back(b);
        return *this;
    }

    #define SHORTCUT(name, type, base)      \
        template<typename... Args>\
        Builder& name(Args... args) {       \
            return base<type>(args...);     \
        }

    SHORTCUT(identifier, Identifier, leaf);
    SHORTCUT(option, Optional, branch);
    SHORTCUT(atom, Atom, leaf);
    SHORTCUT(keyword, Keyword, leaf);
    SHORTCUT(type, Type, leaf);
    SHORTCUT(multiple, Multiple, branch);
    SHORTCUT(either, Either, branch);
    SHORTCUT(docstring, Docstring, leaf);
    SHORTCUT(indent, Indent, branch);
    SHORTCUT(newline, Newline, leaf);
    SHORTCUT(group, Group, branch);
    SHORTCUT(string, String, leaf);
    SHORTCUT(join, Join, branch);

    Builder& one_or_more(int limit) {   return multiple(0, limit + 1); }
    Builder& none_or_more(int limit) {   return multiple(0, limit); }


#define HELPER(name)                                            \
    Builder& name() {                                           \
        expect(lython::name());                                 \
        return *this;                                           \
    }

    HELPER(expr)
    HELPER(body)
    HELPER(statement)
    HELPER(pattern)
    HELPER(slice)
    HELPER(formatted_value)


    // add an expected node that should be generated
    Builder& expect(GNode* b) {
        (*stack.rbegin())->children.push_back(b);
        return *this;
    }

    Branch* finish() {
        return stack[0];
    }

    Builder& end() {
        stack.pop_back();
        return *this;
    }

    Array<Branch*> stack;
};



template<typename T, typename... Args>
Branch* Forest::get(Str const& name, std::function<void(struct Builder&)> lazy, Args... args) {
    for(Pair& p: trees) {
        if (p.name == name) {
            return p.tree;
        }
    }

    T* newtree = new T(args...);
    trees.emplace_back(name, newtree);
    Builder builder(newtree);
    lazy(builder);
    return newtree;
}

}