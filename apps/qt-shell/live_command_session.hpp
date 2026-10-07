#pragma once
#include "live_command_renderer.hpp"
#include "render_control.hpp"
// Application lifecycle service; widgets receive semantic presentation states.
class LiveCommandSession final {
public:
    enum class State {WaitingFrame,Active,Recovering,Fallback,Ended,Stopping};
    explicit LiveCommandSession(GlViewport& viewport):viewport_(viewport){}
    ~LiveCommandSession();
    bool create(const QString& path,quint32 session,quint32 version=1);
    // Discard any backlog and attach through a strict complete owned checkpoint.
    bool attachCheckpoint();
    bool poll();
    bool requestStop();
    bool finishProducer();
    void abort();
    bool ended() const{return state_==State::Ended;}
    QString error() const{return error_;}
    State state() const{return state_;}
    const mnm::render::CommandResult* result() const{return renderer_?renderer_->result():nullptr;}
    unsigned recoveries() const{return retries_;}
    std::function<void()> framePresented;
    std::function<void(State)> stateChanged;
private:
    bool fresh(const QString& path,quint32 session);
    bool fail(const QString& message);
    bool recover(bool checkpoint=false);
    void change(State state);
    GlViewport& viewport_;std::unique_ptr<LiveCommandRenderer> renderer_;RenderControl control_;
    QString path_,error_;quint32 session_=0,version_=0;unsigned retries_=0;
    bool stopAcknowledged_=false;
    State state_=State::WaitingFrame;QElapsedTimer deadline_;
};
