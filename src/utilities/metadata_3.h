#pragma once

namespace lython {

namespace meta {


template<typename T>
void print(std::ostream& ss, T const& value) {
    ClassMetadata const& registry = classmeta<T>();

    if (registry.printer) {
        registry.printer(ss, reinterpret_cast<std::int8_t const*>(&value));
        return;
    }   

    ss << type_name<T>() << "(";
    bool is_first = true;
    for(Member const& mem: registry.members) {
        if (mem.method != nullptr) {
            continue;
        }

        if (!is_first) {
            ss << ", ";
        }
        is_first = false;

        ss << mem.name << "=";

        print(ss, mem.type, member_address(&value, mem));
    }
    ss << ")";
};
}

}