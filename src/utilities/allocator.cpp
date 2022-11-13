#include <cstdlib>
#include <iostream>
#include <spdlog/fmt/bundled/core.h>

#include "dtypes.h"

#include "utilities/allocator.h"
#include "utilities/metadata.h"

#define DISABLE_ALIGNED_ALLOC 0
#define ALIGNMENT             32

#if !((defined WITH_VALGRIND) && WITH_VALGRIND)
#    define USE_MIMALLOC 1
#else
#    define USE_MIMALLOC 0
#endif

#if USE_MIMALLOC
#    include <mimalloc.h>
#endif

namespace lython {
namespace meta {
bool& is_type_registry_available() {
    static bool avail = false;
    return avail;
}

}  // namespace meta
namespace device {

void* CPU::malloc(std::size_t n) {
#if USE_MIMALLOC
    return std::malloc(n);

    // return mi_malloc(n);
    // return mi_malloc_aligned(n, ALIGNMENT);
#elif DISABLE_ALIGNED_ALLOC
    return std::malloc(n);
#else
    // TODO: seems 64bit alignment might be better (this is what tensorflow is using)
    // but I have not found an official document stating so
    static std::size_t alignment = ALIGNMENT;

    // 16-byte aligned.
    void* original = std::malloc(n + alignment);

    if (original == nullptr)
        return nullptr;

    // alignment is a power of 2 (16, 32, 64)
    //            a  = 0001 0000 = 16
    //        a - 1  = 0000 1111
    //     ~ (a - 1) = 1111 0000
    // b & ~ (a - 1) = Keep the top most ones 0 out the rest (i.e) get the closest power of two
    std::size_t cp2 = reinterpret_cast<std::size_t>(original) & ~(std::size_t(alignment - 1));

    // add alignment to it to get a memory address that is inside our allocation & aligned
    void* aligned = reinterpret_cast<void*>(cp2 + alignment);

    // store original pointer before the aligned address for deletion
    *(reinterpret_cast<void**>(aligned) - 1) = original;

    return aligned;
#endif
}

bool CPU::free(void* ptr, std::size_t) {
#if USE_MIMALLOC
    std::free(ptr);

    // mi_free(ptr);
    // mi_free_aligned(ptr, ALIGNMENT);
    return true;
#elif DISABLE_ALIGNED_ALLOC
    std::free(ptr);
    return true;
#else
    if (ptr) {
        std::free(*(reinterpret_cast<void**>(ptr) - 1));
    }
    return true;
#endif
}

}  // namespace device

void show_alloc_stats() {
    metadata_init_names();

    auto const&                                 stat  = meta::stats();
    std::unordered_map<int, std::string> const& names = meta::typenames();

    auto line = String(4 + 40 + 10 + 10 + 10 + 10 + 10 + 10 + 7 + 1, '-');

    std::cout << line << '\n';
    std::cout << fmt::format("{:>4} {:>41} {:>10} {:>10} {:>10} {:>10} {:>10} {:>10}\n",
                             "id",
                             "name",
                             "alloc",
                             "dealloc",
                             "remain",
                             "size",
                             "size_free",
                             "bytes");

    std::cout << line << '\n';
    int total = 0;

    for (size_t i = 0; i < stat.size(); ++i) {
        std::string name = "";
        try {
            name = names.at(int(i));
        } catch (std::out_of_range&) { name = ""; }

        auto init      = stat[i].startup_count;
        auto alloc     = stat[i].allocated - init;
        auto dealloc   = stat[i].deallocated;
        auto size      = stat[i].size_alloc;
        auto size_free = stat[i].size_free;
        auto bytes     = stat[i].bytes;

        total += size * bytes;

        if (alloc != 0) {
            std::cout << fmt::format("{:>4} {:>41} {:>10} {:>10} {:>10} {:>10} {:>10} {:>10}\n",
                                     i,
                                     String(name.c_str()),
                                     alloc,
                                     dealloc,
                                     alloc - dealloc,
                                     size,
                                     size_free,
                                     bytes);
        }
    }
    std::cout << "Total: " << total << std::endl;
    std::cout << line << '\n';

    std::cout
        << "NB: Notice that not everything was `freed`, this is because the accounting happens "
           "before the static variables gets released.\n"
           "which means it does not necessarily means there is a memory leak.\n"
           "use valgrind to make sure everything is released properly.\n"
           "\n"
           "* Pair[String, NativeBinaryOp]: Native operator, allocated once using static\n"
           "* Pair[StringView, size_t]: From the string database, allocated once using static\n"
           "* Constant: builtin constant created once using static\n"
           "\n----\n";
}

}  // namespace lython
