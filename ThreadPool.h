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
#include <unordered_map>
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
    using threadFunc = std::function<void(int)>;

    Thread(threadFunc func);
    ~Thread();

    void start();
    int getId();
private:    
    static int generateId_;

    int threadId_;
    threadFunc func_;
};

class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();

    void start(int size);

    void setMode(ThreadMode mode);
    void setTaskQueMaxSize(int threadHold);
    void setThreadSizeHold(int threadHold);
    Result subMitTask(std::shared_ptr<Task> task);


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

    std::queue<std::shared_ptr<Task>> taskQue_;
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