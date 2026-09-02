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
    pool.setMode(ThreadMode::MODE_CACHED);
    pool.start(4);

    Result res1 = pool.subMitTask(std::make_shared<MyTask>(1, 100));
    Result res2 = pool.subMitTask(std::make_shared<MyTask>(1, 100));
    Result res3 = pool.subMitTask(std::make_shared<MyTask>(1, 100));
    Result res4 = pool.subMitTask(std::make_shared<MyTask>(1, 100));
    Result res5 = pool.subMitTask(std::make_shared<MyTask>(1, 100));
    Result res6 = pool.subMitTask(std::make_shared<MyTask>(1, 100));
    Result res7 = pool.subMitTask(std::make_shared<MyTask>(1, 100));

    int sum1 = res1.get().cast_<int>();
    int sum2 = res2.get().cast_<int>();
    int sum3 = res3.get().cast_<int>();
    std::cout << (sum1 + sum2 + sum3) << std::endl;

    return 0;
}