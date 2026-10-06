#include "effect_animation_selection.hpp"
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool ok){if(!ok)throw std::runtime_error("Animation selection assertion failed");}
int main(){
    require(selectEffectAnimation(6,33,{},0xffffffffu,{})==27);
    require(selectEffectAnimation(7,68,{},0xffffffffu,{})==87);
    require(selectEffectAnimation(3,33,{},0xffffffffu,{})==43);
    require(selectEffectAnimation(35,34,{},0xffffffffu,{})==55);
    require(selectEffectAnimation(36,34,{},0xffffffffu,{})==56);
    require(selectEffectAnimation(3,34,{},0xffffffffu,{})==57);
    require(selectEffectAnimation(5,62,{},0xffffffffu,{})==0);
    require(selectEffectAnimation(2,89,{},0xffffffffu,{})==0);
    require(selectEffectAnimation(3,103,{},0xffffffffu,{})==NoEffectAnimation);
    require(selectEffectAnimation(3,0,{0},0xffffffffu,{})==15);
    require(selectEffectAnimation(3,0,{1},0xffffffffu,{})==16);
    require(selectEffectAnimation(3,0,{2},0xffffffffu,{})==14);
    require(selectEffectAnimation(3,0,{0xffffffffu},0xffffffffu,{})==NoEffectAnimation);
    require(selectEffectAnimation(6,0,{2},0xffffffffu,{})==13);
    require(selectEffectAnimation(3,52,{},0,{2})==17);
    require(selectEffectAnimation(3,52,{},0,{0xffffffffu})==14);
    std::vector<std::uint32_t> metadata(53,1);
    require(selectEffectAnimation(6,52,metadata,0xffffffffu,{})==21); // Override avoids descriptor access.
    auto refused=[&](unsigned type,unsigned kind,unsigned creature){
        bool failed=false;try{selectEffectAnimation(type,kind,{},creature,{});}catch(const std::invalid_argument&){failed=true;}
        require(failed);
    };
    refused(3,0,0);refused(6,34,0);refused(7,103,0);refused(3,52,0);refused(3,52,0xffffffffu);
}
