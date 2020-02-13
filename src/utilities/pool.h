#include <future>
#include <thread>
#include <vector>
#include <optional>
#include <functional>


#include "chrono.h"

namespace lython {

class ThreadPool;
void worker_loop(ThreadPool *pool, std::size_t n);

class ThreadPool{
public:
    using Task_t = std::function<void()>;
    using PopFun = std::function<std::optional<Task_t>()>;

    friend void worker_loop(ThreadPool *pool, std::size_t n);

    //! Instantiate a new thread pool
    //! use system default as the number of threads
    ThreadPool(std::size_t thread_count=std::thread::hardware_concurrency());

    template<typename Fun, typename ... Args>
    using Return_t = typename std::result_of<
        typename std::decay<Fun>::type(typename std::decay<Args>::type...)>::type;

    //! Queue a new Task
    template<typename Fun, typename ... Args>
    std::future<Return_t<Fun, Args...>> queue_task(Fun&& fun, Args&&... args){
        std::lock_guard lock(mux);

        // Make an exception for that
        if (size() == 0){
            throw ;
        }

        auto task = std::bind(fun, args...);
        // promise need to outlive this scope and die when the task is over
        auto prom = std::make_shared<std::promise<Return_t<Fun, Args...>>>();
        auto future = prom->get_future();

        tasks.emplace_back([prom{std::move(prom)}, task](){
            auto result = task();
            prom->set_value(result);
        });

        return future;
    }

    //! Remove a task from the queue
    std::optional<Task_t> pop();

    //! Insert a new worker inside the thread pool
    void insert_worker();

    //! Shutdown the threadpool
    void shutdown(bool wait=false);

    //! Returns the number of threads
    std::size_t size() const;

    //! Print a usage report of the thread pool
    std::ostream& print(std::ostream& out) const;

    ~ThreadPool(){
        shutdown(true);
    }

private:
    struct Stat_t{
        TimeIt::TimePoint start; // time when the worker was instantiated
        float work_time = 0; // time delta the worker was busy with a task
        int   task      = 0; // number of task the worker has completed
        int   error     = 0; // number of errors
        bool  running   = true;
    };

    std::mutex               mux;
    std::vector<std::thread> threads;
    std::vector<Stat_t>      stats;
    std::vector<Task_t>      tasks;
};
}
