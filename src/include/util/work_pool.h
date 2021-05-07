#pragma once

#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "util/util.h"

namespace dawn {

using TaskQueue = std::queue<std::function<void()>>;

class WorkPool {
public:
    WorkPool(int thread_num) : 
        thread_num_(thread_num), busy_num_(0), is_running_(false) {}
    
    ~WorkPool() {
        wait_until_all_finished();
        shutdown();
    }

    void start_up() {
        {
            std::unique_lock<std::mutex> ul(mt_);
            is_running_ = true;
        }

        for (uint32_t i = 0; i < thread_num_; i++) {
            add_thread();
        }
    }

    void shutdown() {
        {
            std::unique_lock<std::mutex> ul(mt_);
            is_running_ = false;
        }
        task_cv_.notify_all();

        for (auto &thd : threads_) {
            thd.join();
        }
        threads_.clear();
    }

    uint32_t get_worker_num() const {
        return thread_num_;
    }

    void add_task(std::function<void()> task);

    void wait_until_all_finished() {
        std::unique_lock<std::mutex> ul(mt_);
        finish_cv_.wait(ul, [&]{ return busy_num_ == 0 && tasks_.empty(); });
    }

private:
    void add_thread();

    std::mutex mt_;
    std::vector<std::thread> threads_;
    const uint32_t thread_num_;
    uint32_t busy_num_;
    TaskQueue tasks_;
    bool is_running_;
    std::condition_variable task_cv_;
    std::condition_variable finish_cv_;
};

} // namespace dawn
