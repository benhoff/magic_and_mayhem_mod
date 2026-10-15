#pragma once
#include "canvas_world.hpp"
#include <optional>

namespace mnm::legacy {
struct DrawingCheckpoint {
    unsigned sequence, ordinal, canvas;
    render::Image image;
};
// Low16 return only, for the sampled World raster domain. Unsupported return
// contracts are explicit; this is not the complete consumer/picking state.
struct DrawingRasterResult {
    unsigned sequence, queue, canvas, backend;
    std::optional<unsigned> ax;
};
// Explicit end of a declared finite scene, independent of diagnostic checkpoint
// numbers or legacy presentation. A caller must supply the complete producer
// prefix through this boundary, including UI work after World returns.
struct DrawingBoundary {
    unsigned sequence, queues, canvas, width, height;
};
// Build-specific input adapter over native services. No widgets, runtime
// pointers, destination seeds, diagnostic backgrounds or oracle callbacks.
// Execution errors poison the instance; a new source history is required.
class DrawingScene final {
public:
    DrawingScene(render::GlBlitter &, const assets::AssetStore &,
                 std::function<std::vector<std::uint8_t>(const std::string &)> source);
    ~DrawingScene();
    DrawingScene(const DrawingScene &) = delete;
    DrawingScene &operator=(const DrawingScene &) = delete;
    // Consume normalized closed records from decodeCanvasProducers.
    std::optional<DrawingCheckpoint> apply(const CanvasProducer &);
    // Publish an explicitly complete frame while retaining native history for
    // subsequent frames. Any next producer invalidates this service's output;
    // previously returned GPU leases retain their complete immutable frame.
    // Menu-only boundaries may have zero completed World queues.
    void publish(DrawingBoundary);
    void seal(DrawingBoundary);
    render::Image read();
    // Owned source snapshot for comparison tools; an active World canvas is
    // unavailable until its completion. This does not publish a session frame.
    render::Image sourceSnapshot(unsigned canvas) const;
    render::GpuFrame presentGpu();
    std::optional<DrawingRasterResult> rasterResult() const;
    unsigned sequence() const { return sequence_; }
    unsigned queues() const { return completedQueues_; }
    std::uint64_t worldDraws() const { return worldDraws_; }
    unsigned checkpoints() const { return checkpoints_; }
    unsigned frames() const { return frames_; }
private:
    void requirePublished() const;
    render::GlBlitter &renderer_;
    assets::AssetStore store_;
    CanvasProducerReplay replay_;
    std::unique_ptr<CanvasWorld> world_;
    std::optional<DrawingRasterResult> rasterResult_;
    render::SurfaceId output_=0;
    unsigned sequence_=0,checkpoints_=0,publishedSequence_=0,frames_=0;
    DrawingBoundary published_{};
    unsigned completedQueues_=0,activeQueue_=0,worldWidth_=0,worldHeight_=0;
    std::uint64_t worldSequence_=0,worldDraws_=0;
    bool poisoned_=false,sealed_=false;
};
}
