#ifndef _TASK_H_
#define _TASK_H_

#include "Any.h"

class Result;

class Task {
public:
    Task();

    void exec();

    virtual Any run() = 0;

    void setResult(Result *result);

private:
    Result* result_;
};

#endif