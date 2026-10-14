#pragma once
#include "../../renderer/gpu_production_queue.hpp"
#include "native_render_target.hpp"
#include "native_frame_trace.hpp"
#include "../../compat/legacy/world_continuous.hpp"
#include "../../compat/legacy/world_snapshot_queue.hpp"
#include <chrono>
#include "../../compat/legacy/world_rolling_session.hpp"
#include "../../compat/legacy/world_rolling_handoff.hpp"
#include <QElapsedTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <optional>
#include <future>
class GlViewport;
class ResidentProfileCapture;
struct ResidentWorldGpuBackend;
// Application service: strictly pairs a completed native World with an ordered
// command import and an independently verified original return before reuse.
class ResidentWorldSession final {
public:
    ResidentWorldSession(GlViewport&,const QString& channel,const QString& game,bool independent=false);
    ResidentWorldSession(NativeRenderTarget,const QString& channel,const QString& game,bool independent=false);
    ~ResidentWorldSession();
    void poll(bool acceptedNormalQuit=false);
    void abort(const QString&);
    mnm::render::GpuFrame import(std::uint64_t generation,std::uint64_t nonce);
    QJsonObject report() const;
    bool ready(std::uint64_t generation,std::uint64_t nonce) const;
    bool independent() const{return independent_;}
    mnm::render::GpuFrame drawIndependent(std::uint64_t generation);
    struct DisplayState {std::chrono::steady_clock::time_point deadline;std::uint64_t generation;bool active,pending;};
    DisplayState displayState() const{return {nextFrame_,completed_,independent_&&(bool(world_)||(asynchronous_&&completed_))&&!normalQuitRetired_&&error_.isEmpty(),bool(ownedPending_)};}
    void pollOwned(bool);
    void noteIndependentPresentation(double uiMilliseconds);
    std::uint64_t nativeFrames() const{return completed_+extraFrames_;}
    NativeFrameSourceTiming lastSourceTiming() const{return lastSourceTiming_;}
    void saveReport(const QString&);
private:
    static bool productionRetired(bool asynchronous,std::uint64_t completed,const mnm::render::GpuProductionQueue::Stats& p){
        return !asynchronous || (p.error.empty() && p.submitted==completed && p.produced==completed && p.taken==completed && !p.outstanding);
    }
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
    NativeRenderTarget target_;
    mnm::legacy::WorldContinuousChannel channel_;
    std::unique_ptr<mnm::render::GlBlitter> renderer_;
    std::unique_ptr<mnm::legacy::WorldRollingSession> world_;
    QString game_;
    bool independent_=false;
    bool retainCompletedImage_=false;
    bool asynchronous_=false;
    std::shared_ptr<ResidentWorldGpuBackend> gpuBackend_;
    std::unique_ptr<mnm::render::GpuProductionQueue> gpuProduction_;
    mnm::render::RenderStats producerStats_;
    QString producerDriver_;
    std::uint64_t submittedWorlds_=0,producerReadbacks_=0,producerCaptures_=0;
    double productionMs_=0,maxProductionMs_=0;
    void pollAsynchronous(bool);
    std::unique_ptr<mnm::legacy::WorldSnapshotQueue> snapshotQueue_;
    std::optional<mnm::legacy::OwnedWorldSnapshot> ownedPending_;
    mnm::render::GpuFrame ownedFrame_;
    std::uint64_t extraFrames_=0,extraPresented_=0,frameMisses_=0;
    double extraWorldMs_=0,extraUiMs_=0,maxExtraUiMs_=0;
    QElapsedTimer renderRate_;
    std::uint64_t renderRateBase_=0;
    std::chrono::steady_clock::time_point nextFrame_{};
    std::optional<Pending> pending_;
    std::uint64_t completed_=0,imports_=0,returns_=0,rasters_=0,regionQueries_=0;
    std::uint32_t firstTick_=0,lastTick_=0;
    std::uint64_t profiled_=0;
    double decodeMs_=0,handoffMs_=0,nativeMs_=0,prepareMs_=0,submitMs_=0,maxNativeMs_=0,producerMs_=0;
    QString error_;
    bool normalQuitRetired_=false;
    bool stageProfile_=false;
    bool frameTrace_=false;
    NativeFrameSourceTiming lastSourceTiming_;
    QJsonArray stageSamples_;
    std::unique_ptr<ResidentProfileCapture> capture_;
    std::uint64_t gpuDropped_=0,samplesDropped_=0;
    std::uint32_t previousStart_=0,previousReturn_=0;
    QElapsedTimer profileClock_;
    void collectGpuTimings();
};
