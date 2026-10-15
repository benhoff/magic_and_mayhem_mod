#include "gpu_canvas_mirror.hpp"
#include "surface_copy.hpp"
#include <stdexcept>
namespace mnm::render {
GpuCanvasMirror::~GpuCanvasMirror() {
  for (const auto &entry : surfaces_)
    renderer_.destroy(entry.second);
}
SurfaceId GpuCanvasMirror::surface(std::uint32_t id) const {
  return surfaces_.at(id);
}
Image GpuCanvasMirror::readForCpu(std::uint32_t id) {
  auto image=renderer_.read(surface(id));++cpuReaderSnapshots_;return image;
}
void GpuCanvasMirror::create(std::uint32_t id, int width, int height) {
  if (!id || surfaces_.count(id))
    throw std::invalid_argument("Duplicate GPU producer canvas");
  const auto handle = renderer_.allocate(width, height,
      {16, {0xf800, 0x07e0, 0x001f}});
  try {
    surfaces_.emplace(id, handle);
  } catch (...) {
    renderer_.destroy(handle);
    throw;
  }
}
void GpuCanvasMirror::release(std::uint32_t id) {
  const auto handle = surface(id);
  renderer_.destroy(handle);
  surfaces_.erase(id);
}
void GpuCanvasMirror::update(std::uint32_t id, int x, int y,
                              const Image &patch) {
  renderer_.update(surface(id), x, y, patch);
}
void GpuCanvasMirror::updateMasked(std::uint32_t id,int x,int y,const Image& patch,
                                  const std::vector<std::uint8_t>& coverage) {
  renderer_.updateMasked(surface(id),x,y,patch,coverage);
}
void GpuCanvasMirror::copy(std::uint32_t source, std::uint32_t destination,
                           Rect rect, int x, int y,
                           std::optional<std::uint16_t> key) {
  // Source/destination have already been cropped by the owned CPU service.
  // surfaceCopy additionally freezes same-ID source samples on the GPU.
  SurfaceCopyRequest request;
  request.source = rect;
  request.destination = {x, y, x + rect.right - rect.left,
                               y + rect.bottom - rect.top};
  request.api = SurfaceCopyApi::BltFast;
  request.flags = key ? 1u : 0u;
  const auto result = renderer_.surfaceCopy(surface(source), surface(destination),
                                           request, key);
  if (result.hresult)
    throw std::runtime_error("Native GPU producer copy refused: source="+std::to_string(source)+" destination="+std::to_string(destination)+" status="+std::to_string(result.hresult));
}
}
