#include "garbage_collector.h"
#include "sema/bindings.h"

namespace lython {

#if 0
GargabeCollector::GargabeCollector(struct Bindings& bindings, struct Expression& root):
    bindings(bindings), root(root)  //
{}

void GargabeCollector::mark() {
    // For generational collection we could start
    // the check later into the binding array
    int i = bindings.global_index;

    for (; i < bindings.bindings.size(); i++) {
        //
        BindingEntry& entry = bindings.bindings[i];

        // Mark the object as viewed
        GCObject* value = entry.value;

        // TODO: fetch the GC metadata
        RTGCObject* mapped = nullptr;
        if (mapped != nullptr) {
            mapped->cycle = cycle;
        }
    }
}

void GargabeCollector::free_object(RTGCObject* obj) {
    root.remove_child((GCObject*)obj->object, true);
}

void GargabeCollector::sweep() { shrink(); }

void GargabeCollector::shrink() {
    int        deleted   = 0;
    int        i         = 0;
    int        free_spot = 0;
    Array<int> free_spots;

    for (RTGCObject& obj: tracked) {
        if (obj.cycle < cycle) {
            deleted += 1;
            free_object(&obj);
            free_spots.push_back(i);
        } else if (!free_spots.empty()) {
            tracked[free_spot] = obj;
            tracked[i]         = RTGCObject{0, nullptr};
            free_spots.push_back(i);
            free_spot += 1;
        }
        i += 1;
    }

    tracked.resize(int(tracked.size()) - deleted);
    cycle += 1;
}
#endif
}  // namespace lython