#include "live_command_session.hpp"
#include "gl_viewport.hpp"
#include <limits>
LiveCommandSession::~LiveCommandSession(){abort();}
void LiveCommandSession::change(State state){state_=state;if(stateChanged)stateChanged(state);}
bool LiveCommandSession::fresh(const QString& path,quint32 session){
    renderer_=std::make_unique<LiveCommandRenderer>(viewport_);
    renderer_->framePresented=[this]{if(state_==State::WaitingFrame)change(State::Active);if(state_!=State::Stopping && framePresented)framePresented();};
    if(!renderer_->create(path,session,version_))return fail(renderer_->error());
    return true;
}
bool LiveCommandSession::create(const QString& path,quint32 session,quint32 version,int initialFrameTimeoutMs){
    initialFrameTimeoutMs_=initialFrameTimeoutMs;
    path_=path;session_=session;version_=version;deadline_.start();
    if(!fresh(path,session))return false;
    if(version==2 && !control_.create(path+".control",session))return fail(control_.error());
    return true;
}
void LiveCommandSession::abort(){control_.cancel();if(renderer_)renderer_->abort();viewport_.setGpuFrame({});}
bool LiveCommandSession::fail(const QString& message){error_=message;abort();change(State::Fallback);return false;}
bool LiveCommandSession::attachCheckpoint(){
    if(state_!=State::WaitingFrame && state_!=State::Active)return false;
    return recover(true);
}
bool LiveCommandSession::recover(bool checkpoint){
    if(version_!=2 || retries_>=MNM_RENDER_CONTROL_V1_MAX_RECOVERIES || session_==std::numeric_limits<quint32>::max()){
        const auto cause=renderer_?renderer_->error():QString();
        return fail("Rendering recovery unavailable or exhausted"+(cause.isEmpty()?QString():": "+cause));
    }
    renderer_->abort();renderer_.reset();viewport_.setGpuFrame({});
    ++retries_;++session_;const auto next=path_+QString(".retry-%1").arg(retries_);
    change(State::Recovering);
    if(!fresh(next,session_))return false;
    if(!(checkpoint?control_.checkpoint(next,session_):control_.recover(next,session_)))return fail(control_.error());
    return true;
}
bool LiveCommandSession::requestStop(){
    if(ended() || state_==State::Stopping)return true;
    if(version_!=2 || state_==State::Recovering || state_==State::Fallback)return fail("Rendering stop unavailable during recovery/fallback");
    if(!control_.stop())return fail(control_.error());
    change(State::Stopping);return true;
}
bool LiveCommandSession::poll(){
    if(state_==State::Fallback)return false;
    if(ended())return true;
    if(state_==State::Stopping){
        const int response=stopAcknowledged_?1:control_.poll();
        if(response<0)return fail(control_.error());
        stopAcknowledged_=response==1;
        if(!renderer_->ended() && !renderer_->poll())return fail(renderer_->error());
        if(stopAcknowledged_ && renderer_->ended()){change(State::Ended);return true;}
        return true;
    }
    if(state_==State::Recovering){
        const int status=control_.poll();if(status<0)return fail(control_.error());
        if(!status)return true;
        deadline_.restart();change(State::WaitingFrame);
    }
    if(!renderer_->poll()){
        if(state_==State::WaitingFrame && retries_)return fail(renderer_->error());
        return recover();
    }
    if(renderer_->ended()){control_.cancel();change(State::Ended);return true;}
    if(version_==2 && state_==State::WaitingFrame && deadline_.elapsed()>=(retries_?MNM_RENDER_CONTROL_V1_FRAME_TIMEOUT_MS:initialFrameTimeoutMs_)){
        if(retries_)return fail("Recovered producer did not publish a complete frame");
        return recover();
    }
    return true;
}
bool LiveCommandSession::finishProducer(){
    control_.cancel();
    if(ended())return true;
    if(state_==State::Recovering || state_==State::Fallback)return fail(error_.isEmpty()?"Producer stopped during recovery":error_);
    if(!renderer_ || !renderer_->finishProducer())return fail(renderer_?renderer_->error():"No command producer");
    change(State::Ended);return true;
}
