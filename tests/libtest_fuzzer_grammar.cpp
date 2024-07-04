// #include "ast/nodes.h"


#include <string>
#include <vector>
#include <iostream>
#include <random>
#include <cassert>
#include <functional>
 
namespace lython {

using Str = std::string;

template<typename T>
using Array = std::vector<T>;



//
// Base GNode
//
struct Generator {
    virtual void write(std::string const& str) {
        if (newline) {
            std::cout << Str(4 * indent, ' ');
            newline = false;
        }
        std::cout << str;
    }

    bool newline = false;
    int indent = 0;
};


struct Context {
    
};


struct GNode {
    virtual ~GNode() {}

    virtual void generate(Generator& generator) = 0;
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

    virtual void generate(Generator& generator) {
        for(GNode* node: children) {
            node->generate(generator);
        }
    }

    Array<GNode*> children;
};


struct Leaf: public GNode {};

//
// Leaf
//

struct Identifier: public Leaf {
    virtual void generate(Generator& generator) {
        generator.write("identifier");
    }
};

struct Newline: public Leaf {
    virtual void generate(Generator& generator) {
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

    virtual void generate(Generator& generator) {
        generator.write(keyword);
        generator.write(" ");
    }
};

//
// Branch
//

struct Arguments: public Branch {};

struct Optional: public Branch {
    virtual void generate(Generator& generator) {
        std::random_device rd;
        std::mt19937 gen(rd()); 
        std::uniform_int_distribution<> distrib(0, 1);

        int idx = distrib(gen);

    
        if (idx > 0) {
            Branch::generate(generator);
        }
    }
};

struct Type: public Identifier {};

struct Multiple: public Branch {
    Multiple(int e = 2):
        s(0), e(e)
    {}

    virtual void generate(Generator& generator) {
        std::random_device rd;
        std::mt19937 gen(rd()); 
        std::uniform_int_distribution<> distrib(s, e);
        int idx = distrib(gen);
        
        for(int i = s; i < idx; i++) {
            for(GNode* node: children) {
                node->generate(generator);
            }
        }
    }

    int s;
    int e;
};

struct Either: public Branch {
    Either() { }

    virtual void generate(Generator& generator) {
        std::random_device rd;
        std::mt19937 gen(rd()); 
        std::uniform_int_distribution<> distrib(0, int(children.size()) - 1);

        int idx = distrib(gen);
        
        // std::cout << children.size() << std::endl;

        if (children.size() > 0) {
            children[idx]->generate(generator);
        }
    }

 
};

struct Body: public Branch {
    Body();
};

struct Group: public Branch {
    Group(Str const& group):
        group(group)
    {}

    Str group;
};

//
struct Indent: public Branch {
    virtual void generate(Generator& generator) {
        generator.indent += 1;
        for(GNode* node: children) {
            node->generate(generator);
        }
        generator.indent -= 1;
    }
};

struct Atom: public Leaf {
    Atom(char c):
        c(c)
    {}

    virtual void generate(Generator& generator) {
        Str ss;
        ss += c;
        generator.write(ss);
    }

    char c;
};

struct Expr: public Branch {
    Expr();
};

struct Stmt: public Branch {
    Stmt();
};

struct Mod: public Branch {
    Mod();
};

struct Pattern: public Branch {
    Pattern();
};

struct Call: public Branch {
    virtual void generate(Generator& generator) {
        generator.write("call(x, y, z)");
    }
};

struct Args: public Branch {
    virtual void generate(Generator& generator) {
        generator.write("x, y, z");
    }
};

struct Docstring: public Leaf {
    Docstring() {}
    Docstring(Str const& str):
        docstring(str)
    {}

    virtual void generate(Generator& generator) {
        generator.write("\"\"\"");
        generator.write(docstring);
        generator.write("\"\"\"");
        generator.write("\n");
        generator.newline = true;
    }

    Str docstring;
};

}


namespace lython {

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


//
//
//
Branch* functiondef();
Branch* boolop();
Branch* binop();
Branch* functiondef();
Branch* unaryop();
Branch* comparison();
Branch* expr();
Branch* body();
Branch* pattern();
Branch* statement();
Branch* mod();

struct Builder {
    static Branch* make(Str const& name, std::function<void(Builder&)> lazy) {
        Forest& forest = Forest::forest();
        Branch* b = forest.get<Group>(name, lazy, name);
        Builder self(b);
        return b;
    }

