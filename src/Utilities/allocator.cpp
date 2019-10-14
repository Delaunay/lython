#include <iostream>

#include "allocator.h"
#include "Types.h"
#include "fmt.h"

#include "metadata.h"

namespace lython {


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

        };

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
