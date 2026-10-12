#pragma once
#include "../../compat/legacy/world_continuous.hpp"
#include "../../compat/legacy/world_rolling_session.hpp"
#include "../../compat/legacy/world_rolling_handoff.hpp"
#include <QElapsedTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <optional>
#include <future>
class GlViewport;
class ResidentProfileCapture;
// Application service: strictly pairs a completed native World with an ordered
// command import and an independently verified original return before reuse.
class ResidentWorldSession final {
public:
    ResidentWorldSession(GlViewport&,const QString& channel,const QString& game);
    ~ResidentWorldSession();
    void poll(bool acceptedNormalQuit=false);
    void abort(const QString&);
    mnm::render::GpuFrame import(std::uint64_t generation,std::uint64_t nonce);
    QJsonObject report() const;
    void saveReport(const QString&);
private:
    unsigned phaseLogRows_=0;
    struct ReportSnapshot {QString path;QByteArray bytes;};
    std::future<void> reportWork_;
    std::optional<ReportSnapshot> latestReport_;
    void serviceReport();
    void queueReport(const QString&);
    void launchReport(ReportSnapshot);
    friend struct ResidentWorldSessionTestAccess;
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
    std::uint64_t completed_=0,imports_=0,returns_=0,rasters_=0,regionQueries_=0;
    std::uint32_t firstTick_=0,lastTick_=0;
    std::uint64_t profiled_=0;
    double decodeMs_=0,handoffMs_=0,nativeMs_=0,prepareMs_=0,submitMs_=0,maxNativeMs_=0,producerMs_=0;
    QString error_;
    bool normalQuitRetired_=false;
    bool stageProfile_=false;
    QJsonArray stageSamples_;
    std::unique_ptr<ResidentProfileCapture> capture_;
    std::uint64_t gpuDropped_=0,samplesDropped_=0;
    std::uint32_t previousStart_=0,previousReturn_=0;
    QElapsedTimer profileClock_;
    void collectGpuTimings();
};
