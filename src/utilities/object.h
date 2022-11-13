#ifndef LYTHON_OBJECT_HEADER
#define LYTHON_OBJECT_HEADER

#include <memory>

#include "dependencies/coz_wrap.h"
#include "dtypes.h"

namespace lython {

struct GCObject {
    public:
    template <typename T, typename... Args>
    T* new_object(Args&&... args) {
        COZ_BEGIN("T::GCObject::new_object");

        auto& alloc = get_allocator<T>();

        // allocate
        T* memory = alloc.allocate(1);

        // construct
        T* obj        = new ((void*)memory) T(std::forward<Args>(args)...);
        obj->class_id = meta::type_id<T>();

        add_child(obj);

        COZ_PROGRESS_NAMED("GCObject::new_object");
        COZ_END("T::GCObject::new_object");
        return obj;
    }

    template <typename T>
    typename std::remove_const<T>::type* copy(T* obj) {
        COZ_BEGIN("T::GCObject::copy");
        using NoConstT = typename std::remove_const<T>::type;

        auto& alloc = get_allocator<NoConstT>();

        // allocate
        NoConstT* memory = alloc.allocate(1);
        // construct
        NoConstT* nobj = new ((void*)memory) NoConstT(*obj);

        nobj->class_id = meta::type_id<NoConstT>();
        nobj->parent   = this;

        COZ_PROGRESS_NAMED("GCObject::copy");
        COZ_END("T::GCObject::copy");
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
        GCObject* obj = child.release();
        add_child(obj);
    }

    void remove_child(GCObject* child, bool dofree);

    void remove_child_if_parent(GCObject* child, bool dofree);

    void move(GCObject* newparent) {
        if (parent != nullptr) {
            parent->remove_child(this, false);
        }
        newparent->add_child(this);
    }

    static void free(GCObject* child);

    void dump(std::ostream& out);

    virtual ~GCObject();

    int class_id;

    private:
    void dump_recursive(std::ostream& out, Array<GCObject*>& visited, int prev, int depth);

    template <typename T>
    AllocatorCPU<T>& get_allocator() {
        static auto alloc = AllocatorCPU<T>();
        return alloc;
    }

    protected:
    Array<GCObject*> children;

    GCObject* get_gc_parent() const { return parent; }

    private:
    GCObject* parent = nullptr;

    static void private_free(GCObject* child);
};

}  // namespace lython
#endif