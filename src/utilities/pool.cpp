#include "pool.h"
#include <fmt/core.h>

namespace lython {

void worker_loop(ThreadPool *pool, std::size_t n);

ThreadPool::ThreadPool(std::size_t thread_count){
    tasks.reserve(128);
    stats.reserve(thread_count);
    threads.reserve(thread_count);

    for(std::size_t i = 0; i < thread_count; ++i){
        insert_worker();
    }
}


std::optional<ThreadPool::Task_t> ThreadPool::pop(){
    std::lock_guard lock(mux);
    std::optional<Task_t> task;

    if (tasks.size() > 0){
        task = *(tasks.end() - 1);
        tasks.pop_back();
    }

    return task;
}

void ThreadPool::insert_worker(){
    std::size_t n = threads.size();
    stats.emplace_back();
    threads.emplace_back(worker_loop, this, n);
}

void ThreadPool::shutdown(bool wait){
    for(auto& state: stats){
        state.running = false;
    }

    if (wait){
        for(auto& thread: threads){
            thread.join();
        }
    }

    threads.erase(std::begin(threads), std::end(threads));
    stats.erase(std::begin(stats), std::end(stats));
}

std::size_t ThreadPool::size() const {
    return threads.size();
}

std::ostream& ThreadPool::print(std::ostream& out) const{
    auto end = TimeIt::Clock::now();

    out << fmt::format("| {:4} | {:5} | {:4} |\n", "#id", "busy%", "task");
    out <<             "|------+-------+------|\n";

    for(std::size_t i = 0; i < size(); ++i){
        Stat_t const& stat = stats[i];
        auto total = TimeIt::diff(stat.start, end);

        out << fmt::format("| {:4} | {:5} | {:4} |\n", i, stat.work_time / total, stat.task);
    }

    return out;
}

void worker_loop(ThreadPool *pool, std::size_t n){
    pool->stats[n].start = TimeIt::Clock::now();

    while (pool->stats[n].running){
        auto maybe_task = pool->pop();

        if (maybe_task.has_value()){
            TimeIt chrono;

            std::function<void()> task = maybe_task.value();
            task();

            pool->stats[n].work_time += chrono.stop();
            pool->stats[n].task += 1;
        } else {
            std::this_thread::yield();
        }
    }
}

}
