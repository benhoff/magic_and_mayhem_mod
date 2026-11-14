#pragma once
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QtEndian>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>
#include <future>
#include <algorithm>
#include <memory>
#include <vector>
#include <utility>
#include <cstdio>
#include "host_timing_trace.hpp"

// Opt-in interchange only. Callers freeze timestamps and owned bytes; one
// writer preserves FIFO records. Command history and campaign/movie compression
// use eight bounded workers. Stateful viewport delta preparation remains FIFO.
// Legacy capture stays sequential. A slow sink cannot block live command
// consumption. Exhaustion refuses the proof instead of dropping observations.
class AcceptanceCapture final {
public:
    explicit AcceptanceCapture(const QString& role){
        directory_=qEnvironmentVariable("MNM_ACCEPTANCE_CAPTURE_DIRECTORY");
        if(directory_.isEmpty())return;
        enabled_=true;role_=role;
        commandRole_=role=="commands";
        // Command polls can emit many tiny fragments while a prior payload is
        // compressing. Normal viewport capture also emits many body-free
        // intro paint/swap records. Bound both streams by4096jobs while keeping
        // the same256MiB charge, including in-flight payloads.
        if(role=="commands" || (role=="viewport" && qEnvironmentVariableIntValue("MNM_ACCEPTANCE_NORMAL")==1))pendingRowLimit_=4096;
        if(qEnvironmentVariableIntValue("MNM_ACCEPTANCE_NORMAL")==1)storedLimit_=1024ll*1024*1024;
        movie_=role=="viewport" && qEnvironmentVariableIntValue("MNM_ACCEPTANCE_IMAGES")==1;
        campaign_=movie_ || (role=="viewport" && qEnvironmentVariableIntValue("MNM_ACCEPTANCE_CAMPAIGN")==1);
        if(campaign_){storedLimit_=1024ll*1024*1024;rawLimit_=64ll*1024*1024*1024;rowLimit_=131072;}
        if(movie_){storedLimit_=2ll*1024*1024*1024;pendingRowLimit_=4096;}
        commandHistory_=role=="commands" && qEnvironmentVariableIntValue("MNM_ACCEPTANCE_COMMAND_HISTORY")==1;
        if(commandHistory_){storedLimit_=1024ll*1024*1024;rowLimit_=65536;}
        static std::atomic<unsigned> identity{0};
        const auto nonce=qEnvironmentVariable("MNM_ACCEPTANCE_CAPTURE_ID");
        if(nonce.isEmpty() || nonce.size()>128 || !QDir::isAbsolutePath(directory_)){
            refuseLocked("Invalid acceptance capture identity");logRefusal();return;
        }
        path_=QDir(directory_).filePath(role+"-"+QString::number(++identity)+".bin");
        try{worker_=std::thread([this]{writeLoop();});}
        catch(const std::exception&){refuseLocked("Cannot start acceptance capture worker");logRefusal();return;}
        QJsonObject open{{"role",role},{"session",nonce},{"version",commandHistory_?4:movie_?3:campaign_?2:1},{"stored_limit_bytes",double(storedLimit_)},
            {"queue_limit_bytes",double(QueueLimit)},{"queue_limit_rows",int(pendingRowLimit_)},
            {"compression_workers",(commandHistory_||campaign_)?int(CompressionWorkers):1}};
        if(campaign_){open["policy"]=movie_?"movie-lossless-viewport-v1":"campaign-lossless-viewport-v1";open["raw_limit_bytes"]=double(rawLimit_);open["row_limit"]=int(rowLimit_);open["keyframe_paints"]=128;}
        if(commandHistory_){open["policy"]="command-history-lossless-v1";open["raw_limit_bytes"]=double(rawLimit_);open["row_limit"]=int(rowLimit_);}
        append("open",open,{});
    }
    ~AcceptanceCapture(){finish();}
    bool enabled() const{return enabled_;}
    void bindProducer(const QString& channel,quint32 identity,quint32 version){
        // Startup-only sideband; existing capture record streams stay unchanged.
        if(!enabled_||!commandRole_||qEnvironmentVariable("MNM_RESIDENT_PROFILE_CAPTURE_DIRECTORY").isEmpty())return;
        if(!QDir::isAbsolutePath(channel)||!identity||(version!=2&&version!=3)){
            fail("Invalid acceptance producer binding");return;
        }
        const QJsonObject value{{"schema",1},{"kind","command-producer-binding"},{"producer",qint64(identity)},
            {"transport_version",int(version)},{"channel",QFileInfo(channel).absoluteFilePath()},
            {"capture",path_},{"host_us",stamp()}};
        QFile file(path_+".producer.json");const auto bytes=QJsonDocument(value).toJson(QJsonDocument::Compact);
        if(!QDir().mkpath(directory_)||!file.open(QIODevice::WriteOnly|QIODevice::NewOnly)||file.write(bytes)!=bytes.size()||!file.flush())
            fail("Cannot record acceptance producer binding");
    }
    void append(const QString& kind,QJsonObject metadata,const QByteArray& bytes={}){
        if(!enabled_)return;
        HostTimingTrace trace("acceptance-append");
        std::lock_guard<std::mutex> lock(mutex_);
        if(!error_.isEmpty() || closed_)return;
        // Reserve the last row for a terminal record, including failures.
        if(rows_>=rowLimit_-1 || bytes.size()>32*1024*1024 || raw_+bytes.size()>rawLimit_){
            refuseLocked("Acceptance capture raw/row budget exceeded");return;
        }
        metadata["host_us"]=stamp();metadata["kind"]=kind;metadata["index"]=int(rows_);
        auto record=QJsonDocument(metadata).toJson(QJsonDocument::Compact);
        const qint64 charge=record.size()+bytes.size()+8;
        if(record.size()>65536){refuseLocked("Acceptance capture metadata budget exceeded");return;}
        if(pendingRows_>=pendingRowLimit_ || charge>QueueLimit-queued_){
            refusalQueued_=queued_;refusalRows_=pendingRows_;refusalCharge_=charge;
            refuseLocked("Acceptance capture queue budget exceeded");return;
        }
        // Deep-copy even fromRawData input. The caller may immediately reuse
        // its framebuffer/command storage while this job is still queued.
        jobs_.push_back({std::move(record),QByteArray(bytes.constData(),bytes.size()),charge});
        ++rows_;raw_+=bytes.size();queued_+=charge;++pendingRows_;
        peakQueued_=std::max(peakQueued_,queued_);peakRows_=std::max(peakRows_,pendingRows_);
        wake_.notify_one();
    }
    void fail(const QString& reason){
        std::lock_guard<std::mutex> lock(mutex_);
        refuseLocked(reason);
    }
    void finish(){
        if(!enabled_)return;
        {std::lock_guard<std::mutex> lock(mutex_);if(closed_)return;closed_=true;}
        wake_.notify_one();
        // Only terminal lifetime cleanup joins. append/fail never wait for
        // compression or I/O, and finish does not renew any producer lease.
        if(worker_.joinable())worker_.join();
    }
private:
    static constexpr qint64 QueueLimit=256ll*1024*1024,CloseReserve=65536+8;
    static constexpr unsigned CompressionWorkers=8;
    struct Job{QByteArray record,bytes;qint64 charge;};
    // At most CompressionWorkers results exist outside jobs_. Every source payload
    // stays charged until its FIFO write, including compression and ready results.
    // Each compressor has at most one32MiBencoded input and one qCompress output.
    // FIFO viewport preparation also retains one prior32MiBpaint and bounded XOR scratch.
    class Compressors final {
    public:
        Compressors(){
            try{for(unsigned i=0;i<CompressionWorkers;++i)threads_.emplace_back([this]{
                for(;;){
                    std::packaged_task<QByteArray()> task;
                    {std::unique_lock<std::mutex> lock(mutex_);wake_.wait(lock,[this]{return stopping_||!tasks_.empty();});
                        if(tasks_.empty())return;
                        task=std::move(tasks_.front());tasks_.pop_front();}
                    task();
                }
            });}catch(...){stop();throw;}
        }
        ~Compressors(){stop();}
        std::future<QByteArray> submit(QByteArray bytes){
            std::packaged_task<QByteArray()> task([bytes=std::move(bytes)]()mutable{
                HostTimingTrace trace("acceptance-worker-compress");
                auto packed=bytes.isEmpty()?QByteArray{}:qCompress(bytes,3);
                bytes.clear();return packed;
            });
            auto result=task.get_future();
            {std::lock_guard<std::mutex> lock(mutex_);tasks_.push_back(std::move(task));}
            wake_.notify_one();return result;
        }
    private:
        void stop(){
            {std::lock_guard<std::mutex> lock(mutex_);stopping_=true;}wake_.notify_all();
            for(auto& thread:threads_)if(thread.joinable())thread.join();
        }
        std::mutex mutex_;std::condition_variable wake_;bool stopping_=false;
        std::deque<std::packaged_task<QByteArray()>> tasks_;std::vector<std::thread> threads_;
    };
    // Called under mutex_, or during construction before a worker exists.
    // Preserve the first refusal even when a failed experiment must terminate
    // the process before its ordinary capture close record can be written.
    void refuseLocked(const QString& reason){
        if(!error_.isEmpty())return;
        error_=reason.left(4096);
        refusalDiagnostic_=QJsonDocument(QJsonObject{{"role",role_},{"path",path_},{"error",error_},
            {"host_us",stamp()},{"rows",int(rows_)},{"raw_bytes",double(raw_)},
            {"queue_bytes",double(queued_)},{"queue_rows",int(pendingRows_)},
            {"incoming_charge_at_refusal",double(refusalCharge_)}}).toJson(QJsonDocument::Compact);
        wake_.notify_one();
    }
    void logRefusal(){
        QByteArray line;
        {std::lock_guard<std::mutex> lock(mutex_);line=std::exchange(refusalDiagnostic_,QByteArray{});}
        if(!line.isEmpty())std::fprintf(stderr,"NATIVE_CAPTURE_REFUSAL %s\n",line.constData());
    }
    static QString stamp(){return QString::number(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());}
    static bool write(QFile& file,const QByteArray& record,const QByteArray& packed){
        char h[8];qToLittleEndian<quint32>(record.size(),h);qToLittleEndian<quint32>(packed.size(),h+4);
        return file.write(h,8)==8 && file.write(record)==record.size() && file.write(packed)==packed.size();
    }
    void writeLoop(){
        try{
            QFile file(path_);
            if(!QDir().mkpath(directory_) || !file.open(QIODevice::WriteOnly|QIODevice::NewOnly|QIODevice::Unbuffered)){
                fail("Acceptance capture already exists or cannot open");logRefusal();return;
            }
            if(file.write("MNMACE01",8)!=8){fail("Acceptance capture header");logRefusal();return;}
            qint64 stored=8,writtenRaw=0;unsigned writtenRows=0,paints=0;bool writable=true;
            QByteArray previous;QJsonArray previousExtent;int previousIndex=-1;
            std::unique_ptr<Compressors> compressors;
            if(commandHistory_||campaign_)compressors=std::make_unique<Compressors>();
            struct Pending{Job job;qint64 rawSize;std::future<QByteArray> packed;QString codecError;};
            std::deque<Pending> pending;
            QString codecError;
            const auto prepare=[&](Job& job,QByteArray& encoded)->QString{
                if(!codecError.isEmpty())return codecError;
                encoded=job.bytes;
                if(campaign_ && !job.bytes.isEmpty()){
                    auto metadata=QJsonDocument::fromJson(job.record).object();
                    const int width=metadata["width"].toInt(),height=metadata["height"].toInt();
                    if(metadata["kind"]!="paint" || width<1 || height<1 || qint64(width)*height*4!=job.bytes.size()){
                        codecError="Campaign capture paint extent";return codecError;
                    }
                    const QJsonArray extent{width,height};
                    const bool delta=!previous.isEmpty() && previous.size()==job.bytes.size() && previousExtent==extent && paints%128!=0;
                    metadata["capture_body_encoding"]=delta?"xor-previous-paint-v1":"full-rgba-v1";
                    metadata["capture_body_base"]=delta?previousIndex:-1;
                    metadata["capture_body_sha256"]=QString::fromLatin1(QCryptographicHash::hash(job.bytes,QCryptographicHash::Sha256).toHex());
                    if(delta){auto* target=encoded.data();const auto* value=job.bytes.constData();const auto* prior=previous.constData();for(qsizetype i=0;i<encoded.size();++i)target[i]=char(uchar(value[i])^uchar(prior[i]));}
                    previous=job.bytes;previousExtent=extent;previousIndex=metadata["index"].toInt();++paints;
                    job.record=QJsonDocument(metadata).toJson(QJsonDocument::Compact);
                }
                return {};
            };
            for(;;){
                Job job;QByteArray commandPacked;qint64 rawSize=0;QString jobCodecError;
                if(compressors){
                    std::vector<Job> batch;batch.reserve(CompressionWorkers);
                    {std::unique_lock<std::mutex> lock(mutex_);
                        if(pending.empty())wake_.wait(lock,[this]{return closed_||!jobs_.empty()||!error_.isEmpty();});
                        while(pending.size()+batch.size()<CompressionWorkers && !jobs_.empty()){
                            batch.push_back(std::move(jobs_.front()));jobs_.pop_front();
                        }
                    }
                    // Delta/SHA preparation may touch a full paint. Keep it out
                    // of the producer mutex, and preserve its original FIFO
                    // predecessor/keyframe order before parallel compression.
                    for(auto& next:batch){
                        const auto size=next.bytes.size();QByteArray encoded;
                        const auto error=prepare(next,encoded);std::future<QByteArray> packed;
                        if(error.isEmpty())packed=compressors->submit(std::move(encoded));
                        next.bytes.clear();
                        pending.push_back({std::move(next),size,std::move(packed),error});
                    }
                    if(pending.empty())break;
                    auto first=std::move(pending.front());pending.pop_front();
                    job=std::move(first.job);rawSize=first.rawSize;jobCodecError=first.codecError;
                    if(first.packed.valid())commandPacked=first.packed.get();
                }else{
                    std::unique_lock<std::mutex> lock(mutex_);
                    wake_.wait(lock,[this]{return closed_ || !jobs_.empty() || !error_.isEmpty();});
                    if(jobs_.empty())break;
                    job=std::move(jobs_.front());jobs_.pop_front();rawSize=job.bytes.size();
                }
                if(writable){
                    if(!jobCodecError.isEmpty()){fail(jobCodecError);writable=false;}
                    QByteArray encoded=job.bytes;
                    QByteArray packed;
                    {HostTimingTrace trace("acceptance-worker-compress");
                        if(compressors)packed=std::move(commandPacked);
                        else if(writable && !encoded.isEmpty())packed=qCompress(encoded,3);}
                    const qint64 size=job.record.size()+packed.size()+8;
                    if(job.record.size()>65536){fail("Acceptance capture metadata budget exceeded");writable=false;}
                    if(!writable){/* Preserve the first codec refusal and drain owned jobs. */}
                    else if(rawSize && packed.isEmpty()){
                        fail("Acceptance capture compression failed");writable=false;
                    }else if(size>storedLimit_-stored-CloseReserve){
                        fail("Acceptance capture stored budget exceeded");writable=false;
                    }else{
                        HostTimingTrace trace("acceptance-worker-write");
                        if(!write(file,job.record,packed)){fail("Acceptance capture write");writable=false;}
                        else{stored+=size;writtenRaw+=rawSize;++writtenRows;}
                    }
                }
                {std::lock_guard<std::mutex> lock(mutex_);queued_-=job.charge;--pendingRows_;}
                logRefusal();
            }
            logRefusal();
            QString failure;
            {std::lock_guard<std::mutex> lock(mutex_);failure=error_;}
            // After an asynchronous refusal, indices/counters describe only
            // the contiguous records actually written, followed by failure.
            const auto terminal=QJsonDocument(QJsonObject{{"kind","close"},{"index",int(writtenRows)},
                {"host_us",stamp()},{"complete",failure.isEmpty()},{"error",failure},
                {"raw_bytes",QString::number(writtenRaw)},{"prior_rows",int(writtenRows)},
                {"peak_queue_bytes",double(peakQueued_)},{"peak_queue_rows",int(peakRows_)},
                {"queue_bytes_at_refusal",double(refusalQueued_)},{"queue_rows_at_refusal",int(refusalRows_)},
                {"incoming_charge_at_refusal",double(refusalCharge_)}}).toJson(QJsonDocument::Compact);
            if(!write(file,terminal,{}) || !file.flush()){fail("Acceptance capture close");logRefusal();}
        }catch(const std::exception&){fail("Acceptance capture worker failed");logRefusal();}
    }
    QString directory_,path_,error_,role_;
    QByteArray refusalDiagnostic_;
    bool enabled_=false,closed_=false,campaign_=false,movie_=false,commandHistory_=false,commandRole_=false;
    unsigned rows_=0,pendingRows_=0,rowLimit_=16384,pendingRowLimit_=256;
    unsigned peakRows_=0,refusalRows_=0;
    qint64 peakQueued_=0,refusalQueued_=0,refusalCharge_=0;
    qint64 raw_=0,queued_=0,storedLimit_=256ll*1024*1024,rawLimit_=8ll*1024*1024*1024;
    std::mutex mutex_;std::condition_variable wake_;std::deque<Job> jobs_;std::thread worker_;
};
