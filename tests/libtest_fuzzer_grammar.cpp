// #include "ast/nodes.h"


#include <string>
#include <vector>
#include <iostream>
#include <random>
 
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


struct GNode {
    virtual ~GNode() {}

    virtual void generate(Generator& generator) = 0;
};

struct Branch: public GNode {
    Branch() {}

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

struct FunctionDef: public Branch  {
    FunctionDef();
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
    }

    Str docstring;
};

}


namespace lython {

struct Builder {
    static Builder make(Str const& name) {
        Builder self(new Group(name));
        return self;
    }

    Builder(Branch* parent) {
        stack.push_back(parent);
    }
    
    template<typename T, typename...Args>
    Builder& leaf(Args... args) {
        Leaf* l = new T(args...);
        (*stack.rbegin())->children.push_back(l);
        return *this;
    }

    template<typename T, typename...Args>
    Builder& branch(Args... args) {
        Branch* b = new T(args...);

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
    SHORTCUT(body, Body, branch);
    SHORTCUT(newline, Newline, leaf);
    SHORTCUT(group, Group, branch);
    SHORTCUT(expr, Expr, branch);
    SHORTCUT(mod, Mod, branch);
    SHORTCUT(stmt, Stmt, branch);
    SHORTCUT(function, FunctionDef, branch);
    SHORTCUT(pattern, Pattern, branch);


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


Branch* boolop() { 
    return Builder::make("boolop")
        .either()
            .keyword("and")
            .keyword("or")
        .end()
    .finish();
}
Branch* binop() {
    return  Builder::make("operator")
        .either()
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
        .end()
    .finish();
}

Branch* unaryop() {
    return Builder::make("unaryop")
        .either()
            .keyword("~")
            .keyword("!")
            .keyword("+")
            .keyword("-")
        .end()
    .finish();
}


Branch* comparison() { 
    return Builder::make("comparison")
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
        .end()
    .finish();
}

Expr::Expr() {
    //
    //  Python Expression
    //
    Builder(this).either()
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
}

Body::Body() {
    Builder(this)
        .multiple().stmt()
    .end();
}

Pattern::Pattern() {
    Builder(this).either()
        .group("match_value").end()
        .group("match_singleton").end()
        .group("match_sequence").end()
        .group("match_mapping").end()
        .group("match_class").end()
        .group("match_star").end()
        .group("match_as").end()
        .group("match_or").end()
    .end();
}


Stmt::Stmt() {
    //
    // Pyton Statement
    //
    Builder(this).either()
        // funcrion & async
        .group("functiondef").function().end()
        .group("classdef").end()
        // return <expr>?
        .group("return")
            .keyword("return")
                .option().expr().end()
            .end()
        .end()
        // del <expr>?
        .group("delete")
            .keyword("del")
                .option().expr().end()
            .end()
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
        .group("expr").expr().end()
        .group("pass").keyword("pass").end()
        .group("break").keyword("break").end()
        .group("continue").keyword("continue").end()
    .end();
}

Mod::Mod() {
    //
    //  Python Module
    //
    Builder(this).either()
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
}

FunctionDef::FunctionDef() {
    Builder(this)
        .option()
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

    }

}


int main() {
    lython::FunctionDef def;

    lython::Generator gen;

    def.generate(gen);


    gen.write("\n\n");

    return 0;
}