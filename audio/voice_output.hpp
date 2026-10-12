#pragma once
#include "qt_output.hpp"
#include <QMediaDevices>

namespace mnm::audio {
// Owns a recoverable output endpoint without changing Device/sample identities.
// Host transport liveness is independent of output availability.
class VoiceOutput:public QObject {
public:
    using Selector=std::function<QAudioDevice()>;
    explicit VoiceOutput(Device&,QObject* parent=nullptr);
    VoiceOutput(std::unique_ptr<QtOutput>,Selector,bool monitor,QObject* parent=nullptr);
    ~VoiceOutput() override;
    bool start(bool allowInitialSilence=false);
    void stop();
    bool recover();
    void rescan();
    bool available() const{return active_ && output_->running();}
    bool silent() const{return active_ && output_->silent();}
    bool audible() const{return available() && !silent();}
    OutputTimeline& timeline(){return output_->timeline();}
    void refreshClock(){output_->refreshClock();}
    QString lastError() const{return error_;}
    std::uint64_t losses() const{return losses_;}
    std::uint64_t recoveries() const{return recoveries_;}
    std::uint64_t attempts() const{return attempts_;}
    std::function<void()> changed;
private:
    bool attempt(const QAudioDevice&);
    void lost(const QString&);
    std::unique_ptr<QtOutput> output_;Selector select_;
    std::unique_ptr<QMediaDevices> devices_;QTimer retry_;QByteArray selected_;
    QString error_;bool active_=false,recovering_=false;
    std::uint64_t losses_=0,recoveries_=0,attempts_=0;
};
}
