#pragma once
#include "buffers.hpp"
#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QByteArray>
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
    explicit PcmQueue(Device& device):device_(device){}
    bool pump(QIODevice& output,qint64 capacity);
    void clear(){pending_.clear();offset_=0;}
    qsizetype pendingBytes() const{return pending_.size()-offset_;}
private:
    Device& device_;QByteArray pending_;qsizetype offset_=0;
};
// All calls, voice controls and pumping belong to this object's Qt thread.
// Device must outlive this adapter. Stop discards queued sound, not voice state.
class QtOutput : public QObject {
public:
    explicit QtOutput(Device&,QObject* parent=nullptr);
    ~QtOutput() override;
    bool start(const QAudioDevice& device);
    void stop();
    bool running() const{return bool(sink_);}
    qint64 processedUSecs() const{return sink_?sink_->processedUSecs():0;}
    QString lastError() const{return error_;}
    std::function<void(const QString&)> failed;
private:
    void pump();void fail(const QString&);
    Device& device_;PcmQueue queue_;QTimer timer_;
    std::unique_ptr<QAudioSink> sink_;QIODevice* writer_=nullptr;QString error_;
};
}
