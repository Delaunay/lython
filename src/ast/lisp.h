#pragma once

#include "dtypes.h"

namespace lython {

enum class SexpTag
{
    List,
    Symbol,
    String,
    Integer,
    Float
};

struct Sexp {
    static Sexp list(std::initializer_list<Sexp> elements) {
        Sexp sexp;
        sexp.tag = SexpTag::List;
        new (&sexp.data.list) Array<Sexp>(elements);
        return sexp;
    }
    static Sexp list(Array<Sexp> const& list) {
        Sexp sexp;
        sexp.tag = SexpTag::List;
        new (&sexp.data.list) Array<Sexp>(list);
        return sexp;
    }
    static Sexp symbol(String const& sym) {
        Sexp sexp;
        sexp.tag = SexpTag::Symbol;
        new (&sexp.data.symbol) String(sym);
        return sexp;
    }
    static Sexp string(String const& str) {
        Sexp sexp;
        sexp.tag = SexpTag::String;
        new (&sexp.data.string) String(str);
        return sexp;
    }
    static Sexp integer(int const& number) {
        Sexp sexp;
        sexp.tag          = SexpTag::Integer;
        sexp.data.integer = number;
        return sexp;
    }
    static Sexp decimal(float const& number) {
        Sexp sexp;
        sexp.tag          = SexpTag::Float;
        sexp.data.decimal = number;
        return sexp;
    }

    ~Sexp() {
        switch (tag) {
        case SexpTag::List: data.list ~Array<Sexp>(); return;
        case SexpTag::String: data.string ~String(); return;
        case SexpTag::Symbol: data.symbol ~String(); return;
        }
    }

    SexpTag tag = SexpTag::Integer;
    union Holder {
        Array<Sexp> list;
        String      symbol;
        String      string;
        int         integer;
        double      decimal;
    } data;
};

}  // namespace lython