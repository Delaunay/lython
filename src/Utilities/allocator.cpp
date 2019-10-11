#include <iostream>

#include "allocator.h"
#include "Types.h"

namespace lython {

void init_names(){
    #define INIT_METADATA(name, typname)\
        type_name<name>();

    TYPES_METADATA(INIT_METADATA)
}

void show_alloc_stats(){
    init_names();

    std::vector<std::pair<int, int>> const& stat = stats();
    std::unordered_map<int, std::string> const& names = typenames();

    std::cout << "id\t Name \t" << "Allo" << "\t" << "Deall" << "\tFree\n";
    for(size_t i = 0; i < stat.size(); ++i){
        std::string name = "";
        try {
            name = names.at(int(i));
        } catch (std::out_of_range&){

        };

        auto alloc = stat[i].first;
        auto dealloc = stat[i].second;

        std::cout << i << "\t" << name << "\t" << alloc << "\t" << dealloc << "\t" << alloc - dealloc << "\n";
    }
}

} // namespace lython
