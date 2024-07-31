#include "dtypes.h"

namespace lython {

struct GCObjectHeader {
    std::size_t size;
    bool   marked : 1;

    // does not work on windows
    // void* data[0];

    void* data() { return ((char*)(this)) + sizeof(GCObjectHeader); }
};

/** very basic garbage collector
 * perf wise pointer lookup is probably the main issue
 * buckets or values could be created to avoid allocating memory too often
 * 
 * Scaning the stack could focus on newer memory instead
 * of scaning everything
 * 
 */
struct BoehmGarbageCollector {
    void* heap_start;
    void* heap_end;

    int is_pointer(void* ptr);

    template <typename T>
    T* malloc() {
        return (T*)malloc(sizeof(T));
    }

    void* malloc(std::size_t size);

    void free(void* ptr);

    void mark(void* obj) { flat_mark(obj); }

    Array<void*> pointers;

    void flat_mark(void* obj);

    bool is_marked(void* ptr) { return header(ptr)->marked; }

    void clear_mark(void* ptr) { header(ptr)->marked = 0; }

    GCObjectHeader* header(void* ptr) {
        return (GCObjectHeader*)((char*)(ptr) - sizeof(GCObjectHeader));
    }

    void collect() { stop_world_collect(); }

    void stop_world_collect();
    void sweep();

    void scan_stack();
    void scan_globals();
    void scan_registers();
    void get_heap_bounds();

    Array<GCObjectHeader*> allocations;

    void dump(std::ostream& out);
};

}