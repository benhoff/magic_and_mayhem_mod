#include "mission.hpp"
#include <algorithm>
#include <set>
#include <stdexcept>
namespace mnm::game {
namespace {
void need(bool ok, const char *why) {
  if (!ok)
    throw std::invalid_argument(why);
}
void text(const std::string &s) {
  need(!s.empty() && s.size() <= 512 && s.find('\0') == std::string::npos,
       "invalid mission text");
}
} // namespace
void validateMissionPlan(const MissionPlan &p, const SkirmishMission &m) {
  need(p.version == 1, "unsupported mission definition version");
  text(p.id);
  text(p.briefing);
  for (const auto &s : p.objectives)
    text(s);
  need(p.rewardSpell == "cure", "unsupported mission reward");
  for (const auto *group : {&p.initial, &p.reinforcement}) {
    need(!group->empty() && group->size() <= 8, "mission group size");
    for (std::size_t i = 0; i < group->size(); ++i) {
      auto a = (*group)[i];
      need(missionCell(m, a), "mission spawn outside standing cells");
      for (std::size_t j = 0; j < i; ++j)
        need(!(a == (*group)[j]), "duplicate mission spawn");
    }
  }
}
bool missionGroupDead(const SkirmishSession &battle,
                      const std::vector<Handle> &group) {
  const auto c = battle.combat().combat();
  bool dead = true;
  for (auto h : group) {
    const auto *e = battle.world().find(h);
    const auto f = std::find_if(c.fighters.begin(), c.fighters.end(),
                                [&](const auto &x) { return x.actor == h; });
    need(e && e->owner == 1 && f != c.fighters.end(),
         "mission group generation/owner mismatch");
    dead &= !f->health;
  }
  return dead;
}
void validateMissionProgress(const MissionPlan &plan, const MissionProgress &m,
                             const SkirmishSession &b) {
  validateMissionPlan(plan, b.mission());
  need(m.stage <= 2 && m.initial.size() == plan.initial.size() &&
           m.wave.size() == (m.reinforcement ? plan.reinforcement.size() : 0),
       "mission progress/group size mismatch");
  const auto state = b.state();
  need(state.missionControlled, "mission control absent");
  need(!state.victoryEnabled || m.stage == 2,
       "mission victory enabled before objectives");
  need(m.stage != 0 || (!m.reinforcement && m.wave.empty()),
       "early mission reinforcement");
  need(m.stage != 2 || m.reinforcement, "mission skips reinforcement");
  std::set<std::pair<unsigned, unsigned>> seen;
  for (const auto *group : {&m.initial, &m.wave})
    for (auto h : *group) {
      need(seen.emplace(h.slot, h.generation).second,
           "duplicate mission group actor");
      need(std::any_of(state.units.begin(), state.units.end(),
                       [&](const auto &u) {
                         return u.actor == h && u.role == UnitRole::sentinel;
                       }),
           "mission group is not a creature");
    }
  const bool initialDead = missionGroupDead(b, m.initial),
             waveDead = missionGroupDead(b, m.wave);
  need((m.stage == 0) == !initialDead, "mission initial objective mismatch");
  if (m.stage == 1 && m.reinforcement)
    need(!waveDead, "mission wave objective mismatch");
  if (m.stage == 2)
    need(waveDead, "mission wizard objective before wave defeat");
}
} // namespace mnm::game
