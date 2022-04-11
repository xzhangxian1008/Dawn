#include "util/work_pool.h"

namespace dawn {

void WorkPool::add_task(Task* task) {
    {
        std::unique_lock<std::mutex> ul(mt_);
        tasks_.push(task);
    }

    task_cv_.notify_one();
}

void WorkPool::add_thread() {
    threads_.emplace_back([this] {
        Task* task;

        while (true) {
            {
                std::unique_lock<std::mutex> ul(mt_);
                task_cv_.wait(ul, [&]{ return !is_running_ || !tasks_.empty();});
                if (!is_running_) {
                    return;
                }
                task = tasks_.front();
                tasks_.pop();
                busy_num_++;
            }
            task->run();
            {
                std::unique_lock<std::mutex> ul(mt_);
                busy_num_--;
            }
            finish_cv_.notify_one();
        }
    });
}

} // namespace dawn
