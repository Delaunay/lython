#include <iostream>

#include "ast/values/value.h"

using uint8  = std::uint8_t;
using int8   = std::int8_t;
using uint32 = std::uint32_t;
using int32  = std::int32_t;
using uint16 = std::uint16_t;
using int16  = std::int16_t;

template <typename T>
struct range_struct {
    T i;
    T start;
    T stop;
    T step;
    enum
    {
        At_start,
        In_loop,
        Done
    } state;

    explicit range_struct(int stop): start(0), stop(stop), step(1), state(At_start) {}

    explicit range_struct(int start, int stop, int step = 1):
        start(start), stop(stop), step(step), state(At_start) {}

    int resume() {
        switch (state) {
        case At_start:
            for (i = start; i < stop;) {
                state = In_loop;
                return i;
            case In_loop: i += step;
            }
            state = Done;
        case Done: return 0;
        }
    }

    struct end_iterator {};
    struct iterator {
        range_struct* self;

        bool operator!=(const end_iterator& end) const { return !((*this) == end); }
        bool operator==(const end_iterator&) const { return self->state == Done; }
        void operator++() { self->resume(); }
        int  operator*() const { return self->i; }
    };

    iterator begin() {
        resume();
        return {this};
    }
    end_iterator end() { return {}; }
};

namespace lython {
struct Iterator {
    Value state;
    Value function;
};

Value resumable(void* rtc, void* ctx, Array<Value>& vals) {}
};  // namespace lython

int main() {
    std::cout << sizeof(range_struct<uint8>) << std::endl;
    std::cout << sizeof(range_struct<uint16>) << std::endl;
    std::cout << sizeof(range_struct<uint32>) << std::endl;
    std::cout << "\n";

    for (int i: range_struct<uint8>(2, 10)) {
        std::cout << i << std::endl;
    }

    return 0;
}
