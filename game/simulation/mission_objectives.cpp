#include "mission.hpp"
#include <algorithm>
#include <functional>
#include <set>
#include <tuple>
#include <stdexcept>
namespace mnm::game {
namespace {
void need(bool ok, const char *why) {
  if (!ok) throw std::invalid_argument(why);
}
void text(const std::string &s) {
  need(!s.empty() && s.size() <= 512 && s.find('\0') == std::string::npos,
       "invalid objective text");
}
bool ready(const std::vector<std::uint32_t> &after, const MissionProgress &m) {
  return std::all_of(after.begin(), after.end(), [&](auto i) {
    return m.objectives.at(i).complete;
  });
}
bool alive(const SkirmishSession &b, MissionTarget target) {
  const auto h = b.state().wizards.at(target == MissionTarget::player ? 0 : 1).actor;
  const auto c = b.combat().combat();
  const auto f = std::find_if(c.fighters.begin(), c.fighters.end(),
                            [&](const auto &a) { return a.actor == h; });
  need(b.world().find(h) && f != c.fighters.end(), "objective actor absent");
  return f->health != 0;
}
bool eliminated(const MissionObjective &o, const MissionProgress &m,
                const SkirmishSession &b) {
  if (o.target != MissionTarget::group) return !alive(b, o.target);
  const auto &group = m.groups.at(o.group);
  return !group.empty() && missionGroupDead(b, group);
}
}
void validateObjectivePlan(const MissionPlan &p, const SkirmishMission &b) {
  need(p.version == 2 && p.initial.empty() && p.reinforcement.empty() &&
       std::all_of(p.objectives.begin(), p.objectives.end(), [](const auto &s) { return s.empty(); }),
       "mixed mission definition versions");
  text(p.id); text(p.briefing);
  need(p.rewardSpell.empty() || p.rewardSpell == "cure", "unsupported objective reward");
  need(!p.nodes.empty() && p.nodes.size() <= 16 && p.groups.size() <= 8,
       "objective definition budget");
  std::set<std::string> ids;
  const auto dependencies = [&](const auto &after, std::size_t limit) {
    need(after.size() <= 16, "objective dependency budget");
    std::set<unsigned> seen;
    for (auto i : after) need(i < limit && seen.insert(i).second, "invalid objective dependency");
  };
  for (const auto &g : p.groups) {
    text(g.id); need(ids.insert(g.id).second, "duplicate mission ID");
    dependencies(g.after, p.nodes.size());
    need(!g.locations.empty() && g.locations.size() <= 8, "objective group budget");
    std::set<std::tuple<int,int,int>> cells;
    for (auto a : g.locations)
      need(missionCell(b, a) && cells.emplace(a.x,a.y,a.z).second, "invalid objective spawn");
  }
  std::vector<bool> used(p.groups.size());
  for (std::size_t i = 0; i < p.nodes.size(); ++i) {
    const auto &o = p.nodes[i]; text(o.id); text(o.text);
    need(ids.insert(o.id).second, "duplicate mission ID");
    // Authored order is topological, so files and saved indexes are unambiguous.
    dependencies(o.after, i);
    need(unsigned(o.kind) <= unsigned(ObjectiveKind::protect) &&
         unsigned(o.target) <= unsigned(MissionTarget::rival), "unknown objective kind/target");
    if (o.kind == ObjectiveKind::survive)
      need(o.ticks && o.ticks <= 4096 && o.target == MissionTarget::group && !o.group,
           "invalid survival timer");
    else {
      need(!o.ticks, "timer on nontimed objective");
      if (o.target == MissionTarget::group) {
        need(o.kind == ObjectiveKind::eliminate && o.group < p.groups.size(), "invalid objective group target");
        used[o.group] = true;
      } else need(!o.group, "group index on wizard objective");
    }
  }
  need(std::all_of(used.begin(), used.end(), [](bool v) { return v; }), "unobserved mission group");
  // Spawn prerequisites can introduce a cycle even with topologically ordered objectives.
  std::vector<unsigned> color(p.nodes.size() + p.groups.size());
  std::function<void(unsigned)> visit = [&](unsigned i) {
    need(color[i] != 1, "cyclic mission spawn dependencies");
    if (color[i] == 2) return;
    color[i] = 1;
    if (i < p.nodes.size()) {
      const auto &o = p.nodes[i];
      for (auto d : o.after) visit(d);
      if (o.kind == ObjectiveKind::eliminate && o.target == MissionTarget::group)
        visit(unsigned(p.nodes.size()) + o.group);
    } else for (auto d : p.groups[i-p.nodes.size()].after) visit(d);
    color[i] = 2;
  };
  for (unsigned i = 0; i < color.size(); ++i) visit(i);
}
MissionProgress startObjectives(const MissionPlan &p, SkirmishSession &b) {
  validateMissionPlan(p, b.mission());
  MissionProgress m; m.version = 2; m.groups.resize(p.groups.size()); m.objectives.resize(p.nodes.size());
  admitMissionGroups(p, m, b);
  for (std::size_t i = 0; i < p.groups.size(); ++i)
    need(!p.groups[i].after.empty() || !m.groups[i].empty(), "initial objective group blocked");
  return m;
}
void admitMissionGroups(const MissionPlan &p, MissionProgress &m, SkirmishSession &b) {
  for (std::size_t i = 0; i < p.groups.size(); ++i)
    if (m.groups[i].empty() && ready(p.groups[i].after, m)) {
      const auto group = b.spawnMissionGroup(p.groups[i].locations);
      if (group) m.groups[i] = *group;
    }
}
void advanceObjectives(const MissionPlan &p, MissionProgress &m, const SkirmishSession &b) {
  const auto before = m;
  for (std::size_t i = 0; i < p.nodes.size(); ++i) {
    const auto &o = p.nodes[i]; auto &state = m.objectives[i];
    if (state.complete || !ready(o.after, before)) continue;
    if (o.kind == ObjectiveKind::survive) state.complete = ++state.elapsed == o.ticks;
    else if (o.kind == ObjectiveKind::eliminate) state.complete = eliminated(o, m, b);
    else state.complete = alive(b, o.target);
  }
}
bool objectivesComplete(const MissionProgress &m) {
  return !m.objectives.empty() && std::all_of(m.objectives.begin(), m.objectives.end(),
                                            [](const auto &o) { return o.complete; });
}
bool protectionFailed(const MissionPlan &p, const SkirmishSession &b) {
  return std::any_of(p.nodes.begin(), p.nodes.end(), [&](const auto &o) {
    return o.kind == ObjectiveKind::protect && !alive(b, o.target);
  });
}
void validateObjectiveProgress(const MissionPlan &p, const MissionProgress &m, const SkirmishSession &b) {
  validateMissionPlan(p, b.mission());
  need(m.version == 2 && !m.stage && !m.reinforcement && m.initial.empty() && m.wave.empty() &&
       m.groups.size() == p.groups.size() && m.objectives.size() == p.nodes.size(), "objective continuation shape");
  const auto state = b.state();
  need(state.missionControlled && !state.victoryEnabled, "objective battle gate mismatch");
  need(!objectivesComplete(m) && !protectionFailed(p, b), "settled objectives retained in active battle");
  std::set<std::pair<unsigned,unsigned>> handles;
  for (std::size_t i = 0; i < p.groups.size(); ++i) {
    const auto &group = m.groups[i];
    need(group.empty() || (ready(p.groups[i].after, m) && group.size() == p.groups[i].locations.size()), "objective wave admission mismatch");
    need(!group.empty() || !p.groups[i].after.empty(), "initial objective group absent");
    for (auto h : group) {
      need(handles.emplace(h.slot,h.generation).second, "duplicate objective actor");
      need(std::any_of(state.units.begin(), state.units.end(), [&](const auto &u) {
        return u.actor == h && u.role == UnitRole::sentinel;
      }), "objective actor role mismatch");
    }
    missionGroupDead(b, group); // Includes generation, owner and fighter checks.
  }
  for (const auto &unit : state.units)
    if (unit.role == UnitRole::sentinel && b.world().find(unit.actor)->owner == 1)
      need(handles.count({unit.actor.slot,unit.actor.generation}) != 0,
           "unbound hostile objective actor");
  for (std::size_t i = 0; i < p.nodes.size(); ++i) {
    const auto &o = p.nodes[i]; const auto &saved = m.objectives[i];
    const bool active = ready(o.after, m);
    need(active || (!saved.complete && !saved.elapsed), "inactive objective has progress");
    if (o.kind == ObjectiveKind::survive)
      need(saved.elapsed <= o.ticks && saved.elapsed <= b.world().state().tick &&
           saved.complete == (saved.elapsed == o.ticks), "objective timer mismatch");
    else {
      need(!saved.elapsed, "nontimed objective has elapsed ticks");
      if (saved.complete) need(o.kind == ObjectiveKind::eliminate ? eliminated(o,m,b) : alive(b,o.target), "objective completion mismatch");
    }
  }
}
}
