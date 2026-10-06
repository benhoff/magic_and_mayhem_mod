#include "effect_animation_binding.hpp"
#include "effect_animation_selection.hpp"
#include <stdexcept>
using namespace mnm::reconstruction;
namespace assets=mnm::assets;
static void require(bool ok){if(!ok)throw std::runtime_error("Effect binding assertion failed");}
int main(){
    assets::Animation a;a.starts={0,2,4};a.records={{0,7,{}},{6,0,{}},{0,9,{}},{6,0,{}}};a.records[2].metadata[4]=123;
    std::vector<assets::Animation> animations{a};std::vector<EffectAnimationEntry> entries{{1,0,0xffffffffu,0x80000000u}};
    auto binding=bindEffectAnimation(selectEffectAnimation(3,71,{},0xffffffffu,{}),entries,animations);
    require(binding.animationOrdinal==0 && binding.entry.sequence==1 && binding.entry.property==0xffffffffu && binding.entry.opaque==0x80000000u);
    require(binding.player.sprite()==9 && binding.player.displayedRecord()->metadata[4]==123 && binding.player.state().pc==0);
    entries.clear();animations.clear();require(binding.player.sprite()==9 && binding.player.tick()==0 && binding.player.state().pc==1);
    require(binding.player.tick()==1 && !binding.player.sprite());binding.player.start();require(binding.player.sprite()==9);
    entries={{0,0,5,6}};animations={a};
    auto refused=[&](){bool failed=false;try{bindEffectAnimation(0,entries,animations);}catch(const std::runtime_error&){failed=true;}catch(const std::invalid_argument&){failed=true;}require(failed && binding.player.sprite()==9);};
    entries[0].assetIndex=1;refused();entries[0].assetIndex=0;entries[0].sequence=2;refused();entries[0].sequence=0;
    animations[0].starts[1]=999;refused();animations[0]=a;animations[0].records[0].argument=-1;refused();
    animations[0]=a;animations[0].records[1].opcode=0;refused();
    animations[0]=a;animations[0].records[0]={4,0,{}};refused(); // Dispatch budget guards a self-loop.
    bool failed=false;try{bindEffectAnimation(NoEffectAnimation,entries,animations);}catch(const std::invalid_argument&){failed=true;}require(failed);
}
