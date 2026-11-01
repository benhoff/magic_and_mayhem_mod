#pragma once
#include <QJsonDocument>
#include <QJsonObject>
#include <chrono>
#include <atomic>
#include <cstdio>

// Opt-in bounded timing only. A slow operation never renews a producer lease.
class HostTimingTrace final {
public:
    explicit HostTimingTrace(const char* stage):stage_(stage),enabled_(qEnvironmentVariableIntValue("MNM_RESIDENT_POLL_TIMING")==1){
        if(enabled_)start_=Clock::now();
    }
    ~HostTimingTrace(){
        if(!enabled_)return;
        const auto elapsed=std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-start_).count();
        if(elapsed<50000)return;
        static std::atomic<unsigned> rows{0};
        if(rows.fetch_add(1,std::memory_order_relaxed)>=256)return;
        const auto row=QJsonDocument(QJsonObject{{"stage",QString::fromLatin1(stage_)},
            {"begin_host_us",QString::number(std::chrono::duration_cast<std::chrono::microseconds>(start_.time_since_epoch()).count())},
            {"elapsed_us",QString::number(elapsed)}}).toJson(QJsonDocument::Compact);
        std::fprintf(stderr,"NATIVE_HOST_OPERATION %s\n",row.constData());
    }
    HostTimingTrace(const HostTimingTrace&)=delete;
    HostTimingTrace& operator=(const HostTimingTrace&)=delete;
private:
    using Clock=std::chrono::steady_clock;
    const char* stage_;bool enabled_;Clock::time_point start_{};
};
