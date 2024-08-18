#pragma once


#include "dtypes.h"

namespace lython {

template <typename T>
struct Finalizer {
    static void finalize(void* data) {
        T* obj = reinterpret_cast<T*>(data);
        obj->~T();
    }
};


#define KIWI_ALLOCATION_DEBUG 0
#if __linux__
#define KIWI_NOINLINE __attribute__ ((noinline))
#else
#define KIWI_NOINLINE __declspec(noinline)
#endif

enum class Mark {
    None,
    Stack,
    Register,
    Global,
    Child,
};

struct GCObjectHeader {
    std::size_t size = 0;
    bool        marked : 1;
    void (*finalizer)(void*) = nullptr;

#if KIWI_ALLOCATION_DEBUG
    uint8_t      mark_source;
    CodeLocation loc = CodeLocation::noloc();
    std::string  name;
#endif

    // does not work on windows
    // void* data[0];

    void* data() { return ((char*)(this)) + sizeof(GCObjectHeader); }
};

struct Generation {
    Array<GCObjectHeader*> allocations;
};

template <typename T>
class ReversedIterable {
public:
    ReversedIterable(T& container) : container_(container) {}

    auto begin() const { return container_.rbegin(); }
    auto end() const { return container_.rend(); }

private:
    T& container_;
};

template <typename T>
ReversedIterable<T> reversed(T& container) {
    return ReversedIterable<T>(container);
}

enum class GCGen
{
    Temporary,  // Gets collected often
    Medium,
    Long,       // Gets rarely collected
    Root,       // Never gets collected
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
 * 
 * The header could also be dynamically allocated separately for common
 * type/size 
 * 
 * Currently the GC never runs, some heuristic needs to be put in place
 * to make it run periodically
 */
struct BoehmGarbageCollector {

public:
    BoehmGarbageCollector() { generations.resize(int(GCGen::Max)); }

    ~BoehmGarbageCollector() {
        // freeing the remaining memory might be tricky
        // usually it is better to free the latest first
        // Note: that we do a simple delete without taking into account
        // where the pointers are we could try to identify members
        // so we delete members first then the main object
        // currently I think the allocation order is mainly kept in our DS
        // so maybe it is not an issue
        for(Generation& gen: generations) {
            for(GCObjectHeader* hdr: reversed(gen.allocations)) {
                if (hdr->finalizer) {
                    hdr->finalizer(hdr->data());
                }
                std::free(hdr);
            }
            gen.allocations.clear();
        }
    }
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
    void mark_obj(void* obj, GCGen gen = GCGen::Temporary, Mark source = Mark::None);

    void possible_pointer(GCGen gen, void** current, Mark source) {
        // maye this could be relocatable
        // Mark object as reachable in GC
        if (is_pointer(gen, *current)) {
            mark_obj(*current, gen, source);
        }
    }

    #if 0
    void possible_pointer(GCGen gen, void* current, Mark source) {
        // Mark object as reachable in GC
        if (is_pointer(gen, current)) {
            mark_obj(current, gen, source);
        }
    }
    #endif

public:

    GCGen promotion(GCGen gen) {
        switch(gen) {
            case GCGen::Temporary:  return GCGen::Medium;
            case GCGen::Medium:     return GCGen::Long;
            case GCGen::Long:       return GCGen::Long;
        }
        return GCGen::Temporary;
    }

    bool should_promote(GCObjectHeader* obj) {
        return false;
    }

    void mark(GCGen gen) {
        get_heap_bounds();

        #if KIWI_ALLOCATION_DEBUG
        for(GCObjectHeader* obj: allocations(gen)) {
            if (obj) {
                obj->mark_source = 0;
            }
        }
        #endif

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

    // insert location info inside the header
    template<typename T>
    T* located(T* ptr, CodeLocation const& loc) {
        #if KIWI_ALLOCATION_DEBUG
        GCObjectHeader* hdr = header(ptr);
        hdr->loc = loc;
        #endif
        return ptr;
    }

    // name the allocation
    template<typename T>
    T* named(T* ptr, std::string const& name) {
        #if KIWI_ALLOCATION_DEBUG
        GCObjectHeader* hdr = header(ptr);
        hdr->name = name;
        #endif
        return ptr;
    }

    template <typename T>
    T* malloc(CodeLocation const& loc) {
        return (T*)malloc(sizeof(T), loc, Finalizer<T>::finalize);
    }
    
    void* malloc(std::size_t size, CodeLocation const& loc, void (*finalizer)(void*) = nullptr) {
        return located(malloc(size, finalizer), loc);
    }
};


inline
BoehmGarbageCollector& garbage_collector() {
    static BoehmGarbageCollector gc;
    return gc;
}

inline
void* kw_malloc(std::size_t n) {
    return garbage_collector().malloc(n);
}

template<typename T>
T* kw_malloc() {
    return garbage_collector().malloc<T>();
}


template<typename T>
void kw_free(T* ptr) {
    return garbage_collector().free(ptr);
}


}  // namespace lython