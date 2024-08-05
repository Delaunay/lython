#include "garbage.h"

#include <algorithm>

namespace lython {

int BoehmGarbageCollector::is_pointer(GCGen gen, void* ptr) {
    // Check if the pointer is within the heap range

    // linux has issues with this
    if (ptr < heap_start || ptr >= heap_end) {
    //    return false;
    }

    // Check alignment: must be aligned to pointer size
    // if (reinterpret_cast<uintptr_t>(ptr) % alignof(void*) != 0) {
    //    return false;
    // }

    // Check if the pointer is an actual allocated object
    GCObjectHeader* hdr = header(ptr);
    if (std::find(allocations(gen).begin(), allocations(gen).end(), hdr) !=
        allocations(gen).end()) {
        // kwdebug(outlog(), "Good");
        return true;
    }

    // this happens all the time
    // kwdebug(outlog(), "Bad {}", ptr);
    return false;
}

void* BoehmGarbageCollector::malloc(std::size_t size, void (*finalizer)(void*)) {
    // TODO handle alignment
    void*           mem = (GCObjectHeader*)std::malloc(size + sizeof(GCObjectHeader));
    GCObjectHeader* hdr = new (mem) GCObjectHeader();
    kwdebug(outlog(), "Allocating {} - {}", (void*)(hdr), hdr->data());

    hdr->size      = size;
    hdr->marked    = 0;
    hdr->finalizer = finalizer;
    add(GCGen::Temporary, hdr);

#if KIWI_ALLOCATION_DEBUG
    get_heap_bounds();
    kwassert(mem >= heap_start && mem < heap_end, "Pointer needs to be inside heap bounds");
#endif

    return hdr->data();
}

void BoehmGarbageCollector::free(void* ptr) {
    if (ptr == nullptr) {
        return;
    }
    // TODO: remove the ptr from the allocation
    GCObjectHeader* hdr = header(ptr);
    if (hdr->finalizer) {
        hdr->finalizer(ptr);
    }
    hdr->~GCObjectHeader();
    kwdebug(outlog(), "free {}", (void*)(hdr));
    std::free(hdr);
}

void BoehmGarbageCollector::mark_obj(void* obj, GCGen gen, Mark source) {
    pointers = {obj};
    // kwdebug(outlog(), "marking {}", obj);
    int j = 0;
    while (!pointers.empty()) {
        void* ptr = *pointers.rbegin();
        pointers.pop_back();

        if (ptr == nullptr || is_marked(ptr)) {
            kwdebug(outlog(), "{} already marked", j);
            continue;
        }

        GCObjectHeader* hdr = header(ptr);
        hdr->marked         = true;
#if KIWI_ALLOCATION_DEBUG
        hdr->mark_source |= (1 << int(source));
        if (j > 0) {
            hdr->mark_source |= (1 << int(Mark::Child));
        }
#endif

        kwdebug(outlog(), "{} marked {}", j, (void*)(hdr));
        j += 1;

        for (int i = 0; i < hdr->size; i += 1) {
            char* member = reinterpret_cast<char*>(ptr) + i;
            if (is_pointer(gen, *(void**)(member))) {
                pointers.push_back(*(void**)(member));
            }
        }
    }
}

void BoehmGarbageCollector::sweep(GCGen gen) {
    Array<GCObjectHeader*> allocs;
    for (GCObjectHeader* obj: allocations(gen)) {
        if (obj == nullptr) {
            continue;
        }
        if (!obj->marked) {
            kwdebug(outlog(), "free {}", (void*)(obj));
            if (obj->finalizer) {
                obj->finalizer(obj->data());
            }
            std::free(obj);
        } else {
            obj->marked = 0;
            allocs.push_back(obj);
        }
    }

    allocations(gen).clear();
    GCGen ngen = promotion(gen);
    for (GCObjectHeader* hdr: allocs) {
        if (should_promote(hdr)) {
            allocations(ngen).push_back(hdr);
        } else {
            allocations(gen).push_back(hdr);
        }
    }
}

std::string sources(uint8_t source) {
    bool        stack  = source & (1 << int(Mark::Stack));
    bool        reg    = source & (1 << int(Mark::Register));
    bool        global = source & (1 << int(Mark::Global));
    bool        child = source & (1 << int(Mark::Child));
    std::string src;
    src.reserve(32);
    if (stack) {
        src += "stack ";
    }
    if (reg) {
        src += "reg ";
    }
    if (global) {
        src += "global ";
    }
    if (child) {
        src += "child ";
    }
    return src;
}

void BoehmGarbageCollector::dump(std::ostream& out) {
    get_heap_bounds();

    for (Generation& gen: generations) {
        if (gen.allocations.size() > 0) {
            out << fmt::format("+-->  {} | {}\n", heap_start, 0);
            for (GCObjectHeader* obj: gen.allocations) {
                auto diff = (char*)(obj) - (char*)(heap_start);
                out << fmt::format("| +-> {} | {}\n", (void*)(obj), diff);
#if KIWI_ALLOCATION_DEBUG
                out << fmt::format("| | name    : {}\n", obj->name);
                out << fmt::format("| | location: {}:{}\n", obj->loc.function_name, obj->loc.line);
                out << fmt::format("| | found_in: {} \n", sources(obj->mark_source));
#else
                out << fmt::format("| | \n");
#endif
                out << fmt::format("| +-> {}\n", obj->size);
            }
            out << fmt::format(
                "+-->  {} | {} \n", heap_end, (char*)(heap_end) - (char*)(heap_start));
        }
    }
}

}  // namespace lython
