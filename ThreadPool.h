#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <chrono>
#include <functional>
#include "Task.h"
#include "Result.h"

enum class ThreadMode {
    MODE_FIXED,
    MODE_CACHED,
};

class Any;
class Result;

class Thread {
public:
    using threadFunc = std::function<void()>;

    Thread(threadFunc func);
    ~Thread();

    void start();
private:    
    threadFunc func_;
};

class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();

    void start(int size);
    void setMode(ThreadMode mode);
    void setTaskQueMaxSize(int threadHold);
    Result subMitTask(std::shared_ptr<Task> task);

private:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ThreadMode mode_;
    size_t initThreadSize_; 
    std::vector<std::unique_ptr<Thread>> threads_;

    std::queue<std::shared_ptr<Task>> taskQue_;
    std::atomic<int> taskSize_;
    int taskQueMaxThreadHold_; 

    std::mutex taskQueMtx_;
    std::condition_variable notFull_;
    std::condition_variable notEmpty_;

    void threadFunc();
};

#endif