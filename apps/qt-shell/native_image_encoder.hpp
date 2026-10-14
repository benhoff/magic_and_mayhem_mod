#pragma once
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <stdexcept>
#include <thread>
// Owned image/file tasks only. No GUI or GL objects belong in submitted work.
class NativeImageEncoder {
public:
    NativeImageEncoder():worker_([this]{run();}){}
    ~NativeImageEncoder(){
        {std::lock_guard<std::mutex> lock(mutex_);stop_=true;}
        wake_.notify_one();worker_.join();
    }
    NativeImageEncoder(const NativeImageEncoder&)=delete;
    NativeImageEncoder& operator=(const NativeImageEncoder&)=delete;
    template<class F> auto submit(F work)->std::future<std::invoke_result_t<F>>{
        using Result=std::invoke_result_t<F>;
        auto task=std::make_shared<std::packaged_task<Result()>>(std::move(work));
        auto result=task->get_future();
        {std::lock_guard<std::mutex> lock(mutex_);
            if(stop_||queue_.size()>=2)throw std::runtime_error("Image encoder queue unavailable");
            queue_.push_back([task]{(*task)();});
        }
        wake_.notify_one();return result;
    }
private:
    void run(){for(;;){
        std::function<void()> task;
        {std::unique_lock<std::mutex> lock(mutex_);wake_.wait(lock,[this]{return stop_||!queue_.empty();});
            if(queue_.empty())return;
            task=std::move(queue_.front());queue_.pop_front();
        }
        task();
    }}
    std::mutex mutex_;std::condition_variable wake_;
    std::deque<std::function<void()>> queue_;bool stop_=false;
    std::thread worker_;
};
