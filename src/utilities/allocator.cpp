#include <iostream>
#include <cstdlib>

#include "dtypes.h"
#include "fmt.h"

#include "utilities/allocator.h"
#include "utilities/metadata.h"

namespace lython {
namespace device{
void* CPU::malloc(std::size_t n){
    // TODO: seems 64bit alignment might be better (this is what tensorflow is using)
    // but I have not found an official document stating so
    static std::size_t alignment = 16;

    // 16-byte aligned.
    void *original = std::malloc(n + alignment);

    if (original == nullptr)
        return nullptr;

    // alignment is a power of 2 (16, 32, 64)
    //            a  = 0001 0000 = 16
    //        a - 1  = 0000 1111
    //     ~ (a - 1) = 1111 0000
    // b & ~ (a - 1) = Keep the top most ones 0 out the rest (i.e) get the closest power of two
    std::size_t cp2 = reinterpret_cast<std::size_t>(original) & ~(std::size_t(alignment - 1));

    // add alignment to it to get a memory address that is inside our allocation & aligned
    void *aligned = reinterpret_cast<void *>(cp2 + alignment);

    // store original pointer before the aligned address for deletion
    *(reinterpret_cast<void**>(aligned) - 1) = original;

    return aligned;
}

bool CPU::free(void* ptr, std::size_t){
    if (ptr){
        std::free(*(reinterpret_cast<void**>(ptr) - 1));
    }
    return true;
}

} // namespace device


void show_alloc_stats(){
    metadata_init_names();

    std::vector<std::pair<int, int>> const& stat = stats();
    std::unordered_map<int, std::string> const& names = typenames();

    auto line = String(4 + 40 + 10 + 10 + 10, '-');

    std::cout << line << '\n'
        << align_right("id", 4)
        << align_right("name", 40)
        << align_right("alloc", 10)
        << align_right("dealloc", 10)
        << align_right("remain", 10) << std::endl;

    std::cout << line << '\n';

    for(size_t i = 0; i < stat.size(); ++i){
        std::string name = "";
        try {
            name = names.at(int(i));
        } catch (std::out_of_range&){

        }

        auto alloc = stat[i].first;
        auto dealloc = stat[i].second;

        std::cout
            << to_string(i, 4)
            << align_right(String(name.c_str()), 40)
            << to_string(alloc, 10)
            << to_string(dealloc, 10)
            << to_string(alloc - dealloc, 10) << std::endl;
    }
    std::cout << line << '\n';
}

} // namespace lython

