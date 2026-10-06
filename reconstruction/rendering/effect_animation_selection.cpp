#include "effect_animation_selection.hpp"
#include <stdexcept>

namespace mnm::reconstruction {
std::uint32_t selectEffectAnimation(std::uint32_t type,std::uint32_t kind,
    const std::vector<std::uint32_t>& kindClasses,std::uint32_t creatureOrdinal,
    const std::vector<std::uint32_t>& creatureDescriptorClasses){
    const auto classified=[&](std::uint32_t zero,std::uint32_t one,std::uint32_t two){
        if(kind>=kindClasses.size())
            throw std::invalid_argument("Effect animation requires an owned kind metadata entry");
        switch(kindClasses[kind]){case 0:return zero;case 1:return one;case 2:return two;default:return NoEffectAnimation;}
    };
    if((type==6 || type==7) && kind!=68){
        if(kind==33)return 27;
        return classified(20,21,13);
    }
    switch(kind){
    case 0:case 1:case 3:case 5:case 10:case 11:case 12:case 13:
    case 14:case 15:case 16:case 19:case 20:case 21:case 22:case 23:
    case 24:case 25:case 26:return classified(15,16,14);
    case 2:case 4:case 6:case 7:case 8:case 9:case 17:case 18:return classified(18,19,17);
    case 33:return 43;
    case 34:return type==35?55:type==36?56:57;
    case 35:return 63;
    case 38:return 51;
    case 41:return 74;
    case 42:return 73;
    case 43:return 48;
    case 44:return 71;
    case 50:return 30;
    case 52:
        if(creatureOrdinal>=creatureDescriptorClasses.size())
            throw std::invalid_argument("Effect animation requires an owned creature descriptor entry");
        return creatureDescriptorClasses[creatureOrdinal]==2?17:14;
    case 54:return 75;
    case 58:return 59;
    case 62:return type==5?0:59;
    case 68:return 87;
    case 71:return 0;
    case 73:return 49;
    case 74:return 34;
    case 77:return 28;
    case 79:return 70;
    case 82:return 69;
    case 89:return type==2?0:59;
    case 93:case 98:return 2;
    case 94:return 3;
    case 95:return 4;
    case 96:return 5;
    case 99:return 11;
    case 102:return 78;
    default:return NoEffectAnimation;
    }
}
}
