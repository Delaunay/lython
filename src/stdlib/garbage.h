#include "dtypes.h"

namespace lython {

template <typename T>
struct Finalizer {
    static void finalize(void* data) {
        T* obj = reinterpret_cast<T*>(data);
        obj->~T();
    }
};

struct GCObjectHeader {
    std::size_t size = 0;
    bool        marked : 1;
    void (*finalizer)(void*) = nullptr;

    // does not work on windows
    // void* data[0];

    void* data() { return ((char*)(this)) + sizeof(GCObjectHeader); }
};

struct Generation {
    Array<GCObjectHeader*> allocations;
};

enum class GCGen
{
    Temporary,
    Medium,
    Long,
    Max,
};

/** very basic garbage collector
 * perf wise pointer lookup is probably the main issue
 *   creating generation of pointers to keep the pointer lookup fast
 *   for the most checked generation
 * 
 * buckets or values could be created to avoid allocating memory too often
 * so the temporary objects can keep reusing the same memory
 *
 * Selecting a smaller memory region to scan might be too dangerous
 * because the GC will free the value so we have to make sure it is nowhere else
 * but if we only scan a subset we cannot know that. so usually we have to scan everything
 *
 * Making it compacting would be difficult because we are still relying on 
 * native pointer.
 * Maybe we could make a compacting step /scan step that 
 * change the memory pointed by the pointers
 * 
 * compute the memory size of generation, allocate a big block
 * change the memory pointers
 */
struct BoehmGarbageCollector {

public:
    BoehmGarbageCollector() { generations.resize(int(GCGen::Max)); }

    int is_pointer(GCGen gen, void* ptr);

    template <typename T>
    T* malloc() {
        return (T*)malloc(sizeof(T), Finalizer<T>::finalize);
    }

    void* malloc(std::size_t size, void (*finalizer)(void*) = nullptr);

    void free(void* ptr);

    bool is_marked(void* ptr) { return header(ptr)->marked; }

    void clear_mark(void* ptr) { header(ptr)->marked = 0; }

    GCObjectHeader* header(void* ptr) {
        return (GCObjectHeader*)((char*)(ptr) - sizeof(GCObjectHeader));
    }

    void collect(GCGen gen = GCGen::Temporary) {
        get_heap_bounds();

        // Mark
        mark(gen);

        // Sweep
        sweep(gen);
    }

    // mark an object as live
    void mark_obj(void* obj, GCGen gen = GCGen::Temporary);

public:

    GCGen promotion(GCGen gen) {
        switch(gen) {
            case GCGen::Temporary: return GCGen::Medium;
            case GCGen::Medium: return GCGen::Long;
            case GCGen::Long: return GCGen::Long;
        }
        return GCGen::Temporary;
    }

    bool should_promote(GCObjectHeader* obj) {
        return false;
    }

    void promote(GCObjectHeader* obj, GCGen gen) {
        if (should_promote(obj)) {
            gen = promotion(gen);
        }
        allocations(gen).push_back(obj);
    }

    void mark(GCGen gen) {
        mark_stack(gen);
        mark_globals(gen);
        mark_registers(gen);
    }
    void sweep(GCGen gen);

    void mark_stack(GCGen gen);
    void mark_globals(GCGen gen);
    void mark_registers(GCGen gen);

    void get_heap_bounds();
    void dump(std::ostream& out);

public:
    void add(GCGen gen, GCObjectHeader* obj) { generations[int(gen)].allocations.push_back(obj); }

    Array<GCObjectHeader*>& allocations(GCGen gen) { return generations[int(gen)].allocations; }

    Array<void*>      pointers;     // pre allocated work space for mark
    Array<Generation> generations;  // list of allocations
    void*             heap_start = nullptr;
    void*             heap_end   = nullptr;
};

}  // namespace lython