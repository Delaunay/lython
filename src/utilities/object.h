#ifndef LYTHON_OBJECT_HEADER
#define LYTHON_OBJECT_HEADER

#include <memory>

#include "../dtypes.h"

namespace lython {

struct GCObject {
    public:
    template <typename T, typename... Args>
    T* new_object(Args&&... args) {
        auto& alloc = get_allocator<T>();

        // allocate
        T* memory = alloc.allocate(1);

        // construct
        T* obj        = new ((void*)memory) T(std::forward<Args>(args)...);
        obj->class_id = meta::type_id<T>();
        obj->parent   = this;

        children.push_back(obj);
        return obj;
    }

    template <typename T>
    std::remove_const<T>::type* copy(T* obj) {
        using NoConstT = std::remove_const<T>::type;

        auto& alloc = get_allocator<NoConstT>();

        // allocate
        NoConstT* memory = alloc.allocate(1);
        // construct
        NoConstT* nobj = new ((void*)memory) NoConstT(*obj);

        nobj->class_id = meta::type_id<NoConstT>();
        nobj->parent   = this;

        return nobj;
    }

    //! Make an object match the lifetime of the parent
    template <typename T>
    void add_child(T* child) {
        children.push_back(child);
        child->parent = this;
    }

    template <typename T, typename D>
    void add_child(Unique<T, D> child) {
        children.push_back(child.release());
    }

    void remove_child(GCObject* child, bool dofree);

    void move(GCObject* newparent) {
        if (parent) {
            parent->remove_child(this, false);
        }
        newparent->add_child(this);
    }

    static void free(GCObject* child);

    void dump(std::ostream&, int depth = 0);

    virtual ~GCObject();

    int class_id;

    private:
    template <typename T>
    AllocatorCPU<T>& get_allocator() {
        static auto alloc = AllocatorCPU<T>();
        return alloc;
    }

    protected:
    Array<GCObject*> children;
    GCObject*        parent = nullptr;

    static void private_free(GCObject* child);
};

}  // namespace lython
#endif