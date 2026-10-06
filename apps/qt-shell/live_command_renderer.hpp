#pragma once
#include "command_channel.hpp"
#include "commands.hpp"
#include <memory>
class GlViewport;
// Application adapter; callers own polling, launch lifecycle and fallback UI.
class LiveCommandRenderer final {
public:
    explicit LiveCommandRenderer(GlViewport& viewport,bool verify=false);
    ~LiveCommandRenderer();
    bool create(const QString& path,quint32 session,quint32 version=1);
    bool open(const QString& path);
    bool poll(quint32 budget=MNM_RENDER_COMMANDS_V1_POLL_BYTES);
    bool finishProducer();
    void abort();
    std::function<void()> framePresented;
    bool ended() const{return ended_;}
    QString error() const{return error_;}
    unsigned presentations() const;
    const mnm::render::CommandResult* result() const;
private:
    void fail(const QString& reason);
    GlViewport& viewport_;bool verify_,ended_=false,closed_=false;
    CommandChannel channel_;mnm::render::CommandDecoder decoder_;
    std::unique_ptr<mnm::render::GlBlitter> renderer_;
    std::unique_ptr<mnm::render::CommandConsumer> consumer_;
    std::vector<mnm::render::SurfaceCommand> pending_;std::size_t cursor_=0;
    QString error_;
};
