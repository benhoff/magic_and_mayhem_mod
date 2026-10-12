#pragma once
#include "buffers.hpp"
#include "output_timeline.hpp"
#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QByteArray>
#include <QElapsedTimer>
#include <QIODevice>
#include <QObject>
#include <QTimer>
#include <functional>
#include <memory>

namespace mnm::audio {
// Negotiate before creating Device: its output clock must match the sink.
QAudioFormat selectOutputFormat(int requestedRate,int preferredRate,
                               const std::function<bool(const QAudioFormat&)>& supported);
QAudioFormat selectOutputFormat(const QAudioDevice&,int requestedRate=48000);
// Bounded push queue. Handles partial byte writes without mixing those frames twice.
class PcmQueue {
public:
    explicit PcmQueue(Device& device,OutputTimeline* timeline=nullptr):device_(device),timeline_(timeline){}
    bool pump(QIODevice& output,qint64 capacity);
    void clear(){pending_.clear();offset_=0;}
    void reset(){clear();acceptedBytes_=0;}
    std::uint64_t acceptedFrames() const{return acceptedBytes_/4;}
    qsizetype pendingBytes() const{return pending_.size()-offset_;}
private:
    Device& device_;OutputTimeline* timeline_;QByteArray pending_;qsizetype offset_=0;std::uint64_t acceptedBytes_=0;
};
// All calls, voice controls and pumping belong to this object's Qt thread.
// Device must outlive this adapter. Stop discards queued sound, not voice state.
class QtOutput : public QObject {
public:
    explicit QtOutput(Device&,QObject* parent=nullptr);
    ~QtOutput() override;
    virtual bool start(const QAudioDevice& device);
    // Native discard clock for initial absence; expired cues still complete.
    void startSilent();
    bool silent() const{return silent_;}
    virtual void stop();
    virtual bool running() const{return bool(sink_) || silent_;}
    qint64 processedUSecs() const{return sink_?sink_->processedUSecs():0;}
    void refreshClock();
    OutputTimeline& timeline(){return timeline_;}
    virtual QString lastError() const{return error_;}
    std::function<void(const QString&)> failed;
private:
    void pump();void fail(const QString&);
    Device& device_;OutputTimeline timeline_;PcmQueue queue_;QTimer timer_;
    std::unique_ptr<QAudioSink> sink_;QIODevice* writer_=nullptr;QString error_;
    QElapsedTimer silentClock_;bool silent_=false;
};
}
