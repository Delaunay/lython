#pragma once

#include "dtypes.h"
#include "utilities/stopwatch.h"

#include <cmath>
#include <tuple>

namespace lython {

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

    T   sum         = 0;
    T   sum_squared = 0;
    int count       = 0;
};

template <class T>
void fakeuse(T&& datum) {
    asm volatile("" : "+r"(datum));
}

template <typename... Args>
struct Benchmark {
    Benchmark(std::string const&           name,
              std::function<void(Args...)> function,
              int                          count  = 100,
              int                          repeat = 100000):
        name(name),
        function(function), count(count), repeat(repeat) {}

    void run(Args... args) {
        val = ValueStream<double>();

        for (int i = 0; i < count; i++) {
            StopWatch<double, std::chrono::milliseconds> time;

            for (int j = 0; j < repeat; j++) {
                function(args...);
            }
            val.add(time.stop());
        }
    }

    void report(std::ostream& out) {
        out << fmt::format(
            "{:>30} | {:10.3f} | {:10.3f} | {:10.3f} \n", name, val.mean(), val.std(), val.total());
    }

    std::string                  name;
    std::function<void(Args...)> function;
    ValueStream<double>          val;
    int                          count;
    int                          repeat;
};

template <typename... Args>
std::ostream& print(std::ostream& out, Args... args) {
    print(out, args...);
    return out;
}

template <typename... Args, typename T, typename V>
std::ostream& print(std::ostream& out, T val, V v, Args... args) {
    print(out, v, args...);
    out << val << "-";
    return out;
}

template <typename... Args, typename T>
std::ostream& print(std::ostream& out, T val, Args... args) {
    print(out, args...);
    out << val;
    return out;
}

template <>
std::ostream& print(std::ostream& out) {
    return out;
}

template <typename... Args>
struct Compare {
    Compare(std::vector<Benchmark<Args...>> const& benchs, int count = 100, int repeat = 100000):
        benchmarks(benchs), count(count), repeat(repeat) {}

    void run(std::ostream& out) {
        int i = 0;

        for (std::tuple<Args...>& args: setups) {
            for (Benchmark<Args...>& bench: benchmarks) {
                progress(out, i);

                bench.count  = count;
                bench.repeat = repeat;

                std::apply([&bench](auto&&... args) { bench.run(args...); }, args);

                std::stringstream ss;
                ss << bench.name << " (";
                std::apply([&ss](auto&&... args) { print(ss, args...); }, args);
                ss << ")";

                results.emplace_back(ss.str(), bench.val);
                i += 1;
            }
        }

        progress(out, i);
    }

    void add_setup(Args... args) { setups.push_back(std::make_tuple(args...)); }

    void progress(std::ostream& out, int i) {
        float n = total();

        out << fmt::format("{:6.2f} % {}/{}\n", float(i) * 100.0 / n, i, n);
    }

    float total() { return float(benchmarks.size()) * float(setups.size()); }

    void report(std::ostream& out) {
        out << fmt::format(
            "{:>30} | {:>10} | {:>10} | {:>10} \n", "bench", "mean (ms)", "std (ms)", "total (ms)");
        out << "---------------------------------------------------------------------\n";

        for (auto const& bench: results) {
            report(out, bench);
        }
    }

    void as_csv(std::ostream& out) {
        out << "bench,mean (ms),std (ms),total (ms)";
        for (auto const& bench: results) {
            as_csv(out, bench);
        }
    }

    private:
    void report(std::ostream& out, std::tuple<String, ValueStream<double>> const& result) {
        String              name = std::get<0>(result);
        ValueStream<double> val  = std::get<1>(result);
        out << fmt::format(
            "{:>30} | {:10.3f} | {:10.3f} | {:10.3f} \n", name, val.mean(), val.std(), val.total());
    }

    void as_csv(std::ostream& out, std::tuple<String, ValueStream<double>> const& result) {
        String              name = std::get<0>(result);
        ValueStream<double> val  = std::get<1>(result);
        out << fmt::format("{},{},{},{}\n", name, val.mean(), val.std(), val.total());
    }

    std::vector<std::tuple<String, ValueStream<double>>> results;
    std::vector<std::tuple<Args...>>                     setups;
    std::vector<Benchmark<Args...>>                      benchmarks;
    int                                                  count;
    int                                                  repeat;
    int                                                  col_size = 0;
};

}  // namespace lython