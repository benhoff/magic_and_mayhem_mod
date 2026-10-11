#pragma once
#include "../../compat/legacy/world_continuous.hpp"
#include "../../compat/legacy/world_rolling_session.hpp"
#include "../../compat/legacy/world_rolling_handoff.hpp"
#include <QElapsedTimer>
#include <QJsonObject>
#include <optional>
class GlViewport;
// Application service: strictly pairs a completed native World with an ordered
// command import and an independently verified original return before reuse.
class ResidentWorldSession final {
public:
    ResidentWorldSession(GlViewport&,const QString& channel,const QString& game);
    ~ResidentWorldSession();
    void poll();
    void abort(const QString&);
    mnm::render::GpuFrame import(std::uint64_t generation,std::uint64_t nonce);
    QJsonObject report() const;
    void saveReport(const QString&) const;
private:
    struct Pending {
        mnm::legacy::WorldContinuousChannel::Block block;
        mnm::legacy::RollingHandoff handoff;
        mnm::render::GpuFrame frame;
        bool imported=false,returned=false;
        QElapsedTimer deadline;
    };
    GlViewport& viewport_;
    mnm::legacy::WorldContinuousChannel channel_;
    std::unique_ptr<mnm::render::GlBlitter> renderer_;
    std::unique_ptr<mnm::legacy::WorldRollingSession> world_;
    QString game_;
    std::optional<Pending> pending_;
    std::uint64_t completed_=0,imports_=0,returns_=0,rasters_=0;
    std::uint32_t firstTick_=0,lastTick_=0;
    QString error_;
};
