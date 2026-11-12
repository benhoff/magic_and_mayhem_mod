#pragma once
#include <algorithm>
#include <cstddef>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

namespace mnm::render::detail {
// One FIFO sink. Reservations include in-flight work and its temporary memory.
// Slow/failed evidence work never waits under the producer's queue lock.
class DiagnosticWorker final {
public:
    DiagnosticWorker(std::size_t bytes,std::size_t jobs):limit_(bytes),jobLimit_(jobs),thread_([this]{loop();}){}
    ~DiagnosticWorker(){finish();}
    bool reserve(std::size_t bytes){
        std::lock_guard<std::mutex> lock(mutex_);
        if(closed_ || !error_.empty() || jobs_>=jobLimit_ || bytes>limit_-bytes_)return false;
        reservations_.push_back(bytes);bytes_+=bytes;++jobs_;
        peak_=std::max(peak_,bytes_);return true;
    }
    void post(std::size_t bytes,std::function<void()> work){
        std::lock_guard<std::mutex> lock(mutex_);
        if(closed_ || reservations_.empty() || reservations_.front()!=bytes)
            throw std::logic_error("Diagnostic reservation identity changed");
        queue_.push_back({bytes,std::move(work)});reservations_.pop_front();wake_.notify_one();
    }
    void finish(){
        {std::lock_guard<std::mutex> lock(mutex_);closed_=true;}
        wake_.notify_one();if(thread_.joinable())thread_.join();
    }
    std::string error() const{std::lock_guard<std::mutex> lock(mutex_);return error_;}
    std::size_t peak() const{std::lock_guard<std::mutex> lock(mutex_);return peak_;}
    std::size_t outstanding() const{std::lock_guard<std::mutex> lock(mutex_);return bytes_;}
private:
    struct Job{std::size_t bytes;std::function<void()> work;};
    void loop(){
        for(;;){
            Job job;
            {std::unique_lock<std::mutex> lock(mutex_);
                wake_.wait(lock,[this]{return closed_ || !queue_.empty();});
                if(queue_.empty())return;
                job=std::move(queue_.front());queue_.pop_front();}
            std::string failure;
            try{job.work();}catch(const std::exception& e){failure=e.what();}
            catch(...){failure="Unknown diagnostic worker failure";}
            // Release captured snapshots before admitting their replacement.
            job.work={};
            {std::lock_guard<std::mutex> lock(mutex_);
                if(error_.empty() && !failure.empty())error_=failure.substr(0,4096);
                bytes_-=job.bytes;--jobs_;}
        }
    }
    const std::size_t limit_,jobLimit_;
    mutable std::mutex mutex_;
    std::condition_variable wake_;
    std::deque<Job> queue_;
    std::deque<std::size_t> reservations_;
    std::size_t bytes_=0,jobs_=0,peak_=0;
    bool closed_=false;
    std::string error_;
    std::thread thread_;
};
}
