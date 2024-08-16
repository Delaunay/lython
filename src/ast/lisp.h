#pragma once

#include "dtypes.h"
#include "utilities/magic.h"
#include "kmeta.h"

namespace lython {

enum class SexpTag
{
    List,
    Symbol,
    String,
    Integer,
    Float
};


struct KIGNORE() Sexp {
    template<typename ...Args>
    static Sexp list(Args... elements) {
        Sexp sexp;
        sexp.tag = SexpTag::List;
        new (&sexp.data.list) Array<Sexp>{elements...};
        return sexp;
    }

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
    static Sexp string(StringRef const& s) {
        return string(str<StringRef>(s));
    }
    static Sexp symbol(StringRef const& s) {
        return symbol(str<StringRef>(s));
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
        case SexpTag::List:   data.list  .~Array<Sexp>();   return;
        case SexpTag::String: data.string.~String();        return;
        case SexpTag::Symbol: data.symbol.~String();        return;
        }
    }

    struct list_t   {};
    struct decimal_t{};
    struct symbol_t {};
    struct string_t {};
    struct integer_t{};

    SexpTag tag = SexpTag::Integer;
    union Holder {
        constexpr Holder(): integer(0) {}

        // not sure if useful
        Holder(list_t _)   : Holder() {   new (&list)    Array<Sexp>();   }
        Holder(string_t _) : Holder() {   new (&string)  String();        }   
        Holder(symbol_t _) : Holder() {   new (&symbol)  String();        }
        Holder(decimal_t _): Holder() {   new (&decimal) float();         }
        Holder(integer_t _): Holder() {   new (&integer) int();           }

        ~Holder() {}
        
        Array<Sexp> list;
        String      symbol;
        String      string;
        int         integer;
        double      decimal;
    } data;

private:
    Sexp() = default;

    static SexpTag copy(Sexp& dest, Sexp const& src) {
        switch(src.tag) {
            case SexpTag::Float:  dest.data.decimal = src.data.decimal;
            case SexpTag::Integer:dest.data.integer = src.data.integer;
            case SexpTag::List:   dest.data.list    = src.data.list;
            case SexpTag::Symbol: dest.data.symbol  = src.data.symbol;
            case SexpTag::String: dest.data.string  = src.data.string;
        }
        return src.tag;
    }

public:
    Sexp(Sexp const& sexp):
        tag(copy(*this, sexp)) 
    {}

    Sexp& operator=(Sexp const& src) {
        copy(*this, src);
        return *this;
    }

};

}  // namespace lython