#pragma once

#include "dtypes.h"
#include "utilities/object.h"

namespace lython {

#if 0
// runtime gcobject, as opposed to the compiler gcobject that behave differently
struct RTGCObject {
    int   cycle  = false;  // collection cycle, collect if gc cycle <= collection cycle
    void* object = nullptr;
};

// Very simple garbage collector
class GargabeCollectorOld {
    GargabeCollector(struct Bindings& bindings, struct Expression& root);

    // Allocation hook
    template <typename T, typename... Args>
    T* new_object(Args... args) {
        T* obj = root.new_object(args...);
        tracked.push_back(RTGCObject{false, obj});
        return obj;
    }

    void free_object(RTGCObject* obj);

    void mark();

    void sweep();

    // Compact and collect memory
    void shrink();

    private:
    // this is the state of the execution
    struct Bindings& bindings;

    // this is the root node that holds all the temporaries
    struct GCObject& root;

    int               cycle = 0;
    Array<RTGCObject> tracked;
};

#endif
}  // namespace lython