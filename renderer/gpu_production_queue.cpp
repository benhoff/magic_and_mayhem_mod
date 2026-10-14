#include "gpu_production_queue.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::render {
GpuProductionQueue::GpuProductionQueue(QOpenGLContext* share,QOffscreenSurface* surface,std::function<void()> cleanup)
    :share_(share),surface_(surface),cleanup_(std::move(cleanup)) {
    if(!share||!surface)throw std::invalid_argument("GPU production requires an owned platform target");
    owner_=std::thread([this]{run();});
}
GpuProductionQueue::~GpuProductionQueue(){join();}
bool GpuProductionQueue::canSubmit() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return !stopping_&&!state_.stopped&&state_.error.empty()&&state_.outstanding<2;
}
bool GpuProductionQueue::submit(std::uint64_t sequence,Work work,std::shared_ptr<void> metadata){
    std::lock_guard<std::mutex> lock(mutex_);
    if(!state_.error.empty())throw std::runtime_error(state_.error);
    if(stopping_||state_.stopped)throw std::runtime_error("GPU production is closed");
    if(!sequence||sequence!=state_.submitted+1||!work)throw std::invalid_argument("GPU production sequence or work");
    if(state_.outstanding==2)return false;
    jobs_.push_back({sequence,std::move(work),std::move(metadata)});
    ++state_.submitted;++state_.outstanding;state_.highWater=std::max(state_.highWater,state_.outstanding);
    wake_.notify_one();return true;
}
bool GpuProductionQueue::take(Completion& result){
    std::lock_guard<std::mutex> lock(mutex_);
    if(!state_.error.empty())throw std::runtime_error(state_.error);
    if(ready_.empty())return false;
    if(ready_.front().sequence!=state_.taken+1)throw std::logic_error("GPU production completion order");
    result=std::move(ready_.front());ready_.pop_front();++state_.taken;--state_.outstanding;
    wake_.notify_one();return true;
}
GpuProductionQueue::Stats GpuProductionQueue::stats() const{std::lock_guard<std::mutex> lock(mutex_);return state_;}
void GpuProductionQueue::join(){
    {std::lock_guard<std::mutex> lock(mutex_);stopping_=true;}
    wake_.notify_one();if(owner_.joinable())owner_.join();
}
void GpuProductionQueue::run(){
    std::vector<GpuFrame> exports;
    std::unique_ptr<GlBlitter> renderer;
    try{
        renderer=std::make_unique<GlBlitter>(share_,surface_);
        for(;;){
            Job job;
            {std::unique_lock<std::mutex> lock(mutex_);wake_.wait(lock,[this]{return stopping_||!jobs_.empty();});
             if(stopping_)break;
             job=std::move(jobs_.front());jobs_.pop_front();}
            auto available=exports.end();
            for(;;){
                available=std::find_if(exports.begin(),exports.end(),[](const auto& f){return f.unique();});
                if(available!=exports.end()||exports.size()<2)break;
                std::unique_lock<std::mutex> lock(mutex_);if(stopping_)break;
                wake_.wait_for(lock,std::chrono::milliseconds(1));
            }
            {std::lock_guard<std::mutex> lock(mutex_);if(stopping_)break;}
            Completion result;result.sequence=job.sequence;result.metadata=std::move(job.metadata);result.started=Clock::now();
            auto frame=job.work(*renderer);job.work={};
            if(!frame.valid())throw std::runtime_error("GPU production returned an empty frame");
            if(available!=exports.end()){
                if(available->size()!=frame.size())throw std::runtime_error("GPU production extent changed");
                result.frame=frame.snapshot(std::move(*available));*available=result.frame;
            }else{
                result.frame=frame.snapshot();exports.push_back(result.frame);
                std::lock_guard<std::mutex> lock(mutex_);++state_.allocations;
            }
            result.completed=Clock::now();
            {std::lock_guard<std::mutex> lock(mutex_);++state_.produced;
             if(!stopping_)ready_.push_back(std::move(result));}
        }
    }catch(const std::exception& error){std::lock_guard<std::mutex> lock(mutex_);state_.error=error.what();}
    catch(...){std::lock_guard<std::mutex> lock(mutex_);state_.error="Unknown native GPU production failure";}
    // Internal completions are returned on this owner. External leases must be
    // returned by the compositor before join; polling never destroys them there.
    {std::lock_guard<std::mutex> lock(mutex_);ready_.clear();jobs_.clear();}
    while(std::any_of(exports.begin(),exports.end(),[](const auto& f){return !f.unique();})){
        std::unique_lock<std::mutex> lock(mutex_);wake_.wait_for(lock,std::chrono::milliseconds(1));
    }
    exports.clear();
    try{if(cleanup_)cleanup_();}
    catch(const std::exception& error){std::lock_guard<std::mutex> lock(mutex_);if(state_.error.empty())state_.error=error.what();}
    catch(...){std::lock_guard<std::mutex> lock(mutex_);if(state_.error.empty())state_.error="Unknown native GPU cleanup failure";}
    renderer.reset();
    {std::lock_guard<std::mutex> lock(mutex_);state_.stopped=true;}
}
}
