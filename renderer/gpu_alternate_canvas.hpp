#pragma once
#include "alternate_canvas.hpp"
#include "blit.hpp"
#include <optional>

namespace mnm::render {
// Native resident policy: one zero-padding GPU World plane. CPU ownership is
// used for publication identity, initialization and an exclusive frame lease;
// subsequent pixels stay on the GPU. The renderer must outlive this owner.
// This does not publish either original executable pointer slot.
class GpuAlternateCanvas final {
public:
  GpuAlternateCanvas(GlBlitter& renderer, int width, int height) : renderer_(renderer) {
    if (!((width == 640 && height == 480) || (width == 800 && height == 600)))
      throw std::invalid_argument("Live alternate canvas requires a video extent");
    handle_ = owner_.publish({width == 800, 0, 0});
    auto initial = owner_.acquire(handle_);
    Image clear{width, height, {}};
    clear.pixels.assign(initial.words().begin(), initial.words().end());
    surface_ = renderer_.create(clear, {16, {0xf800, 0x07e0, 0x001f}});
  }
  ~GpuAlternateCanvas() {
    frame_.reset();
    owner_.retire();
    if (surface_) renderer_.destroy(surface_);
  }
  GpuAlternateCanvas(const GpuAlternateCanvas&) = delete;
  GpuAlternateCanvas& operator=(const GpuAlternateCanvas&) = delete;
  SurfaceId surface() const { return surface_; }
  void begin(SurfaceId nativeHistory) {
    if (frame_) throw std::logic_error("Alternate World frame already active");
    // Full native history is required before copying; no original destination
    // or diagnostic screenshot can enter this service.
    const auto history = renderer_.presentGpu(nativeHistory);
    if (history.size() != renderer_.dimensions(surface_))
      throw std::invalid_argument("Alternate World history extent");
    auto next = owner_.acquire(handle_);
    renderer_.restoreSurface(nativeHistory, surface_);
    frame_.emplace(std::move(next));
  }
  void copyBack(SurfaceId ordinary) {
    if (!frame_) throw std::logic_error("Alternate copy outside World frame");
    renderer_.restoreSurface(surface_, ordinary);
  }
  void end() {
    if (!frame_) throw std::logic_error("Alternate return outside World frame");
    frame_.reset();
  }
private:
  GlBlitter& renderer_;
  AlternateCanvas owner_;
  AlternateCanvas::Handle handle_;
  std::optional<AlternateCanvas::Lease> frame_;
  SurfaceId surface_ = 0;
};
} // namespace mnm::render
