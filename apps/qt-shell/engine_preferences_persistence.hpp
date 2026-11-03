#pragma once
#include "engine_preferences_store.hpp"
#include "host_timing_trace.hpp"
#include <condition_variable>
#include <deque>
#include <exception>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

// Application persistence only. Capture each original write before returning
// ownership, then serialize durable commits away from the active GUI/leases.
class EnginePreferencesPersistence final {
public:
    struct Result {bool saved=false;QString error;};
    using Commit=std::function<bool(const QByteArray&,const std::array<int,7>&,QString*)>;
    static constexpr std::size_t Capacity=16;
    explicit EnginePreferencesPersistence(Commit commit={}):commit_(std::move(commit)),owner_(std::this_thread::get_id()),worker_([this]{run();}){}
    ~EnginePreferencesPersistence(){
        {std::lock_guard<std::mutex> lock(mutex_);stopping_=true;}
        wake_.notify_all();worker_.join();
    }
    EnginePreferencesPersistence(const EnginePreferencesPersistence&)=delete;
    EnginePreferencesPersistence& operator=(const EnginePreferencesPersistence&)=delete;
    void begin(QString path,QByteArray revision){
        if(std::this_thread::get_id()!=owner_)throw std::logic_error("Preferences persistence owner changed");
        wait();initialized_=!path.isEmpty()&&!revision.isEmpty();store_.begin(std::move(path),std::move(revision));
    }
    bool accept(const QString& profile,const std::array<int,7>& expected,QString* error=nullptr){
        if(error)error->clear();
        const auto reject=[&](const QString& reason){if(error)*error=reason;return false;};
        if(std::this_thread::get_id()!=owner_)return reject("Preference persistence owner changed.");
        if(!initialized_)return reject("Preference persistence was not initialized.");
        if(!EnginePreferencesStore::valid(expected))return reject("Invalid accepted Preferences values.");
        QByteArray bytes;std::array<int,7> actual{};
        if(!EnginePreferencesStore::readProfileSnapshot(profile,bytes,error)||!EnginePreferencesStore::parseProfileSnapshot(bytes,actual,error))return false;
        if(actual!=expected)return reject("Original preference writer did not write the accepted settings.");
        {std::lock_guard<std::mutex> lock(mutex_);
         if(stopping_||jobs_.size()+results_.size()+std::size_t(busy_)>=Capacity)return reject("Preference persistence backlog is full.");
         jobs_.push_back({std::move(bytes),expected});}
        wake_.notify_one();return true;
    }
    std::deque<Result> takeResults(){
        std::lock_guard<std::mutex> lock(mutex_);std::deque<Result> result;result.swap(results_);return result;
    }
    bool pending() const {
        std::lock_guard<std::mutex> lock(mutex_);return busy_||!jobs_.empty();
    }
    // Used before staging a new session; no original producer is active there.
    void wait(){
        std::unique_lock<std::mutex> lock(mutex_);wake_.wait(lock,[this]{return !busy_&&jobs_.empty();});
    }
private:
    struct Job {QByteArray profile;std::array<int,7> expected;};
    void run(){
        for(;;){
            Job job;
            {std::unique_lock<std::mutex> lock(mutex_);wake_.wait(lock,[this]{return stopping_||!jobs_.empty();});
             if(jobs_.empty())return;
             job=std::move(jobs_.front());jobs_.pop_front();busy_=true;}
            Result result;
            try {
                HostTimingTrace trace("preferences-persistence-worker");
                result.saved=commit_?commit_(job.profile,job.expected,&result.error):store_.acceptSnapshot(job.profile,job.expected,&result.error);
            }catch(const std::exception& error){result.error=QString::fromUtf8(error.what());}
            catch(...){result.error="Unknown preference persistence error.";}
            {std::lock_guard<std::mutex> lock(mutex_);results_.push_back(std::move(result));busy_=false;}
            wake_.notify_all();
        }
    }
    EnginePreferencesStore store_;
    Commit commit_;
    const std::thread::id owner_;
    bool initialized_=false;
    mutable std::mutex mutex_;
    std::condition_variable wake_;
    std::deque<Job> jobs_;
    std::deque<Result> results_;
    bool stopping_=false,busy_=false;
    std::thread worker_;
};
