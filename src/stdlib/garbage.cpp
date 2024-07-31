#include "garbage.h"

#include <algorithm>

namespace lython {

        int BoehmGarbageCollector::is_pointer(void* ptr) {
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
        if (std::find(allocations.begin(), allocations.end(), hdr) != allocations.end()) {
            // kwdebug(outlog(), "Good");
            return true;
        }

        // this happens all the time
        // kwdebug(outlog(), "Bad {}", ptr);
        return false;
    }

void* BoehmGarbageCollector::malloc(std::size_t size) {
        // TODO handle alignment
        GCObjectHeader* hdr = (GCObjectHeader*)std::malloc(size + sizeof(GCObjectHeader));
        kwdebug(outlog(), "Allocating {} - {}", (void*)(hdr), hdr->data());

        hdr->size   = size;
        hdr->marked = 0;
        allocations.push_back(hdr);
        return hdr->data();
    }

void BoehmGarbageCollector::free(void* ptr) {
        if (ptr == nullptr) {
            return;
        }
        GCObjectHeader* hdr = header(ptr);
        kwdebug(outlog(), "free {}", (void*)(hdr));
        std::free(hdr);
    }


void BoehmGarbageCollector::flat_mark(void* obj) {
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
            if (is_pointer(*member)) {
                pointers.push_back(*member);
            }
        }
    }
}




void BoehmGarbageCollector::stop_world_collect() {
        get_heap_bounds();

        // Mark
        kwdebug(outlog(), "mark stack");
        scan_stack();
        kwdebug(outlog(), "mark globals");
        scan_globals();
        kwdebug(outlog(), "mark registers");
        scan_registers();

        // Sweep
        kwdebug(outlog(), "Sweep");
        sweep();
    }

    void BoehmGarbageCollector::sweep() {
        Array<GCObjectHeader*> allocs;
        for (GCObjectHeader* obj: allocations) {
            if (!obj->marked) {
                kwdebug(outlog(), "free {}", (void*)(obj));
                std::free(obj);
            } else {
                kwdebug(outlog(), "still used");
                obj->marked = 0;
                allocs.push_back(obj);
            }
        }
        allocations = allocs;
    }


void BoehmGarbageCollector::dump(std::ostream& out) {
        get_heap_bounds();

        out << fmt::format("+-->  {} | {}\n", heap_start, 0);
        for (GCObjectHeader* obj: allocations) {
            auto diff = (char*)(obj) - (char*)(heap_start);
            out << fmt::format("| +-> {} | {}\n", (void*)(obj), diff);
            out << fmt::format("| | \n");
            out << fmt::format("| +-> {}\n", obj->size);
        }
        out << fmt::format("+-->  {} | {} \n", heap_end, (char*)(heap_end) - (char*)(heap_start));
    }
}


