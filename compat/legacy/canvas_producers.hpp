#pragma once
#include "../../renderer/canvas_sequence.hpp"
#include <array>
#include <functional>
#include <stdexcept>
namespace mnm::legacy {
struct CanvasProducer {
  std::array<std::uint32_t, 24> fields;
  std::vector<std::uint8_t> payload;
};
struct CanvasProducerStream {
  unsigned queues;
  std::vector<CanvasProducer> operations;
  unsigned version = 0;
};
void appendCanvasProducers(CanvasProducerStream &, const std::vector<std::uint8_t> &, bool complete = false);
CanvasProducerStream decodeCanvasProducers(const std::vector<std::uint8_t> &, bool complete = true);
// The source callback accepts encoded source assets only. Destination oracles
// have no path through this adapter or the native canvas service.
class CanvasProducerReplay {
public:
  explicit CanvasProducerReplay(
      std::function<std::vector<std::uint8_t>(const std::string &)> source)
      : source_(std::move(source)) {}
  void apply(const CanvasProducer &);
  mnm::render::Image read(std::uint32_t id) const { return canvases_.read(id); }
  void commitNativeWorld(std::uint32_t id, const mnm::render::Image &image) {
    const auto prior = canvases_.read(id);
    if (prior.width != image.width || prior.height != image.height)
      throw std::invalid_argument("Native World completion changed producer extent");
    canvases_.update(id, 0, 0, image);
  }

private:
  mnm::render::CanvasSequence canvases_;
  std::function<std::vector<std::uint8_t>(const std::string &)> source_;
};
} // namespace mnm::legacy
