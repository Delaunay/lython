#include "object.h"
#include "logging/logging.h"

namespace lython {

void GCObject::remove_child(GCObject *child, bool dofree) {
    auto elem = std::find(children.rbegin(), children.rend(), child);
    // This is why people hate C++
    //
    // This removes the element found by the reverse iterator.
    // Reverse iterator do not point to the found element
    // but the element before it.
    // erase is not specialized to take reverse iterator
    // so we need to convert ifreet ourselves and KNOW that this is what is
    // expected but nobody could have guessed that
    auto n = children.size();
    children.erase(std::next(elem).base());
    assert(n > children.size(), "Child was not removed");

    child->parent = nullptr;
    if (dofree) {
        free(child);
    }
}

void GCObject::dump(std::ostream &out, int depth) {
    out << String(depth * 2, ' ') << meta::type_name(class_id) << std::endl;

    for (auto obj: children) {
        obj->dump(out, depth + 1);
    }
}

void GCObject::free(GCObject *child) {
    child->~GCObject();

    int cclass_id = child->class_id;

    // FIXME: this breaks the metadata
    manual_free(cclass_id, 1);
    device::CPU().free((void *)child, 1);

    // info("freed {}", meta::type_name(cclass_id));
}

GCObject::~GCObject() {
    // logging here generate:
    // info("freeing {}: {}", (void *)this, meta::type_name(class_id));
    int ccclass_id = class_id;

    for (auto obj: children) {
        free(obj);
    }

    // info("freed {}", meta::type_name(ccclass_id));
}

} // namespace lython