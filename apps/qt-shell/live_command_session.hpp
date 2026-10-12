#pragma once
#include "native_render_target.hpp"
#include "live_command_renderer.hpp"
#include "render_control.hpp"
// Application lifecycle service; widgets receive semantic presentation states.
class LiveCommandSession final {
public:
    enum class State {WaitingFrame,Active,Recovering,Fallback,Ended,Stopping};
    explicit LiveCommandSession(GlViewport& viewport):target_(viewportRenderTarget(viewport)){}
    explicit LiveCommandSession(NativeRenderTarget target):target_(std::move(target)){}
    ~LiveCommandSession();
    bool create(const QString& path,quint32 session,quint32 version=1,int initialFrameTimeoutMs=10000);
    // Discard any backlog and attach through a strict complete owned checkpoint.
    bool attachCheckpoint();
    bool poll();
    // A native movie owns presentation while the producer waits for it.
    void setMediaActive(bool active);
    bool requestStop();
    bool finishProducer();
    void abort();
    bool ended() const{return state_==State::Ended;}
    bool hasPendingCommands() const{return renderer_ && state_!=State::Recovering && state_!=State::Fallback && state_!=State::Ended && renderer_->hasPendingCommands();}
    QString error() const{return error_;}
    State state() const{return state_;}
    const mnm::render::CommandResult* result() const{return renderer_?renderer_->result():nullptr;}
    unsigned recoveries() const{return retries_;}
    std::function<void()> framePresented;
    std::function<mnm::render::GpuFrame(std::uint64_t,std::uint64_t)> residentWorld;
    std::function<bool(std::uint64_t,std::uint64_t)> residentReady;
    bool prepareResidentFrames=false,reuseSealedUi=false;
    bool waitingForWorld() const{return renderer_ && renderer_->waitingForWorld();}
    std::uint64_t preparedGeneration() const{return renderer_?renderer_->preparedGeneration():0;}
    bool presentPrepared(const mnm::render::GpuFrame& frame,std::uint64_t generation){return renderer_ && renderer_->presentPrepared(frame,generation);}
    std::uint64_t sealedGeneration() const{return renderer_?renderer_->sealedGeneration():0;}
    mnm::render::CommandDisplayState displayState() const{return renderer_?renderer_->displayState():mnm::render::CommandDisplayState{};}
    bool presentSealed(const mnm::render::GpuFrame& frame,std::uint64_t generation){return renderer_ && renderer_->presentSealed(frame,generation);}
    std::function<void(State)> stateChanged;
private:
    friend struct LiveCommandSessionTestAccess;
    qint64 waitingElapsed() const;
    bool fresh(const QString& path,quint32 session);
    bool fail(const QString& message);
    bool recover(bool checkpoint=false);
    void change(State state);
    NativeRenderTarget target_;std::unique_ptr<LiveCommandRenderer> renderer_;RenderControl control_;
    QString path_,error_;quint32 session_=0,version_=0;unsigned retries_=0;
    int initialFrameTimeoutMs_=10000;
    bool stopAcknowledged_=false;
    bool mediaActive_=false;
    qint64 mediaStartedAt_=0,mediaPausedMs_=0;
    State state_=State::WaitingFrame;QElapsedTimer deadline_;
};
