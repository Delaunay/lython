#ifndef LYTHON_OBJECT_HEADER
#define LYTHON_OBJECT_HEADER

#include <memory>

#include "../dtypes.h"

namespace lython {

enum class ObjectKind {
    Module,
    Statement,
    Expression,
    Pattern,
};

struct GCObject {
public:
    template<typename T, typename... Args>
    T* new_object(Args&&... args) {
        auto& alloc = get_allocator<T>();

        // allocate
        T* memory = alloc.allocate(1);

        // construct
        T* obj = new((void*)memory) T(std::forward<Args>(args)...);

        children.push_back(obj);
        return obj;
    }

    //! Make an object match the lifetime of the parent
    template<typename T>
    void add_child(T* child) {
        children.push_back(child);
    }

    template<typename T, typename D>
    void add_child(Unique<T, D> child) {
        children.push_back(child.release());
    }

    void remove_child(GCObject* child, bool free) {
        auto elem = std::find(children.rbegin(), children.rend(), child);
        // This is why people hate C++
        //
        // This removes the element found by the reverse iterator.
        // Reverse iterator do not point to the found element
        // but the element before it.
        // erase is not specialized to take reverse iterator
        // so we need to convert it ourselves and KNOW that this is what is
        // expected but nobody could have guessed that
        auto n = children.size();
        children.erase(std::next(elem).base());
        assert(n > children.size(), "Child was not removed");

        if (free) {
            // FIXME: this breaks the metadata
            device::CPU().free((void*)child, 1);
        }
    }

    ~GCObject(){
        for (auto obj: children) {
            // FIXME: this breaks the metadata
            device::CPU().free((void*)obj, 1);
        }
    }

    GCObject(ObjectKind k):
        kind(k)
    {}

    ObjectKind const kind;

private:
    template<typename T>
    AllocatorCPU<T>& get_allocator() {
        static auto alloc = AllocatorCPU<T>();
        return alloc;
    }

private:
    Array<GCObject*> children;
};

} // lython
#endif