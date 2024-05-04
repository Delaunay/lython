#ifndef LYTHON_SEMA_BINDINGS_HEADER
#define LYTHON_SEMA_BINDINGS_HEADER

#include "dependencies/coz_wrap.h"
#include "sema/builtin.h"

namespace lython {

struct BindingEntry {
    BindingEntry(StringRef a       = StringRef(),
                 Node*     b       = nullptr,
                 TypeExpr* c       = nullptr,
                 int       type_id = -1,
                 int       store_id = -1):
        name(a),
        value(b), type(c), type_id(type_id), store_id(store_id)
    {}

    bool operator==(BindingEntry const& b) const {
        return name == b.name && value == b.value && type == b.type;
    }

    StringRef name;
    Node*     value   = nullptr;
    TypeExpr* type    = nullptr;
    int       type_id = -1;
    int       store_id = 0;
    int       load_id  = 0;
};

std::ostream& print(std::ostream& out, BindingEntry const& entry);

struct Bindings {
    Bindings();

    struct Name* make_reference(Node* parent, StringRef const& name, ExprNode* type = nullptr);

    // returns the varid it was inserted as
    int add(StringRef const& name, Node* value, TypeExpr* type, int type_id=-1);

    BindingEntry* find(StringRef const& name) {
        lyassert(bindings.size() > 0 , "");
        int last = int(bindings.size()) - 1;

        for(int i = last; i >= 0; i--){
            BindingEntry& entry = bindings[i];
            if (name == entry.name) {
                return &entry;
            }
        }

        return nullptr;
    } 

#define GETTER(type, attr, default)             \
    type attr(StringRef const& name) {          \
        if (BindingEntry* entry = find(name)) { \
            return entry->attr;                 \
        }                                       \
        return default;                         \
    }

#define SETTER(type, attr)                              \
    void set_##attr(StringRef const& name, type value) {  \
        if (BindingEntry* entry = find(name)) {         \
            entry->attr = value;                        \
        }                                               \
    }

    GETTER(Node*, value, nullptr)
    GETTER(TypeExpr*, type, nullptr)

    SETTER(Node*, value)
    SETTER(TypeExpr*, type)

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