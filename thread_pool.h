#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace mb {
/**
 * @class ThreadPool
 * @brief A simple thread pool for executing tasks concurrently.
 *
 * Manages a fixed number of worker threads that execute submitted tasks.
 *
 * This implementation is inspired by concepts presented in Anthony Williams'
 * book *"C++ Concurrency in Action"*, particularly the producer-consumer model
 * using condition variables and task queues.
 */
class ThreadPool final {
    using Task = std::function<void()>;

public:
    /**
     * @brief Constructs a ThreadPool with the specified number of threads.
     * @param num_threads The number of worker threads to create.
     */
    explicit ThreadPool(size_t num_threads);
    /**
     * @brief Destroys the thread pool and joins all threads.
     *
     * Waits for all active tasks to complete and stops all threads.
     */
    ~ThreadPool();
    /**
    * @brief Submits a task to be executed by the thread pool.
    * @param task The task to be executed.
    */
    void submit(Task task);

private:
    std::vector<std::thread> workers_;
    std::queue<Task> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic_bool stop_flag_{false};
};
} // namespace mb