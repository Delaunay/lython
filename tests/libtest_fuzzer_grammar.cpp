// #include "ast/nodes.h"


#include <string>
#include <vector>
#include <iostream>

namespace lython {

using Str = std::string;

template<typename T>
using Array = std::vector<T>;



//
// Base Node
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


struct Node {
    virtual ~Node() {}

    virtual void generate(Generator& generator) = 0;
};

struct Branch: public Node {
    template<typename... Args>
    Branch() {
    }
    ~Branch() {
        for(Node* node: children) {
            delete node;
        }
    }

    virtual void generate(Generator& generator) {
        for(Node* node: children) {
            node->generate(generator);
        }
    }

    Array<Node*> children;
};


struct Leaf: public Node {};

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
    virtual void generate(Generator& generator) {
        for(int i = s; i < e; i++) {
            for(Node* node: children) {
                node->generate(generator);
            }
        }
    }

    int s = 0;
    int e = 10;
};

struct Either: public Branch {

};

struct Body: public Branch {};

//
struct Indent: public Branch {
    virtual void generate(Generator& generator) {
        generator.indent += 1;
        for(Node* node: children) {
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


/*
-- ASDL's 4 builtin types are:
-- identifier, int, string, constant
* /
namespace Python
{
    mod = Module(stmt* body, type_ignore* type_ignores)
        | Interactive(stmt* body)
        | Expression(expr body)
        | FunctionType(expr* argtypes, expr returns)

    stmt = FunctionDef(identifier name, arguments args,
                       stmt* body, expr* decorator_list, expr? returns,
                       string? type_comment, type_param* type_params)
          | AsyncFunctionDef(identifier name, arguments args,
                             stmt* body, expr* decorator_list, expr? returns,
                             string? type_comment, type_param* type_params)

          | ClassDef(identifier name,
             expr* bases,
             keyword* keywords,
             stmt* body,
             expr* decorator_list,
             type_param* type_params)
          | Return(expr? value)
;
          | Delete(expr* targets)
          | Assign(expr* targets, expr value, string? type_comment)
          | TypeAlias(expr name, type_param* type_params, expr value)
          | AugAssign(expr target, operator op, expr value)
          -- 'simple' indicates that we annotate simple name without parens
          | AnnAssign(expr target, expr annotation, expr? value, int simple)

          -- use 'orelse' because else is a keyword in target languages
          | For(expr target, expr iter, stmt* body, stmt* orelse, string? type_comment)
          | AsyncFor(expr target, expr iter, stmt* body, stmt* orelse, string? type_comment)
          | While(expr test, stmt* body, stmt* orelse)
          | If(expr test, stmt* body, stmt* orelse)
          | With(withitem* items, stmt* body, string? type_comment)
          | AsyncWith(withitem* items, stmt* body, string? type_comment)

          | Match(expr subject, match_case* cases)

          | Raise(expr? exc, expr? cause)
          | Try(stmt* body, excepthandler* handlers, stmt* orelse, stmt* finalbody)
          | TryStar(stmt* body, excepthandler* handlers, stmt* orelse, stmt* finalbody)
          | Assert(expr test, expr? msg)

          | Import(alias* names)
          | ImportFrom(identifier? module, alias* names, int? level)

          | Global(identifier* names)
          | Nonlocal(identifier* names)
          | Expr(expr value)
          | Pass | Break | Continue

          -- col_offset is the byte offset in the utf8 string the parser uses
          attributes (int lineno, int col_offset, int? end_lineno, int? end_col_offset)

          -- BoolOp() can use left & right?
    expr = BoolOp(boolop op, expr* values)
         | NamedExpr(expr target, expr value)
         | BinOp(expr left, operator op, expr right)
         | UnaryOp(unaryop op, expr operand)
         | Lambda(arguments args, expr body)
         | IfExp(expr test, expr body, expr orelse)
         | Dict(expr* keys, expr* values)
         | Set(expr* elts)
         | ListComp(expr elt, comprehension* generators)
         | SetComp(expr elt, comprehension* generators)
         | DictComp(expr key, expr value, comprehension* generators)
         | GeneratorExp(expr elt, comprehension* generators)
         -- the grammar constrains where yield expressions can occur
         | Await(expr value)
         | Yield(expr? value)
         | YieldFrom(expr value)
         -- need sequences for compare to distinguish between
         -- x < 4 < 3 and (x < 4) < 3
         | Compare(expr left, cmpop* ops, expr* comparators)
         | Call(expr func, expr* args, keyword* keywords)
         | FormattedValue(expr value, int conversion, expr? format_spec)
         | JoinedStr(expr* values)
         | Constant(constant value, string? kind)

         -- the following expression can appear in assignment context
         | Attribute(expr value, identifier attr, expr_context ctx)
         | Subscript(expr value, expr slice, expr_context ctx)
         | Starred(expr value, expr_context ctx)
         | Name(identifier id, expr_context ctx)
         | List(expr* elts, expr_context ctx)
         | Tuple(expr* elts, expr_context ctx)

         -- can appear only in Subscript
         | Slice(expr? lower, expr? upper, expr? step)

          -- col_offset is the byte offset in the utf8 string the parser uses
          attributes (int lineno, int col_offset, int? end_lineno, int? end_col_offset)

    expr_context = Load | Store | Del

    boolop = And | Or

    operator = Add | Sub | Mult | MatMult | Div | Mod | Pow | LShift
                 | RShift | BitOr | BitXor | BitAnd | FloorDiv

    unaryop = Invert | Not | UAdd | USub

    cmpop = Eq | NotEq | Lt | LtE | Gt | GtE | Is | IsNot | In | NotIn

    comprehension = (expr target, expr iter, expr* ifs, int is_async)

    excepthandler = ExceptHandler(expr? type, identifier? name, stmt* body)
                    attributes (int lineno, int col_offset, int? end_lineno, int? end_col_offset)

    arguments = (arg* posonlyargs, arg* args, arg? vararg, arg* kwonlyargs,
                 expr* kw_defaults, arg? kwarg, expr* defaults)

    arg = (identifier arg, expr? annotation, string? type_comment)
           attributes (int lineno, int col_offset, int? end_lineno, int? end_col_offset)

    -- keyword arguments supplied to call (NULL identifier for **kwargs)
    keyword = (identifier? arg, expr value)
               attributes (int lineno, int col_offset, int? end_lineno, int? end_col_offset)

    -- import name with optional 'as' alias.
    alias = (identifier name, identifier? asname)
             attributes (int lineno, int col_offset, int? end_lineno, int? end_col_offset)

    withitem = (expr context_expr, expr? optional_vars)

    match_case = (pattern pattern, expr? guard, stmt* body)

    pattern = MatchValue(expr value)
            | MatchSingleton(constant value)
            | MatchSequence(pattern* patterns)
            | MatchMapping(expr* keys, pattern* patterns, identifier? rest)
            | MatchClass(expr cls, pattern* patterns, identifier* kwd_attrs, pattern* kwd_patterns)

            | MatchStar(identifier? name)
            -- The optional "rest" MatchMapping parameter handles capturing extra mapping keys

            | MatchAs(pattern? pattern, identifier? name)
            | MatchOr(pattern* patterns)

             attributes (int lineno, int col_offset, int end_lineno, int end_col_offset)

    type_ignore = TypeIgnore(int lineno, string tag)

    type_param = TypeVar(identifier name, expr? bound)
               | ParamSpec(identifier name)
               | TypeVarTuple(identifier name)
               attributes (int lineno, int col_offset, int end_lineno, int end_col_offset)
}
*/


}


namespace lython {

struct Builder {
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
        (*stack.rbegin())->children.push_back(b);
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


    Builder& end() {
        stack.pop_back();
        return *this;
    }

    Array<Branch*> stack;
};


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
                    .docstring().newline()
                    .body().end()
                    .keyword("pass").newline()
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