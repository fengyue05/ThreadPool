#include "ThreadPool.h"
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

constexpr int TASK_MAX_THREADHOLD = INT32_MAX;
constexpr int THREAD_MAX_SIZE = 10;
constexpr int Thread_IDLE_MAX_TIME = 10;

int Thread::generateId_ = 0;

ThreadPool::ThreadPool()
    : taskQueMaxThreadHold_(TASK_MAX_THREADHOLD)
    , initThreadSize_(4)
    , mode_(ThreadMode::MODE_FIXED)
    , taskSize_(0)
    , isCheckRunning(false)
    , threadSizeHold_(THREAD_MAX_SIZE)
    , curThreadSize_(0)
    , idleThreadSize_(0)
{}

ThreadPool::~ThreadPool() {
    isCheckRunning = false;
    notEmpty_.notify_all();

    // 等待线程池里面的所有线程返回  有两种状态：一种是有任务，一种是没任务
    std::unique_lock<std::mutex> lock(taskQueMtx_);
    exitCond_.wait(lock, [&]() -> bool {return curThreadSize_ == 0;});
}

void ThreadPool::start(int size) {
    initThreadSize_ = size;
    curThreadSize_ = size;
    isCheckRunning = true;
    setTaskQueMaxSize(size);

    for (int i = 0; i < initThreadSize_; i++) {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        int threadId = ptr->getId();
        threads_.emplace(threadId, std::move(ptr));
        idleThreadSize_++;
    }
    
    for (int i = 0; i < initThreadSize_; i++) {
        threads_[i]->start();
    }
} 

void ThreadPool::setMode(ThreadMode mode) { mode_ = mode; }

void ThreadPool::setTaskQueMaxSize(int threadHold) {
    if (checkRunningState()) {
        return;
    }
    taskQueMaxThreadHold_ = threadHold;
}

// 创建在cached模式下的线程数量
void ThreadPool::setThreadSizeHold(int threadHold) {
    if (checkRunningState()) {
        return;
    }
    if (ThreadMode::MODE_CACHED == mode_) {
        threadSizeHold_ = threadHold;
    }
}

Result ThreadPool::subMitTask(std::shared_ptr<Task> task)
{
    std::unique_lock<std::mutex> lock_(taskQueMtx_);

    if (!notFull_.wait_for(lock_, std::chrono::seconds(1), [&]() -> bool{ return taskQue_.size() < taskQueMaxThreadHold_; })) {
        std::cerr << "task queue is full, submit task fail" << std::endl;
        return Result(task, false);
    }

    taskQue_.emplace(task);
    taskSize_++;
    
    notEmpty_.notify_all();
    
    // cached任务主要是在比较多的小而紧的任务，因为如果是很多大的任务，那么会创建很多的线程，这样子会导致性能的下降
    if (ThreadMode::MODE_CACHED == mode_ && taskSize_ > idleThreadSize_ && curThreadSize_ < threadSizeHold_) {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        int threadId = ptr->getId();
        threads_.emplace(threadId, std::move(ptr));

        std::cout << "new thread" << std::endl;

        curThreadSize_++;
        idleThreadSize_++;
        threads_[threadId]->start();
    }

    return Result(task, true);
}

bool ThreadPool::checkRunningState() const
{
    return isCheckRunning;
}

void ThreadPool::threadFunc(int threadId) {
    auto lastTime = std::chrono::high_resolution_clock::now();
    while(isCheckRunning) {
        std::shared_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock_(taskQueMtx_);

            std::cout << "tid:" << std::this_thread::get_id() << "尝试获取任务" << std::endl;

            // 如果是cached模式，那么在超过60s之后如果还是没有任务使用到多的线程，那么就删除这些线程
            // 每一秒中轮询一次
            while(taskQue_.size() == 0 && isCheckRunning) {
                if (ThreadMode::MODE_CACHED == mode_) {
                    if (std::cv_status::timeout == notEmpty_.wait_for(lock_, std::chrono::seconds(1))) {
                        auto nowTime = std::chrono::high_resolution_clock::now();
                        auto duration = std::chrono::duration_cast<std::chrono::seconds>(nowTime - lastTime);
                        if (duration.count() > Thread_IDLE_MAX_TIME && curThreadSize_ > initThreadSize_) {
                            // 用对应的线程id删除线程
                            threads_.erase(threadId);
                            curThreadSize_--;
                            idleThreadSize_--;

                            std::cout << "ThreadId:" << std::this_thread::get_id() << "exit" << std::endl;
                            return;
                        }         
                    }      
                }
                else {
                    notEmpty_.wait(lock_);
                }
            }

            // 被唤醒看一下是不是线程池被关闭了
            if (!isCheckRunning) {
                threads_.erase(threadId);
                std::cout << "ThreadId:" << std::this_thread::get_id() << "exit" << std::endl;
                curThreadSize_--; 
                exitCond_.notify_all();
                return;
            }

            task = taskQue_.front();
            taskQue_.pop();

            std::cout << "tid:" << std::this_thread::get_id() << "已经获取到了任务" << std::endl;
            if (taskQue_.size() > 0) {
                notEmpty_.notify_all();
            }

            notFull_.notify_all();

            idleThreadSize_--;
            taskSize_--;
        }
        if (task != nullptr) {
            task->exec();
        }

        lastTime = std::chrono::high_resolution_clock::now();
        idleThreadSize_++;
    }
    threads_.erase(threadId);
    curThreadSize_--;
    exitCond_.notify_all();
    std::cout << "ThreadId:" << std::this_thread::get_id() << "exit" << std::endl;
}

Thread::Thread(threadFunc func) : func_(func), threadId_(generateId_++)
{}

void Thread::start() {
    std::thread tmp(func_, threadId_);
    tmp.detach();
}

int Thread::getId()
{
    return threadId_;
}

Thread::~Thread() {}
