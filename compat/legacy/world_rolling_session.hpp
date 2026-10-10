#pragma once
#include "canvas_world.hpp"
#include "world_rolling_packet.hpp"
#include <optional>
namespace mnm::legacy {
class WorldRollingSession final {
public:
  WorldRollingSession(render::GlBlitter&,const assets::AssetStore&);
  render::Image consume(std::uint64_t sequence,const std::vector<std::uint8_t>&,
    const std::function<void(const CanvasProducer&,const render::Image&)>& checkpoint={});
  const CanvasWorld::Profile& profile() const {return world_.profile();}
  CanvasFrameCacheStats producerCacheStats() const {return producer_.frameCacheStats();}
  std::uint64_t completed() const {return completed_;}
  double nativeWorkMs() const {return nativeMs_;}
  double checkpointAssertionMs() const {return assertionMs_;}
  bool failed() const {return failed_;}
  struct PacketCacheStats {std::uint64_t decodes=0,hits=0;std::size_t bytes=0,records=0,sources=0,sourceBytes=0;};
  PacketCacheStats packetCacheStats() const;
private:
  CanvasProducerReplay producer_;
  CanvasWorld world_;
  std::uint64_t completed_=0;
  bool failed_=false;
  double nativeMs_=0,assertionMs_=0;
  std::vector<std::uint8_t> cachedBytes_;
  std::optional<CanvasProducerStream> cached_;
  std::uint64_t decodes_=0,hits_=0;
};
}
