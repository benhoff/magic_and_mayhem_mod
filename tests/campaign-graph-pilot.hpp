#pragma once
#include "simulation/campaign_session.hpp"
#include <algorithm>
#include <cstdlib>
namespace mnm::tests {
namespace g=mnm::game;
template<class Session> inline void campaignPilot(Session& s) {
  if(!s.battle()) return;
  const auto& b=*s.battle();const auto m=b.state();const auto wizard=m.wizards[0];
  const auto fighters=b.combat().combat().fighters;
  // Clear a pending arrival area with ordinary player movement, then rejoin combat.
  if(s.progress().mission && s.progress().mission->version==2) {
    const auto& progress=*s.progress().mission;const auto& plan=*s.missionPlan();
    for(std::size_t i=0;i<plan.groups.size();++i) {
      if(!progress.groups[i].empty() || !std::all_of(plan.groups[i].after.begin(),plan.groups[i].after.end(),[&](auto n){return progress.objectives[n].complete;})) continue;
      for(const auto& f:fighters) {
        const auto* actor=b.world().find(f.actor);
        if(!f.health || actor->owner!=0 || f.actor==wizard.actor || f.control==g::CombatControl::move) continue;
        bool blocking=false;
        for(auto p:plan.groups[i].locations) blocking|=actor->z==p.z && std::abs(actor->x-p.x)<2 && std::abs(actor->y-p.y)<2;
        if(!blocking) continue;
        auto candidates=b.mission().standing;
        std::stable_sort(candidates.begin(),candidates.end(),[&](auto a,auto c){return std::max(std::abs(a.x-actor->x),std::abs(a.y-actor->y))<std::max(std::abs(c.x-actor->x),std::abs(c.y-actor->y));});
        for(auto p:candidates) {
          bool clear=true;
          for(std::size_t j=0;j<plan.groups.size();++j) if(progress.groups[j].empty())
            for(auto origin:plan.groups[j].locations) clear&=p.z!=origin.z || std::abs(p.x-origin.x)>=2 || std::abs(p.y-origin.y)>=2;
          if(clear && b.combat().driver()->canSpawn(b.combat(),p) && b.combat().navigation()->planInWorld(b.world().state(),f.actor,p,256).status==g::PlanStatus::reachable) {s.enqueue({g::SkirmishOrderKind::move,f.actor,{},p});return;}
        }
      }
    }
    for(const auto& f:fighters) if(f.health && b.world().find(f.actor)->owner==0 && !(f.actor==wizard.actor) && f.control!=g::CombatControl::automatic && f.control!=g::CombatControl::attack) {
      for(const auto& group:progress.groups) for(auto h:group)
        for(const auto& target:fighters) if(target.actor==h && target.health) {s.enqueue({g::SkirmishOrderKind::attack,f.actor,h,{}});return;}
    }
  }
  if(wizard.spell!=g::Spell::none) return;
  if(!s.progress().mission) {
    if(wizard.mana>=g::SkirmishRules::boltCost) s.enqueue({g::SkirmishOrderKind::bolt,wizard.actor,m.wizards[1].actor,{}});
    return;
  }

  unsigned allies=0;
  for(const auto& f:b.combat().combat().fighters)
    if(f.health && b.world().find(f.actor)->owner==0 && !(f.actor==wizard.actor)) ++allies;
  if(wizard.mana>=g::SkirmishRules::summonCost && allies<3) {
    const auto* w=b.world().find(wizard.actor);
    for(auto p:b.mission().standing)
      if(std::max(std::abs(p.x-w->x),std::abs(p.y-w->y))<=2 && b.combat().driver()->canSpawn(b.combat(),p)) {
        s.enqueue({g::SkirmishOrderKind::summon,wizard.actor,{},p});return;
      }
  }
  if(wizard.mana>=g::SkirmishRules::boltCost) {
    auto target=m.wizards[1].actor;bool found=false;
    if(s.progress().mission && s.progress().mission->version==2)
      for(const auto& group:s.progress().mission->groups) for(auto h:group)
        for(const auto& f:b.combat().combat().fighters) if(!found && f.actor==h && f.health) {target=h;found=true;}
    const auto fighters=b.combat().combat().fighters;
    if(std::none_of(fighters.begin(),fighters.end(),[&](const auto& f){return f.actor==target && f.health;})) return;
    s.enqueue({g::SkirmishOrderKind::bolt,wizard.actor,target,{}});
  }
}
}
