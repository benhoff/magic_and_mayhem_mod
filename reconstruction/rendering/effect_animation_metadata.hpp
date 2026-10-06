#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
namespace mnm::reconstruction {
using EffectAnimationMetadataFields=std::vector<std::array<std::string,4>>;
using EffectAnimationMetadataWords=std::vector<std::array<std::uint32_t,4>>;
// Selected C-locale atoi/count clamp from 0049c6e2..0049c735.
std::uint32_t effectAnimationFileCount(const std::string& profileValue);
// 0049c836..0049caae metadata loop with explicit preexisting allocation bytes.
// Missing/unmatched fields retain their word; no invented initial/default data.
// Native harness bounds: 1..64 files, 1..4096 rows, ASCII strings <=255 bytes.
EffectAnimationMetadataWords decodeEffectAnimationMetadata(
    const EffectAnimationMetadataFields&,std::uint32_t fileCount,
    const EffectAnimationMetadataWords& initial);
}
