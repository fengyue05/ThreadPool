#include "Task.h"
#include "Result.h"

Task::Task() : result_(nullptr)
{}

void Task::exec()
{
    if (result_) {
         result_->setVal(run());
    }
}

void Task::setResult(Result *result)
{
    result_ = result;
}
