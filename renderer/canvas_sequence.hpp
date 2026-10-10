#pragma once
#include "../assets/sprite_loader.hpp"
#include "blit.hpp"
#include "minimap/overlays.hpp"
#include <functional>
#include <array>
#include <unordered_map>
namespace mnm::render {
// Owned native producer state. Storage remains undefined until admitted source
// operations define it. Diagnostic destination images are never accepted here.
class CanvasSequence final {
public:
  void create(std::uint32_t, int width, int height);
  void terrainMap(std::uint32_t id, int x, int y, int width, int height, int centerX, int centerY, const std::vector<std::uint16_t> &colours, const std::vector<unsigned char> &hidden = {}, std::uint32_t previous = 0);
  std::size_t minimap(std::uint32_t, int stride, const std::function<std::size_t(MinimapPlane &)> &);
  void fade(std::uint32_t id, unsigned stride = 0,
            std::optional<std::size_t> physicalWords = {}, std::uint16_t mask = 0x7bef);
  void release(std::uint32_t);
  void panel(std::uint32_t, Rect, unsigned mode, unsigned percent = 0,
             bool pressed = false, bool reverse = true);
  void addRgb(std::uint32_t, Rect, std::array<unsigned, 3>);
  void fill(std::uint32_t, Rect, std::uint16_t);
  void copy(std::uint32_t source, std::uint32_t destination, Rect, int x, int y,
            std::optional<std::uint16_t> key = {});
  void update(std::uint32_t, int x, int y, const Image &);
  void glyph(std::uint32_t, const assets::SpriteFrame &, int x, int y, Rect,
             std::array<unsigned, 3> tint, const std::array<float, 64> &);
  void raster(std::uint32_t, const assets::SpriteFrame &, int x, int y, Rect,
              unsigned mode, int backend,
              const std::array<std::uint16_t, 256> &,
              const std::array<int, 16> &offsets = {}, unsigned period = 16);
  Image read(std::uint32_t) const;
  std::size_t surfaces() const { return surfaces_.size(); }

private:
  struct Surface {
    Image image;
    std::vector<unsigned char> defined;
  };
  std::unordered_map<std::uint32_t, Surface> surfaces_;
  std::size_t pixels_ = 0;
  Surface &surface(std::uint32_t);
};
} // namespace mnm::render
