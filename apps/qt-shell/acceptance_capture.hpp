#pragma once
#include <QFile>
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
#include "host_timing_trace.hpp"

// Opt-in interchange only. Callers freeze timestamps and owned bytes; one
// worker compresses/writes FIFO records. A slow sink cannot block live command
// consumption. Exhaustion refuses the proof instead of dropping observations.
class AcceptanceCapture final {
public:
    explicit AcceptanceCapture(const QString& role){
        directory_=qEnvironmentVariable("MNM_ACCEPTANCE_CAPTURE_DIRECTORY");
        if(directory_.isEmpty())return;
        enabled_=true;
        // Command polls can emit many tiny fragments while a prior payload is
        // compressing. Keep the same byte bound, with a separate finite row
        // bound for that stream. Image capture retains its256-job limit.
        if(role=="commands")pendingRowLimit_=4096;
        if(qEnvironmentVariableIntValue("MNM_ACCEPTANCE_NORMAL")==1)storedLimit_=1024ll*1024*1024;
        campaign_=role=="viewport" && qEnvironmentVariableIntValue("MNM_ACCEPTANCE_CAMPAIGN")==1;
        if(campaign_){storedLimit_=1024ll*1024*1024;rawLimit_=64ll*1024*1024*1024;rowLimit_=131072;}
        static std::atomic<unsigned> identity{0};
        const auto nonce=qEnvironmentVariable("MNM_ACCEPTANCE_CAPTURE_ID");
        if(nonce.isEmpty() || nonce.size()>128 || !QDir::isAbsolutePath(directory_)){
            error_="Invalid acceptance capture identity";return;
        }
        path_=QDir(directory_).filePath(role+"-"+QString::number(++identity)+".bin");
        try{worker_=std::thread([this]{writeLoop();});}
        catch(const std::exception&){error_="Cannot start acceptance capture worker";return;}
        QJsonObject open{{"role",role},{"session",nonce},{"version",campaign_?2:1},{"stored_limit_bytes",double(storedLimit_)},
            {"queue_limit_bytes",double(QueueLimit)},{"queue_limit_rows",int(pendingRowLimit_)}};
        if(campaign_){open["policy"]="campaign-lossless-viewport-v1";open["raw_limit_bytes"]=double(rawLimit_);open["row_limit"]=int(rowLimit_);open["keyframe_paints"]=128;}
        append("open",open,{});
    }
    ~AcceptanceCapture(){finish();}
    bool enabled() const{return enabled_;}
    void append(const QString& kind,QJsonObject metadata,const QByteArray& bytes={}){
        if(!enabled_)return;
        HostTimingTrace trace("acceptance-append");
        std::lock_guard<std::mutex> lock(mutex_);
        if(!error_.isEmpty() || closed_)return;
        // Reserve the last row for a terminal record, including failures.
        if(rows_>=rowLimit_-1 || bytes.size()>32*1024*1024 || raw_+bytes.size()>rawLimit_){
            error_="Acceptance capture raw/row budget exceeded";return;
        }
        metadata["host_us"]=stamp();metadata["kind"]=kind;metadata["index"]=int(rows_);
        auto record=QJsonDocument(metadata).toJson(QJsonDocument::Compact);
        const qint64 charge=record.size()+bytes.size()+8;
        if(record.size()>65536){error_="Acceptance capture metadata budget exceeded";return;}
        if(pendingRows_>=pendingRowLimit_ || charge>QueueLimit-queued_){
            error_="Acceptance capture queue budget exceeded";return;
        }
        // Deep-copy even fromRawData input. The caller may immediately reuse
        // its framebuffer/command storage while this job is still queued.
        jobs_.push_back({std::move(record),QByteArray(bytes.constData(),bytes.size()),charge});
        ++rows_;raw_+=bytes.size();queued_+=charge;++pendingRows_;
        wake_.notify_one();
    }
    void fail(const QString& reason){
        std::lock_guard<std::mutex> lock(mutex_);
        if(error_.isEmpty())error_=reason.left(4096);
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
    struct Job{QByteArray record,bytes;qint64 charge;};
    static QString stamp(){return QString::number(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());}
    static bool write(QFile& file,const QByteArray& record,const QByteArray& packed){
        char h[8];qToLittleEndian<quint32>(record.size(),h);qToLittleEndian<quint32>(packed.size(),h+4);
        return file.write(h,8)==8 && file.write(record)==record.size() && file.write(packed)==packed.size();
    }
    void writeLoop(){
        try{
            QFile file(path_);
            if(!QDir().mkpath(directory_) || !file.open(QIODevice::WriteOnly|QIODevice::NewOnly|QIODevice::Unbuffered)){
                fail("Acceptance capture already exists or cannot open");return;
            }
            if(file.write("MNMACE01",8)!=8){fail("Acceptance capture header");return;}
            qint64 stored=8,writtenRaw=0;unsigned writtenRows=0,paints=0;bool writable=true;
            QByteArray previous;QJsonArray previousExtent;int previousIndex=-1;
            for(;;){
                Job job;
                {std::unique_lock<std::mutex> lock(mutex_);
                    wake_.wait(lock,[this]{return closed_ || !jobs_.empty();});
                    if(jobs_.empty())break;
                    job=std::move(jobs_.front());jobs_.pop_front();
                }
                if(writable){
                    QByteArray encoded=job.bytes;
                    if(campaign_ && !job.bytes.isEmpty()){
                        auto metadata=QJsonDocument::fromJson(job.record).object();
                        const int width=metadata["width"].toInt(),height=metadata["height"].toInt();
                        if(metadata["kind"]!="paint" || width<1 || height<1 || qint64(width)*height*4!=job.bytes.size()){
                            fail("Campaign capture paint extent");writable=false;
                        }else{
                            const QJsonArray extent{width,height};
                            const bool delta=!previous.isEmpty() && previous.size()==job.bytes.size() && previousExtent==extent && paints%128!=0;
                            metadata["capture_body_encoding"]=delta?"xor-previous-paint-v1":"full-rgba-v1";
                            metadata["capture_body_base"]=delta?previousIndex:-1;
                            metadata["capture_body_sha256"]=QString::fromLatin1(QCryptographicHash::hash(job.bytes,QCryptographicHash::Sha256).toHex());
                            if(delta){auto* target=encoded.data();const auto* value=job.bytes.constData();const auto* prior=previous.constData();for(qsizetype i=0;i<encoded.size();++i)target[i]=char(uchar(value[i])^uchar(prior[i]));}
                            previous=job.bytes;previousExtent=extent;previousIndex=metadata["index"].toInt();++paints;
                            job.record=QJsonDocument(metadata).toJson(QJsonDocument::Compact);
                        }
                    }
                    QByteArray packed;
                    {HostTimingTrace trace("acceptance-worker-compress");if(writable && !encoded.isEmpty())packed=qCompress(encoded,3);}
                    const qint64 size=job.record.size()+packed.size()+8;
                    if(job.record.size()>65536){fail("Acceptance capture metadata budget exceeded");writable=false;}
                    if(!writable){/* Preserve the first codec refusal and drain owned jobs. */}
                    else if(!job.bytes.isEmpty() && packed.isEmpty()){
                        fail("Acceptance capture compression failed");writable=false;
                    }else if(size>storedLimit_-stored-CloseReserve){
                        fail("Acceptance capture stored budget exceeded");writable=false;
                    }else{
                        HostTimingTrace trace("acceptance-worker-write");
                        if(!write(file,job.record,packed)){fail("Acceptance capture write");writable=false;}
                        else{stored+=size;writtenRaw+=job.bytes.size();++writtenRows;}
                    }
                }
                {std::lock_guard<std::mutex> lock(mutex_);queued_-=job.charge;--pendingRows_;}
            }
            QString failure;
            {std::lock_guard<std::mutex> lock(mutex_);failure=error_;}
            // After an asynchronous refusal, indices/counters describe only
            // the contiguous records actually written, followed by failure.
            const auto terminal=QJsonDocument(QJsonObject{{"kind","close"},{"index",int(writtenRows)},
                {"host_us",stamp()},{"complete",failure.isEmpty()},{"error",failure},
                {"raw_bytes",QString::number(writtenRaw)},{"prior_rows",int(writtenRows)}}).toJson(QJsonDocument::Compact);
            if(!write(file,terminal,{}) || !file.flush())fail("Acceptance capture close");
        }catch(const std::exception&){fail("Acceptance capture worker failed");}
    }
    QString directory_,path_,error_;
    bool enabled_=false,closed_=false,campaign_=false;
    unsigned rows_=0,pendingRows_=0,rowLimit_=16384,pendingRowLimit_=256;
    qint64 raw_=0,queued_=0,storedLimit_=256ll*1024*1024,rawLimit_=8ll*1024*1024*1024;
    std::mutex mutex_;std::condition_variable wake_;std::deque<Job> jobs_;std::thread worker_;
};
