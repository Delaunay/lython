#ifndef LYTHON_UTILITIES_CHRONO_HEADER
#define LYTHON_UTILITIES_CHRONO_HEADER

#include <chrono>
#include <ratio>

namespace lython {
template <typename T = double, typename Unit = std::chrono::milliseconds>
class StopWatch {
    public:
    using TimePoint = std::chrono::high_resolution_clock::time_point;
    using Clock     = std::chrono::high_resolution_clock;
    using Duration  = std::chrono::duration<T>;

    TimePoint const start = Clock::now();

    double stop() const {
        TimePoint end = Clock::now();
        return StopWatch::diff(start, end);
    }

    static double diff(TimePoint start, TimePoint end) {
        Duration time_delta = end - start;
        auto     delta      = std::chrono::duration_cast<Unit>(time_delta);
        return double(delta.count());
    }

    StopWatch operator=(StopWatch p) { return StopWatch(p); }

    StopWatch(const StopWatch& p): start(p.start) {}

    StopWatch() = default;
};

}  // namespace lython

#endif
