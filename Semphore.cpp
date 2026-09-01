#include "Semphore.h"

Semaphore::Semaphore(int limit) : resLimit_(limit)
{}

void Semaphore::post()
{
    std::unique_lock<std::mutex> lock_(mutex_);
    resLimit_++;
    cond_.notify_all();
}

void Semaphore::wait() {
    std::unique_lock<std::mutex> lock_(mutex_);
    cond_.wait(lock_, [&]() -> bool{ return resLimit_ > 0; });
    resLimit_--;
}
