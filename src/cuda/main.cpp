#include "utilities/pool.h"

#include "logging/logging.h"


int main(){
    info("");

    lython::ThreadPool pool;

    std::future<int> res = pool.queue_task([]() -> int{
        return 1 + 2;
    });

    pool.shutdown(true);

    return 0;
}
