#include "terrain_submission.hpp"
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace mnm::reconstruction {
static std::int32_t bits(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
TerrainDefinition decodeTerrainDefinition(const std::array<std::uint8_t,356>& record){
    const auto word=[&](unsigned at){return std::uint32_t(record[at])|(std::uint32_t(record[at+1])<<8)|(std::uint32_t(record[at+2])<<16)|(std::uint32_t(record[at+3])<<24);};
    TerrainDefinition result;
    for(unsigned view=0;view<4;++view){result.body[view]=word(0x84+4*view);result.first[view]=word(0xd0+4*view);result.second[view]=word(0xe0+4*view);}
    return result;
}
std::vector<TerrainDraw> submitTerrain(const TerrainDefinition& definition,TerrainTile& tile,
                                     const TerrainAdmission& admission,std::uint32_t highestFrame){
    if(admission.view>3 || admission.player>3 || tile.level<0 || tile.level>0x07ffffff)
        throw std::invalid_argument("Unsupported terrain orientation, player or level");
    const bool concealed=((tile.flags8&(8U<<admission.view)) &&
                          (tile.flags10&(0x200U<<((admission.player-1)&31)))) ||
                         ((tile.flags10&0x40) && (tile.flags8&4) &&
                          (tile.flags8&0x8000) && !admission.forceVisible);
    if(concealed && std::uint32_t(tile.level)<admission.cutLevel-1)return {};
    if(admission.mode==1 && (tile.flags8&3)>admission.player)return {};
    const std::array<std::uint32_t,3> frames{{definition.body[admission.view],definition.first[admission.view],definition.second[admission.view]}};
    if(!frames[0])return {};
    SpriteDepth depth{bits(std::uint32_t(tile.column)*32+16),bits(std::uint32_t(tile.row)*32+16),tile.level*16,tile.priority};
    const auto shade=std::max(-127,bits(std::uint32_t(std::int32_t(tile.light))-std::uint32_t(admission.lightBias)));
    std::vector<TerrainDraw> result;
    const auto add=[&](unsigned role){result.push_back({frames[role]>highestFrame?0:frames[role],role,depth,tile.anchorX,tile.anchorY,spriteDepthKey(depth,admission.view),role==0 && (tile.flags10&8)?31:33,shade});};
    add(0);
    if(tile.flags8&4)return result;
    if(!frames[1]){tile.flags8|=0x8004;return result;}
    add(1);
    if(tile.flags8&0x8000)return result;
    if(!frames[2])tile.flags8|=0x8000;
    else add(2);
    return result;
}
}
