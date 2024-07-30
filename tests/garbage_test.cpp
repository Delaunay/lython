#include "dtypes.h"
// #include "vm/garbage_collector.h"

#include <algorithm>

namespace lython {

class GargabeCollector;

template <typename T, bool auto_destroy = false>
struct PointerProxy {
    // overhead is:
    //      Version check  (2 int compare)
    //      Segment Lookup (ptr addition)
    //      Memory Lookup  (ptr addition)
    T*   operator->();
    T&   operator*() const;
    bool is_valid() const;
    void destroy();
    T*   data();

    PointerProxy(PointerProxy&& proxy) {
        collector        = proxy.collector;
        segment_id       = proxy.segment_id;
        version          = proxy.version;
        proxy.segment_id = -1;
    }

    PointerProxy(PointerProxy const& copy) = default;
    PointerProxy& operator=(PointerProxy const& copy) = default;

    ~PointerProxy() {
        if (auto_destroy) {
            destroy();
        }
    }

    private:
    friend class GargabeCollector;

    PointerProxy(GargabeCollector& c, int id, int version):
        collector(c), segment_id(id), version(version) {}

    // 2x the size of a pointer, because of that reference
    // if it becomes global then it would be the same size
    GargabeCollector& collector;
    int               segment_id;
    int               version;  // expected version
};

struct MemorySegment {
    int idx     = -1;
    int start   = -1;
    int size    = -1;
    int used    = -1;
    int version = -1;  // everytime the segment is reused version increase

    MemorySegment(int idx, int s, int ss, int v, int u):
        idx(idx), start(s), size(ss), version(v), used(u) {}

    void zeroout() {
        size  = 0;
        start = 0;
        used  = -1;
    }
};

#define REUSE_SEGMENTS_BEFORE_COMPACT 1

//
// Special allocator that cannot lose memory
// Memory is allocated in chunks and can be compacted
//
// You should call `free` or PointerProxy.destroy to trigger the destructor
//
//  Can this become a STL allocator ?
//  https://stackoverflow.com/questions/65645904/implementing-a-custom-allocator-with-fancy-pointers
//  it can-ish because the STL will assume raw pointers and shit
//
class GargabeCollector {
    public:
    using byte = uint8_t;

    template <typename T, typename... Args>
    PointerProxy<T> malloc(Args&&... args) {
        auto [segment_id, version] = allocate(sizeof(T));

        if (segment_id < 0) {
            throw std::runtime_error("Could not allocate memory");
        }

        new (memory(segment_id)) T(std::forward<Args>(args)...);

        return PointerProxy<T>(*this, segment_id, version);
    }

    static GargabeCollector& instance() {
        static GargabeCollector collector;
        return collector;
    }

    template <typename T>
    void free(PointerProxy<T> proxy) {
        proxy.data()->~T();
        free(proxy.segment_id);
    }

    void display_memory() {
        int size = int(raw_memory.size());

        std::cout << " +---> " << 0 << "\n";
        int k = 0;
        for (int i: sorted_segments()) {
            if (k != 0) {
                std::cout << " |"
                          << "\n";
            }
            MemorySegment& segment = segments[i];

            if (segment.size != 0) {
                std::cout << " | +-> (start: " << segment.start << ") (idx:" << segment.idx
                          << ") (size: " << segment.size << ") (version: " << segment.version
                          << ")\n";

                std::cout << " | |   " << (segment.used == 1 ? "taken" : "empty") << "\n";
                std::cout << " | +-> " << segment.start + segment.size << "\n";
                k += 1;
            }
        }
        std::cout << " +---> cursor: " << free_idx << "\n";
        std::cout << " |     "
                  << "\n";
        std::cout << " +---> capacity: " << raw_memory.size() << "\n";
    }

    // private:
    Array<int> sorted_segments() {
        Array<int> segment_order;
        segment_order.reserve(segments.size());
        for (int i = 0; i < segments.size(); i++) {
            segment_order.push_back(i);
        }
        std::sort(segment_order.begin(), segment_order.end(), [&](int a_id, int b_id) {
            return segments[a_id].start < segments[b_id].start;
        });
        return segment_order;
    }

    template <typename T, bool auto_delete>
    friend struct PointerProxy;

    bool is_valid(int segment_id) const {
        return segment_id >= 0 && get_segment(segment_id).start >= 0;
    }

    bool is_valid(int segment_id, int version) const {
        return segment_id >= 0 && get_segment(segment_id).version == version;
    }

