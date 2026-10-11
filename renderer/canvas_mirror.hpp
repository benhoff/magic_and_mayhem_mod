#pragma once
#include "blit.hpp"
namespace mnm::render {
// Optional native storage observer. Copies remain storage operations; uploaded
// patches contain only independently composed, known CPU pixels. A CPU shadow
// may become undefined when a GPU producer takes ownership of a region.
class CanvasMirror {
public:
  virtual ~CanvasMirror() = default;
  virtual void create(std::uint32_t id, int width, int height) = 0;
  virtual void release(std::uint32_t id) = 0;
  virtual void update(std::uint32_t id, int x, int y, const Image &) = 0;
  virtual void copy(std::uint32_t source, std::uint32_t destination, Rect,
                    int x, int y, std::optional<std::uint16_t> key) = 0;
};
}
