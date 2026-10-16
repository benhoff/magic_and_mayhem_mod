#pragma once
#include "regional-danger-encounter-state.hpp"
namespace mnm::encounter::regional_danger {
inline paired::Projection knowledgeProjection(const cs::model::OwnerKnowledge &s) {
  paired::Projection out;
  auto add=[&](std::string name,unsigned v){out.push_back({"knowledge."+name,v});};
  add("reset",s.reset);add("owner",s.owner);add("cursor",s.cursor);
  add("previous",s.previousSweep);add("current",s.currentSweep);
  for(unsigned owner=0;owner<8;++owner){
    const auto prefix=std::to_string(owner)+".";
    add(prefix+"next",s.nextKey[owner]);add(prefix+"count",s.records[owner].size());
    for(const auto &r:s.records[owner]){
      const auto key=prefix+std::to_string(r.memory.key)+".";
      add(key+"mark",r.markedTick);add(key+"slot",r.memory.actor.slot);
      add(key+"owner",r.memory.actor.owner);add(key+"type",r.memory.actor.type);
      add(key+"fine.x",r.finePosition.x);add(key+"fine.y",r.finePosition.y);add(key+"fine.z",r.finePosition.z);
    }
  }
  for(unsigned i=0;i<125;++i)add("matched."+std::to_string(i),s.matched[i]);
  return out;
}
}