    byte* memory(int segment_id) {
        MemorySegment& segment = get_segment(segment_id);
        return raw_memory.data() + segment.start;
    }

    void free(int segment_id) {
        if (segment_id >= 0) {
            MemorySegment& segment = get_segment(segment_id);
            segment.used           = -1;  // segment is not used
            segment.version += 1;         // Old ref will have version mismatch
#if REUSE_SEGMENTS_BEFORE_COMPACT
            free_list.push_back(segment_id);
#else
            segment.zeroout();
            zero_list.push_back(segment_id);
#endif
        }
    }

    // split a segment in two, first segment is of size `size`
    // return the idx of the first segment
    int split_segment(MemorySegment& segment, int idx, int size) {
        // Note that spliting segments only makes sense if
        // REUSE_SEGMENTS_BEFORE_COMPACT is enabled
        // This insert a new segment, and resize the segment it is spliting
        int insert_idx = int(segments.size());

        segments.emplace_back(insert_idx, segment.start + size, segment.size - size, 0, 0);
#if REUSE_SEGMENTS_BEFORE_COMPACT
        free_list.push_back(insert_idx);
#endif

        segment.size = size;
        segment.used = -1;
        segment.version += 1;
        return idx;
    }

    void merge_segments() {
        // find unused segments and merge them together
        // Note that merging segments only makes sense if
        // REUSE_SEGMENTS_BEFORE_COMPACT is enabled
        int candidate = -1;
        int start     = -1;
        int end       = -1;

        // we could sort first to improve the matching

        for (int i = 0; i < segments.size(); i++) {
            MemorySegment& segment = segments[i];

            // an empty segment does not prevent merging
            if (segment.size == 0) {
                continue;
            }

            // a used segment prevents merging
            if (segment.used != -1) {
                candidate = -1;
                continue;
            }

            // Found an unused segment that could be merged
            if (candidate == -1 && segment.start >= 0) {
                candidate = i;
                start     = segment.start;
                end       = start + segment.size;
            }

            // we are looking to extend this segment
            if (candidate >= 0) {
                // is the memory contiguous ?
                if (segment.start == end) {

                    // increase the candidate size to eat the other segment
                    MemorySegment& merged_segment = segments[candidate];
                    merged_segment.size += segment.size;

                    // the other segment cannot be used anymore
                    segment.zeroout();
                    zero_list.push_back(i);
                } else {
                    candidate = -1;
                }
            }
        }
    }

    public:
    // make all the memory segment contiguous
    void compact_memory() {
        Array<byte> new_raw_memory(raw_memory.size(), 0);
        int         new_free_ptr = 0;
        zero_list.clear();

        for (int segment_id: sorted_segments()) {
            MemorySegment& segment = segments[segment_id];

            // Do not copy unused segments
            if (segment.size == 0 || segment.used != 1) {
                // zeroout all the segments
                segment.zeroout();
                zero_list.push_back(segment_id);
                continue;
            }

            void* dest = new_raw_memory.data() + new_free_ptr;
            void* src  = raw_memory.data() + segment.start;
            memcpy(dest, src, segment.size);
            segment.start = new_free_ptr;
            new_free_ptr += segment.size;
        }

        free_idx   = new_free_ptr;
        raw_memory = new_raw_memory;
#if REUSE_SEGMENTS_BEFORE_COMPACT
        free_list.clear();
#endif
    }

    private:
    MemorySegment& get_segment(int idx) { return segments[idx]; }

    MemorySegment const& get_segment(int idx) const { return segments[idx]; }

    Tuple<int, int> allocate(int size) {
#if REUSE_SEGMENTS_BEFORE_COMPACT
        // Some memory segment got freed
        for (int i = int(free_list.size()) - 1; i >= 0; i--) {
            int segment_id = free_list[i];

            MemorySegment& segment = segments[segment_id];
            if (segment.used == -1 && segment.size >= size) {
                segment.used = 1;
                free_list.erase(std::remove(free_list.begin(), free_list.end(), segment_id),
                                free_list.end());
                return std::make_tuple(i, segment.version);
            }
        }
#endif

        // we do not have enough memory
        if (raw_memory.size() < free_idx + size) {
            raw_memory.resize(std::max(int(raw_memory.size() * 1.5), 256));
        }

        // we have free space
        if (raw_memory.size() > free_idx + size) {
            if (!zero_list.empty()) {
                // we have zeroed out segments we could reuse
                int idx = (*zero_list.rbegin());
                zero_list.pop_back();
                MemorySegment& segment = segments[idx];
                segment.used           = 1;
                segment.size           = size;
                segment.start          = free_idx;
                return std::make_tuple(idx, 0);
            } else {
                // make a new segment
                int idx = int(segments.size());
                segments.emplace_back(idx, free_idx, size, 0, 1);
                free_idx += size;
                return std::make_tuple(idx, 0);
            }
        }

        // Allocation failed
        return std::make_tuple(-1, -1);
    }

