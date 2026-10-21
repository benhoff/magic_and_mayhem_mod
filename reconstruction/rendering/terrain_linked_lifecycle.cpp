#include "terrain_linked_lifecycle.hpp"
#include <stdexcept>
namespace mnm::reconstruction {
std::int32_t
terrainLinkedTerminalFrame(const std::vector<assets::AnimationRecord> &records,
                           const AnimationControllerState &state) {
  if (records.empty() || records.size() > 65536 ||
      state.forward.pc > records.size())
    throw std::invalid_argument("Linked terminal frame sequence bounds");
  if (!state.reverse) {
    if (!state.forward.pc)
      throw std::out_of_range("Original terminal scan precedes sequence");
    for (auto at = state.forward.pc; at > 0;) {
      const auto &record = records[--at];
      if (!record.opcode)
        return record.argument;
    }
    return -1;
  }
  for (auto at = std::uint64_t(state.forward.pc) + 1; at < records.size();
       ++at) {
    const auto &record = records[at];
    if (!record.opcode)
      return record.argument;
    if (record.opcode == 6)
      return -1;
  }
  throw std::out_of_range("Original reverse terminal scan exceeds sequence");
}
TerrainFrameWrite
setTerrainAnimationFrame(assets::MapAsset &map, std::size_t at,
                         const assets::TerrainCatalog &catalog,
                         std::uint32_t frame,
                         const std::vector<std::uint8_t> &occupied) {
  if (!map.width || map.width > 128 || !map.height || map.height > 128 ||
      !map.layers || map.layers > 32 ||
      map.cells.size() != std::size_t(map.width) * map.height * map.layers ||
      at >= map.cells.size() || catalog.records.size() > 65536 ||
      (!occupied.empty() && occupied.size() != map.cells.size()))
    throw std::invalid_argument("Linked animation map/catalog bounds");
  if (frame > catalog.records.size())
    return {};
  if (frame == catalog.records.size())
    throw std::out_of_range("Original animation frame equals one-past catalog");
  const auto classification = [&](unsigned definition) {
    const auto &r = catalog.records.at(definition);
    return std::uint32_t(r[0x94]) | (std::uint32_t(r[0x95]) << 8) |
           (std::uint32_t(r[0x96]) << 16) | (std::uint32_t(r[0x97]) << 24);
  };
  auto candidate = map.cells[at];
  const auto before = classification(candidate.definition),
             after = classification(frame);
  if (after != 16 && !frame)
    return setTerrainLinkedFrame(map, at, catalog, 0, occupied);
  if (before == 16)
    candidate.flags10 &= 0xfffe;
  candidate.definition = std::uint16_t(frame);
  if (after == 16)
    candidate.flags10 |= 1;
  map.cells[at] = candidate;
  return {true, false};
}
} // namespace mnm::reconstruction
