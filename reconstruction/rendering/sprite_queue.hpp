#pragma once
#include <cstdint>
#include <vector>
namespace mnm::reconstruction {
// Build-specific NoCD queue contract. Coordinates are queue inputs, not pixels.
struct SpriteDepth {std::int32_t x=0,y=0,height=0,priority=0;};
struct SpriteQueueEntry {std::int32_t key=0;std::uint32_t payload=0;};
std::int32_t spriteDepthKey(SpriteDepth position,std::uint32_t view);
// Preserves the original unstable equal-key permutation; payload is opaque.
void sortSpriteQueue(std::vector<SpriteQueueEntry>& entries);
}
