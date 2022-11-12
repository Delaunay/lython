
#include "bench.h"

#include <iostream>
#include <random>

#define XXH_VECTOR XXH_AVX2

// only from Zen 4
// #define XXH_VECTOR XXH_AVX512

#include "xxh3.h"
#include "xxhash.c"
#include "xxhash.h"

using namespace lython;

std::size_t old_hash(String const& k) noexcept {
    auto a = std::hash<std::string>();
    return a(std::string(k.c_str()));
}

std::size_t new_hash(String const& k) noexcept {
    return std::_Hash_impl::hash(k.data(), k.length());
}
std::size_t xx_hash_32(String const& k) noexcept { return XXH32(k.data(), k.length(), 0); }
std::size_t xx_hash_64(String const& k) noexcept { return XXH64(k.data(), k.length(), 0); }
std::size_t xx_hash_3(String const& k) noexcept { return XXH3_64bits(k.data(), k.length()); }

// check https://github.com/google/benchmark

lython::String make_string(size_t size = 64) {
    static String                                                   str(size, ' ');
    static std::random_device                                       dev;
    static std::mt19937                                             rng(dev());
    static std::uniform_int_distribution<std::mt19937::result_type> uniform(0, 256);

    str.resize(size);

    for (int i = 0; i < size; i++) {
        str[i] = uniform(rng);
    }

    return str;
}

int main() {
    lython::String string = "owjfopwejfpwejfopwejfpwoejfpwef";

    // but we need to check for collision too

    make_string(64);

    // clang-format off
    auto comp = lython::Compare<int>({
        lython::Benchmark<int>("OLD HASH", [](int size) {
            //
            lython::fakeuse(old_hash(make_string(size)));
        }),
        lython::Benchmark<int>("NEW HASH", [](int size) {
            //
            lython::fakeuse(new_hash(make_string(size)));
        }),
        lython::Benchmark<int>("XX HASH 32", [](int size) {
            //
            lython::fakeuse(xx_hash_32(make_string(size)));
        }),
        lython::Benchmark<int>("XX HASH 64", [](int size) {
            //
            lython::fakeuse(xx_hash_64(make_string(size)));
        }),
        lython::Benchmark<int>("XX HASH 3", [](int size) {
            //
            lython::fakeuse(xx_hash_3(make_string(size)));
        })
    }, 100, 10000);
    // clang-format on

    comp.add_setup(8);
    comp.add_setup(16);
    comp.add_setup(32);
    comp.add_setup(64);

    comp.run(std::cout);
    comp.report(std::cout);

    return 0;
}