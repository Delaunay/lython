#include "pool.h"

namespace lython {

ThreadPool::ThreadPool(std::size_t thread_count){
    tasks.reserve(128);
    running.reserve(thread_count);
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
    std::size_t n = running.size();
    running.push_back(true);
    threads.emplace_back(worker_loop, this, n);
}

void ThreadPool::shutdown(bool wait){
    for(auto& state: running){
        state = false;
    }

    if (wait){
        for(auto& thread: threads){
            thread.join();
        }
    }
}

std::size_t ThreadPool::size() const {
    return threads.size();
}

void worker_loop(ThreadPool *pool, std::size_t n){
    while (pool->running[n]){
        auto maybe_task = pool->pop();

        if (maybe_task.has_value()){
            std::function<void()> task = maybe_task.value();
            task();
        } else {
            std::this_thread::yield();
        }
    }
}

}
