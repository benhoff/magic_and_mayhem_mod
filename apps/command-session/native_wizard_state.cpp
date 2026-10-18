#include "native_wizard_state.hpp"
#include "runtime.hpp"
#include <algorithm>
#include <cmath>
#include <set>
#include <stdexcept>
namespace mnm::command_session {
void validateNativeWizards(const Continuation &c) {
  if (!c.native.wizards)
    return;
  const auto &s = *c.native.wizards;
  const auto n = c.world.creatures.size();
  auto need = [](bool b, const char *why) {
    if (!b)
      throw std::invalid_argument(why);
  };
  need(c.native.present && !c.native.ownerPlanner && s.tick == c.native.tick &&
           s.tick <= INT32_MAX && s.controllers.size() >= 1 && s.controllers.size() <= 8 && n <= 8,
       "native wizard host identity");
  need(s.spells.size() == 104 && s.admission.spells.size() == 104 &&
           s.admission.resource.size() == n &&
           s.admission.direction.size() == n &&
           s.admission.profiles.size() == 8 && s.effectCapacity >= 1 &&
           s.effectCapacity <= 32 && s.effects.size() == s.effectCapacity &&
           s.planningAllocation >= 1 && s.planningAllocation <= 64 &&
           s.regenerationPeriod >= 1 && s.regenerationPeriod <= 10000 &&
           s.regenerationRate <= 1024 && s.nextSerial &&
           s.retirements < s.nextSerial && s.random.left < 250 &&
           s.random.right < 250 &&
           (s.random.right + 250 - s.random.left) % 250 == 103,
       "native wizard storage/configuration");
  for (unsigned i = 0; i < 104; ++i) {
    const auto &v = s.spells[i];
    need(v.range >= 0 && v.range <= 128 && v.speed >= 0 && v.speed <= 64 &&
             v.duration >= 0 && v.duration <= 1000000 &&
             s.admission.spells[i].cost <= 0x7fffff &&
             v.magnitude >= 0 && v.magnitude <= 1000000,
         "native wizard spell definition");
  }
  for (const auto &p : s.admission.profiles) {
    need(p.notices.size() <= 20, "native wizard notice bound");
    for (auto k : p.known)
      need(k <= 1, "native wizard learned flag");
  }
  for (auto direction : s.admission.direction)
    need(direction < 8, "native wizard facing");
  auto cell = [&](model::Cell p) {
    return p.x >= 0 && p.y >= 0 && p.z >= 0 &&
           p.x < c.world.dimensions.x && p.y < c.world.dimensions.y &&
           p.z < c.world.dimensions.z;
  };
  std::set<unsigned> actors, serials;
  for (const auto &v : s.controllers) {
    need(v.actor < n && actors.insert(v.actor).second &&
             c.world.creatures[v.actor].ai.type == 0 &&
             c.world.creatures[v.actor].ai.kind == 0 &&
             !c.world.creatures[v.actor].form && v.maximumMana >= 1 &&
             v.maximumMana <= 0x7fffff &&
             s.admission.resource[v.actor] <= v.maximumMana * 256u &&
             v.cursor <= n && v.nextThink <= INT32_MAX,
         "native wizard actor/mana identity");
    model::validateOwnerTaskSelection(v.selection, 4);
    auto plan = [&](const auto &p, bool active) {
      if (!p)
        return;
      need((p->spell == 17 || p->spell == 41 || p->spell == 73) &&
               v.learned[p->spell] && p->target < n && cell(p->stand) && cell(p->aim) &&
               std::isfinite(p->score) && p->score >= -1 && p->score <= 1,
           "native wizard plan admission");
      if (active)
        need(p->serial && p->serial < s.nextSerial &&
                 serials.insert(p->serial).second && p->started <= s.tick &&
                 p->expires <= INT32_MAX && p->expires >= p->started && p->expires - p->started <= 1000000,
             "native wizard active task identity");
      else
        need(!p->serial && !p->started && !p->expires && !p->issued,
             "native wizard unsaved task identity");
    };
    plan(v.candidate, false);
    plan(v.best, false);
    plan(v.active, true);
  }
  need(s.retirements + serials.size() == s.nextSerial - 1,
       "native wizard task conservation");
  for (const auto &e : s.effects)
    if (e.active) {
      need(e.generation && (e.spell == 41 || e.spell == 73),
           "native wizard effect kind");
      need(e.animationAsset == (e.spell == 41 ? s.cureAsset : s.projectileAsset) &&
               e.animationSequence == (e.spell == 41 ? s.cureSequence : s.projectileSequence) &&
               !e.animation.reverse && e.animation.forward.pc <= 1000000 &&
               e.animation.forward.delay <= 1000000,
           "native wizard effect animation");
      if (e.spell == 41)
        need(e.cure.source < n && e.cure.target < n && e.cure.spell == 41 &&
                 (e.cure.type == 11 || e.cure.type == 77) &&
                 (!e.cure.cachedActor || *e.cure.cachedActor < n),
             "native wizard Cure effect");
      else {
        const auto &p = e.placement;
        need(p.active && p.initialized && p.type == 3 && p.parameters[8] == 73 &&
                 p.parameters[9] <= 64 && p.parameters[6] == e.projectile.attacker &&
                 p.units[0] < unsigned(c.world.dimensions.x)*32u &&
                 p.units[1] < unsigned(c.world.dimensions.y)*32u &&
                 p.units[2] < unsigned(c.world.dimensions.z)*16u &&
                 p.position == std::array<unsigned,3>{p.units[0]>>5,p.units[1]>>5,p.units[2]>>4} &&
                 p.cell == (p.position[2]*unsigned(c.world.dimensions.y)+p.position[1])*unsigned(c.world.dimensions.x)+p.position[0] &&
                 e.movement.movement.motion.changes <= 8192 &&
                 e.projectile.remaining <= unsigned(s.spells[73].duration),
             "native wizard projectile position/lifetime");
        need(e.projectile.type == 3 && e.projectile.spell == 73 &&
                 e.projectile.source < n && e.projectile.attacker < n &&
                 (e.projectile.target == UINT32_MAX ||
                  e.projectile.target < n) &&
                 (!e.projectile.cachedActor || *e.projectile.cachedActor < n) &&
                 (e.movement.candidate == reconstruction::NoCreature ||
                  e.movement.candidate < n),
             "native wizard projectile effect");
      }
    }
}
void validateNativeWizardConfiguration(const Continuation &c, const Continuation &initial) {
  auto need = [](bool b) {
    if (!b) throw std::invalid_argument("native wizard immutable configuration differs");
  };
  need(bool(c.native.wizards) == bool(initial.native.wizards));
  if (!c.native.wizards) return;
  const auto &a=*c.native.wizards, &b=*initial.native.wizards;
  need(a.planningAllocation==b.planningAllocation && a.regenerationRate==b.regenerationRate &&
       a.regenerationPeriod==b.regenerationPeriod && a.effectCapacity==b.effectCapacity &&
       a.cureAsset==b.cureAsset && a.cureSequence==b.cureSequence &&
       a.projectileAsset==b.projectileAsset && a.projectileSequence==b.projectileSequence &&
       a.controllers.size()==b.controllers.size() && a.spells.size()==b.spells.size() &&
       a.admission.spells.size()==b.admission.spells.size() && a.admission.specialValue==b.admission.specialValue);
  need(c.world.creatures.size()==initial.world.creatures.size());
  for(unsigned i=0;i<c.world.creatures.size();++i){
    const auto &x=c.world.creatures[i], &y=initial.world.creatures[i];
    need(x.ai.type==y.ai.type && x.ai.kind==y.ai.kind && x.ai.owner==y.ai.owner &&
         x.ai.maximumHealth==y.ai.maximumHealth && x.width==y.width && x.height==y.height &&
         x.aimXY==y.aimXY && x.aimZ==y.aimZ && x.movementKind==y.movementKind && x.ai.hth==y.ai.hth);
  }
  for(unsigned i=0;i<a.controllers.size();++i){
    const auto &x=a.controllers[i], &y=b.controllers[i];
    need(x.actor==y.actor && x.maximumMana==y.maximumMana && x.producing==y.producing && x.learned==y.learned);
  }
  for(unsigned i=0;i<a.spells.size();++i){
    const auto &x=a.spells[i], &y=b.spells[i];
    need(x.magnitude==y.magnitude && x.duration==y.duration && x.range==y.range && x.speed==y.speed);
    const auto &u=a.admission.spells[i], &v=b.admission.spells[i];
    need(u.cost==v.cost && u.pathMode==v.pathMode && u.resourceMode==v.resourceMode &&
         u.resourceDelta==v.resourceDelta && u.travelKind==v.travelKind);
  }
  for(unsigned i=0;i<28;++i)for(unsigned j=0;j<8;++j)
    need(a.admission.muzzle[i][j]==b.admission.muzzle[i][j]);
}
} // namespace mnm::command_session
