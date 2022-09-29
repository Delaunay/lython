#include "object.h"
#include "logging/logging.h"

namespace lython {

void GCObject::remove_child(GCObject* child, bool dofree) {
    auto elem = std::find(children.rbegin(), children.rend(), child);
    // This is why people hate C++
    //
    // This removes the element found by the reverse iterator.
    // Reverse iterator do not point to the found element
    // but the element before it.
    // erase is not specialized to take reverse iterator
    // so we need to convert ifreet ourselves and KNOW that this is what is
    // expected but nobody could have guessed that
    auto n     = children.size();
    auto found = std::next(elem).base();

    assert(*found == child, "Child should match");
    children.erase(found);
    assert(n > children.size(), "Child was not removed");

    child->parent = nullptr;

    if (dofree) {
        free(child);
    }
}

void GCObject::dump(std::ostream& out, int depth) {
    out << String(depth * 2, ' ') << meta::type_name(class_id) << std::endl;

    for (auto obj: children) {
        obj->dump(out, depth + 1);
    }
}

void GCObject::private_free(GCObject* child) {
    int cclass_id = child->class_id;

    child->~GCObject();

    manual_free(cclass_id, 1);
    device::CPU().free((void*)child, 1);
}

void GCObject::free(GCObject* child) {
    if (child == nullptr) {
        return;
    }

    // Remove from parent right away
    if (child->parent != nullptr) {
        child->parent->remove_child(child, false);
        assert(child->parent == nullptr, "parent should be null");
    }

    private_free(child);
}

GCObject::~GCObject() {
    // free children
    int n = int(children.size());

    while (n > 0) {
        n -= 1;

        GCObject* obj = children[n];
        obj->parent   = nullptr;
        children.pop_back();

        private_free(obj);
    }

    assert(children.size() == 0,
           "Makes sure nobody added more nodes while we were busy destroying them");
}

}  // namespace lython