    // Memory can be compacted
    Array<byte> raw_memory;

    // The segments can become fragmented
    Array<MemorySegment> segments;  // Points to the memory segment the object is using
    int                  free_idx = 0;
    // Array of zeroed out segment that can be used for fresh assignment
    Array<int> zero_list;

#if REUSE_SEGMENTS_BEFORE_COMPACT
    // Array of free MemorySegment that could be reused;
    //  when a segment is freed it could be reused if an object of the same size
    //  is allocated right after
    //  during compaction the free_list is cleared
    Array<int> free_list;
#endif
};

template <typename T, bool auto_destroy>
T* PointerProxy<T, auto_destroy>::operator->() {
    if (collector.is_valid(segment_id, version)) {
        return reinterpret_cast<T*>(collector.memory(segment_id));
    }
    throw std::runtime_error("Memory freed");
}

template <typename T, bool auto_destroy>
T& PointerProxy<T, auto_destroy>::operator*() const {
    if (collector.is_valid(segment_id, version)) {
        return *reinterpret_cast<T*>(collector.memory(segment_id));
    }
    throw std::runtime_error("Memory freed");
}

template <typename T, bool auto_destroy>
bool PointerProxy<T, auto_destroy>::is_valid() const {
    return collector.is_valid(segment_id, version);
}

template <typename T, bool auto_destroy>
void PointerProxy<T, auto_destroy>::destroy() {
    if (collector.is_valid(segment_id, version)) {
        collector.free(*this);
    }
}

template <typename T, bool auto_destroy>
T* PointerProxy<T, auto_destroy>::data() {
    if (collector.is_valid(segment_id, version)) {
        return reinterpret_cast<T*>(collector.memory(segment_id));
    }
    return nullptr;
}

}  // namespace lython

#include <catch2/catch_all.hpp>

using namespace lython;

TEST_CASE("Allocator_Concept") {

    GargabeCollector gc;

    std::cout << sizeof(int*) << " vs " << sizeof(PointerProxy<int>) << "\n";

    PointerProxy<int> intptr = gc.malloc<int>(20);

    REQUIRE((*intptr) == 20);

    (*intptr) = 10;

    REQUIRE((*intptr) == 10);

    intptr.destroy();

    REQUIRE_THROWS((*intptr));

    // reused memory segment
    PointerProxy<int> intptr_2 = gc.malloc<int>(20);
    REQUIRE((*intptr_2) == 20);

    auto array = gc.malloc<Array<int>>(20, 0);
    REQUIRE((*array).size() == 20);

    intptr_2.destroy();
    gc.display_memory();

    std::cout << "===================\n";
    std::cout << "Compact\n";
    std::cout << "===================\n";

    gc.compact_memory();
    REQUIRE((*array).size() == 20);
    gc.display_memory();
}

template <typename T>
class CompactingAllocator {
    public:
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer         = PointerProxy<T>;
    using const_pointer   = const PointerProxy<T>;
    using reference       = T&;
    using const_reference = const T&;
    using value_type      = T;

    template <typename _Tp1>
    struct rebind {
        using other = CompactingAllocator<_Tp1>;
    };
    bool operator==(CompactingAllocator const&) const { return true; }

    bool operator!=(CompactingAllocator const& alloc) const { return !(*this == alloc); }

    static void deallocate(pointer p, std::size_t n) { p.destroy(); }

    static pointer allocate(std::size_t n, const void* = nullptr) {
        return GargabeCollector::instance().malloc<T>(n);
    }

    CompactingAllocator() noexcept {}

    CompactingAllocator(const CompactingAllocator& a) noexcept {}

    template <class U>
    CompactingAllocator(const CompactingAllocator<U>& a) noexcept {}

    ~CompactingAllocator() noexcept = default;
};

template <typename V>
using CompactorAlloc = CompactingAllocator<V>;

template <typename V>
using CompArray = std::vector<V, CompactorAlloc<V>>;

