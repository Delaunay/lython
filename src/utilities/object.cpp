#include "object.h"
#include "logging/logging.h"
#include "ast/nodes.h"

namespace lython {

void GCObject::remove_child(GCObject* child, bool dofree) {

    GCObject* result = nullptr;
    int       i      = int(children.size()) - 1;

    for (; i >= 0; i--) {
        if (children[i] == child) {
            break;
        }
    }

    // FIXME: this should never happen
    if (i == -1) {
        kwerror(outlog(), "Trying to remove (child: {}) from (parent: {}), (child->parent: {});"
                "but the child was not found",
                (void*)child,
                (void*)this,
                (void*)child->parent);
        return;
    } else {
        kwinfo(outlog(), "Removed (child: {}) from (parent: {})", (void*)child, (void*)this);
    }
    // lyassert(i != -1, "Should find child");

    Array<GCObject*>::iterator data = children.begin() + i;
    children.erase(data);
    child->parent = nullptr;

    if (dofree) {
        free(child);
    }
}

void GCObject::remove_child_if_parent(GCObject* child, bool dofree) {
    if (child && child->parent == this) {
        remove_child(child, dofree);
    }
}

void GCObject::dump(std::ostream& out) {
    Array<GCObject*> visited;

    dump_recursive(out, visited, -1, 0);
}

int in(GCObject* obj, Array<GCObject*>& visited) {
    int i = 0;

    for (auto* item: visited) {
        if (item == obj)
            return true;

        i += 1;
    }

    return -1;
}

void GCObject::dump_recursive(std::ostream& out, Array<GCObject*>& visited, int prev, int depth) {
    // Cycles should be impossible here
    int    index   = prev < 0 ? int(visited.size()) : prev;
    String warning = prev >= 0 ? "DUPLICATE" : "";

    if (class_id == meta::type_id<Constant>()) {
        Constant* value = reinterpret_cast<Constant*>(this);

        std::stringstream ss;
        ss << value->value;

        out << String(depth * 2, ' ') << index << ". " 
            << meta::type_name(class_id) << warning << " " << ss.str()
            << std::endl;

    }
    else {
        out << String(depth * 2, ' ') << index << ". " 
            << meta::type_name(class_id) << warning
            << std::endl;
    }

    

    for (auto obj: children) {
        int found = in(obj, visited);

        if (found < 0) {
            visited.push_back(obj);
        }

        obj->dump_recursive(out, visited, found, depth + 1);
    }
}

void GCObject::private_free(GCObject* child) {
    int cclass_id = child->class_id;

    child->~GCObject();

    manual_free(cclass_id, 1);
    device::CPU().free((void*)child, 1);
}

void GCObject::free(GCObject* child) {
    // Remove from parent right away
    if (child->parent != nullptr) {
        child->parent->remove_child(child, false);
        lyassert(child->parent == nullptr, "parent should be null");
    }

    private_free(child);
}

GCObject::~GCObject() {
    COZ_BEGIN("T::GCObject::delete");

    // free children
    int n = int(children.size());

    while (n > 0) {
        n -= 1;

        GCObject* obj = children[n];
        obj->parent   = nullptr;
        children.pop_back();

        private_free(obj);
    }

    COZ_PROGRESS_NAMED("GCObject::delete");
    COZ_END("T::GCObject::delete");

    lyassert(children.size() == 0,
           "Makes sure nobody added more nodes while we were busy destroying them");
}

}  // namespace lython