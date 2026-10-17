#include "../../reconstruction/creature-ai/owner_group_features.hpp"
#include "../../reconstruction/pathfinding/route_neighbors.hpp"
#include "native_owner_planner.hpp"
#include "native_owner_sources.hpp"
#include "native_owner_task_inputs.hpp"
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
void validateNativeOwnerGroupPlanner(const Continuation &c,
                                     const NativeOwnerPlanner *value) {
  const auto &s = value ? *value : *c.native.ownerPlanner;
  if (!s.grouped)
    return;
  const auto &g = *s.grouped;
  need(s.selection.reset || s.selection.evaluator != 1 ||
           !g.groups.nodes.empty() ||
           (g.replanning && g.replanning->prepared && !g.replanning->eligible),
       "group planner unprepared hostile evaluator");
  if (g.replanning) {
    const auto &r = *g.replanning;
    if (r.eventInputs) {
      const auto &roots = c.actions.segments.at(s.actor).contributionRoots;
      need(!roots[0] && !roots[1],
           "event planner has duplicate contribution roots");
      const auto &in = c.native.regionalDanger->inputs;
      const int commander = in.commander.at(s.owner);
      need(commander < 0 ||
               c.world.creatures.at(unsigned(commander)).ai.owner ==
                   int(s.owner),
           "event planner commander owner differs");
      for (auto point : in.observationPoints)
        need(point.x >= 0 && point.x < c.world.dimensions.x && point.y >= 0 &&
                 point.y < c.world.dimensions.y && point.z >= 0 &&
                 point.z < c.world.dimensions.z,
             "event planner special cell domain");
    }
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
  if (g.scheduler) {
    need(g.replanning.has_value(), "scheduled planner requires replanning");
    model::validateOwnerTaskScheduler(*g.scheduler, s.activeSerial);
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
  need(g.memberCursor < nativeOwnerSourceActors(s).size() &&
           g.paths.travel >= 0 && g.paths.auxiliary >= 0 &&
           g.paths.auxiliary <= 8000000 && g.paths.danger >= 0 &&
           g.paths.danger <= 8000000 && g.paths.risk >= 0 &&
           g.paths.risk <= 1000000 &&
           (g.perMember ||
            (!g.memberCursor && !g.paths.travel && !g.paths.auxiliary &&
             !g.paths.danger && !g.paths.risk)),
       "group member path continuation");
  need(g.observations.size() <= 125, "group planner snapshot bound");
  std::set<int> snapshots;
  for (const auto &r : g.observations) {
    const auto position = r.memory.actor.position;
    need(position.x >= 0 && position.x < c.world.dimensions.x &&
             position.y >= 0 && position.y < c.world.dimensions.y &&
             position.z >= 0 && position.z < c.world.dimensions.z,
         "group planner observation position");
    need(r.memory.actor.owner >= 0 && r.memory.actor.owner < 8 &&
             r.memory.actor.slot >= 0 &&
             unsigned(r.memory.actor.slot) < c.world.creatures.size() &&
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
                                   model::OwnedCreatureCommands &commands,
                                   NativeOwnerProposalEvaluation *proposal,
                                   bool executeOnly) {
  auto &s = *c.native.ownerPlanner;
  auto &g = *s.grouped;
  auto &actor = c.world.creatures.at(s.actor);
  const int tick = int(c.native.tick);
  NativeCreatureGeometry geometry(c, p);
  auto *replanning = g.replanning ? &*g.replanning : nullptr;
  updateNativeOwnerTaskInputs(c);
  auto execute = [&](int &budget) {
    if (g.active && (g.scheduler || !replanning ||
                     (!replanning->previous->completed &&
                      tick < replanning->previous->expires))) {
      const auto members = nativeOwnerSourceActors(s);
      for (auto slot : members)
        if (!c.world.creatures.at(slot).ai.active ||
            !c.world.creatures.at(slot).ai.health)
          return false;
      std::vector<int> raw, net, threat;
      for (auto slot : members) {
        const auto &member = c.world.creatures.at(slot);
        int r = 0, n = 0;
        if (member.region >= 0) {
          const auto &region = c.native.regions.at(unsigned(member.region));
          r = model::regionDanger(member, region, c.native.regionalRiskPowers);
          n = model::regionDanger(member, region, c.native.regionalRiskPowers,
                                  true);
        }
        raw.push_back(r);
        net.push_back(n);
      }
      for (const auto &e : c.execution)
        threat.push_back(e.combatValue[5]);
      const auto out = model::executeOwnerSourceGroupTask(
          *g.active, *c.native.ownerKnowledge, tick, s.owner, members, c.world,
          raw, net, g.excessPercent, threat, budget);
      for (unsigned i = 0; i < members.size(); ++i) {
        const auto slot = members[i];
        auto &member = c.world.creatures.at(slot);
        const auto &order = out.orders[i];
        if (member.ai.action == 8)
          continue;
        if (order.stop) {
          auto stop = member.ai.position;
          if (member.ai.motion) {
            const auto &route = c.actions.segments.at(slot);
            need(route.cursor < route.count, "group task stop endpoint");
            stop = route.route.at(route.cursor).position;
          }
          if (member.ai.behavior != 2 || !(member.destination == stop)) {
            c.execution[slot].commandMode = 0;
            commands.move(member, stop);
          }
        } else if (order.kind == model::OwnerHostileOrderKind::target)
          commands.target(member, order.target);
        else if (order.kind == model::OwnerHostileOrderKind::move &&
                 (member.ai.behavior != 2 ||
                  !(member.destination == order.position))) {
          c.execution[slot].commandMode = 0;
          commands.move(member, order.position);
        }
      }
      if (!out.continuing) {
        if (replanning && !g.scheduler)
          replanning->previous->completed = true;
        else if (!g.scheduler) {
          g.active.reset();
          s.active.reset();
          s.activeSerial = 0;
        }
      } else {
        s.active->position = g.active->groups.nodes[g.active->root].position;
        if (!replanning)
          return true;
      }
      return out.continuing;
    }
    return false;
  };
  auto plan = [&](int &budget) {
    if (!actor.ai.active || !actor.ai.health)
      return;
    if (executeOnly)
      return;
    if (s.selection.reset && !proposal) {
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
      g.memberCursor = 0;
      g.paths.travel = g.paths.auxiliary = g.paths.danger = g.paths.risk = 0;
      g.from = g.to = -1;
      g.candidate.reset();
      if (replanning) {
        replanning->prepared = true;
        replanning->eligible = model::admitOwnerTask6(replanning->admission);
        if (!replanning->eligible)
          return;
      }
      model::OwnerMemoryGroupServices lower;
      const reconstruction::CreatureMovementParameters shape{4, 1, 1, 3,
                                                             0, 0, 0};
      lower.regional = [&](int a, int b, int &middle) {
        return model::regionalDangerCenter(c.native.regions, a, b, shape,
                                           c.world.dimensions, middle);
      };
      lower.line = [&](auto a, auto b) { return line(geometry, a, b, shape); };
      model::buildOwnerMemoryGroups(g.groups, *c.native.ownerKnowledge, s.owner,
                                    tick, true, c.world.dimensions,
                                    c.world.relation, lower);
      // 40eff0 kind6 deadline. World/cache epoch production remains a native
      //  policy even with the recovered ordinary count-mode caller enabled.
      const int duration = 20 + 2 * int(nativeOwnerSourceActors(s).size());
      need(tick <= std::numeric_limits<int>::max() - duration,
           "group planner deadline overflow");
      if (!replanning)
        (void)model::nextRandom(s.random);
      g.deadline = tick + duration;
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
        const auto members = g.perMember ? nativeOwnerSourceActors(s)
                                         : std::vector<unsigned>{s.actor};
        need(g.memberCursor < members.size(), "member probe cursor");
        const auto memberSlot = members[g.memberCursor];
        const auto &member = c.world.creatures.at(memberSlot);
        if (member.region < 0 || node.region < 0) {
          ++g.cursor;
          g.memberCursor = 0;
          g.paths.travel = g.paths.auxiliary = g.paths.danger = g.paths.risk =
              0;
          g.probe = {};
          g.from = g.to = -1;
          continue;
        }
        // Native admission: restart a probe when its friendly origin changes.
        // The original task assumes its search inputs remain stable.
        if (g.from != member.region || g.to != node.region) {
          g.probe = {};
          g.from = member.region;
          g.to = node.region;
        }
        auto services = nativeRegionalServices(c, p, geometry, member);
        model::RegionalQuery query{reconstruction::creature_movement_parameters(
                                       geometry.actor(memberSlot)),
                                   true, false, false};
        const auto result = model::advanceOwnerTaskProbe(
            g.probe, c.native.regions, g.from, g.to, query, c.world.dimensions,
            budget, services);
        if (result == model::OwnerTaskProbeResult::pending)
          return {};
        if (result == model::OwnerTaskProbeResult::failed) {
          ++g.cursor;
          g.memberCursor = 0;
          g.paths.travel = g.paths.auxiliary = g.paths.danger = g.paths.risk =
              0;
          g.probe = {};
          g.from = g.to = -1;
          continue;
        }
        const auto at = std::find_if(
            g.probe.search.records.begin(), g.probe.search.records.end(),
            [&](const auto &v) { return v.node == g.to; });
        need(at != g.probe.search.records.end(), "group probe result missing");
        const auto payload = reconstruction::neighbor_movement(at->movement);
        int risk = 0;
        for (auto id : g.probe.path)
          risk = std::max(risk, services.danger(id));
        model::addOwnerTaskPath(g.paths, payload.accumulated_cost,
                                nativeOwnerPlannerActor(c, p, memberSlot).speed,
                                payload.unknown_18, services.danger(g.to),
                                risk);
        g.probe = {};
        g.from = g.to = -1;
        if (++g.memberCursor < members.size())
          continue;
        ++g.cursor;
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
                remembered,
                c.native.regions.at(unsigned(r->memory.actor.region)),
                c.native.regionalRiskPowers);
          } else
            snapshot.danger = 0;
          enemy.push_back(snapshot);
        }
        if (enemy.empty()) {
          g.memberCursor = 0;
          g.paths.travel = g.paths.auxiliary = g.paths.danger = g.paths.risk =
              0;
          g.probe = {};
          g.from = g.to = -1;
          continue;
        }
        std::vector<model::OwnerPlannerActor> source;
        for (auto slot : nativeOwnerSourceActors(s))
          source.push_back(nativeOwnerPlannerActor(c, p, slot));
        const auto prefix =
            model::hostileTaskGroupFeatures(source, enemy, c.world.dimensions);
        s.features.assign(prefix.begin(), prefix.end());
        if (replanning)
          replanning->candidateDuration =
              model::ownerTask6Duration(s.random, g.paths.travel);
        const int destination = node.region;
        const auto &dest = c.native.regions.at(unsigned(destination));
        const bool covered =
            !dest.parameter &&
            !line(geometry,
                  {dest.position.x, dest.position.y, c.world.dimensions.z - 1},
                  dest.position, {1, 1, 1, 0, 0, 0, 0});
        s.features.insert(
            s.features.end(),
            {int(std::sqrt(static_cast<long double>(node.uncertaintySquared))),
             g.paths.travel, g.paths.auxiliary / int(members.size()),
             g.paths.danger / int(members.size()), int(covered)});
        const auto inference = model::inferOwnerTask(s.model, s.features);
        s.candidateScore = model::ownerTaskValue(
            {{inference[0], inference[1], inference[2]}}, s.weights, s.percent);
        g.candidate = model::OwnerGroupTask{g.groups, root, g.paths.risk};
        s.candidate = {model::ownerGroupKeys(g.groups, root).front(),
                       node.position};
        s.candidateReady = true;
        g.probe = {};
        g.from = g.to = -1;
        g.memberCursor = 0;
        g.paths.travel = g.paths.auxiliary = g.paths.danger = g.paths.risk = 0;
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
    if (proposal) {
      services.root = proposal->root;
      proposal->changed = model::advanceOwnerTaskReplanning(
          s.selection, budget, s.random, services, proposal->old, tick,
          replanning->policy, proposal->bias, true, proposal->rejectOne);
      return;
    }
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
      if (g.scheduler) {
        g.scheduler->pendingExecution = 0;
        g.scheduler->executionComplete = false;
      }
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
      if (g.scheduler) {
        g.scheduler->pendingExecution = 0;
        g.scheduler->executionComplete = false;
      }
    }
  };
  if (proposal) {
    need(proposal->budget, "proposal budget missing");
    plan(*proposal->budget);
    return;
  }
  if (g.scheduler) {
    auto cleanup = [&] {
      if (replanning->previous &&
          model::retireOwnerTask(*replanning->previous,
                                 g.scheduler->executionComplete, tick,
                                 replanning->policy)) {
        g.active.reset();
        s.active.reset();
        s.activeSerial = 0;
        replanning->previous.reset();
        g.scheduler->pendingExecution = 0;
        g.scheduler->executionComplete = false;
      }
    };
    // Authored native host allocation4, not a recovered World budget producer.
    // Only ordinary direct-root scheduling is admitted here. Pending queues for
    // specialized source controllers and multiple simultaneous tasks excluded.
    model::advanceOwnerTaskScheduler(
        *g.scheduler, tick, 4,
        {cleanup, plan,
         [&] {
           return model::OwnerScheduledTask{
               s.activeSerial,
               replanning->previous ? replanning->previous->expires : 0,
               actor.ai.active && actor.ai.health != 0};
         },
         execute});
  } else {
    int executionBudget = 8;
    const bool continuing = execute(executionBudget);
    if (continuing && !replanning)
      return;
    int planningBudget = 2;
    plan(planningBudget);
  }
}
} // namespace mnm::command_session