TEST_CASE("Allocator_Concept_Compact") {
    CompactorAlloc<int>::allocate(123);

    // does not work
    // CompArray<int> array_test;
}

struct GCObjectHeader {
    size_t size;
    bool   marked : 1;

    // does not work on windows
    // void* data[0];

    void* data() { return ((char*)(this)) + sizeof(GCObjectHeader); }
};

struct BoehmGarbageCollector {
    void* heap_start;
    void* heap_end;

    int is_pointer(void* ptr) {
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

    template <typename T>
    T* malloc() {
        return (T*)malloc(sizeof(T));
    }

    void* malloc(std::size_t size) {
        // TODO handle alignment
        GCObjectHeader* hdr = (GCObjectHeader*)std::malloc(size + sizeof(GCObjectHeader));
        kwdebug(outlog(), "Allocating {} - {}", (void*)(hdr), hdr->data());

        hdr->size   = size;
        hdr->marked = 0;
        allocations.push_back(hdr);
        return hdr->data();
    }

    void free(void* ptr) {
        if (ptr == nullptr) {
            return;
        }
        GCObjectHeader* hdr = header(ptr);
        kwdebug(outlog(), "free {}", (void*)(hdr));
        std::free(hdr);
    }

    void mark(void* obj) { flat_mark(obj); }

    Array<void*> pointers;

    void flat_mark(void* obj) {
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

    bool is_marked(void* ptr) { return header(ptr)->marked; }

    void clear_mark(void* ptr) { header(ptr)->marked = 0; }

    GCObjectHeader* header(void* ptr) {
        return (GCObjectHeader*)((char*)(ptr) - sizeof(GCObjectHeader));
    }

    void collect() { stop_world_collect(); }

    void stop_world_collect() {
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

    void sweep() {
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

    void scan_stack();
    void scan_globals();
    void scan_registers();
    void get_heap_bounds();

    Array<GCObjectHeader*> allocations;

    void dump(std::ostream& out) {
        get_heap_bounds();

        out << "+--> 0 | " << heap_start << "\n";
        for (GCObjectHeader* obj: allocations) {
            auto diff = (char*)(obj) - (char*)(heap_start);
            out << fmt::format("| +-> {} | {}\n", diff, (void*)(obj));
            out << "| | \n";
            out << "| +-> size: " << obj->size << "\n";
        }
        out << "+--> " << (char*)(heap_end) - (char*)(heap_start) << " | " << heap_end << "\n";
    }
};

#if __has_include(<windows.h>)
#ifndef BUILD_WINDOWS
#define BUILD_WINDOWS 1
#endif
#include <intrin.h>
#include <windows.h>
#endif

void BoehmGarbageCollector::scan_registers() {
    void* registers[16];

    CONTEXT context;
    RtlCaptureContext(&context);

    registers[0] = (void*)context.Rax;
    registers[1] = (void*)context.Rbx;
    registers[2] = (void*)context.Rcx;
    registers[3] = (void*)context.Rdx;
    registers[4] = (void*)context.Rsi;
    registers[5] = (void*)context.Rdi;
    registers[6] = (void*)context.Rbp;
    registers[7] = (void*)context.Rsp;
    registers[8] = (void*)context.R8;
    registers[9] = (void*)context.R9;
    registers[10] = (void*)context.R10;
    registers[11] = (void*)context.R11;
    registers[12] = (void*)context.R12;
    registers[13] = (void*)context.R13;
    registers[14] = (void*)context.R14;
    registers[15] = (void*)context.R15;

    // Check each register to see if it contains a pointer
    for (int i = 0; i < 16; ++i) {
        if (is_pointer(registers[i])) {
            mark(registers[i]);
        }
    }
}
void BoehmGarbageCollector::scan_stack() {
#if BUILD_WINDOWS
// Obtain stack bounds for the current thread
#if 0
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(&mbi, &mbi, sizeof(mbi));

    // Stack limits
    void* stack_base = mbi.BaseAddress;
    void* stack_top  = (char*)mbi.BaseAddress + mbi.RegionSize;
#elif 1
    CONTEXT context;
    RtlCaptureContext(&context);
    PNT_TIB pTib = (PNT_TIB)NtCurrentTeb();

    // Use Rsp for stack pointer in x86_64
    void** stack_pointer = (void**)context.Rsp;
    void** stack_base = (void**)pTib->StackBase;
#else

    PNT_TIB pTib = (PNT_TIB)NtCurrentTeb();

    // The stack limits are in the TEB
    void* stack_base = pTib->StackBase;   // High address
    void* stack_top  = pTib->StackLimit;  // Low address
#endif

    // Iterate over stack
    for (void** current = stack_pointer; current < stack_base; ++current) {
        if (is_pointer(*current)) {
            mark(*current);
        }
    }
#else
#endif
}

void BoehmGarbageCollector::scan_globals() {
// i.e root set
#if BUILD_WINDOWS
    // Get a handle to the current module (application)
    HMODULE hModule = GetModuleHandle(NULL);

    // Variables to store section info
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((char*)hModule + dosHeader->e_lfanew);

    // Get the data section
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders);
    for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i, ++section) {
        if (strcmp((char*)section->Name, ".data") == 0 ||
            strcmp((char*)section->Name, ".bss") == 0) {
            void* start = (char*)hModule + section->VirtualAddress;
            void* end   = (char*)start + section->Misc.VirtualSize;

            // Scan each potential pointer location
            for (void** current = (void**)start; current < (void**)end; ++current) {
                if (is_pointer(*current)) {
                    // Mark object as reachable in GC
                    mark(*current);
                }
            }
        }
    }
#else
#endif
}

void BoehmGarbageCollector::get_heap_bounds() {
#if BUILD_WINDOWS
    HANDLE             heap = GetProcessHeap();  // Get the default process heap
    PROCESS_HEAP_ENTRY entry;
    entry.lpData = NULL;

    SIZE_T lowestAddress  = (SIZE_T)-1;  // Start with the highest possible address
    SIZE_T highestAddress = 0;           // Start with the lowest possible address

    while (HeapWalk(heap, &entry) != 0) {
        if (entry.wFlags & PROCESS_HEAP_ENTRY_BUSY) {  // Check if this entry is used
            SIZE_T start = (SIZE_T)entry.lpData;
            SIZE_T end   = start + entry.cbData;

            // Update the lowest and highest addresses
            if (start < lowestAddress) {
                lowestAddress = start;
            }
            if (end > highestAddress) {
                highestAddress = end;
            }
        }
    }

    heap_start = (void*)lowestAddress;
    heap_end   = (void*)highestAddress;

    kwdebug(outlog(),
            "heap {} - {} = {}",
            heap_start,
            heap_end,
            (char*)(heap_end) - (char*)(heap_start));
#else
#endif
}

struct ListBool {
    bool      val;
    ListBool* next = nullptr;
};



ListBool* make_list(BoehmGarbageCollector& gc, bool val, ListBool* prev) {
    ListBool* lst = gc.malloc<ListBool>();
    lst->val      = val;
    lst->next     = prev;
    return lst;
}

#include <setjmp.h>

TEST_CASE("BoehmGarbageCollector_Marks recursively find pointers") {
    BoehmGarbageCollector gc;

    ListBool* n5 = make_list(
        gc,
        true,
        make_list(
            gc, true, make_list(gc, true, make_list(gc, true, make_list(gc, true, nullptr)))));

    gc.mark(n5);

    int count = 0;
    for (GCObjectHeader* hdr: gc.allocations) {
        count += hdr->marked;
    }

    REQUIRE(count == 5);
}

ListBool* _global = nullptr;

TEST_CASE("BoehmGarbageCollector_Globals") {
    BoehmGarbageCollector gc;

    _global = make_list(gc, true, nullptr);

    REQUIRE(gc.allocations.size() == 1);
}


TEST_CASE("BoehmGarbageCollector_AllReachable") {
    BoehmGarbageCollector gc;

    ListBool* n1 = make_list(gc, true, nullptr);
    ListBool* n2 = make_list(gc, true, n1);
    ListBool* n3 = make_list(gc, true, n2);
    ListBool* n4 = make_list(gc, true, n3);
    ListBool* n5 = make_list(gc, true, n4);

    // why is it collecting pointers
    gc.dump(std::cout);
    gc.collect();
    gc.dump(std::cout);

    REQUIRE(gc.allocations.size() == 5);
}

TEST_CASE("BoehmGarbageCollector_AllReachable_Nested") {
    BoehmGarbageCollector gc;

    ListBool* n5 = make_list(
        gc,
        true,
        make_list(
            gc, true, make_list(gc, true, make_list(gc, true, make_list(gc, true, nullptr)))));

    gc.dump(std::cout);
    gc.collect();
    gc.dump(std::cout);

    REQUIRE(gc.allocations.size() == 5);
}
