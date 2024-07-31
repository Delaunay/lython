#include "garbage.h"

#include <algorithm>

namespace lython {

        int BoehmGarbageCollector::is_pointer(GCGen gen, void* ptr) {
        // Check if the pointer is within the heap range
        // if (ptr < heap_start || ptr >= heap_end) {
        //    return false;
        //}

        // Check alignment: must be aligned to pointer size
        // if (reinterpret_cast<uintptr_t>(ptr) % alignof(void*) != 0) {
        //    return false;
        // }

        // Check if the pointer is an actual allocated object
        GCObjectHeader* hdr = header(ptr);
        if (std::find(allocations(gen).begin(), allocations(gen).end(), hdr) != allocations(gen).end()) {
            // kwdebug(outlog(), "Good");
            return true;
        }

        // this happens all the time
        // kwdebug(outlog(), "Bad {}", ptr);
        return false;
    }

void* BoehmGarbageCollector::malloc(std::size_t size, void (*finalizer)(void*)) {
        // TODO handle alignment
        GCObjectHeader* hdr = (GCObjectHeader*)std::malloc(size + sizeof(GCObjectHeader));
        kwdebug(outlog(), "Allocating {} - {}", (void*)(hdr), hdr->data());

        hdr->size   = size;
        hdr->marked = 0;
        hdr->finalizer = finalizer;
        add(GCGen::Temporary, hdr);
        return hdr->data();
    }

void BoehmGarbageCollector::free(void* ptr) {
    if (ptr == nullptr) {
        return;
    }
    GCObjectHeader* hdr = header(ptr);
    if (hdr->finalizer) {
        hdr->finalizer(ptr);
    }
    kwdebug(outlog(), "free {}", (void*)(hdr));
    std::free(hdr);
}


void BoehmGarbageCollector::compact() {


}


void BoehmGarbageCollector::mark_obj(void* obj, GCGen gen) {
    pointers = {obj};
    kwdebug(outlog(), "marking {}", obj);

    while (!pointers.empty()) {
        void* ptr = *pointers.rbegin();
        pointers.pop_back();

        if (ptr == nullptr || is_marked(ptr)) {
            kwdebug(outlog(), "already marked");
            continue;
        }

        GCObjectHeader* hdr = header(ptr);
        hdr->marked         = true;
        kwdebug(outlog(), "marked {}", (void*)(hdr));

        for (int i = 0; i < hdr->size; i++) {
            void** member = reinterpret_cast<void**>(ptr) + i;
            if (is_pointer(gen, *member)) {
                pointers.push_back(*member);
            }
        }
    }
}

void BoehmGarbageCollector::sweep(GCGen gen) {
    Array<GCObjectHeader*> allocs;
    for (GCObjectHeader* obj: allocations(gen)) {
        if (!obj->marked) {
            kwdebug(outlog(), "free {}", (void*)(obj));
            std::free(obj);
        } else {
            obj->marked = 0;
            allocs.push_back(obj);
        }
    }
    allocations(gen) = allocs;
}

void BoehmGarbageCollector::dump(std::ostream& out) {
    get_heap_bounds();

    for(Generation& gen: generations) {
        if (gen.allocations.size() > 0) {
            out << fmt::format("+-->  {} | {}\n", heap_start, 0);
            for (GCObjectHeader* obj: gen.allocations) {
                auto diff = (char*)(obj) - (char*)(heap_start);
                out << fmt::format("| +-> {} | {}\n", (void*)(obj), diff);
                out << fmt::format("| | \n");
                out << fmt::format("| +-> {}\n", obj->size);
            }
            out << fmt::format("+-->  {} | {} \n", heap_end, (char*)(heap_end) - (char*)(heap_start));
        }
    }
}

}


