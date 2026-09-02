#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <chrono>
#include <iostream>
#include <functional>
#include <unordered_map>
#include <future>
#include <thread>

enum class ThreadMode {
    MODE_FIXED,
    MODE_CACHED,
};


class Thread {
public:
    using threadFunc = std::function<void(int)>;

    Thread(threadFunc func);
    ~Thread() = default;

    void start();
    int getId();
private:    
    static int generateId_;

    int threadId_;
    threadFunc func_;
};

class ThreadPool {
public:
    using Task = std::function<void()>;

    ThreadPool();
    ~ThreadPool();

    void start(int size);

    void setMode(ThreadMode mode);
    void setTaskQueMaxSize(int threadHold);
    void setThreadSizeHold(int threadHold);

    template<typename Func, typename... Args>
    auto subMitTask(Func&& func, Args... args) -> std::future<decltype(func(args...))> {
        using RType = decltype(func(args...));
        auto task = std::make_shared<std::packaged_task<RType()>> (
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        std::future<RType> result = task->get_future();
        
        std::unique_lock<std::mutex> lock_(taskQueMtx_);
        
        if (!notFull_.wait_for(lock_, std::chrono::seconds(1), [&]() -> bool {return taskQue_.size() < taskQueMaxThreadHold_;})) {
            std::cerr << "task queue is full, submit task fail" << std::endl;
            auto taskErr = std::make_shared<std::packaged_task<RType()>>(
                []()->RType {return RType();});
            (*task)();
            return taskErr->get_future();
        }

        taskQue_.emplace([task](){
            (*task)();
        });
        taskSize_++;

        notEmpty_.notify_all();

        if (ThreadMode::MODE_CACHED == mode_ && taskSize_ > initThreadSize_ && taskSize_ < threadSizeHold_) {
            auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
            int threadId = ptr->getId();
            threads_.emplace(threadId, std::move(ptr));

            std::cout << "new Thread" << std::endl;

            curThreadSize_++;
            idleThreadSize_++;
            threads_[threadId]->start();
        }

        return result;
        
    }


    bool checkRunningState() const;

private:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ThreadMode mode_;
    size_t initThreadSize_; // 初始的线程数量
    std::unordered_map<int, std::unique_ptr<Thread>> threads_;
    int threadSizeHold_; // 线程的数量上限
    std::atomic<int> curThreadSize_; // 当前线程池中的总线程数量
    std::atomic<int> idleThreadSize_; // 当前线程池中的空闲数量

    std::queue<Task> taskQue_;
    std::atomic<int> taskSize_;
    int taskQueMaxThreadHold_; 

    std::mutex taskQueMtx_;
    std::condition_variable notFull_;
    std::condition_variable notEmpty_;
    std::condition_variable exitCond_; // 等待线程的资源回收

    bool isCheckRunning;

    void threadFunc(int threadId);
};

#endif