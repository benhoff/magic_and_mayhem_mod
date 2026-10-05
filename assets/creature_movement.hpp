#pragma once
#include "persistence.hpp"
namespace mnm::assets {
// Selected CFG fields only; this is not a complete creature type record.
struct CreatureMovementConfig {
    std::uint32_t type=0;
    std::int32_t height=1,width=1,acceleration=0,swimming=0;
    bool canFly=false;
    std::int32_t groundSpeed=0,flyingSpeed=0;
};
CreatureMovementConfig creatureMovementConfig(const Config&,std::uint32_t type);
}
