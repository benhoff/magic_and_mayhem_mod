#include "creature_movement.hpp"
#include <charconv>
#include <stdexcept>
namespace mnm::assets {
CreatureMovementConfig creatureMovementConfig(const Config& cfg,std::uint32_t type){
    if(type>=28)throw std::invalid_argument("Creature type outside original table");
    const auto section="CREATURE_"+std::to_string(type);
    const auto value=[&](const char* key){auto* p=cfg.find(section,key);if(!p)throw std::invalid_argument("Missing creature movement field: "+std::string(key));return *p;};
    const auto number=[&](const char* key){auto s=value(key);std::int32_t n=0;auto result=std::from_chars(s.data(),s.data()+s.size(),n);if(result.ec!=std::errc{} || result.ptr!=s.data()+s.size())throw std::invalid_argument("Creature field requires complete decimal: "+std::string(key));return n;};
    auto fly=value("CanFly");for(auto& c:fly)if(c>='A'&&c<='Z')c=char(c-'A'+'a');
    if(fly!="true" && fly!="false")throw std::invalid_argument("Creature CanFly requires TRUE or FALSE");
    return {type,number("TileHeight"),number("TileSizeXY"),number("Acceleration"),number("SwimmingAbility"),fly=="true",number("GroundSpeed"),number("FlyingSpeed")};
}
}
