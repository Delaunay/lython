#ifndef LYTHON_SEMA_BINDINGS_HEADER
#define LYTHON_SEMA_BINDINGS_HEADER

#include "sema/builtin.h"

namespace lython {

struct BindingEntry {
    BindingEntry(StringRef a = StringRef(), Node *b = nullptr, TypeExpr *c = nullptr):
        name(a), value(b), type(c) {}

    bool operator==(BindingEntry const &b) const {
        return name == b.name && value == b.value && type == b.type;
    }

    StringRef name;
    Node *    value = nullptr;
    TypeExpr *type  = nullptr;
};

std::ostream &print(std::ostream &out, BindingEntry const &entry);

struct Bindings {
    Bindings() {
        bindings.reserve(128);

#define TYPE(name) add(String(#name), name##_t(), Type_t());

        BUILTIN_TYPES(TYPE)

#undef TYPE

        // Builtin constant
        add(String("None"), None(), None_t());
        add(String("True"), True(), bool_t());
        add(String("False"), False(), bool_t());
    }

    // returns the varid it was inserted as
    inline int add(StringRef const &name, Node *value, TypeExpr *type) {
        auto size = int(bindings.size());
        bindings.push_back({name, value, type});
        return size;
    }

    inline void set_type(int varid, TypeExpr *type) {
        if (varid < 0 && varid > bindings.size())
            return;

        bindings[varid].type = type;
    }

    inline void set_value(int varid, Node* value) {
        bindings[varid].value = value;
    }

    inline TypeExpr *get_type(int varid) const {
        if (varid < 0 && varid > bindings.size())
            return nullptr;
        return bindings[varid].type;
    }

    inline Node *get_value(int varid) const {
        if (varid < 0 && varid > bindings.size())
            return nullptr;
        return bindings[varid].value;
    }

    StringRef get_name(int varid) const {
        if (varid < 0 && varid > bindings.size())
            return StringRef();
        return bindings[varid].name;
    }

    int get_varid(StringRef name) const {
        auto start = std::rbegin(bindings);
        auto end   = std::rend(bindings);

        int i = 0;
        while (start != end) {
            if (start->name == name) {
                return int(bindings.size()) - i - 1;
            }
            ++start;
            i += 1;
        }
        return -1;
    }

    String __str__() const {
        StringStream ss;
        dump(ss);
        return ss.str();
    }

    void dump(std::ostream &out) const;

    Array<BindingEntry> bindings;
};

template <typename T, typename U>
struct PopGuard {
    PopGuard(T &array, U const &v): array(array), oldsize(array.size()) { array.push_back(v); }

    ~PopGuard() { array.pop_back(); }

    U const &last(int offset, U const &default_value) const {
        if (oldsize >= offset) {
            return array[oldsize - offset];
        }
        return default_value;
    }

    T &         array;
    std::size_t oldsize;
};

struct Scope {
    Scope(Bindings &array): bindings(array), oldsize(bindings.bindings.size()) {}

    ~Scope() { bindings.bindings.resize(oldsize); }

    Bindings &  bindings;
    std::size_t oldsize;
};

struct ScopedFlag {
    ScopedFlag(Dict<StringRef, bool> &array, StringRef flag): flags(array), flag(flag) {
        flags[flag] = true;
    }

    ~ScopedFlag() { flags.erase(flag); }

    Dict<StringRef, bool> &flags;
    StringRef              flag;
};

} // namespace lython

#endif