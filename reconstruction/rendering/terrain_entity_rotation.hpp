#pragma once
#include "../../assets/animation.hpp"
#include "../../assets/map.hpp"
#include "../../assets/terrain_catalog.hpp"
namespace mnm::reconstruction {
// 00464bd0 scans records linearly, skipping control opcodes rather than running
// them. A stop before display returns raw -1. The owned sequence bounds replace
// original unbounded reads; other display arguments retain their raw word bits.
std::int32_t terrainEntityFirstFrame(const assets::Animation&,std::uint32_t sequence);
enum class TerrainLinkedRotationAction {unchanged,replace,reset};
struct TerrainLinkedRotation {
 TerrainLinkedRotationAction action=TerrainLinkedRotationAction::unchanged;
 std::uint16_t definition=0;
 std::int32_t sourceFrame=-1;
 std::uint32_t targetFrame=0xffffffffu;
};
// 004efb90 selection only. A reset decision requires the native map owner to
// retire linked state and perform its full cell/navigation reset; this function
// never partially clears a flag and pretends that dependency is implemented.
TerrainLinkedRotation rotateTerrainLinkedDefinition(const assets::MapCell&,
 const assets::Animation&,const assets::TerrainCatalog&,unsigned rotation);
}
