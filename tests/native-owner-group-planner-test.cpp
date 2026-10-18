#include "../apps/command-session/native_owner_planner.hpp"
#include "../apps/native-encounter/encounter.hpp"
#include <iostream>
namespace cs = mnm::command_session;
namespace en = mnm::encounter;
int main(int argc, char **argv) try {
  if (argc != 2)
    throw std::runtime_error("INSTALL");
  unsigned checks = 0;
  auto check = [&](bool v, const char *s) {
    if (!v)
      throw std::runtime_error(s);
    ++checks;
  };
  auto p = en::decodeInputs(en::loadInputs(argv[1], {0, 0, 32, 32}, 0));
  en::enableRegionalNavigation(p);
  en::enableOwnerPlanner(p, std::filesystem::path(argv[1]) / "AI/Brain.dat",
                         17);
  // Controlled external sightings/world movement isolate the composed native
  // planner from creature motion. Every slice also runs from a CMD1 restore.
  auto journey = p.initial;
  // Retain the historical v13/v14 native-cadence regression domain.
  journey.native.ownerPlanner->grouped->scheduler.reset();
  journey.native.ownerPlanner->grouped->replanning.reset();
  journey.native.regions.assign(16, {});
  for (unsigned i = 0; i < 16; ++i) {
    auto &r = journey.native.regions[i];
    r.position = {int(i * 2), 2, 2};
    r.parameter = 1;
    r.dangerBase = i % 5;
    if (i + 1 < 16) {
      auto &e = r.connections[0];
      e.destination = int(i + 1);
      e.cost = 40;
      e.clearance.fill(8);
    }
  }
  auto &danger = *journey.native.regionalDanger;
  danger.state = {};
  danger.state.pendingDanger.resize(16);
  danger.state.pendingDeduction.resize(16);
  danger.state.pendingBase.resize(16);
  danger.state.actorSupport.resize(danger.inputs.actors.size());
  for (auto &a : danger.inputs.actors)
    a.region = 0;
  journey.world.creatures[0].region = 0;
  journey.world.creatures[1].region = 15;
  journey.world.creatures[0].ai.engaged =
      journey.world.creatures[0].ai.incoming =
          journey.world.creatures[0].ai.readiness =
              journey.world.creatures[0].ai.dirty932 = 0;
  auto projection = cs::nativeOwnerPlannerActor(journey, p.native, 1);
  for (unsigned i = 0; i < 3; ++i) {
    cs::model::OwnerMemory r;
    r.memory.key = int(i);
    r.memory.actor.slot = 1;
    r.memory.actor.owner = 1;
    r.memory.actor.health = 100;
    r.memory.actor.region = int(15 - i * 7);
    r.memory.actor.position =
        journey.native.regions[unsigned(r.memory.actor.region)].position;
    r.planning = projection;
    r.planning->position = r.memory.actor.position;
    r.memory.observedTick = 1;
    r.markedTick = 1;
    journey.native.ownerKnowledge->records[0].push_back(r);
  }
  journey.native.ownerKnowledge->nextKey[0] = 3;
  for (const auto &r : journey.native.ownerKnowledge->records[0])
    danger.inputs.memories[0].push_back(r.memory);
  auto plannerSlice = [&](auto &c) {
    auto h = cs::nativeProviders(p.native, en::consequences(p))(c);
    cs::model::OwnedCreatureCommands commands(
        c.world, c.execution, c.bindings, h.execution, h.idle, h.follow,
        h.queue, h.body, h.supplied, h.release, h.peer);
    commands.installBodies(h.commandBodies, h.originalBehaviorBindings);
    commands.installActions(h.actionTransition, h.actionTick);
    cs::updateNativeOwnerPlanner(c, p.native, commands);
  };
  unsigned probeFrames = 0, groupCandidates = 0, lost = 0, reacquired = 0,
           riskStopped = 0;
  bool installed = false;
  for (unsigned tick = 1; tick <= 40; ++tick) {
    journey.native.tick = tick;
    auto &planner = *journey.native.ownerPlanner;
    for (auto &r : journey.native.ownerKnowledge->records[0])
      r.markedTick = int(tick);
    auto resume = cs::decode(cs::encode(journey, 13));
    plannerSlice(journey);
    plannerSlice(resume);
    check(cs::encode(journey, 13) == cs::encode(resume, 13),
          "group probe restore differs");
    probeFrames += !planner.grouped->probe.search.records.empty();
    groupCandidates += planner.grouped->cursor;
    if (planner.grouped->active) {
      installed = true;
      break;
    }
  }
  check(installed && groupCandidates && probeFrames,
        "multi-memory cross-region planner absent");
  for (unsigned phase = 0; phase < 4; ++phase) {
    journey.native.tick += 1;
    auto &planner = *journey.native.ownerPlanner;
    for (auto &r : journey.native.ownerKnowledge->records[0])
      r.markedTick = phase == 1 ? 0 : int(journey.native.tick);
    journey.world.creatures[0].ai.engaged =
        journey.world.creatures[0].ai.incoming =
            journey.world.creatures[0].ai.readiness =
                journey.world.creatures[0].ai.dirty932 = 0;
    if (phase == 3)
      journey.native.regions[0].dangerBase = 250;
    auto resume = cs::decode(cs::encode(journey, 13));
    plannerSlice(journey);
    plannerSlice(resume);
    check(cs::encode(journey, 13) == cs::encode(resume, 13),
          "lost/reacquired task restore differs");
    if (phase == 1) {
      lost += journey.world.creatures[0].ai.behavior == 2;
    }
    if (phase == 2) {
      reacquired += journey.world.creatures[0].ai.target == 1;
    }
    if (phase == 3) {
      riskStopped += !planner.grouped->active;
    }
  }
  check(lost && reacquired && riskStopped, "pursuit lifecycle witnesses");
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"probe_frames\":" << probeFrames
            << ",\"candidate_progress\":" << groupCandidates
            << ",\"lost\":" << lost << ",\"reacquired\":" << reacquired
            << ",\"risk_stops\":" << riskStopped << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
