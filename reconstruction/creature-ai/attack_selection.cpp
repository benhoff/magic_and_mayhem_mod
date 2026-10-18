#include "attack_selection.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
constexpr int small[25]={-1,-1,-1,-1,-1,24,32,40,24,-1,40,-1,-1,32,-1,32,-1,-1,40,-1,24,40,32,24,-1};
constexpr int large[25]={24,32,24,40,24,40,-1,-1,-1,32,24,-1,-1,-1,24,32,-1,-1,-1,40,24,40,24,32,24};
constexpr int objectFacing[25]={-1,-1,-1,-1,-1,5,6,6,7,-1,4,4,6,0,-1,4,2,0,0,-1,3,2,2,1,-1};
int difference(int a,int b,int dimension) {
  if(dimension<=0 || a<0 || b<0 || a>=dimension || b>=dimension)
    throw std::invalid_argument("attack selection canonical coordinates");
  int result=a-b;
  if(result>dimension/2)result-=dimension;
  if(result<-dimension/2)result+=dimension;
  return result;
}
}
AttackSelectionResult selectAttackAnimation(AttackSelectionState state,Extent dimensions,
    unsigned cameraTurn,std::optional<AttackSelectionTarget> target,
    std::optional<AttackSelectionObject> object) {
  if(state.width<1 || state.width>2 || state.facing>7 || cameraTurn>3)
    throw std::invalid_argument("attack selection profile");
  AttackSelectionResult result{state,{}};
  auto &s=result.state;
  auto play=[&](unsigned offset){s.offset=offset;result.events.push_back({AttackSelectionEffect::play,offset+s.facing});};
  if(s.width!=2 || (!target&&(!object||object->kind!=26))) {
    play(24);return result;
  }
  const auto position=target?target->position:object->position;
  const int x=2-difference(s.position.x,position.x,dimensions.x);
  const int y=2+difference(s.position.y,position.y,dimensions.y);
  if(x<0 || x>4 || y<0 || y>4){play(24);return result;}
  const auto index=unsigned(x*5+y);
  if(!target && objectFacing[index]>=0 && objectFacing[index]<=7) {
    s.direction=unsigned(objectFacing[index]);
    s.facing=(s.direction-2*cameraTurn-(s.reverse?4:0))&7;
    if(s.animationKind==3)result.events.push_back({AttackSelectionEffect::visualRepair,0});
    else if(s.animationKind==2 && s.attached)result.events.push_back({AttackSelectionEffect::attachmentFacing,s.facing});
    if(s.secondary)result.events.push_back({AttackSelectionEffect::secondarySwitch,s.facing+8});
  }
  const int selected=target&&target->width==2?large[index]:small[index];
  play((s.facing&1)&&selected!=24 ? 24 : selected==-1 ? 24 : unsigned(selected));
  return result;
}
bool corpseFlightAnimation(unsigned health,unsigned corpseA,unsigned corpseB) {
  return !health&&(corpseA||corpseB);
}
} // namespace mnm::reconstruction::combat
