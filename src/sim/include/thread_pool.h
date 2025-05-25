/**
 * @file thread_pool.h
 * @brief 定义了仿真系统中的线程池实现
 * 
 * 该文件实现了一个通用的线程池，具有以下特性：
 * - 支持动态任务提交和执行
 * - 自动管理线程生命周期
 * - 支持任务返回值的异步获取
 * - 基于条件变量的任务调度
 * - 优雅的关闭机制
 * 
 * 线程池默认使用硬件支持的最大并发线程数，
 * 可以在构造时指定具体的线程数量。
 */

#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>

namespace sim {

class ThreadPool {
public:
    /**
     * @brief 构造线程池
     * @param num_threads 线程数量，默认为硬件支持的最大并发线程数
     */
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency()) {
        for(size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] {
                while(true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        condition_.wait(lock, [this] {
                            return stop_ || !tasks_.empty();
                        });
                        
                        if(stop_ && tasks_.empty()) {
                            return;
                        }
                        
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    /**
     * @brief 向线程池提交任务
     * @tparam F 函数类型
     * @tparam Args 参数类型
     * @param f 要执行的函数
     * @param args 函数参数
     * @return 任务的future对象，可用于获取返回值
     */
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        using return_type = typename std::invoke_result<F, Args...>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
            
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if(stop_) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            tasks_.emplace([task](){ (*task)(); });
        }
        condition_.notify_one();
        return res;
    }

    /**
     * @brief 析构函数，停止所有线程并等待任务完成
     */
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for(std::thread &worker: workers_) {
            worker.join();
        }
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_ = false;
};

} // namespace sim 