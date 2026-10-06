#pragma once
#include <cstdint>
#include <vector>

namespace mnm::reconstruction {
inline constexpr std::uint32_t NoEffectAnimation=0xffffffffu;
// Read-only whole 00489200 selection. Kind classes replace metadata DWORDs at
// 006b13cd + kind*721. Descriptor classes replace creature+ac -> descriptor+8;
// creatureOrdinal is the raw selector at effect+48, not an inferred ID mapping.
// Storage is consulted only by the selected branch. Missing required owned
// entries throw; unsupported classes/kinds retain the original -1 result.
std::uint32_t selectEffectAnimation(std::uint32_t type,std::uint32_t kind,
    const std::vector<std::uint32_t>& kindClasses,std::uint32_t creatureOrdinal,
    const std::vector<std::uint32_t>& creatureDescriptorClasses);
}
