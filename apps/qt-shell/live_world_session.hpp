#pragma once
#include "gl_viewport.hpp"
#include "world_channel.hpp"
#include "world_resources.hpp"
#include <QJsonObject>
#include <QJsonArray>
class LiveWorldSession final {
public:
    LiveWorldSession(GlViewport&,const QString& channel,const QString& root,const QString& diagnostics={});
    ~LiveWorldSession();
    bool poll();
    void close();
    unsigned presentations() const{return presentations_;}
    bool ended() const{return channel_.state()==MNM_WCH_ENDED;}
    const QString& error() const{return error_;}
    const QString& status() const{return status_;}
    QJsonObject report() const;
private:
    GlViewport& viewport_;mnm::legacy::WorldChannel channel_;
    mnm::assets::AssetStore store_;mnm::assets::ResourceManager resources_;
    mnm::legacy::WorldResources catalogue_;
    std::unique_ptr<mnm::render::GlBlitter> renderer_;
    std::unique_ptr<mnm::render::SceneRenderer> scene_;
    QSize size_;QString error_,diagnostics_,status_="Waiting for World drawing from the game.";bool closed_=false;
    unsigned presentations_=0;quint64 compared_=0,mismatches_=0,readbacks_=0,remaining_=0;
    qint64 maxPollMs_=0;QJsonArray frames_;
    quint64 refused_=0;QJsonObject refusalCounts_;QJsonArray refusals_;
    std::optional<mnm::legacy::WorldPacket> pending_;
    std::optional<mnm::legacy::WorldFrame> frame_;
};
