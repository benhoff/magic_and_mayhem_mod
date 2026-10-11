#pragma once
#include "canvas_mirror.hpp"
#include <unordered_map>
namespace mnm::render {
// Owns native GPU canvases. The original process never supplies destination
// pixels and never receives this storage's contents during ordinary delivery.
class GpuCanvasMirror final : public CanvasMirror {
public:
  explicit GpuCanvasMirror(GlBlitter &renderer) : renderer_(renderer) {}
  ~GpuCanvasMirror();
  void create(std::uint32_t, int, int) override;
  void release(std::uint32_t) override;
  void update(std::uint32_t, int, int, const Image &) override;
  void updateMasked(std::uint32_t,int,int,const Image&,
                    const std::vector<std::uint8_t>&) override;
  void copy(std::uint32_t, std::uint32_t, Rect, int, int,
            std::optional<std::uint16_t>) override;
  SurfaceId surface(std::uint32_t) const;
private:
  GlBlitter &renderer_;
  std::unordered_map<std::uint32_t, SurfaceId> surfaces_;
};
}
