#include "drawing_session.hpp"
#include <stdexcept>

namespace mnm::legacy {
bool operator==(const DrawingFrameIdentity &a,const DrawingFrameIdentity &b){
    const auto &x=a.source,&y=b.source;
    return a.session==b.session&&a.segment==b.segment&&a.presentation==b.presentation&&
        a.stage==b.stage&&x.sequence==y.sequence&&x.queues==y.queues&&x.canvas==y.canvas&&
        x.width==y.width&&x.height==y.height;
}
DrawingSession::DrawingSession(render::GlBlitter &renderer,const assets::AssetStore &store,
                              Source source,DrawingSessionLimits limits)
    :renderer_(renderer),store_(store),source_(std::move(source)),limits_(limits){
    if(!source_||!limits.segments||limits.segments>64||!limits.frames||limits.frames>8192||
       !limits.records||limits.records>1048576)
        throw std::invalid_argument("Invalid native drawing session limits or source");
}
DrawingSession::~DrawingSession(){stop();}
void DrawingSession::begin(std::uint64_t session){
    if(state_==DrawingSessionState::Running||!session||session<=session_)
        throw std::invalid_argument("Drawing session is active or its identity was reused");
    stop();session_=session;segments_=frames_=records_=0;
    state_=DrawingSessionState::Running;
}
void DrawingSession::requireRunning(std::uint64_t session) const {
    if(state_!=DrawingSessionState::Running||session!=session_)
        throw std::invalid_argument("Drawing session is not the active session");
}
void DrawingSession::requireSegment(unsigned segment) const {
    if(!scene_||segment!=segments_)
        throw std::invalid_argument("Drawing source belongs to a different segment");
}
void DrawingSession::fail() noexcept {
    current_.reset();scene_.reset();state_=DrawingSessionState::Failed;
}
void DrawingSession::transition(std::uint64_t session,unsigned segment,DrawingStage stage){
    // Calls after terminal states cannot change a completed frame. Invalid
    // mutations of a running session fail closed, including stale identities.
    if(state_!=DrawingSessionState::Running)requireRunning(session);
    try{
        requireRunning(session);
        if(segments_==limits_.segments||segment!=segments_+1||
           stage<DrawingStage::Scene||stage>DrawingStage::Results)
            throw std::invalid_argument("Drawing segment order, stage or budget is invalid");
        current_.reset();scene_.reset();
        scene_=std::make_unique<DrawingScene>(renderer_,store_,source_);
        segments_=segment;stage_=stage;
    }catch(...){fail();throw;}
}
std::optional<DrawingCheckpoint> DrawingSession::apply(std::uint64_t session,unsigned segment,
                                                     const CanvasProducer &record){
    if(state_!=DrawingSessionState::Running)requireRunning(session);
    try{
        requireRunning(session);requireSegment(segment);
        if(records_==limits_.records)throw std::invalid_argument("Drawing session record budget exhausted");
        current_.reset();auto checkpoint=scene_->apply(record);++records_;return checkpoint;
    }catch(...){fail();throw;}
}
DrawingFrameIdentity DrawingSession::publish(std::uint64_t session,unsigned segment,
                                            unsigned presentation,DrawingBoundary boundary){
    if(state_!=DrawingSessionState::Running)requireRunning(session);
    try{
        requireRunning(session);requireSegment(segment);
        if(frames_==limits_.frames||presentation!=frames_+1)
            throw std::invalid_argument("Drawing presentation order or budget is invalid");
        current_.reset();scene_->publish(boundary);++frames_;
        current_=DrawingFrameIdentity{session_,segments_,frames_,stage_,boundary};return *current_;
    }catch(...){fail();throw;}
}
void DrawingSession::requireFrame(const DrawingFrameIdentity &frame) const {
    if((state_!=DrawingSessionState::Running&&state_!=DrawingSessionState::Finished)||
       !current_||!(*current_==frame)||!scene_)
        throw std::invalid_argument("Drawing frame is stale, incomplete or from another session");
}
render::Image DrawingSession::read(const DrawingFrameIdentity &frame){
    requireFrame(frame); // Stale queries do not poison a newer valid session.
    try{return scene_->read();}catch(...){fail();throw;}
}
render::GpuFrame DrawingSession::presentGpu(const DrawingFrameIdentity &frame){
    requireFrame(frame);
    try{return scene_->presentGpu();}catch(...){fail();throw;}
}
void DrawingSession::finish(std::uint64_t session){
    if(state_!=DrawingSessionState::Running)requireRunning(session);
    try{
        requireRunning(session);
        if(!current_||!scene_)throw std::invalid_argument("Drawing session has no complete terminal frame");
        scene_->seal(current_->source);state_=DrawingSessionState::Finished;
    }catch(...){fail();throw;}
}
void DrawingSession::stop() noexcept {
    current_.reset();scene_.reset();state_=DrawingSessionState::Stopped;
}
}
