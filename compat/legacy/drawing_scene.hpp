#pragma once
#include "canvas_world.hpp"
#include <optional>

namespace mnm::legacy {
struct DrawingCheckpoint {
    unsigned sequence, ordinal, canvas;
    render::Image image;
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
    render::GpuFrame presentGpu();
    unsigned sequence() const { return sequence_; }
    unsigned queues() const { return world_.completedQueues(); }
    std::uint64_t worldDraws() const { return world_.draws(); }
    unsigned checkpoints() const { return checkpoints_; }
    unsigned frames() const { return frames_; }
private:
    void requirePublished() const;
    render::GlBlitter &renderer_;
    CanvasProducerReplay replay_;
    CanvasWorld world_;
    render::SurfaceId output_=0;
    unsigned sequence_=0,checkpoints_=0,publishedSequence_=0,frames_=0;
    DrawingBoundary published_{};
    bool poisoned_=false,sealed_=false;
};
}
