#pragma once
#include "sprite_queue.hpp"
#include <array>
#include <vector>

namespace mnm::reconstruction {
struct TerrainDefinition {
    std::array<std::uint32_t,4> body{},first{},second{};
};
TerrainDefinition decodeTerrainDefinition(const std::array<std::uint8_t,356>&);
struct TerrainTile {
    std::int32_t row=0,column=0,level=0,anchorX=0,anchorY=0,priority=0;
    std::uint16_t flags8=0,flags10=0;
    std::int8_t light=0;
};
struct TerrainAdmission {
    std::uint32_t view=0,cutLevel=1,player=1,mode=0;
    std::int32_t lightBias=0;
    bool forceVisible=false;
};
struct TerrainDraw {
    std::uint32_t frame=0,role=0;
    SpriteDepth depth;
    std::int32_t anchorX=0,anchorY=0,key=0,kind=33,shade=0;
};
// Selected ordinary terrain path in 0x4f8960. No map traversal, objects,
// pointer picking, lighting-palette construction or original pointer ownership.
// Mutates only the absent-layer flags, as the original producer does.
std::vector<TerrainDraw> submitTerrain(const TerrainDefinition&,TerrainTile&,
                                     const TerrainAdmission&,std::uint32_t highestFrame);
}
