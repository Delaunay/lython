
#include "dtypes.h"
#include "utilities/stopwatch.h"

#include <cmath>
#include <iostream>

#define XXH_VECTOR XXH_AVX2

#include "xxh3.h"
#include "xxhash.c"
#include "xxhash.h"

namespace lython {
std::size_t old_hash(String& k) noexcept {
    auto a = std::hash<std::string>();
    return a(std::string(k.c_str()));
}

std::size_t new_hash(String& k) noexcept { return std::_Hash_impl::hash(k.data(), k.length()); }

std::size_t xx_hash_32(String& k) noexcept { return XXH32(k.data(), k.length(), 0); }
std::size_t xx_hash_64(String& k) noexcept { return XXH64(k.data(), k.length(), 0); }
std::size_t xx_hash_3(String& k) noexcept { return XXH3_64bits(k.data(), k.length()); }

template <typename T = double>
struct ValueStream {

    void add(T value) {
        sum += value;
        count += 1;
        sum_squared += value * value;
    }

    double mean() const { return sum / double(count); }

    double var() const {
        double m = mean();
        return sum_squared / double(count) - m * m;
    }

    double total() const { return sum; }

    double std() const { return sqrt(var()); }

    T   sum;
    T   sum_squared;
    int count;
};

template <class T>
void fakeuse(T&& datum) {
    asm volatile("" : "+r"(datum));
}

struct Benchmark {
    Benchmark(std::string const&    name,
              std::function<void()> function,
              int                   count  = 100,
              int                   repeat = 100000):
        name(name),
        function(function), count(count), repeat(repeat) {}

    void run() {
        for (int i = 0; i < count; i++) {
            StopWatch<double, std::chrono::milliseconds> time;

            for (int j = 0; j < repeat; j++) {
                function();
            }
            val.add(time.stop());
        }
    }

    void report(std::ostream& out) {
        out << fmt::format(
            "{:>30} | {:10.3f} | {:10.3f} | {:10.3f} \n", name, val.mean(), val.std(), val.total());
    }

    std::string           name;
    std::function<void()> function;
    ValueStream<double>   val;
    int                   count;
    int                   repeat;
};

struct Compare {
    Compare(std::vector<Benchmark> const& benchs, int count = 100, int repeat = 100000):
        benchmarks(benchs), count(count), repeat(repeat) {}

    void run(std::ostream& out) {
        int i = 0;

        for (Benchmark& bench: benchmarks) {
            progress(out, i);

            bench.count  = count;
            bench.repeat = repeat;
            bench.run();

            i += 1;
        }

        progress(out, i);
    }

    void progress(std::ostream& out, int i) {
        out << fmt::format(
            "{:6.2f} % {}/{}\n", float(i) / float(benchmarks.size()), i, benchmarks.size());
    }

    void report(std::ostream& out) {
        out << fmt::format(
            "{:>30} | {:>10} | {:>10} | {:>10} \n", "bench", "mean (ms)", "std (ms)", "total (ms)");
        out << "---------------------------------------------------------------------\n";

        for (Benchmark& bench: benchmarks) {
            bench.report(out);
        }
    }

    std::vector<Benchmark> benchmarks;
    int                    count;
    int                    repeat;
};

}  // namespace lython

// check https://github.com/google/benchmark

int main() {
    lython::String string = "owjfopwejfpwejfopwejfpwoejfpwef";

    // but we need to check for collision too

    // clang-format off
    auto comp = lython::Compare({
        lython::Benchmark("OLD HASH", [&string]() {
            //
            lython::fakeuse(old_hash(string));
        }),
        lython::Benchmark("NEW HASH", [&string]() {
            //
            lython::fakeuse(new_hash(string));
        }),
        lython::Benchmark("XX HASH 32", [&string]() {
            //
            lython::fakeuse(xx_hash_32(string));
        }),
        lython::Benchmark("XX HASH 64", [&string]() {
            //
            lython::fakeuse(xx_hash_64(string));
        })
        ,
        lython::Benchmark("XX HASH 3", [&string]() {
            //
            lython::fakeuse(xx_hash_3(string));
        })
    });
    // clang-format on

    comp.run(std::cout);
    comp.report(std::cout);

    return 0;
}