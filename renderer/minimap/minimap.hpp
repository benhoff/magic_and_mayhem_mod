#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>

namespace mnm::render {
// Owned RGB565 storage; padding words are retained and never drawn.
struct MinimapPlane {
  int width, height, stride;
  std::vector<std::uint16_t> words;
  // Optional owned service write receipt; never a wire or game pointer.
  std::vector<std::size_t>* writeCandidates = nullptr;
};
struct MinimapTerrain {
  int width, height, centerX, centerY, rootX, rootY;
  unsigned orientation;
  std::vector<std::uint16_t> colours;
  std::vector<unsigned char> hidden;
};
// Recovered 0x553a40 terrain placement. Rejects unsupported inputs before
// mutation. Fog reads only caller-owned prior auxiliary storage.
void drawMinimapTerrain(MinimapPlane &destination, const MinimapTerrain &,
                        const MinimapPlane *previous = nullptr);
}
