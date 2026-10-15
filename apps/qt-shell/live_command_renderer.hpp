#pragma once
#include "native_render_target.hpp"
#include "command_channel.hpp"
#include "commands.hpp"
#include <memory>
class GlViewport;
// Application adapter; callers own polling, launch lifecycle and fallback UI.
class LiveCommandRenderer final {
public:
    explicit LiveCommandRenderer(GlViewport& viewport,bool verify=false);
    explicit LiveCommandRenderer(NativeRenderTarget,bool verify=false);
    ~LiveCommandRenderer();
    bool create(const QString& path,quint32 session,quint32 version=1);
    bool open(const QString& path);
    bool poll(quint32 budget=MNM_RENDER_COMMANDS_V1_POLL_BYTES);
    bool finishProducer();
    void abort();
    void setPresentationSuspended(bool suspended);
    std::function<void()> framePresented;
    std::function<void(std::uint64_t)> cpuReadPending;
    std::function<mnm::render::GpuFrame(std::uint64_t,std::uint64_t)> residentWorld;
    std::function<bool(std::uint64_t,std::uint64_t)> residentReady;
    bool prepareResidentFrames=false,reuseSealedUi=false;
    bool waitingForWorld() const{return waitingWorld_;}
    std::uint64_t preparedGeneration() const;
    std::uint64_t sealedGeneration() const;
    mnm::render::CommandDisplayState displayState() const;
    bool presentSealed(const mnm::render::GpuFrame&,std::uint64_t);
    bool presentPrepared(const mnm::render::GpuFrame&,std::uint64_t);
    bool ended() const{return ended_;}
    bool hasPendingCommands() const{return !closed_ && (cursor_<pending_.size() || !channel_.drained());}
    QString error() const{return error_;}
    unsigned presentations() const;
    const mnm::render::CommandResult* result() const;
private:
    void fail(const QString& reason);
    NativeRenderTarget target_;bool verify_,ended_=false,closed_=false;
    CommandChannel channel_;std::unique_ptr<mnm::render::CommandDecoder> decoder_;
    std::unique_ptr<mnm::render::GlBlitter> renderer_;
    std::unique_ptr<mnm::render::CommandConsumer> consumer_;
    std::vector<mnm::render::SurfaceCommand> pending_;std::size_t cursor_=0;
    QString error_,cpuReplyPath_;quint32 cpuReplySession_=0;
    void publishCpuRead(const mnm::render::SurfaceCommand&,const mnm::render::Image&);
    bool presentationSuspended_=false;
    mnm::render::GpuFrame deferredPresentation_;
    bool waitingWorld_=false;
};
