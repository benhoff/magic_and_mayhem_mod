#pragma once
#include "live_command_session.hpp"
#include "resident_world_session.hpp"
#include "render_deadline_probe.hpp"
#include "native_frame_trace.hpp"
#include "native_worker_status_gate.hpp"
#include <QThread>
#include <QOffscreenSurface>
#include <mutex>
#include <atomic>
#include <condition_variable>
// GUI facade. Only immutable completed frames and copied reports cross threads.
class NativeRenderWorker final:private QThread {
public:
    NativeRenderWorker(GlViewport&,const QString& worldPath,const QString& game,const QString& reportPath);
    ~NativeRenderWorker() override;
    bool create(const QString&,quint32,quint32,int);
    bool poll(bool normalQuitAccepted,bool drawAllowed,bool paused=false);
    bool requestStop();
    bool requestGraphicsRecovery();
    bool finishProducer();
    void abort();
    LiveCommandSession::State state() const{return status_.state;}
    bool ended() const{return state()==LiveCommandSession::State::Ended;}
    std::uint64_t pendingCpuRead() const{return pendingCpuRead_.load(std::memory_order_acquire);}
    unsigned recoveries() const{return status_.recoveries;}
    QString error() const{return status_.error;}
    const mnm::render::CommandResult* result() const{return status_.hasResult?&status_.result:nullptr;}
    QJsonObject report() const{return status_.report;}
    std::function<void()> framePresented;
    std::function<void(LiveCommandSession::State)> stateChanged;
private:
    std::atomic<std::uint64_t> pendingCpuRead_{0};
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
    QOffscreenSurface worldSurface_;
    QOpenGLContext* share_=nullptr;
    QString reportPath_;
    std::unique_ptr<ResidentWorldSession> world_;
    std::unique_ptr<LiveCommandSession> commands_;
    mnm::render::GpuFrame rendered_,readyFrame_;
    std::mutex mutex_;
    std::condition_variable wake_;
    bool frameChanged_=false,started_=false,stop_=false,producerExited_=false,requestStop_=false;
    const bool durableGraphics_=qEnvironmentVariableIntValue("MNM_NATIVE_DEVICE_RECOVERY")==1;
    bool graphicsRecoveryPending_=false;
    QOpenGLContext* recoveryShare_=nullptr;
    unsigned graphicsRecoveries_=0;
    bool normalQuit_=false,drawAllowed_=false,paused_=false;
    bool baselineUiClock_=false;
    bool statusEveryTurn_=qEnvironmentVariableIntValue("MNM_WORKER_STATUS_CADENCE")!=1 || qEnvironmentVariableIntValue("MNM_WORKER_STATUS_EVERY_TURN")==1;
    std::uint64_t statusBuilds_=0,statusSkipped_=0;
    std::uint64_t uiPublicationRedraws_=0,sealedUiRedraws_=0,inheritedUiRedraws_=0;
    RenderDeadlineProbe deadlineProbe_;
    NativeFrameTrace frameTrace_{qEnvironmentVariableIntValue("MNM_NATIVE_FRAME_TRACE")==1,qEnvironmentVariableIntValue("MNM_NATIVE_HITCH_CAPTURE")==1};
    std::uint64_t readySerial_=0,readyGeneration_=0,guiSerial_=0,guiGeneration_=0,paintedSerial_=0,paintedGeneration_=0;
    RenderStageTiming worldTiming_,commandTiming_,extraTiming_,exportTiming_,turnTiming_;
    RenderStageTiming activeWorldTiming_,activeCommandTiming_;
    QJsonArray slowTurns_;
    std::uint64_t exports_=0,skipped_=0,highWater_=0,turns_=0,exportAllocations_=0;
};
