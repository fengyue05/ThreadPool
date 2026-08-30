#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <type_traits>
#include <vector>

enum class ThreadMode {
    MODE_FIXED,
    MODE_CACHED,
};

class Task {
public:
    virtual void run() = 0;
};

class Thread {
public:

private:
};

class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();

    void start();
    void setMode(ThreadMode mode);
    void setTaskQueMaxSize(int threadHold);
    void subMitTask(std::shared_ptr<Task> task);
private:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ThreadMode mode_;
    size_t initThreadSize_; 
    std::vector<Thread*> threads_;

    std::queue<std::shared_ptr<Task>> taskQue_;
    std::atomic<int> taskSize_;
    int taskQueMaxThreadHold_; 

    std::mutex taskQueMtx_;
    std::condition_variable notFull_;
    std::condition_variable notEmpty_;
};

#endif