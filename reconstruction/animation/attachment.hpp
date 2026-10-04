#pragma once
#include <cstdint>
namespace mnm::reconstruction {
// No-CD 51ff00 mode-1 entry: effect configuration entry 36, base + facing.
struct ModeOneSelection {std::uint32_t assetIndex=0,sequence=0;};
ModeOneSelection modeOneSelection(std::uint32_t base,std::uint32_t assetIndex,std::uint32_t facing);
// Selected draw/update gates. Health is tested for nonzero, not positivity.
bool modeOneVisible(std::int32_t health,std::uint32_t mode);
bool modeOneTicks(std::uint32_t mode);
}
