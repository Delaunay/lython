#include "sema/bindings.h"
#include "ast/ops.h"
#include "utilities/printing.h"
#include "utilities/strings.h"

namespace lython {

Bindings::Bindings() {
    bindings.reserve(128);

#define TYPE(name, native) add(String(#name), name##_t(), Type_t(), meta::type_id<native>());

    BUILTIN_TYPES(TYPE)

#undef TYPE

    // Builtin constant
    add(String("None"), None(), None_t());
    add(String("True"), True(), bool_t());
    add(String("False"), False(), bool_t());
}

std::ostream& print(std::ostream& out, int i, BindingEntry const& entry);

void Bindings::dump(std::ostream& out) const {
    auto big   = String(40, '-');
    auto small = String(20, '-');
    auto sep   = fmt::format("{:>40}-+-{:>20}-+-{}", big, small, small);

    out << sep << '\n';
    out << fmt::format("    {:40} | {:20} | {}", "name", "type", "value") << "\n";
    out << sep << '\n';
    int i = 0;
    for (auto& e: bindings) {
        print(out, i, e);
        i += 1;
    }
    out << sep << '\n';
}

inline std::ostream& print(std::ostream& out, int i, BindingEntry const& entry) {
    String n = str(entry.name);
    String v = str(entry.value);
    String t = str(entry.type);

    auto frags = split('\n', v);

    out << fmt::format("{:3d} {:>40} | {:>20} | {}", i, n, t, frags[0]) << '\n';

    for (int i = 1; i < frags.size(); i++) {
        if (strip(frags[i]) == "") {
            continue;
        }
        out << fmt::format("    {:>40} | {:>20} | {}", "", "", frags[i]) << '\n';
    }
    return out;
}

// returns the varid it was inserted as
int Bindings::add(StringRef const& name, Node* value, TypeExpr* type, int type_id) {
    COZ_BEGIN("T::Bindings::add");

    // It is possile the name is missing during edit
    // lyassert(name != StringRef(), "Should have a name");
    auto size = int(bindings.size());

    bool dynamic = !nested;
    bindings.push_back({name, value, type, type_id, size});

    if (!nested) {
        global_index += 1;
    }

    COZ_PROGRESS_NAMED("Bindings::add");
    COZ_END("T::Bindings::add");
    return size;
}

struct Name* Bindings::make_reference(Node* parent, StringRef const& name, ExprNode* type) {
    Name* ref = parent->new_object<Name>();
    ref->id   = name;
    ref->ctx  = ExprContext::Load;
    ref->type = type;
    return ref;
}

}  // namespace lython