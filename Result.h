#ifndef _RESULT_H_
#define _RESULT_H_

#include "Any.h"
#include "Semphore.h"
#include"Task.h"
#include <atomic>

class Result {
public:
    Result(std::shared_ptr<Task> task, bool isValid);
    ~Result();

    void setVal(Any any);

    Any get();
private:
    Any any_;
    Semaphore sem_;
    std::shared_ptr<Task> task_;
    std::atomic<bool> isValid_;
};

#endif