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

enum class ThreadMode {
    MODE_FIXED,
    MODE_CACHED,
};

class Any;

class Task {
public:
    virtual Any run() = 0;
};

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
    void subMitTask(std::shared_ptr<Task> task);

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

class Any {
public:
    Any() = default;
    ~Any() = default;
    Any(const Any&) = delete;
    Any& operator=(const Any&) = delete;

    template<typename T>
    Any(T data) : base_(std::make_unique<Derive<T>>(data)) 
    {}

    template <typename T>
    T cast_() {
        Derive<T>* pd = dynamic_cast<Derive<T>*>(base_.get());
        if (pd == nullptr) {
            throw "type is incompatiable";
        }
        return pd->data_;
    }

private:
    class Base {
    public:
        virtual ~Base() = default;
    };

    template<typename T>
    class Derive : public Base {
    public:
        Derive(T data) : data_(data) 
        {}
    private:
        T data_;
    };

    std::unique_ptr<Base> base_;
};


#endif