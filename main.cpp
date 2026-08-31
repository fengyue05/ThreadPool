#include "ThreadPool.h"
#include <iostream>
#include <memory>
#include <thread>

class MyTask : public Task {
public:
    MyTask(int begin, int end) : begin_(begin), end_(end) 
    {}

    Any run () override {
        std::cout << "tid" << std::this_thread::get_id() << "begin" << std::endl;
        int sum = 0;
        for (int i = begin_; i < end_; i++) {
            sum += i;
        }
        std::cout << "tid" << std::this_thread::get_id() << "end" << std::endl;
        return sum;
    }
    
private:
    int begin_;
    int end_;
};


int main () {
    ThreadPool pool;
    pool.start(4);

    pool.subMitTask(std::make_shared<MyTask>(1, 4));
    pool.subMitTask(std::make_shared<MyTask>(1, 4));
    pool.subMitTask(std::make_shared<MyTask>(1, 4));
    pool.subMitTask(std::make_shared<MyTask>(1, 4));
    pool.subMitTask(std::make_shared<MyTask>(1, 4));
    pool.subMitTask(std::make_shared<MyTask>(1, 4));
    pool.subMitTask(std::make_shared<MyTask>(1, 4));
    
    return 0;
}