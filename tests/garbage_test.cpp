#include "dtypes.h"
// #include "vm/garbage_collector.h"



#define WITH_LOG 1
#include "stdlib/garbage.h"
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

struct ListBool {
    int      val;
    ListBool* next = nullptr;
};

ListBool* make_list(BoehmGarbageCollector& gc, int val, ListBool* prev) {
    ListBool* lst = gc.malloc<ListBool>(LOC);
    lst->val      = val;
    lst->next     = prev;
    return lst;
}

void KIWI_NOINLINE print(ListBool* list) {
    int i = 0;
    while(list != nullptr) {
        std::cout << "(" << i << ": " << list->val << ") ";
        list = list->next;
        i += 1;
    }
    std::cout << "\n";
}


#define reset_frame()

#define reset_frame__() {                       \
    int size = 1024 * 2;                        \
    volatile char* data = (char*) alloca(size); \
    memset((void*)data, size, 0);               \
}truetrue

KIWI_NOINLINE char reset_frame_(int size=1024) {
    volatile char* data = (char*) alloca(size);
    memset((void*)data, size, 0);

    volatile char* data2 = (char*) malloc(size);
    memset((void*)data2, size, 0);

    return data[size - 1] + data2[size - 1];
}


KIWI_NOINLINE ListBool* test_all_there(BoehmGarbageCollector& gc) {
    ListBool* n1 = make_list(gc, 1, nullptr);
    ListBool* n2 = make_list(gc, 2, n1);
    ListBool* n3 = make_list(gc, 3, n2);
    ListBool* n4 = make_list(gc, 4, n3);
    ListBool* n5 = make_list(gc, 5, n4);

    // need to return it else it might not be in the stack anymore
    return n5;
}

KIWI_NOINLINE ListBool* test_only_two(BoehmGarbageCollector& gc) {
    ListBool* n1 = gc.named(make_list(gc, 1, nullptr), "n1");
    ListBool* n2 = gc.named(make_list(gc, 2, n1), "n2");

    // Orphans
    ListBool* n3 = gc.named(make_list(gc, 3, nullptr), "n3");
    ListBool* n4 = gc.named(make_list(gc, 4, nullptr), "n4");
    ListBool* n5 = gc.named(make_list(gc, 5, nullptr), "n5");

    n3 = nullptr;
    n4 = nullptr;
    n5 = nullptr;
    return n2;
}

KIWI_NOINLINE ListBool* test_nested(BoehmGarbageCollector& gc) {
    ListBool* n5 = make_list(
        gc,
        1,
        make_list(
            gc, 2, make_list(gc, 3, make_list(gc, 4, make_list(gc, 5, nullptr)))));

    return n5;
}

KIWI_NOINLINE ListBool* test_relocated(BoehmGarbageCollector& gc) {

    ListBool* n5 = 
        gc.named(make_list(gc, 1,
        gc.named(make_list(gc, 2, 
        gc.named(make_list(gc, 3, 
        gc.named(make_list(gc, 4, 
        gc.named(make_list(gc, 5, nullptr), "n1")), "n2")), "n3")), "n4")), "n5");


    auto replace = [&](void* obj, void* replacement) {
        Array<void*> pointers = {obj};

        while (!pointers.empty()) {
            void* ptr = *pointers.rbegin();
            pointers.pop_back();

            if (ptr == nullptr || gc.is_marked(ptr)) {
                continue;
            }

            GCObjectHeader* hdr = gc.header(ptr);

            for (int i = 0; i < hdr->size; i++) {
                void** member = reinterpret_cast<void**>(ptr) + i;
                if (gc.is_pointer(GCGen::Temporary, *member)) {
                    member[0] = replacement;
                }
            }
        }
    };

    print(n5); printf("\n");
    replace(n5, gc.named(make_list(gc, 6, nullptr), "n6"));
    print(n5); printf("\n");
    
    return n5;
}


TEST_CASE("BoehmGarbageCollector_AllReachable") {
    BoehmGarbageCollector gc;

    auto l = test_all_there(gc);

    reset_frame();      // try to clean the stack
    gc.collect();       // Garbage collect
    print(l);           // print list because it should still be reachable
    gc.dump(std::cout);

    REQUIRE(gc.allocations(GCGen::Temporary).size() == 5);
}


TEST_CASE("BoehmGarbageCollector_AllReachable_Nested") {
    std::cout << "Test: "<< sizeof(ListBool) << std::endl;
    BoehmGarbageCollector gc;

    auto l = test_nested(gc);

    reset_frame();
    gc.collect();
    print(l);
    gc.dump(std::cout);

    REQUIRE(gc.allocations(GCGen::Temporary).size() == 5);
}


TEST_CASE("BoehmGarbageCollector_Collect_Garbage") {
    BoehmGarbageCollector gc;

    auto l = test_only_two(gc);
    REQUIRE(gc.allocations(GCGen::Temporary).size() == 5);

    reset_frame();
    gc.collect();
    print(l);
    gc.dump(std::cout);

    REQUIRE(gc.allocations(GCGen::Temporary).size() == 2);
}


TEST_CASE("BoehmGarbageCollector_Relocate") {
    BoehmGarbageCollector gc;

    ListBool* l = test_relocated(gc);

    reset_frame();
    gc.collect();
    print(l);
    gc.dump(std::cout);

    // 6 allocations
    // only 2 should remain reachable
    REQUIRE(gc.allocations(GCGen::Temporary).size() == 2);
}


TEST_CASE("BoehmGarbageCollector_Marks recursively find pointers") {
    BoehmGarbageCollector gc;

    ListBool* n5 = make_list(
        gc,
        true,
        make_list(
            gc, true, make_list(gc, true, make_list(gc, true, make_list(gc, true, nullptr)))));

    gc.mark_obj(n5);

    int count = 0;
    for (GCObjectHeader* hdr: gc.allocations(GCGen::Temporary)) {
        count += hdr->marked;
    }

    REQUIRE(count == 5);
}


ListBool* _global = nullptr;
TEST_CASE("BoehmGarbageCollector_Globals") {
    BoehmGarbageCollector gc;

    _global = make_list(gc, true, nullptr);

    gc.collect();
    gc.dump(std::cout);

    REQUIRE(gc.allocations(GCGen::Temporary).size() == 1);

}