    Builder(Branch* parent) {
        stack.push_back(parent);
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
    SHORTCUT(args, Arguments, branch);
    SHORTCUT(call, Call, branch);
    SHORTCUT(keyword, Keyword, leaf);
    SHORTCUT(type, Type, leaf);
    SHORTCUT(multiple, Multiple, branch);
    SHORTCUT(either, Either, branch);
    SHORTCUT(docstring, Docstring, leaf);
    SHORTCUT(indent, Indent, branch);
    SHORTCUT(newline, Newline, leaf);
    SHORTCUT(mod, Mod, branch);
    SHORTCUT(pattern, Pattern, branch);
    SHORTCUT(group, Group, branch);

#define HELPER(name)                                            \
    Builder& name() {                                           \
        expect(lython::name());                                 \
        return *this;                                           \
    }

    HELPER(expr)
    HELPER(body)
    HELPER(statement)
    HELPER(pattern)


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

//
//
//

Branch* boolop() { 
    return Builder::make("boolop", [](Builder& self){
        self.either()
            .keyword("and")
            .keyword("or")
        .end();
    });
}
Branch* binop() {
    return  Builder::make("operator", [](Builder& self){
        self.either()
            .keyword("+")
            .keyword("-")
            .keyword("*")
            .keyword("@")
            .keyword("/")
            .keyword("%")
            .keyword("**")
            .keyword("<<")
            .keyword(">>")
            .keyword("|")
            .keyword("^")
            .keyword("&")
            .keyword("//")
        .end();
    });
}

Branch* unaryop() {
    return Builder::make("unaryop", [](Builder& self){
        self
        .either()
            .keyword("~")
            .keyword("!")
            .keyword("+")
            .keyword("-")
        .end();
    });
}


Branch* comparison() { 
    return Builder::make("comparison", [](Builder& self){
        self
        .either()
            .keyword("==")
            .keyword("!=")
            .keyword("<")
            .keyword("<=")
            .keyword(">")
            .keyword(">=")
            .keyword("is")
            .keyword("is not")
            .keyword("in")
            .keyword("not in")
        .end();
    });
}

Branch* expr() {
    //
    //  Python Expression
    //
    return Builder::make("expression", [](Builder& self){
        self.either()
        // <expr> <boolop> <expr>
        .group("boolop").end()
        // <name> := <expr>
        .group("namedexpr").end()
        .group("binop").expr().expect(binop()).expr().end()
        // <unary> <expr>
        // + (2 + 2)
        // - a
        // ~ a
        // ! a
        .group("unaryop").expect(unaryop()).expr().end()
        // <expr> (<comp> epxr)+
        .group("compare").end()
        // lamba <args>: <expr>
        .group("lambda").end()
        // <expr> if <cond> else <expr>
        .group("ifexp").expr().keyword("if").expr().keyword("else").expr().end()
        // {<key>: <value> for <val> in <iter> (if <cond>)*}
        .group("dict").end()
        .group("set").end()
        .group("listcomp").end()
        .group("setcomp").end()
        .group("dictcomp").end()
        .group("generatorexp").end()
        .group("await").end()
        .group("yield").end()
        .group("yield_from").end()
        .group("call").end()
        .group("formatte_value").end()
        .group("joined_str").end()
        .group("constant").end()
        .group("attribute").end()
        .group("subscript").end()
        .group("starred").end()
        .group("name").end()
        .group("list").end()
        .group("tuple").end()
        .group("slice").end()
    .end();
    });
}

Branch* body() {
    return Builder::make("body", [](Builder& self){
        self.multiple()
            .statement()
        .end();
    });
}

Branch* pattern() {
    return Builder::make("pattern", [](Builder& self){
        self.either()
            .group("match_value").end()
            .group("match_singleton").end()
            .group("match_sequence").end()
            .group("match_mapping").end()
            .group("match_class").end()
            .group("match_star").end()
            .group("match_as").end()
            .group("match_or").end()
        .end();
    });
}


Branch* statement() {
    //
    // Pyton Statement
    //
    return Builder::make("statement", [](Builder& self){
        self.either()
            // funcrion & async
            .group("functiondef").expect(functiondef()).end()
            .group("classdef").end()
            // return <expr>?
            .group("return")
                .keyword("return")
                    .option().expr().end()
            .end()
            // del <expr>?
            .group("delete")
                .keyword("del")
                    .option().expr().end()
                .end()
            // <name> = <expr>
            .group("assign").end()
            // ignore this
            .group("typealias").end()
            // <name> <op>= <expr>
            .group("aug_assign").end()
            // <name>: <type> = <expr>
            .group("ann_assign").end()
            .group("for").end()
            .group("while").end()
            .group("if").end()
            .group("with").end()
            .group("match").end()
            .group("raise").end()
            .group("try").end()
            .group("try_star").end()
            .group("assert").end()
            .group("import").end()
            .group("import_from").end()
            .group("global").end()
            .group("nonlocal").end()
            // any expression
            .group("expr").expect(expr()).end()
            .group("pass").keyword("pass").end()
            .group("break").keyword("break").end()
            .group("continue").keyword("continue").end()
        .end()
        .newline();
    });
}

Branch* mod() {
    //
    //  Python Module
    //
    return Builder::make("mod", [](Builder& self){
        self.either()
            // Module
            .group("module")
                .multiple().body().end()
                .end()
            // Interactive
            .group("interactive")
                .end()
            // Expression
            .group("expression")
                .expr()
                .end()
            // FunctionType
            .group("function_type")
                .multiple().expr().end().keyword(" -> ").expr()
                .end()
        .end();
    });
}


Branch* functiondef() {
    return Builder::make("functiondef", [](Builder& self){
        self.option()
            .multiple()
                .atom('@').call().end().newline()
            .end()
        .end()
        .option().keyword("async").end()
        .keyword("def").identifier().atom('(').args().atom(')')
            .option()
                .keyword(" -> ").type()
            .end()
            .atom(':')
            .newline()
            .indent()
                .either()
                    .docstring()
                    .body().end()
                    .keyword("pass")
                .end()
            .end();
    });
}

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

int main() 
{
    lython::Branch* def = lython::functiondef();
    
    for (int i = 0; i < 10; i++) {
        lython::Generator gen;
        def->generate(gen);
        gen.write("\n\n");
    }

    return 0;
}