#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace mb {
class ThreadPool final {
    using Task = std::function<void()>;

public:
    explicit ThreadPool(size_t num_threads);

    ~ThreadPool();

    void submit(Task task);

private:
    std::vector<std::thread> workers_;
    std::queue<Task> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic_bool stop_flag_{false};
};
} // namespace mb