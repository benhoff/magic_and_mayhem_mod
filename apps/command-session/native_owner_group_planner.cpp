#include "../../reconstruction/creature-ai/owner_group_features.hpp"
#include "../../reconstruction/pathfinding/route_neighbors.hpp"
#include "native_owner_planner.hpp"
#include "native_regional.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <stdexcept>
namespace mnm::command_session {
namespace {
void need(bool v, const char *s) {
  if (!v)
    throw std::invalid_argument(s);
}
bool line(NativeCreatureGeometry &g, model::Cell from, model::Cell to,
          reconstruction::CreatureMovementParameters shape) {
  auto h = g.movement();
  return reconstruction::test_creature_line(
      {from.x, from.y, from.z}, {to.x, to.y, to.z}, shape,
      {h.dimensions,
       [h](auto a, auto b, const auto &p, int &category, int original) {
         return reconstruction::test_movement(a, b, p, category, original, h);
       }});
}
} // namespace
void validateNativeOwnerGroupPlanner(const Continuation &c) {
  const auto &s = *c.native.ownerPlanner;
  if (!s.grouped)
    return;
  const auto &g = *s.grouped;
  need(s.selection.reset || s.selection.evaluator != 1 ||
           !g.groups.nodes.empty() ||
           (g.replanning && g.replanning->prepared && !g.replanning->eligible),
       "group planner unprepared hostile evaluator");
  if (g.replanning) {
    const auto &r = *g.replanning;
    model::validateOwnerReplacementPolicy(r.policy);
    need(r.admission.brain <= 1 && r.candidateDuration >= 0 &&
             r.bestDuration >= 0,
         "group planner replanning domain");
    need(!r.eligible || r.prepared, "group planner admission phase");
    need(r.previous.has_value() == g.active.has_value(),
         "group planner previous task identity");
    if (r.previous) {
      model::validateOwnerPreviousTask(*r.previous);
      need(unsigned(r.previous->started) <= c.native.tick,
           "group planner task from future");
    }
  }
  model::validateOwnerMemoryGroups(g.groups, c.world.dimensions);
  need(g.cursor <= (g.groups.nodes.empty()
                        ? 0
                        : g.groups.nodes[0].children.size()) &&
           g.excessPercent >= 0 && g.excessPercent <= 10000 &&
           g.probe.remaining >= 0 && g.probe.remaining <= 20 &&
           unsigned(g.probe.result) <=
               unsigned(model::OwnerTaskProbeResult::reached),
       "group planner continuation");
  need(g.observations.size() <= 125, "group planner snapshot bound");
  std::set<int> snapshots;
  for (const auto &r : g.observations) {
    const auto position = r.memory.actor.position;
    need(position.x >= 0 && position.x < c.world.dimensions.x &&
             position.y >= 0 && position.y < c.world.dimensions.y &&
             position.z >= 0 && position.z < c.world.dimensions.z,
         "group planner observation position");
    need(r.memory.actor.owner >= 0 && r.memory.actor.owner < 8 &&
             r.memory.actor.slot >= 0 && r.memory.actor.slot < 2 &&
             r.memory.actor.region >= -1 &&
             (r.memory.actor.region < 0 ||
              unsigned(r.memory.actor.region) < c.native.regions.size()),
         "group planner observation reference");
    need(r.memory.key >= 0 && snapshots.insert(r.memory.key).second &&
             r.planning.has_value() &&
             r.planning->position == r.memory.actor.position,
         "group planner snapshot identity");
    model::validateOwnerPlannerActor(*r.planning);
  }
  const auto count = c.native.regions.size();
  auto region = [&](int id) { return id >= 0 && unsigned(id) < count; };
  for (const auto &n : g.groups.nodes)
    need(n.region == -1 || region(n.region), "group planner node region");
  need(g.from == -1 || region(g.from), "group planner origin");
  need(g.to == -1 || region(g.to), "group planner destination");
  const auto &p = g.probe.search;
  if (!p.initialized)
    need(p.dirty && p.records.empty() && p.open.empty() && g.probe.path.empty(),
         "group planner empty search state");
  need(p.open.size() <= 4096 && p.records.size() <= 4096 &&
           g.probe.path.size() <= 4096,
       "group planner search bounds");
  if (p.initialized)
    need(region(p.start) && region(p.destination) && region(p.best) &&
             p.start == g.from && p.destination == g.to,
         "group planner search identity");
  std::set<int> records;
  for (const auto &r : p.records)
    need(region(r.node) && (r.predecessor == -1 || region(r.predecessor)) &&
             records.insert(r.node).second,
         "group planner search records");
  for (auto r : p.open)
    need(records.count(r.node), "group planner search queue");
  std::set<int> path;
  for (auto id : g.probe.path)
    need(region(id) && records.count(id) && path.insert(id).second,
         "group planner search path");
  if (p.initialized)
    need(records.count(p.start) && records.count(p.best),
         "group planner search roots");
  for (const auto *task : {&g.candidate, &g.best, &g.active})
    if (*task) {
      model::validateOwnerMemoryGroups((*task)->groups, c.world.dimensions);
      need((*task)->root > 0 && (*task)->root < (*task)->groups.nodes.size() &&
               (*task)->groups.nodes[(*task)->root].memoryKey < 0 &&
               (*task)->risk >= 0 && (*task)->risk <= 1000000,
           "group planner task identity");
      for (const auto &n : (*task)->groups.nodes)
        need(n.region == -1 || region(n.region), "group planner task region");
    }
  need(g.active.has_value() == s.active.has_value(),
       "group planner active identity");
  need(s.candidateReady == g.candidate.has_value(),
       "group planner candidate identity");
}
void updateNativeOwnerGroupPlanner(Continuation &c,
                                   const NativeCreatureProfile &p,
                                   model::OwnedCreatureCommands &commands) {
  auto &s = *c.native.ownerPlanner;
  auto &g = *s.grouped;
  auto &actor = c.world.creatures.at(s.actor);
  const int tick = int(c.native.tick);
  NativeCreatureGeometry geometry(c, p);
  auto *replanning = g.replanning ? &*g.replanning : nullptr;
  if (g.active && (!replanning || (!replanning->previous->completed &&
                                   tick < replanning->previous->expires))) {
    int raw = 0, net = 0;
    if (actor.region >= 0) {
      const auto &r = c.native.regions.at(unsigned(actor.region));
      raw = model::regionDanger(actor, r, c.native.regionalRiskPowers);
      net = model::regionDanger(actor, r, c.native.regionalRiskPowers, true);
    }
    int budget = 8;
    const auto out = model::executeOwnerGroupTask(
        *g.active, *c.native.ownerKnowledge, tick, s.owner, s.actor, c.world,
        raw, net, g.excessPercent,
        {{c.execution[0].combatValue[5], c.execution[1].combatValue[5]}},
        budget);
    if (out.stop && actor.ai.action != 8) {
      auto stop = actor.ai.position;
      if (actor.ai.motion) {
        const auto &route = c.actions.segments.at(s.actor);
        need(route.cursor < route.count, "group task stop endpoint");
        stop = route.route.at(route.cursor).position;
      }
      if (actor.ai.behavior != 2 || !(actor.destination == stop)) {
        c.execution[s.actor].commandMode = 0;
        commands.move(actor, stop);
      }
    }
    if (!out.continuing) {
      if (replanning)
        replanning->previous->completed = true;
      else {
        g.active.reset();
        s.active.reset();
        s.activeSerial = 0;
      }
    } else {
      s.active->position = g.active->groups.nodes[g.active->root].position;
      if (actor.ai.action != 8) {
        if (out.kind == model::OwnerHostileOrderKind::target)
          commands.target(actor, out.target);
        else if (out.kind == model::OwnerHostileOrderKind::move &&
                 (actor.ai.behavior != 2 ||
                  !(actor.destination == out.position))) {
          c.execution[s.actor].commandMode = 0;
          commands.move(actor, out.position);
        }
      }
      if (!replanning)
        return;
    }
  }
  if (!actor.ai.active || !actor.ai.health)
    return;
  if (s.selection.reset) {
    s.selection.value = -2;
    s.selection.ties = 0;
    s.selection.selected = -1;
    g.best.reset();
  }
  auto prepare = [&](unsigned evaluator) {
    if (!evaluator) {
      s.fallbackEmitted = false;
      return;
    }
    s.hostileEmitted = false;
    s.candidateReady = false;
    s.candidate = {};
    s.features.clear();
    g.groups = {};
    g.observations = c.native.ownerKnowledge->records[s.owner];
    g.cursor = 0;
    g.probe = {};
    g.from = g.to = -1;
    g.candidate.reset();
    if (replanning) {
      replanning->prepared = true;
      replanning->eligible = model::admitOwnerTask6(replanning->admission);
      if (!replanning->eligible)
        return;
    }
    model::OwnerMemoryGroupServices lower;
    const reconstruction::CreatureMovementParameters shape{4, 1, 1, 3, 0, 0, 0};
    lower.regional = [&](int a, int b, int &middle) {
      return model::regionalDangerCenter(c.native.regions, a, b, shape,
                                         c.world.dimensions, middle);
    };
    lower.line = [&](auto a, auto b) { return line(geometry, a, b, shape); };
    model::buildOwnerMemoryGroups(g.groups, *c.native.ownerKnowledge, s.owner,
                                  tick, true, c.world.dimensions,
                                  c.world.relation, lower);
    // 40eff0's kind6 deadline; native lab invokes this at a fresh selection
    // sweep rather than claiming original World scheduling or cache epochs.
    need(tick <= std::numeric_limits<int>::max() - 22,
         "group planner deadline overflow");
    if (!replanning)
      (void)model::nextRandom(s.random);
    g.deadline = tick + 22;
  };
  auto next = [&](unsigned evaluator, int &budget) -> std::optional<float> {
    if (!evaluator) {
      if (s.fallbackEmitted || budget < 1)
        return {};
      s.fallbackEmitted = true;
      --budget;
      return -1.f;
    }
    if (replanning && !replanning->eligible)
      return {};
    auto &roots = g.groups.nodes[0].children;
    while (g.cursor < roots.size() && tick <= g.deadline) {
      const auto root = roots[g.cursor];
      const auto &node = g.groups.nodes[root];
      if (actor.region < 0 || node.region < 0) {
        ++g.cursor;
        g.probe = {};
        g.from = g.to = -1;
        continue;
      }
      // Native admission: restart a probe when its friendly origin changes.
      // The original task assumes its search inputs remain stable.
      if (g.from != actor.region || g.to != node.region) {
        g.probe = {};
        g.from = actor.region;
        g.to = node.region;
      }
      auto services = nativeRegionalServices(c, p, geometry, actor);
      model::RegionalQuery query{
          reconstruction::creature_movement_parameters(geometry.actor(s.actor)),
          true, false, false};
      const auto result = model::advanceOwnerTaskProbe(
          g.probe, c.native.regions, g.from, g.to, query, c.world.dimensions,
          budget, services);
      if (result == model::OwnerTaskProbeResult::pending)
        return {};
      ++g.cursor;
      if (result == model::OwnerTaskProbeResult::failed) {
        g.probe = {};
        g.from = g.to = -1;
        continue;
      }
      std::vector<model::OwnerPlannerActor> enemy;
      for (int key : model::ownerGroupKeys(g.groups, root)) {
        const auto &rows = g.observations;
        auto r = std::find_if(rows.begin(), rows.end(), [&](const auto &v) {
          return v.memory.key == key;
        });
        if (r == rows.end() || !r->planning) {
          enemy.clear();
          break;
        }
        auto snapshot = *r->planning;
        if (r->memory.actor.region >= 0) {
          model::AutonomousCreature remembered;
          remembered.ai.owner = r->memory.actor.owner;
          remembered.ai.physical = snapshot.physical;
          remembered.ai.bravery = snapshot.bravery;
          remembered.flag104 = snapshot.flag104;
          snapshot.danger = model::regionDanger(
              remembered, c.native.regions.at(unsigned(r->memory.actor.region)),
              c.native.regionalRiskPowers);
        } else
          snapshot.danger = 0;
        enemy.push_back(snapshot);
      }
      if (enemy.empty()) {
        g.probe = {};
        g.from = g.to = -1;
        continue;
      }
      if (replanning)
        replanning->candidateDuration = model::ownerTaskDuration(s.random);
      const auto friendly = nativeOwnerPlannerActor(c, p, s.actor);
      const auto prefix = model::hostileTaskGroupFeatures({friendly}, enemy,
                                                          c.world.dimensions);
      s.features.assign(prefix.begin(), prefix.end());
      const auto at = std::find_if(
          g.probe.search.records.begin(), g.probe.search.records.end(),
          [&](const auto &v) { return v.node == g.to; });
      need(at != g.probe.search.records.end(), "group probe result missing");
      const auto payload = reconstruction::neighbor_movement(at->movement);
      const auto &dest = c.native.regions.at(unsigned(g.to));
      const bool covered =
          !dest.parameter &&
          !line(geometry,
                {dest.position.x, dest.position.y, c.world.dimensions.z - 1},
                dest.position, {1, 1, 1, 0, 0, 0, 0});
      s.features.insert(
          s.features.end(),
          {int(std::sqrt(static_cast<long double>(node.uncertaintySquared))),
           model::ownerTaskTravelTime(payload.accumulated_cost, friendly.speed),
           payload.unknown_18, services.danger(g.to), int(covered)});
      const auto inference = model::inferOwnerTask(s.model, s.features);
      s.candidateScore = model::ownerTaskValue(
          {{inference[0], inference[1], inference[2]}}, s.weights, s.percent);
      int risk = 0;
      for (auto id : g.probe.path)
        risk = std::max(risk, services.danger(id));
      g.candidate = model::OwnerGroupTask{g.groups, root, risk};
      s.candidate = {model::ownerGroupKeys(g.groups, root).front(),
                     node.position};
      s.candidateReady = true;
      g.probe = {};
      g.from = g.to = -1;
      return s.candidateScore;
    }
    s.hostileEmitted = true;
    return {};
  };
  model::OwnerTaskSelectionServices services{
      2,
      true,
      prepare,
      next,
      [&](unsigned i) {
        if (i) {
          g.best = g.candidate;
          if (replanning)
            replanning->bestDuration = replanning->candidateDuration;
        } else
          g.best.reset();
      },
      []() {}};
  int budget = 2;
  model::advanceOwnerTaskSelection(s.selection, budget, s.random, services);
  if (replanning && s.selection.reset && s.selection.selected == 0 &&
      model::replaceOwnerTask(s.selection.value,
                              replanning->previous ? &*replanning->previous
                                                   : nullptr,
                              tick, replanning->policy)) {
    g.active.reset();
    s.active.reset();
    s.activeSerial = 0;
    replanning->previous.reset();
  }
  if (s.selection.reset && s.selection.selected == 1 &&
      (!replanning ||
       model::replaceOwnerTask(s.selection.value,
                               replanning->previous ? &*replanning->previous
                                                    : nullptr,
                               tick, replanning->policy))) {
    need(g.best.has_value() &&
             s.nextSerial < std::numeric_limits<unsigned>::max(),
         "group planner selected task");
    if (replanning) {
      need(replanning->bestDuration <= std::numeric_limits<int>::max() - tick,
           "group planner task expiration overflow");
      model::OwnerPreviousTask previous;
      previous.started = tick;
      previous.expires = tick + replanning->bestDuration;
      previous.value = s.selection.value;
      previous.weighted = s.selection.weighted;
      const auto &policy = replanning->policy;
      previous.baseline =
          policy.ownerAvailable
              ? float(static_cast<long double>(policy.ownerNumerator) /
                      (policy.ownerDenominator + 1))
              : 0;
      replanning->previous = previous;
    }
    g.active = g.best;
    s.active = model::OwnerHostileTask{
        model::ownerGroupKeys(g.active->groups, g.active->root).front(),
        g.active->groups.nodes[g.active->root].position};
    s.activeSerial = s.nextSerial++;
  }
}
} // namespace mnm::command_session
