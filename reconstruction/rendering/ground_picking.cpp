#include "ground_picking.hpp"
#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <utility>

namespace mnm::reconstruction {
namespace {
std::int32_t signedBits(std::uint32_t n) noexcept {
    std::int32_t result;
    std::memcpy(&result,&n,sizeof(result));
    return result;
}
bool admitted(const GroundTile& t, const TerrainCamera& c,
              std::uint32_t player, bool forceVisible, std::uint32_t level) {
    const bool concealed=((t.flags8&(8u<<c.view)) &&
        (t.flags10&(0x200u<<((player-1)&31)))) ||
        ((t.flags10&0x40) && (t.flags8&4) && (t.flags8&0x8000) && !forceVisible);
    if(concealed && level<c.cutLevel-1)return false;
    return c.mode!=1 || (t.flags8&3)<=player;
}
}
PickingMask::PickingMask(std::uint32_t width, std::uint32_t height,
    std::int32_t originX, std::int32_t originY, std::vector<std::uint8_t> opaque)
    : width_(width),height_(height),originX_(originX),originY_(originY),opaque_(std::move(opaque)) {
    if(width>2048 || height>2048 || ((width==0)!=(height==0)) ||
       opaque_.size()!=std::uint64_t(width)*height ||
       std::any_of(opaque_.begin(),opaque_.end(),[](auto n){return n>1;}))
        throw std::invalid_argument("Invalid owned picking mask");
}
bool PickingMask::hit(std::int32_t ax, std::int32_t ay,
                     std::int32_t px, std::int32_t py) const noexcept {
    // Match x86 DWORD wrap and signed comparisons, including wrapped right /
    // bottom bounds. A wider mathematical rectangle would change the result.
    const auto left=std::uint32_t(ax)-std::uint32_t(originX_);
    const auto top=std::uint32_t(ay)-std::uint32_t(originY_);
    if(px<signedBits(left) || px>=signedBits(left+width_) ||
       py<signedBits(top) || py>=signedBits(top+height_))return false;
    const auto x=std::uint32_t(px)-left,y=std::uint32_t(py)-top;
    return opaque_[std::size_t(y)*width_+x]!=0;
}
GroundPickingResult pickWorldGround(std::uint32_t width,std::uint32_t height,
    std::uint32_t layers,const TerrainCamera& c,TerrainViewport viewport,
    std::uint32_t player,bool forceVisible,const std::vector<GroundTile>& tiles,
    const std::vector<TerrainDefinition>& definitions,const std::vector<PickingMask>& masks,
    std::int32_t px,std::int32_t py) {
    if(!width || width>128 || !height || height>128 || !layers || layers>32 ||
       tiles.size()!=std::size_t(width)*height*layers || player>3 ||
       definitions.empty() || definitions.size()>65536 || masks.empty() || masks.size()>4096)
        throw std::invalid_argument("Invalid owned ground picking input");
    // Validate the complete owned definition domain before traversal can emit
    // any result; none of the original unchecked pointer behavior is adopted.
    for(const auto& t:tiles)if(t.definition>=definitions.size())
        throw std::invalid_argument("Unknown terrain picking definition");
    GroundPickingResult result;
    result.terrainDepth=-std::int32_t(width*64); // Original scene outer seed.
    result.flags.reserve(tiles.size());
    for(const auto& t:tiles)result.flags.push_back({t.flags8,t.flags10});
    const auto visits=traverseOwnedTerrain(width,height,layers,c,viewport,result.flags);
    for(const auto& v:visits){
        const auto& source=tiles.at(v.cell);
        const GroundTile state{source.definition,result.flags[v.cell][0],result.flags[v.cell][1]};
        const auto& def=definitions[state.definition];
        const auto frame=[&](std::uint32_t n)->const PickingMask& {
            return masks[n<masks.size()?n:0];
        };
        const bool hit=def.body[c.view] && admitted(state,c,player,forceVisible,v.layer) &&
            (frame(def.body[c.view]).hit(v.anchorX,v.anchorY,px,py) ||
             (def.first[c.view] && frame(def.first[c.view]).hit(v.anchorX,v.anchorY,px,py)) ||
             (def.second[c.view] && frame(def.second[c.view]).hit(v.anchorX,v.anchorY,px,py)));
        // Original probes all declared roles before the absent-layer draw gates.
        // XYZ is overwritten by every hit, even when its depth is not a maximum.
        if(hit)result.ground=PickedGround{v.column,v.row,v.layer};
        TerrainTile t{std::int32_t(v.row),std::int32_t(v.column),std::int32_t(v.layer),
            v.anchorX,v.anchorY,v.priority,state.flags8,state.flags10,0};
        const auto draws=submitTerrain(def,t,{c.view,c.cutLevel,player,c.mode,0,forceVisible},
                                       std::uint32_t(masks.size()-1));
        if(hit){
            if(draws.empty())throw std::logic_error("Picking/terrain admission disagreement");
            if(draws.front().key>result.terrainDepth){
                result.terrainDepth=draws.front().key;result.depthRow=result.submittedRows;
            }
        }
        if(draws.size()>std::numeric_limits<std::uint32_t>::max()-result.submittedRows)
            throw std::length_error("Ground picking row count");
        result.submittedRows+=std::uint32_t(draws.size());
        result.flags[v.cell]={t.flags8,t.flags10};
    }
    return result;
}
WorldHitClass classifyWorldHit(const WorldHitDepths& d) noexcept {
    if(d.actor>d.artifact && d.actor>d.terrain && d.actorPresent)return WorldHitClass::actor;
    if(d.artifact>d.actor && d.artifact>d.terrain && d.artifactPresent)return WorldHitClass::artifact;
    return d.groundPresent?WorldHitClass::ground:WorldHitClass::none;
}
}
