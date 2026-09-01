#include "Result.h"

Result::Result(std::shared_ptr<Task> task, bool isValid) : isValid_(isValid), task_(task)
{
    task_->setResult(this);
}

Result::~Result() {}

void Result::setVal(Any any)
{
    this->any_ = std::move(any);
    sem_.post();
}

Any Result::get()
{
    if (!isValid_) {
        return "";
    }
    sem_.wait();
    return std::move(any_);
}


