#ifndef LYTHON_SEMA_BINDINGS_HEADER
#define LYTHON_SEMA_BINDINGS_HEADER

#include "dependencies/coz_wrap.h"
#include "sema/builtin.h"

namespace lython {

struct BindingEntry {
    BindingEntry(StringRef a       = StringRef(),
                 Node*     b       = nullptr,
                 TypeExpr* c       = nullptr,
                 bool      dynamic = false):
        name(a),
        value(b), type(c), dynamic(dynamic) {}

    bool operator==(BindingEntry const& b) const {
        return name == b.name && value == b.value && type == b.type;
    }

    StringRef name;
    Node*     value   = nullptr;
    TypeExpr* type    = nullptr;
    bool      dynamic = false;  // used to specify that this entry is dynamic
                                // and its address can change at runtime
};

std::ostream& print(std::ostream& out, BindingEntry const& entry);

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
    inline int add(StringRef const& name, Node* value, TypeExpr* type) {
        COZ_BEGIN("T::Bindings::add");

        auto size = int(bindings.size());

        bool dynamic = !nested;
        bindings.push_back({name, value, type, dynamic});

        if (!nested) {
            global_index += 1;
        }

        COZ_PROGRESS_NAMED("Bindings::add");
        COZ_END("T::Bindings::add");
        return size;
    }

    inline void set_type(int varid, TypeExpr* type) {
        if (varid < 0 && varid > bindings.size())
            return;

        bindings[varid].type = type;
    }

    inline void set_value(int varid, Node* value) { bindings[varid].value = value; }

    inline TypeExpr* get_type(int varid) const {
        if (varid < 0 && varid > bindings.size())
            return nullptr;
        return bindings[varid].type;
    }

    inline Node* get_value(int varid) const {
        COZ_BEGIN("T::Bindings::get_value");

        Node* result = nullptr;

        if (!(varid < 0 && varid > bindings.size()))
            result = bindings[varid].value;

        COZ_PROGRESS_NAMED("Bindings::get_value");
        COZ_END("T::Bindings::get_value");
        return result;
    }

    StringRef get_name(int varid) const {
        if (varid < 0 && varid > bindings.size())
            return StringRef();
        return bindings[varid].name;
    }

    bool is_dynamic(int varid) const { return varid >= global_index; }

    int get_varid(StringRef name) const {
        COZ_BEGIN("T::Bindings::get_varid");

        auto start = std::rbegin(bindings);
        auto end   = std::rend(bindings);

        int value = -1;
        int i     = 0;
        while (start != end) {
            if (start->name == name) {
                value = int(bindings.size()) - i - 1;
                break;
            }
            ++start;
            i += 1;
        }

        COZ_PROGRESS_NAMED("Bindings::get_varid");
        COZ_END("T::Bindings::get_varid");
        return value;
    }

    String __str__() const {
        StringStream ss;
        dump(ss);
        return ss.str();
    }

    void dump(std::ostream& out) const;

    Array<BindingEntry> bindings;

    // We keep track of when the global binding starts
    // so we know when we need to do a dynamic lookup of a static one
    int  global_index = 0;
    bool nested       = false;
};

struct Scope {
    Scope(Bindings& array): bindings(array), oldsize(bindings.bindings.size()) {
        bindings.nested = true;
    }

    ~Scope() {
        bindings.bindings.resize(oldsize);
        bindings.nested = false;
    }

    Bindings&   bindings;
    std::size_t oldsize;
};

struct ScopedFlag {
    ScopedFlag(Dict<StringRef, bool>& array, StringRef flag): flags(array), flag(flag) {
        flags[flag] = true;
    }

    ~ScopedFlag() { flags.erase(flag); }

    Dict<StringRef, bool>& flags;
    StringRef              flag;
};

}  // namespace lython

#endif