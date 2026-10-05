#pragma once
#include "effect_lighting.hpp"
namespace mnm::reconstruction {
inline constexpr std::uint16_t NoEffect=0xffff;
struct EffectCell {
    std::uint16_t head=NoEffect,terrain=0;
    std::uint32_t flags=0;
};
struct EffectPlacementRecord {
    bool active=false;
    unsigned type=0;
    std::array<std::uint32_t,63> parameters{};
    std::array<unsigned,3> position{},units{},initialPosition{};
    std::array<std::uint32_t,12> sentinels{};
    std::uint16_t next=NoEffect;
    unsigned cell=0;
    bool initialized=false;
    // Common creation leaves these pending; movement produces them later.
    TerrainLightObject lightingSource(const EffectLightingTable&) const;
};
class EffectPlacementPool {
public:
    EffectPlacementPool(unsigned width,unsigned height,unsigned layers,unsigned capacity);
    // Selected common 00493f20 placement for installed emitting types
    // 3,13,22,24,36, with creator=-1. Type-specific setup is deferred.
    void place(unsigned slot,unsigned type,const std::array<std::uint32_t,63>&);
    const auto& records() const{return records_;}
    const auto& cells() const{return cells_;}
    auto& records(){return records_;}
    auto& cells(){return cells_;}
    unsigned scan() const{return scan_;}
    void setScan(unsigned);
private:
    unsigned width_,height_,layers_,scan_=0;
    std::vector<EffectPlacementRecord> records_;
    std::vector<EffectCell> cells_;
};
}
