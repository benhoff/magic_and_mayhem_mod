#pragma once
#include "canvas_world.hpp"
#include "world_rolling_packet.hpp"
#include "../../renderer/gpu_canvas_mirror.hpp"
#include <optional>
namespace mnm::legacy {
class WorldRollingSession final {
public:
  struct OwnerCosts {
    bool enabled=false;
    double validationMs=0,entryMs=0,appendMs=0,completeMs=0,returnMs=0,presentMs=0,totalMs=0;
    std::array<double,28> producerMs{};
    std::array<std::uint64_t,28> producerCalls{};
  };
  WorldRollingSession(render::GlBlitter &, const assets::AssetStore &,
                      bool verify = true, bool resident = false, bool preparedReplay = false,
                      bool retainCompletedImage = false);
  render::Image consume(
      std::uint64_t sequence, const std::vector<std::uint8_t> &,
      const std::function<void(const CanvasProducer &, const render::Image &)>
          &checkpoint = {},
      const std::function<void(const CanvasProducer &, const render::Image &)>
          &entry = {});
  render::Image consumeOwned(std::uint64_t sequence,
                             const CanvasProducerStream &);
  render::GpuFrame consumeResident(std::uint64_t, const CanvasProducerStream &);
  void consumeSource(const CanvasProducerStream &);
  render::GpuFrame replayPrepared();
  std::uint64_t preparedReplays() const { return world_.preparedReplays(); }
  std::uint64_t retainedImageCaptures() const { return world_.retainedImageCaptures(); }
  render::Image diagnosticReadResident(render::GlBlitter &);
  std::uint64_t worldReadbacks() const { return world_.readbacks(); }
  std::uint64_t cpuReaderSnapshots() const { return mirror_?mirror_->cpuReaderSnapshots():0; }
  render::GpuFrame presentGpu() {
    if (failed_ || !completed_)
      throw std::logic_error("Rolling GPU presentation before completion");
    return world_.presentGpu();
  }
  const CanvasWorld::Profile &profile() const { return world_.profile(); }
  render::ColourRectangleCacheStats colourCacheStats() const { return world_.colourCacheStats(); }
  CanvasFrameCacheStats producerCacheStats() const {
    return producer_.frameCacheStats();
  }
  std::uint64_t completed() const { return completed_; }
  unsigned firstWorldCanvas() const { return firstWorldCanvas_; }
  unsigned worldCanvas() const { return world_.canvas(); }
  std::uint64_t worldCanvasChanges() const { return worldCanvasChanges_; }
  std::uint64_t worldContextChanges() const { return world_.residentContextChanges(); }
  std::uint64_t copyRectRefusals() const { return producer_.copyRectRefusals(); }
  double nativeWorkMs() const { return nativeMs_; }
  double producerWorkMs() const { return producerMs_; }
  double checkpointAssertionMs() const { return assertionMs_; }
  bool failed() const { return failed_; }
  const OwnerCosts& ownerCosts() const { return ownerCosts_; }
  struct PacketCacheStats {
    std::uint64_t decodes = 0, hits = 0;
    std::size_t bytes = 0, records = 0, sources = 0, sourceBytes = 0;
  };
  PacketCacheStats packetCacheStats() const;

private:
  render::Image consumeStream(
      std::uint64_t sequence, const CanvasProducerStream &,
      const std::function<void(const CanvasProducer &, const render::Image &)>
          &checkpoint,
      const std::function<void(const CanvasProducer &, const render::Image &)>
          &entry);
  render::GlBlitter &renderer_;
  std::unique_ptr<render::GpuCanvasMirror> mirror_;
  CanvasProducerReplay producer_;
  CanvasWorld world_;
  std::uint64_t completed_ = 0;
  unsigned firstWorldCanvas_=0,lastWorldCanvas_=0;
  std::uint64_t worldCanvasChanges_=0;
  bool failed_ = false, verify_ = true, resident_ = false;
  bool ownerProfile_=false;
  OwnerCosts ownerCosts_;
  double nativeMs_ = 0, assertionMs_ = 0, producerMs_ = 0;
  std::vector<std::uint8_t> cachedBytes_;
  std::optional<CanvasProducerStream> cached_;
  std::uint64_t decodes_ = 0, hits_ = 0;
};
} // namespace mnm::legacy
