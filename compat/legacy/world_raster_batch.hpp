#pragma once
#include "canvas_producers.hpp"
namespace mnm::legacy {
struct WorldRasterBatch {
  std::array<std::uint32_t,16> header;
  std::vector<std::array<std::uint32_t,4>> descriptors;
};
std::uint32_t worldBatchHash(const std::vector<std::uint8_t>&);
std::uint32_t worldRasterSourceHash(const CanvasProducer&);
unsigned worldRasterAX(const CanvasProducer&);
WorldRasterBatch validateWorldRasterBatch(const std::vector<std::uint8_t>& request,
  unsigned ordinal,unsigned queue,unsigned sequence,unsigned canvas,
  unsigned width,unsigned height,unsigned priorRasters,
  const std::vector<const CanvasProducer*>& rasters);
std::vector<std::uint8_t> worldRasterBatchReply(const WorldRasterBatch&,
  const std::vector<std::uint16_t>& pixels);
}
