#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
#include <assert.h>

class ThreadPool {
public:
    explicit ThreadPool(size_t thread_count = 8);
    ThreadPool() = default;
    ThreadPool(ThreadPool &&) = default;
    ~ThreadPool();

    template<class F>
    void AddTask(F &&task);

private:
    struct Pool {
        std::mutex mtx_;
        std::condition_variable cond;
        bool is_close_;
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<Pool> pool_;
 };

#endif //THREADPOOL_H