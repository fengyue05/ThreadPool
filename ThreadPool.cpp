#include "ThreadPool.h"
#include <iostream>
#include <mutex>
#include <thread>

#define MAX_SIZE 1024

ThreadPool::ThreadPool()
    : taskQueMaxThreadHold_(0)
    , initThreadSize_(4)
    , mode_(ThreadMode::MODE_FIXED)
    , taskSize_(0)
{}

ThreadPool::~ThreadPool() {}

void ThreadPool::start(int size) {
    initThreadSize_ = size;

    for (int i = 0; i < initThreadSize_; i++) {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this));
        threads_.emplace_back(std::move(ptr));
    }
    
    for (int i = 0; i < initThreadSize_; i++) {
        threads_[i]->start();
    }
}

void ThreadPool::setMode(ThreadMode mode) { mode_ = mode; }

void ThreadPool::setTaskQueMaxSize(int threadHold) {
    taskQueMaxThreadHold_ = MAX_SIZE;
}

void ThreadPool::subMitTask(std::shared_ptr<Task> task)
{
    std::unique_lock<std::mutex> lock_(taskQueMtx_);

    if (notFull_.wait_for(lock_, std::chrono::seconds(1), [&]() -> bool{ return taskQue_.size() < taskQueMaxThreadHold_; })) {
        std::cerr << "task queue is full, submit task fail" << std::endl;
        return;
    }

    taskQue_.emplace(task);
    taskSize_++;
    
    notEmpty_.notify_all();
}

void ThreadPool::threadFunc() {
    while(true) {
        std::shared_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock_(taskQueMtx_);

            std::cout << "tid:" << std::this_thread::get_id() << "尝试获取任务" << std::endl;
            notFull_.wait(lock_, [&]() -> bool { return taskQue_.size() > 0 ;});

            task = taskQue_.front();
            taskQue_.pop();

            std::cout << "tid:" << std::this_thread::get_id() << "已经获取到了任务" << std::endl;
            if (taskQue_.size() > 0) {
                notEmpty_.notify_all();
            }

            notFull_.notify_all();

            taskSize_--;
        }
        task->run();
    }
}

Thread::Thread(threadFunc func) : func_(func)
{}

void Thread::start() {
    std::thread tmp(func_);
    tmp.detach();
}

Thread::~Thread() {}