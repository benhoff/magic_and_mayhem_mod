#pragma once
#include "alternate_canvas.hpp"
#include "blit.hpp"
#include <optional>

namespace mnm::render {
// Native resident policy: bounded padded GPU backing and a visible World plane.
// Coordinates stay visible-relative; padding is native storage, not a recovered
// original offset. CPU ownership is
// used for publication identity, initialization and an exclusive frame lease;
// subsequent pixels stay on the GPU. The renderer must outlive this owner.
// This does not publish either original executable pointer slot.
class GpuAlternateCanvas final {
public:
  GpuAlternateCanvas(GlBlitter& renderer, int width, int height,
                     std::uint32_t xPadding=0,std::uint32_t yPadding=0)
      : renderer_(renderer), layout_{width==800,xPadding,yPadding} {
    if (!((width == 640 && height == 480) || (width == 800 && height == 600)))
      throw std::invalid_argument("Live alternate canvas requires a video extent");
    handle_ = owner_.publish(layout_);
    auto initial = owner_.acquire(handle_);
    Image clear{int(layout_.stride()), int(layout_.height()), {}};
    clear.pixels.assign(initial.words().begin(), initial.words().end());
    backing_ = renderer_.create(clear, {16, {0xf800, 0x07e0, 0x001f}});
    try {
      surface_ = padded()?renderer_.allocate(width,height,{16,{0xf800,0x07e0,0x001f}}):backing_;
    } catch (...) { renderer_.destroy(backing_); throw; }
  }
  ~GpuAlternateCanvas() {
    frame_.reset();
    owner_.retire();
    if (surface_ && surface_!=backing_) renderer_.destroy(surface_);
    if (backing_) renderer_.destroy(backing_);
  }
  GpuAlternateCanvas(const GpuAlternateCanvas&) = delete;
  GpuAlternateCanvas& operator=(const GpuAlternateCanvas&) = delete;
  SurfaceId surface() const { return surface_; }
  // Scoped GPU borrow for storage inspection; owner must remain alive.
  SurfaceId backingSurface() const { return backing_; }
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
    if(padded()) {
      const int x=int(layout_.xPadding),y=int(layout_.yPadding);
      const int w=int(layout_.visibleWidth()),h=int(layout_.visibleHeight());
      if(renderer_.dimensions(ordinary)!=QSize(w,h))
        throw std::invalid_argument("Alternate copy-back extent");
      renderer_.copy(surface_,backing_,{0,0,w,h},x,y);
      renderer_.copy(backing_,ordinary,{x,y,x+w,y+h},0,0);
    } else renderer_.restoreSurface(surface_, ordinary);
  }
  void end() {
    if (!frame_) throw std::logic_error("Alternate return outside World frame");
    frame_.reset();
  }
private:
  GlBlitter& renderer_;
  const AlternateCanvas::Layout layout_;
  bool padded() const { return layout_.xPadding || layout_.yPadding; }
  AlternateCanvas owner_;
  AlternateCanvas::Handle handle_;
  std::optional<AlternateCanvas::Lease> frame_;
  SurfaceId surface_ = 0;
  SurfaceId backing_ = 0;
};
} // namespace mnm::render
