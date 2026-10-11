#pragma once
#include "drawing_scene.hpp"
#include <cstdint>

namespace mnm::legacy {
// Native host labels, not recovered screen numbers or simulation actions.
enum class DrawingStage { Scene, Menu, Loading, World, Results };
enum class DrawingSessionState { Idle, Running, Finished, Failed, Stopped };
struct DrawingSessionLimits {
    unsigned segments = 64, frames = 8192, records = 1048576;
};
struct DrawingFrameIdentity {
    std::uint64_t session;
    unsigned segment, presentation;
    DrawingStage stage;
    DrawingBoundary source;
};
bool operator==(const DrawingFrameIdentity &, const DrawingFrameIdentity &);

// Thread-confined native lifecycle over finite, closed producer histories.
// A segment starts undefined; transition discards the previous source history.
// These policies neither execute gameplay actions nor recover transition state.
class DrawingSession final {
public:
    using Source = std::function<std::vector<std::uint8_t>(const std::string &)>;
    DrawingSession(render::GlBlitter &, const assets::AssetStore &, Source,
                   DrawingSessionLimits = {});
    ~DrawingSession();
    DrawingSession(const DrawingSession &) = delete;
    DrawingSession &operator=(const DrawingSession &) = delete;
    // IDs increase across restarts, including recovery after failure/stop.
    void begin(std::uint64_t session);
    void transition(std::uint64_t session, unsigned segment, DrawingStage);
    std::optional<DrawingCheckpoint> apply(std::uint64_t session, unsigned segment,
                                           const CanvasProducer &);
    DrawingFrameIdentity publish(std::uint64_t session, unsigned segment,
                                 unsigned presentation, DrawingBoundary);
    render::Image read(const DrawingFrameIdentity &);
    render::GpuFrame presentGpu(const DrawingFrameIdentity &);
    void finish(std::uint64_t session);
    // Idempotent host shutdown. Previously leased GPU frames remain immutable
    // until their external owners release them; session access is invalidated.
    void stop() noexcept;
    DrawingSessionState state() const { return state_; }
    std::uint64_t session() const { return session_; }
    unsigned segments() const { return segments_; }
    unsigned frames() const { return frames_; }
    unsigned records() const { return records_; }
    std::optional<DrawingFrameIdentity> current() const { return current_; }
private:
    void requireRunning(std::uint64_t) const;
    void requireSegment(unsigned) const;
    void requireFrame(const DrawingFrameIdentity &) const;
    void fail() noexcept;
    render::GlBlitter &renderer_;
    assets::AssetStore store_;
    Source source_;
    DrawingSessionLimits limits_;
    std::unique_ptr<DrawingScene> scene_;
    std::optional<DrawingFrameIdentity> current_;
    DrawingSessionState state_ = DrawingSessionState::Idle;
    DrawingStage stage_ = DrawingStage::Scene;
    std::uint64_t session_ = 0;
    unsigned segments_ = 0, frames_ = 0, records_ = 0;
};
}
