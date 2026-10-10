#pragma once
#include "../../renderer/canvas_sequence.hpp"
#include <array>
#include <functional>
#include <list>
#include <map>
#include <memory>
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
struct CanvasFrameCacheLimits {
  std::size_t frames = 4096, bytes = 64 * 1024 * 1024;
};
struct CanvasFrameCacheStats {
  std::size_t decodes = 0, hits = 0, evictions = 0, bypasses = 0,
              frames = 0, bytes = 0;
};
// Exact closed frame bytes and storage tag; colour/effect tables remain per draw.
// The byte bound includes owned encoded and decoded vector capacities, excluding
// map/list/shared_ptr/object overhead. Returned immutable frames survive eviction.
class CanvasFrameCache final {
public:
  explicit CanvasFrameCache(CanvasFrameCacheLimits limits = {});
  std::shared_ptr<const assets::SpriteFrame> frame(const CanvasProducer &, bool indexed);
  CanvasFrameCacheStats stats() const { return stats_; }
  CanvasFrameCache(const CanvasFrameCache &) = delete;
  CanvasFrameCache &operator=(const CanvasFrameCache &) = delete;

private:
  struct View { bool indexed; const std::uint8_t *data; std::size_t size; };
  struct Key {
    bool indexed;
    std::vector<std::uint8_t> encoded;
    operator View() const { return {indexed, encoded.data(), encoded.size()}; }
  };
  struct Less {
    using is_transparent = void;
    bool operator()(View, View) const;
  };
  struct Entry {
    std::shared_ptr<const assets::SpriteFrame> frame;
    std::size_t bytes;
    std::list<const Key *>::iterator recent;
  };
  CanvasFrameCacheLimits limits_;
  CanvasFrameCacheStats stats_;
  std::map<Key, Entry, Less> entries_;
  std::list<const Key *> recent_;
};
// The source callback accepts encoded source assets only. Destination oracles
// have no path through this adapter or the native canvas service.
class CanvasProducerReplay {
public:
  explicit CanvasProducerReplay(
      std::function<std::vector<std::uint8_t>(const std::string &)> source)
      : source_(std::move(source)) {}
  void apply(const CanvasProducer &);
  CanvasFrameCacheStats frameCacheStats() const { return frames_.stats(); }
  mnm::render::Image read(std::uint32_t id) const { return canvases_.read(id); }
  void commitNativeWorld(std::uint32_t id, const mnm::render::Image &image) {
    const auto prior = canvases_.read(id);
    if (prior.width != image.width || prior.height != image.height)
      throw std::invalid_argument("Native World completion changed producer extent");
    canvases_.update(id, 0, 0, image);
  }

private:
  mnm::render::CanvasSequence canvases_;
  CanvasFrameCache frames_;
  std::function<std::vector<std::uint8_t>(const std::string &)> source_;
};
} // namespace mnm::legacy
