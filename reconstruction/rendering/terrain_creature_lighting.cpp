#include "terrain_creature_lighting.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction {
bool admitsTerrainCreatureLight(const TerrainLightCreature& c,int player,const TerrainLightRelations& r){
 if(player<0 || player>7)throw std::invalid_argument("Creature light player must be 0..7");
 if(!c.active || !c.lightEnabled)return false;
 if(c.owner<0 || c.owner>7)throw std::invalid_argument("Active creature light owner must be 0..7");
 return c.owner==player || (r.first[c.owner][player]!=0 && r.second[c.owner][player]!=0);
}
void TerrainCreatureLightCycle::step(TerrainLightField& field,const std::vector<TerrainLightCreature>& creatures,
 unsigned scanLimit,int player,const TerrainLightRelations& relations,TerrainCreatureLightControls controls){
 if(player<0 || player>7 || scanLimit>65536 || creatures.size()>65536)
  throw std::invalid_argument("Creature light input outside bounded contract");
 if(!controls.enabled)return;
 const auto capacity=static_cast<unsigned>(creatures.size());
 // Validate the owned snapshot before mutating the cycle or field.
 for(unsigned i=0;i<std::min(scanLimit,capacity);++i){
  const auto& c=creatures[i];
  if(admitsTerrainCreatureLight(c,player,relations) &&
     (field.width_<17 || field.height_<17 || c.column>=field.width_ || c.row>=field.height_ || c.layer>=field.mapLayers_))
   throw std::invalid_argument("Admitted creature light outside field stamp contract");
 }
 if(state_.phase==0 || controls.changed!=0){
  field.buffers_[3]=field.buffers_[4];state_.remaining=0;
  for(unsigned i=0;i<std::min(scanLimit,capacity);++i)
   if(admitsTerrainCreatureLight(creatures[i],player,relations))++state_.remaining;
  state_.cursor=0;
 }
 if(state_.remaining!=0){
  if(controls.changed==1){state_.quota=state_.remaining;state_.cursor=0;state_.phase=7;}
  else state_.quota=(state_.remaining+7)/(8-state_.phase);
  unsigned processed=0;
  while(state_.cursor<scanLimit && processed<state_.quota){
   const auto i=state_.cursor++;
   if(i>=capacity || !admitsTerrainCreatureLight(creatures[i],player,relations))continue;
   const auto& c=creatures[i];field.stamp({c.column,c.row,c.layer,17});++processed;
   // Original unsigned state can wrap when admission grows mid-cycle.
   --state_.remaining;
  }
 }
 state_.phase=(state_.phase+1)%8;
 if(state_.phase==0){
  state_.published=!controls.transitionActive && !controls.transitionRequested;
  field.buffers_[state_.published?0:2]=field.buffers_[3];
 }
}
}
