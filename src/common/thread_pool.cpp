#include "thread_pool.hpp"

ThreadPool::ThreadPool(int threads) : stop_(false){
    if (threads < 0){
        throw std::runtime_error("Number of threads can't be negative");
    }
    for (int i = 0; i < threads; i++){
        workers_.emplace_back([this]{
            while(true){
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    condition_.wait(lock, [this]{
                        return stop_ || !tasks_.empty();
                    });

                    if (stop_ && tasks_.empty()) return;

                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool(){
    {
        std::scoped_lock<std::mutex> lock(mutex_);
        stop_ = true;
    }
    condition_.notify_all();
    for(std::thread &worker : workers_){
        if(worker.joinable()){
            worker.join();
        }
    }
}