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
  journey.native.ownerPlanner->grouped->replanning->eventInputs = false;
  // Authored 72-dimensional model isolates a changed enemy valuation. The
  // real grouped feature producer and inference run; the selected score and
  // active lifetime are never assigned by this fixture.
  auto &model = journey.native.ownerPlanner->model;
  for (unsigned outcome = 0; outcome < model.layers.size(); ++outcome) {
    auto &layer = model.layers[outcome];
    layer.observations = 1;
    layer.outside = 0;
    layer.nodes.assign(72, {});
    for (auto &node : layer.nodes)
      node[1] = 10;
    layer.matrix.assign(72 * 72, 0);
    layer.matrix[35 * 72 + 35] = 1;
    layer.nodes[35][2] = outcome == 0 ? 0 : outcome == 2 ? 4 : 10;
    layer.nodes[35][3] = outcome == 2 ? 0 : outcome == 0 ? 4 : 10;
  }
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
  for (unsigned i = 0; i < 1; ++i) {
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
    r.planning->combatValue.fill(0);
    r.memory.observedTick = 1;
    r.markedTick = 1;
    journey.native.ownerKnowledge->records[0].push_back(r);
  }
  journey.native.ownerKnowledge->nextKey[0] = 1;
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
  unsigned restores = 0, probeFrames = 0, replanned = 0, retained = 0,
           blocked = 0, moved = 0;
  auto slice = [&] {
    ++journey.native.tick;
    auto restored = cs::decode(cs::encode(journey));
    plannerSlice(journey);
    plannerSlice(restored);
    check(cs::encode(journey) == cs::encode(restored),
          "replanning restore differs");
    ++restores;
    probeFrames +=
        !journey.native.ownerPlanner->grouped->probe.search.records.empty();
  };
  auto &planner = *journey.native.ownerPlanner;
  // Copies/reassignments above are complete; these references survive slices.
  planner.grouped->replanning->admission.enabled = false;
  for (unsigned i = 0; i < 8; ++i)
    slice();
  check(!planner.active && !planner.grouped->replanning->eligible,
        "disabled task6 admitted");
  planner.grouped->replanning->admission.enabled = true;
  journey.world.creatures[0].ai.dirty932 = 1;
  for (unsigned i = 0; i < 80 && !planner.active; ++i)
    slice();
  check(planner.active.has_value() && probeFrames,
        "initial probe did not install");
  const auto firstSerial = planner.activeSerial;
  check(planner.active->memoryKey == 0, "initial task identity");
  auto &records = journey.native.ownerKnowledge->records[0];
  auto replacement = records.front();
  replacement.memory.key = 1;
  replacement.memory.actor.region = 8;
  replacement.memory.actor.position = journey.native.regions[8].position;
  replacement.planning->position = replacement.memory.actor.position;
  replacement.markedTick = 0;
  records = {replacement};
  danger.inputs.memories[0] = {replacement.memory};
  journey.native.ownerKnowledge->nextKey[0] = 2;
  const float initialValue = planner.grouped->replanning->previous->value;
  check(initialValue > -1 && initialValue < -.99f, "initial inferred value");
  auto sweep = [&] {
    for (unsigned i = 0; i < 80; ++i) {
      slice();
      if (planner.selection.reset)
        return;
    }
    throw std::runtime_error("replanning sweep did not finish");
  };
  sweep();
  retained +=
      planner.activeSerial == firstSerial && planner.selection.selected == 1;
  check(retained, "weaker fresh task displaced pursuit");
  records.front().planning->combatValue[0] = 1;
  sweep();
  replanned += planner.activeSerial != firstSerial && planner.active &&
               planner.active->memoryKey == 1;
  check(replanned, "better fresh task did not replace active task");
  check(planner.grouped->replanning->previous->value > .99f &&
            planner.features.at(35) == 1,
        "changed observation did not drive inferred improvement");
  // A current sighting moves the active root through the real memory resolver.
  journey.world.creatures[0].ai.dirty932 = 0;
  journey.world.creatures[1].ai.position = {30, 4, 2};
  records.front().markedTick = int(journey.native.tick + 1);
  slice();
  moved += planner.grouped->active->groups.nodes[planner.grouped->active->root]
                   .position == journey.world.creatures[1].ai.position &&
           journey.world.creatures[0].ai.target == 1;
  check(moved, "moving target was not reacquired");
  records.front().markedTick = 0;
  journey.world.creatures[0].ai.dirty932 = 1;
  const auto secondSerial = planner.activeSerial;
  for (auto &r : journey.native.regions)
    r.connections = {};
  // Finish any frozen in-flight probe, then begin a fresh blocked sweep.
  sweep();
  sweep();
  blocked +=
      planner.activeSerial == secondSerial && planner.selection.selected == 0;
  check(blocked, "failed fresh routes displaced retained task");
  journey.native.regions[0].dangerBase = 250;
  slice();
  const bool stopped =
      !planner.active || planner.grouped->replanning->previous->completed;
  check(stopped, "risk stop did not complete previous task");
  if (planner.active)
    sweep();
  check(!planner.active && !planner.grouped->replanning->previous,
        "completed task was not replaced by fallback");
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"restores\":" << restores
            << ",\"probe_frames\":" << probeFrames
            << ",\"retained\":" << retained << ",\"replanned\":" << replanned
            << ",\"blocked_retained\":" << blocked
            << ",\"moving_target\":" << moved
            << ",\"risk_completed\":" << unsigned(stopped) << "}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
