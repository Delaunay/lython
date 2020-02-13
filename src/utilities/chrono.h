#include <chrono>
#include <ratio>

class TimeIt {
  public:
    using TimePoint       = std::chrono::high_resolution_clock::time_point;
    using Clock           = std::chrono::high_resolution_clock;
    TimePoint const start = Clock::now();

    double stop() const {
        TimePoint end = Clock::now();
        return TimeIt::diff(start, end);
    }

    static double diff(TimePoint start, TimePoint end){
        std::chrono::duration<double> time_span =
            std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        return time_span.count();
    }

    TimeIt operator=(TimeIt p){
        return TimeIt(p);
    }

    TimeIt(const TimeIt& p):
        start(p.start)
    {}

    TimeIt() = default;
};
