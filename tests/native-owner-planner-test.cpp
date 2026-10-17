#include "../apps/command-session/native_owner_planner.hpp"
#include "../apps/native-encounter/encounter.hpp"
#include "../game/persistence/snapshot.hpp"
#include <fstream>
#include <iostream>
#include <limits>
namespace cs = mnm::command_session;
namespace en = mnm::encounter;
namespace game = mnm::game;
int main(int argc, char **argv) try {
  if (argc != 3)
    throw std::runtime_error("INSTALL NEW_OUTPUT");
  const auto output = std::filesystem::path(argv[2]);
  if (!std::filesystem::create_directories(output))
    throw std::runtime_error("output exists");
  auto p = en::decodeInputs(en::loadInputs(argv[1], {0, 0, 32, 32}, 0));
  en::enableRegionalNavigation(p);
  en::enableOwnerPlanner(p, std::filesystem::path(argv[1]) / "AI/Brain.dat",
                         17);
  unsigned checks = 0, restores = 0, refusals = 0, active = 0, selecting = 0,
           memories = 0;
  auto check = [&](bool b, const char *why) {
    if (!b)
      throw std::runtime_error(why);
    ++checks;
  };
  auto rejects = [&](auto fn) {
    bool failed = false;
    try {
      fn();
    } catch (const std::exception &) {
      failed = true;
    }
    check(failed, "expected planner refusal");
    ++refusals;
  };
  const auto initial = cs::encode(p.initial);
  check(initial.at(4) == 15, "CMD1v15");
  for (unsigned version : {10u, 11u, 12u, 13u, 14u})
    rejects([&] { (void)cs::encode(p.initial, version); });
  auto oldGrouped = p.initial;
  oldGrouped.native.ownerPlanner->grouped->replanning.reset();
  auto v13 = cs::encode(oldGrouped, 13);
  check(cs::encode(cs::decode(v13), 13) == v13 &&
            !cs::decode(v13).native.ownerPlanner->grouped->replanning,
        "v13 planner compatibility");
  auto oldPlanner = p.initial;
  oldPlanner.native.ownerPlanner->grouped.reset();
  auto v12 = cs::encode(oldPlanner, 12);
  check(cs::encode(cs::decode(v12), 12) == v12 &&
            !cs::decode(v12).native.ownerPlanner->grouped,
        "v12 planner compatibility");
  auto bad = [&](auto change) {
    auto c = p.initial;
    change(c);
    rejects([&] { (void)cs::encode(c); });
  };
  bad([](auto &c) { c.native.ownerPlanner->model.dimension = 71; });
  bad([](auto &c) { c.native.ownerPlanner->model.layers[0].nodes[0][1] = 0; });
  bad([](auto &c) {
    c.native.ownerPlanner->model.layers[0].matrix[0] =
        std::numeric_limits<float>::quiet_NaN();
  });
  bad([](auto &c) {
    c.native.ownerPlanner->model.layers[0].matrix.pop_back();
  });
  bad([](auto &c) { c.native.ownerPlanner->selection.evaluator = 3; });
  bad([](auto &c) { c.native.ownerPlanner->random.left = 250; });
  bad([](auto &c) { c.native.ownerPlanner->activeSerial = 1; });
  bad([](auto &c) { c.native.ownerPlanner->candidateReady = true; });
  bad([](auto &c) { c.native.ownerPlanner->features = {1}; });
  bad([](auto &c) { c.native.ownerKnowledge.reset(); });
  bad([](auto &c) { c.native.ownerPlanner->grouped->cursor = 1; });
  bad([](auto &c) { c.native.ownerPlanner->grouped->probe.remaining = 21; });
  bad([](auto &c) { c.native.ownerPlanner->grouped->from = 9999; });
  bad([](auto &c) { c.native.ownerPlanner->grouped->excessPercent = -1; });
  bad([](auto &c) { c.native.ownerPlanner->grouped->active.emplace(); });
  bad([](auto &c) {
    auto &g = c.native.ownerPlanner->grouped->groups;
    g.nodes.resize(2);
    g.nodes[0].children = {1};
    g.nodes[1].parent = 0;
    g.nodes[1].children = {1};
  });
  for (unsigned axis : {0u, 1u, 2u})
    bad([axis](auto &c) {
      cs::model::OwnerMemory r;
      r.memory.key = 0;
      r.memory.actor.owner = 1;
      r.memory.actor.slot = 1;
      r.memory.actor.position = c.world.creatures[1].ai.position;
      if (axis == 0)
        r.memory.actor.position.x = -1;
      if (axis == 1)
        r.memory.actor.position.y = c.world.dimensions.y;
      if (axis == 2)
        r.memory.actor.position.z = c.world.dimensions.z;
      r.planning.emplace();
      r.planning->position = r.memory.actor.position;
      c.native.ownerPlanner->grouped->observations.push_back(r);
    });
  bad([](auto &c) {
    c.native.ownerPlanner->selection.reset = false;
    c.native.ownerPlanner->selection.evaluator = 1;
  });
  bad([](auto &c) {c.native.ownerPlanner->grouped->replanning->policy.expiry = -1;});
  bad([](auto &c) {c.native.ownerPlanner->grouped->replanning->previous.emplace();});
  bad([](auto &c) {c.native.ownerPlanner->grouped->replanning->bestDuration = -1;});
  auto truncated = initial;
  truncated.pop_back();
  rejects([&] { (void)cs::decode(truncated); });
  auto legacy = p.initial;
  legacy.native.ownerPlanner.reset();
  const auto old = cs::encode(legacy, 11);
  check(cs::encode(cs::decode(old), 11) == old,
        "v11 compatibility without planner");
  rejects([&] { cs::prepareNativeOwnerPlanner(p.initial, {}, 0, 0, 17); });
  check(cs::encode(p.initial) == initial, "failed activation published");
  auto fault = p;
  for (auto &layer : fault.initial.native.ownerPlanner->model.layers)
    for (auto &node : layer.nodes) {
      node[0] = 1e30f;
      node[1] = 1;
    }
  auto failing = en::create(fault, 17);
  bool rolledBack = false;
  for (unsigned tick = 0; tick < 16 && !rolledBack; ++tick) {
    const auto before = game::encodeSnapshot(failing.world().state());
    try {
      failing.step();
    } catch (const std::invalid_argument &) {
      check(before == game::encodeSnapshot(failing.world().state()),
            "failed planner frame published");
      rolledBack = true;
    }
  }
  check(rolledBack, "planner failure branch not reached");
  std::uint64_t digest = 1469598103934665603ull;
  for (unsigned seed : {17u, 91u}) {
    auto profile = p;
    profile.initial.native.ownerPlanner->random = cs::model::seedRandom(seed);
    auto live = en::create(profile, seed);
    auto restored = en::create(profile, seed);
    if (seed == 17)
      game::writeSnapshot(output / "initial.snapshot", live.world().state());
    const auto paused = game::encodeSnapshot(live.world().state());
    live.step({false, false, false});
    check(paused == game::encodeSnapshot(live.world().state()),
          "paused planner changed");
    for (unsigned tick = 1; tick <= 512; ++tick) {
      live.step();
      restored.step();
      const auto &state = live.world().state();
      check(game::encodeSnapshot(state) ==
                game::encodeSnapshot(restored.world().state()),
            "planner continuation diverged");
      auto c = cs::decode(state.systems);
      active += c.native.ownerPlanner->active.has_value();
      selecting += !c.native.ownerPlanner->selection.reset;
      memories += !c.native.ownerKnowledge->records[0].empty();
      for (auto v : state.systems) {
        digest ^= v;
        digest *= 1099511628211ull;
      }
      if (tick % 8 == 1) {
        game::World world(0);
        world.restore(game::decodeSnapshot(game::encodeSnapshot(state)));
        restored =
            game::CreatureRuleSession(std::move(world), en::runtime(profile));
        ++restores;
      }
      if (seed == 17 &&
          (tick == 1 || tick == 3 || tick == 64 || tick == 257 || tick == 512))
        game::writeSnapshot(output / (std::to_string(tick) + ".snapshot"),
                            state);
      if (tick == 16) {
        auto orphan = c;
        orphan.native.ownerPlanner.reset();
        rejects([&] { (void)cs::encode(orphan); });
        if (!c.native.ownerKnowledge->records[0].empty()) {
          c.native.ownerKnowledge->records[0].front().planning.reset();
          rejects([&] { (void)cs::encode(c); });
        }
      }
    }
  }
  check(active && selecting && memories, "planner lifecycle not exercised");
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"restores\":" << restores << ",\"refusals\":" << refusals
            << ",\"active_frames\":" << active
            << ",\"selection_frames\":" << selecting
            << ",\"memory_frames\":" << memories << ",\"digest\":\"" << digest
            << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
