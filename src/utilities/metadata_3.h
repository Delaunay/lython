#pragma once

namespace lython {

namespace meta {

void print(std::ostream& out, int type_id, void* obj);

template<typename T>
void print(std::ostream& ss, T& value) {
    void* obj = (void*)(&value);
    print(ss, type_id<T>(), obj);
};
}

}