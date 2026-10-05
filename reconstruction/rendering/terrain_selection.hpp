#pragma once
#include "../../assets/map.hpp"
#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace mnm::reconstruction {
// Selected No-CD generator RNG. Each selection starts again at its supplied seed.
class TerrainRegionRandom {
public:
    explicit TerrainRegionRandom(std::uint32_t seed);
    std::uint32_t next();
    const std::array<std::uint32_t,250>& words() const { return words_; }
    unsigned left() const { return left_; }
    unsigned right() const { return right_; }
private:
    std::array<std::uint32_t,250> words_{};
    unsigned left_=0, right_=103;
};
using RegionEdges=std::array<std::int32_t,4>; // North, east, south, west.
struct SingleRegionDescriptor {
    std::uint32_t section=0;
    bool specific=false, special=false;
    std::int32_t maximum=1, placed=0;
    RegionEdges edges{};
};
// Only header block dimensions 1x1. Does not retain the asset or its cells.
SingleRegionDescriptor describeSingleRegionSection(const assets::MapAsset&,
    unsigned section, bool specific, std::int32_t maximum);
struct RegionSelectionCell {
    bool occupied=false;
    RegionEdges edges{{-1,-1,-1,-1}};
};
struct RegionSelectionGrid {
    unsigned columns=0, rows=0;
    // Owned column-major cells: column*rows + row. No original packed layout.
    std::vector<RegionSelectionCell> cells;
};
RegionEdges rotateRegionEdges(const RegionEdges&, unsigned rotation);
bool admitSingleRegionSection(const RegionSelectionGrid&, const SingleRegionDescriptor&,
    unsigned column, unsigned row, unsigned rotation);
struct RegionCandidate { unsigned descriptor=0; std::uint8_t rotations=0; };
struct RegionLocation { unsigned column=0, row=0; std::uint8_t rotations=0; };
std::vector<RegionCandidate> singleRegionCandidates(const RegionSelectionGrid&,
    const std::vector<SingleRegionDescriptor>&, unsigned column, unsigned row, bool special);
std::vector<RegionLocation> singleRegionLocations(const RegionSelectionGrid&,
    const SingleRegionDescriptor&, int rotation); // -1 tries all four.
struct RegionChoice { unsigned index=0, rotation=0; };
// index addresses the compact input table, rather than the original descriptor.
std::optional<RegionChoice> chooseRegionCandidate(std::uint32_t seed,
    const std::vector<RegionCandidate>&);
std::optional<RegionChoice> chooseRegionLocation(std::uint32_t seed,
    const std::vector<RegionLocation>&);
std::optional<unsigned> chooseRegionRotation(std::uint32_t seed, std::uint8_t mask);
}
