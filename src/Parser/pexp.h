#pragma once

#include "../Lexer/Tokens.h"


namespace lython{

    enum ArgumentKind{
        Arg_Explicit,
        Arg_Implicit,
        Arg_Erasable
    };

    class ImplPexp{
    public:
        virtual ~ImplPexp() {}
    };

    typedef std::shared_ptr<ImplPexp> Pexp;

    // For Block/String/Integer/Float
    class Pimm: public ImplPexp{
    public:
        Pimm(Sexp val): _val(val) {}
        Sexp val() {    return _val;    }
        Sexp _val;
    };

    class Pvar: public ImplPexp{
    public:
        Pvar(Sexp symbol): _sym(symbol) {}
        Sexp symbol() { return _sym;    }
        Sexp _sym;
    };

    class Plet: public ImplPexp{
    public:
        struct Declaration{
            Declaration(Sexp var, Pexp val, bool type):
                var(var), val(val), type(type)
            {}

            Sexp var;  // varname
            Pexp val;  // type or expr
            bool type; // type or expr ?
        };

        typedef std::vector<Declaration> Params;

        Params& params() {  return _params; }
        Pexp body() {   return _body;  }

        Params _params;
        Pexp _body;
    };

    class Parrow: public ImplPexp{
        Parrow(ArgumentKind kind, Sexp sym, Pexp first, Pexp sec):
            _kind(kind), _sym(true), _symbol(sym), _first(first), _second(sec)
        {}

        Parrow(ArgumentKind kind, Pexp first, Pexp sec):
            _kind(kind), _sym(false), _first(first), _second(sec)
        {}

        ArgumentKind kind() {   return _kind;   }
        bool    has_symbol(){   return _sym;    }
        Sexp    symbol() {  return _symbol; }
        Pexp    first()  {  return _first;  }
        Pexp    second() {  return _second; }

        ArgumentKind _kind;
        bool         _sym;
        Sexp         _symbol;
        Pexp         _first;
        Pexp         _second;
    };

    class Plambda: public ImplPexp{
    public:
        Plambda(ArgumentKind kind, Sexp sym, Pexp first, Pexp sec):
            _kind(kind), _hasargs(true), _symbol(sym), _args(first), _body(sec)
        {}

        Plambda(ArgumentKind kind, Sexp sym, Pexp sec):
            _kind(kind), _hasargs(false), _symbol(sym), _body(sec)
        {}

        ArgumentKind kind() {  return _kind;    }
        Sexp    symbol()    {  return _symbol;  }
        bool    has_args()  {  return _hasargs; }
        Pexp    args()      {  return _args;    }
        Pexp    body()      {  return _body;    }

        ArgumentKind _kind;
        bool         _hasargs;
        Sexp         _symbol;
        Pexp         _args;
        Pexp         _body;
    };

    class PCall: public ImplPexp{
    public:
        typedef std::vector<Sexp> Arguments;

        PCall(Pexp call, Arguments args):
            _call(call), _args(args)
        {}

        Pexp call() { return _call; }
        Arguments& args() { return _args;   }

        Pexp _call;
        Arguments _args;
    };

    class PInductive: public ImplPexp{
    public:
        struct Constructor{
            Sexp _name;
        };

        typedef std::vector<Constructor> Constructors;

        PInductive()
        {}

        Sexp _label;

    };

    class PCons: public ImplPexp{
    public:

        Sexp _name;
        Sexp _sym;
    };
}
