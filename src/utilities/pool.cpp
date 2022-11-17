#include "pool.h"
#include "logging/logging.h"
#include <iostream>

namespace lython {

#if BUILD_WEBASSEMBLY
#else

void worker_loop(ThreadPool* pool, std::size_t n);

ThreadPool::ThreadPool(std::size_t thread_count) {
    tasks.reserve(128);
    stats.reserve(thread_count);
    threads.reserve(thread_count);

    for (std::size_t i = 0; i < thread_count; ++i) {
        insert_worker();
    }
}

std::optional<ThreadPool::Task_t> ThreadPool::pop() {
    std::lock_guard       lock(mux);
    std::optional<Task_t> task;

    if (tasks.size() > 0) {
        task = *(tasks.end() - 1);
        tasks.pop_back();
    }

    return task;
}

void ThreadPool::insert_worker() {
    std::size_t n = threads.size();
    stats.emplace_back();
    threads.emplace_back(worker_loop, this, n);
}

void ThreadPool::shutdown(bool wait) {
    for (auto& state: stats) {
        state.running = false;
    }

    if (wait) {
        for (auto& thread: threads) {
            thread.join();
        }
    }

    threads.erase(std::begin(threads), std::end(threads));
    stats.erase(std::begin(stats), std::end(stats));
}

std::size_t ThreadPool::size() const { return threads.size(); }

std::ostream& ThreadPool::print(std::ostream& out) const {
    auto end         = StopWatch<>::Clock::now();
    int  total_tasks = 0;

    out << fmt::format("| {:4} | {:6} | {:4} | {} |\n", "#id", "busy%", "task", "sleep");
    out << "|------+--------+------+-------|\n";

    for (std::size_t i = 0; i < size(); ++i) {
        Stat_t const& stat  = stats[i];
        auto          total = float(StopWatch<>::diff(stat.start, end));
        auto          busy  = stat.work_time * 100 / total;

        total_tasks += stat.task;

        out << fmt::format("| {:4} | {:6.2f} | {:4} | {:5} |\n", i, busy, stat.task, stat.sleeping);
    }

    out << fmt::format("    Total Tasks: {}\n", total_tasks);
    out << fmt::format("Remaining Tasks: {}\n", tasks.size());
    return out;
}

void worker_loop(ThreadPool* pool, std::size_t n) {
    pool->stats[n].start = StopWatch<>::Clock::now();

    while (pool->stats[n].running) {
        auto maybe_task = pool->pop();

        if (maybe_task.has_value()) {
            pool->stats[n].sleeping = false;
            StopWatch<> chrono;

            std::function<void()> task = maybe_task.value();
            task();

            pool->stats[n].work_time += float(chrono.stop());
            pool->stats[n].task += 1;
            pool->stats[n].sleeping = true;
        } else {
            std::this_thread::yield();
        }
    }
}

#endif
}  // namespace lython
