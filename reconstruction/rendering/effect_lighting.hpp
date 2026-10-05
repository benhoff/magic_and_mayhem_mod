#pragma once
#include "terrain_static_lighting.hpp"
#include <array>
#include <string>
namespace mnm::reconstruction {
struct EffectLightingEntry {std::uint32_t diameter=0,affected=0,clippedToHeight=0;};
class EffectLightingTable {
public:
    using Fields=std::array<std::array<std::string,3>,89>;
    // 0049c2e0: strings are already returned by profile reads (<=255 bytes).
    // Reload preserves missing/empty fields and flags whose value is not TRUE.
    void reload(const Fields&);
    const std::array<EffectLightingEntry,89>& entries() const{return entries_;}
    std::array<EffectLightingEntry,89>& entries(){return entries_;}
    // Bounded 0049cf60 selection; original has no type-index bounds check.
    unsigned lightIndex(unsigned type) const;
    // Caller supplies owned positions; this does not reconstruct record creation.
    TerrainLightObject source(unsigned type,TerrainLightObject positions) const;
private:
    std::array<EffectLightingEntry,89> entries_{};
};
}
