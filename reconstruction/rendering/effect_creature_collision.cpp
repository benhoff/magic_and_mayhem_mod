#include "effect_creature_collision.hpp"
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
constexpr int neighbors[27][3]={{0,0,0},{0,-1,0},{1,-1,0},{1,0,0},{1,1,0},{0,1,0},{-1,1,0},{-1,0,0},{-1,-1,0},
 {0,0,1},{0,-1,1},{1,-1,1},{1,0,1},{1,1,1},{0,1,1},{-1,1,1},{-1,0,1},{-1,-1,1},
 {0,0,-1},{0,-1,-1},{1,-1,-1},{1,0,-1},{1,1,-1},{0,1,-1},{-1,1,-1},{-1,0,-1},{-1,-1,-1}};
std::int32_t signedBits(std::uint32_t n) {
 return n<0x80000000u?std::int32_t(n):std::int32_t(std::int64_t(n)-0x100000000ll);
}
}
unsigned transitionEffectCreatureWorld(EffectPlacementPool& pool,unsigned slot,
 EffectCreatureTransitionState& state,unsigned width,unsigned height,unsigned layers,
 const EffectCreatureWorld& world,const std::vector<EffectCleanupColumn>& columns,
 bool updateMembership,const EffectTerrainOccupancy& occupancy) {
 if(!width || !height || !layers || width>128 || height>128 || layers>32 ||
    world.cells.size()!=std::size_t(width)*height*layers || slot>=pool.records().size() ||
    world.creatures.size()>65535 || (state.candidate!=NoCreature && state.candidate>=world.creatures.size()))
  throw std::invalid_argument("Creature collision world outside owned bounds");
 auto candidatePool=pool;auto next=state;
 const auto creator=candidatePool.records()[slot].parameters[6],iterations=candidatePool.records()[slot].parameters[9],kind=candidatePool.records()[slot].parameters[8];
 if(kind!=0 && kind!=34 && kind!=68)throw std::invalid_argument("Unsupported creature collision kind");
 if(iterations>8)throw std::invalid_argument("Creature collision iteration bound exceeded");
 // Reuse bounded movement bookkeeping. Kind 34 differs in creator selection;
 // the public empty-world operation retains its original admission.
 candidatePool.records()[slot].parameters[8]=kind==34?0:kind;
 candidatePool.records()[slot].parameters[6]=0xffffffffu;candidatePool.records()[slot].parameters[9]=0;
 transitionEffectEmptyWorld(candidatePool,slot,next.movement,width,height,layers,columns,updateMembership,occupancy);
 candidatePool.records()[slot].parameters[8]=kind;
 candidatePool.records()[slot].parameters[6]=creator;candidatePool.records()[slot].parameters[9]=iterations;
 std::array<unsigned,27> candidates;candidates.fill(NoCreature);
 auto gather=[&]() { candidates.fill(NoCreature);for(unsigned i=0;i<27;++i) {
  auto x=int(candidatePool.records()[slot].position[0])+neighbors[i][0],y=int(candidatePool.records()[slot].position[1])+neighbors[i][1],z=int(candidatePool.records()[slot].position[2])+neighbors[i][2];
  if(z<0 || z>=int(layers))continue;
  x=(x+int(width))%int(width);y=(y+int(height))%int(height);
  auto ordinal=world.cells[(unsigned(z)*height+unsigned(y))*width+unsigned(x)];
  if(ordinal!=NoEffect && ordinal<world.creatures.size() && (ordinal!=creator || kind==34))candidates[i]=ordinal;
 }
 };
 if(kind!=68)gather();
 unsigned result=3;
 for(unsigned step=0;step<iterations;++step) {
  const auto previousIndex=candidatePool.records()[slot].cell;
  const auto previousFlags=candidatePool.cells()[previousIndex].flags;
  const auto previousCell=candidatePool.records()[slot].position;
  candidatePool.records()[slot].parameters[8]=kind==34?0:kind;
  candidatePool.records()[slot].parameters[6]=0xffffffffu;candidatePool.records()[slot].parameters[9]=1;
  auto code=transitionEffectEmptyWorld(candidatePool,slot,next.movement,width,height,layers,columns,updateMembership,occupancy);
  candidatePool.records()[slot].parameters[8]=kind;
  candidatePool.records()[slot].parameters[6]=creator;candidatePool.records()[slot].parameters[9]=iterations;
  if(updateMembership && previousCell!=candidatePool.records()[slot].position && world.cells[previousIndex]!=NoEffect)
   candidatePool.cells()[previousIndex].flags=(candidatePool.cells()[previousIndex].flags&~0x80u)|(previousFlags&0x80u);
  if(code!=3){result=code;break;}
  if(previousCell!=candidatePool.records()[slot].position)gather();
  if(kind==68)continue;
  for(auto ordinal:candidates) {
   next.candidate=ordinal;
   if(ordinal==NoCreature)continue;
   const auto& c=world.creatures[ordinal];
   if(c.status==27)continue;
   if(!c.footprint.occupied(signedBits(candidatePool.records()[slot].units[0]-c.units[0]),signedBits(candidatePool.records()[slot].units[1]-c.units[1]),signedBits(candidatePool.records()[slot].units[2]-c.units[2])))continue;
   candidatePool.records()[slot].parameters[7]=c.id;result=0;break;
  }
  if(result==0 && candidatePool.records()[slot].type!=3)break;
 }
 pool=std::move(candidatePool);state=std::move(next);return result;
}
}
