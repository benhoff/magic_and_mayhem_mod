#pragma once
#include "live_command_session.hpp"
#include "resident_world_session.hpp"
#include <QThread>
#include <QOffscreenSurface>
#include <mutex>
#include <condition_variable>
// GUI facade. Only immutable completed frames and copied reports cross threads.
class NativeRenderWorker final:private QThread {
public:
    NativeRenderWorker(GlViewport&,const QString& worldPath,const QString& game,const QString& reportPath);
    ~NativeRenderWorker() override;
    bool create(const QString&,quint32,quint32,int);
    bool poll(bool normalQuitAccepted,bool drawAllowed,bool paused=false);
    bool requestStop();
    bool finishProducer();
    void abort();
    LiveCommandSession::State state() const{return status_.state;}
    bool ended() const{return state()==LiveCommandSession::State::Ended;}
    unsigned recoveries() const{return status_.recoveries;}
    QString error() const{return status_.error;}
    const mnm::render::CommandResult* result() const{return status_.hasResult?&status_.result:nullptr;}
    QJsonObject report() const{return status_.report;}
    std::function<void()> framePresented;
    std::function<void(LiveCommandSession::State)> stateChanged;
private:
    void run() override;
    void join(bool producerExited);
    struct Status {
        LiveCommandSession::State state=LiveCommandSession::State::WaitingFrame;
        unsigned recoveries=0;
        QString error;
        bool hasResult=false;
        mnm::render::CommandResult result;
        QJsonObject report;
    } status_,published_;
    GlViewport& viewport_;
    // Platform surfaces are created/destroyed on the GUI; worker contexts use them.
    QOffscreenSurface surface_;
    QOpenGLContext* share_=nullptr;
    QString reportPath_;
    std::unique_ptr<ResidentWorldSession> world_;
    std::unique_ptr<LiveCommandSession> commands_;
    mnm::render::GpuFrame rendered_,readyFrame_;
    std::mutex mutex_;
    std::condition_variable wake_;
    bool frameChanged_=false,started_=false,stop_=false,producerExited_=false,requestStop_=false;
    bool normalQuit_=false,drawAllowed_=false,paused_=false;
    std::uint64_t exports_=0,skipped_=0,highWater_=0,turns_=0,exportAllocations_=0;
};
