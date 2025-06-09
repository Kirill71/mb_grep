#include "thread_pool.h"

namespace mb {
ThreadPool::ThreadPool(const size_t num_threads) {
    workers_.reserve(num_threads);
    for (size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back([this] {
            while (true) {
                Task task{};
                {
                    std::unique_lock lock{queue_mutex_};
                    condition_.wait(lock, [this] { return stop_flag_ || !tasks_.empty(); });
                    if (stop_flag_ && tasks_.empty())
                        return;
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    stop_flag_ = true;
    condition_.notify_all();
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::submit(Task task) {
    {
        std::lock_guard lock{queue_mutex_};
        tasks_.push(std::move(task));
    }
    condition_.notify_one();
}
} // namespace mb