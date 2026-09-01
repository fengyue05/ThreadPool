#ifndef _SEMPHORE_H_
#define _SEMPHORE_H_

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    Semaphore(int limit = 0);
    ~Semaphore() = default;

    void post();

    void wait();
private:
    std::mutex mutex_;
    std::condition_variable cond_;
    int resLimit_;
};

#endif