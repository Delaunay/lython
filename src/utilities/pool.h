#include <future>
#include <thread>
#include <array>
#include <vector>
#include <optional>
#include <functional>

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

        auto task = std::bind(fun, args...);
        // promise need to outlive this scope and die when the task is over
        auto prom = std::make_shared<std::promise<Return_t<Fun, Args...>>>();

        tasks.emplace_back([prom, task](){
            auto result = task();
            prom->set_value(result);
        });

        return prom->get_future();
    }

    //! Remove a task from the queue
    std::optional<Task_t> pop();

    //! Insert a new worker inside the thread pool
    void insert_worker();

    //! Shutdown the threadpool
    void shutdown(bool wait=false);

    //! Returns the number of threads
    std::size_t size() const;

private:
    std::mutex mux;
    std::vector<std::thread> threads;
    std::vector<int>         running; // CAN'T be bool, we need atomic write and bool wont cut it
    std::vector<Task_t>      tasks;
};
}
