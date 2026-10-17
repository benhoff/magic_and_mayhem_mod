#include "../apps/command-session/native_owner_sources.hpp"
#include "../apps/command-session/native_owner_task_inputs.hpp"
#include "../apps/native-encounter/encounter.hpp"
#include "../game/persistence/snapshot.hpp"
#include <algorithm>
#include <iostream>
#include <set>
namespace cs = mnm::command_session;
namespace en = mnm::encounter;
namespace game = mnm::game;
int main(int argc, char **argv) try {
  if (argc != 3)
    throw std::runtime_error("INSTALL NEW_OUTPUT");
  const auto output = std::filesystem::path(argv[2]);
  if (!std::filesystem::create_directories(output))
    throw std::runtime_error("output exists");
  auto p = en::decodeInputs(en::loadInputs(argv[1], {0, 0, 32, 32}, 8));
  en::enableRegionalNavigation(p);
  en::enableOwnerPlanner(p, std::filesystem::path(argv[1]) / "AI/Brain.dat",
                         17);
  unsigned checks = 0, restores = 0, refusals = 0, active = 0, multiple = 0,
           split = 0, merged = 0;
  auto check = [&](bool b, const char *why) {
    if (!b)
      throw std::runtime_error(why);
    ++checks;
  };
  auto reject = [&](auto f) {
    bool failed = false;
    try {
      f();
    } catch (const std::exception &) {
      failed = true;
    }
    check(failed, "expected source refusal");
    ++refusals;
  };
  const auto initial = cs::encode(p.initial);
  check(initial[4] == 17, "CMD1v17 source continuation");
  check(cs::encode(cs::decode(initial)) == initial, "initial source roundtrip");
  for (unsigned version = 1; version <= 16; ++version)
    reject([&] { (void)cs::encode(p.initial, version); });
  const auto &storage = p.initial.native.ownerSources->storage;
  check(storage.sources.graph.size() == 3 && storage.manager.nodes.size() == 3,
        "two leaves and formed group");
  check(storage.sources.graph[2].children == std::vector<unsigned>({0, 1}),
        "original controller join order");
  check(storage.pending.general.size() == 2 &&
            storage.pending.other.size() == 2,
        "pending source registration");
  auto bad = [&](auto f) {
    auto c = p.initial;
    f(c);
    reject([&] { (void)cs::encode(c); });
  };
  bad([](auto &c) {
    c.native.ownerSources->storage.manager.nodes.push_back(0);
  });
  bad([](auto &c) {
    c.native.ownerSources->storage.sources.graph[2].children.push_back(2);
  });
  bad([](auto &c) {
    c.native.ownerSources->storage.pendingRoots.push_back(999);
  });
  bad([](auto &c) {
    c.native.ownerSources->planners[0].planner->members = {0, 0};
  });
  bad([](auto &c) {
    c.native.ownerSources->storage.sources.information[2].recipient = 7;
  });
  bad([](auto &c) {
    c.native.ownerSources->storage.sources.recipientRoot = {2};
  });
  bad([](auto &c) { c.native.ownerSources->storage.sources.mode = 1; });
  bad([](auto &c) { c.actions.segments[1].contributionRoots[0] = 1; });
  auto rollback = p.initial;
  reject([&] { cs::splitNativeOwnerSource(rollback, 2, {0, 1}); });
  check(cs::encode(rollback) == initial, "failed split rollback");
  reject([&] { cs::mergeNativeOwnerSources(rollback, 0, 1); });
  check(cs::encode(rollback) == initial, "failed merge rollback");
  std::uint64_t digest = 1469598103934665603ull;
  std::set<unsigned> orderedActors;
  bool sharedCredit = false, separateCredit = false;
  for (unsigned journey = 0; journey < 2; ++journey) {
    auto session = en::create(p, 17);
    if (!journey)
      game::writeSnapshot(output / "initial.snapshot", session.world().state());
    for (unsigned frame = 0; frame < 256; ++frame) {
      if (journey && (frame == 32 || frame == 96)) {
        auto state = session.world().state();
        auto c = cs::decode(state.systems);
        auto &s = *c.native.ownerSources;
        if (frame == 32) {
          cs::splitNativeOwnerSource(c, 2, {0});
          ++split;
          check(c.native.ownerSources->planners.size() == 2,
                "split creates independent root planner");
        } else {
          std::vector<unsigned> roots;
          for (auto id : s.storage.manager.nodes)
            if (!s.storage.sources.graph[id].parent)
              roots.push_back(id);
          check(roots.size() == 2, "two roots before merge");
          cs::mergeNativeOwnerSources(c, roots[0], roots[1]);
          ++merged;
        }
        check(cs::encode(cs::decode(cs::encode(c))) == cs::encode(c),
              "topology checkpoint");
        state.systems = cs::encode(c);
        game::World world(0);
        world.restore(std::move(state));
        session = game::CreatureRuleSession(std::move(world), en::runtime(p));
      }
      session.step({});
      const auto state = session.world().state();
      const auto c = cs::decode(state.systems);
      check(c.world.creatures.size() == 3 && c.native.ownerSources.has_value(),
            "three-actor source roots");
      unsigned tasks = 0;
      for (const auto &row : c.native.ownerSources->planners)
        if (row.planner && row.planner->active) {
          ++tasks;
          ++active;
          for (auto actor : row.planner->members)
            if (c.world.creatures[actor].ai.behavior == 3 ||
                c.world.creatures[actor].ai.motion)
              orderedActors.insert(actor);
        }
      multiple += tasks > 1;
      if ((!journey && tasks && !sharedCredit) ||
          (journey && tasks > 1 && !separateCredit)) {
        auto charged = c;
        auto &rows = charged.native.ownerSources->planners;
        std::vector<std::pair<int, int>> before;
        for (const auto &row : rows) {
          if (row.planner && row.planner->grouped->replanning->previous) {
            const auto &t = *row.planner->grouped->replanning->previous;
            before.push_back({t.successes, t.attempts});
          } else
            before.push_back({-1, -1});
        }
        cs::creditNativeOwnerDamage(charged, 0, 1, 11);
        cs::creditNativeOwnerMovement(charged, 1, 7);
        for (unsigned i = 0; i < rows.size(); ++i)
          if (before[i].first >= 0) {
            const auto &planner = *rows[i].planner;
            const auto &t = *planner.grouped->replanning->previous;
            const auto &members = planner.members;
            const bool source =
                std::find(members.begin(), members.end(), 1) != members.end();
            const bool victim =
                std::find(members.begin(), members.end(), 0) != members.end();
            check(t.successes == before[i].first + int(source) * 11,
                  "nonleader source task reward once");
            check(t.attempts ==
                      before[i].second + int(source) * 7 + int(victim) * 11,
                  "source-specific victim and movement costs");
          }
        check(cs::encode(cs::decode(cs::encode(charged))) ==
                  cs::encode(charged),
              "credited task continuation");
        if (journey)
          separateCredit = true;
        else
          sharedCredit = true;
      }

      if (frame % 8 == 0 || (journey && (frame == 31 || frame == 32 ||
                                         frame == 95 || frame == 96))) {
        game::World world(0);
        world.restore(state);
        auto resumed =
            game::CreatureRuleSession(std::move(world), en::runtime(p));
        resumed.step({});
        auto direct = session;
        direct.step({});
        check(game::encodeSnapshot(resumed.world().state()) ==
                  game::encodeSnapshot(direct.world().state()),
              "source per-slice resumed execution");
        ++restores;
      }
      if (!journey && (frame == 0 || frame == 2 || frame == 63 ||
                       frame == 127 || frame == 255))
        game::writeSnapshot(output / (std::to_string(frame + 1) + ".snapshot"),
                            state);
      if (journey &&
          (frame == 32 || frame == 95 || frame == 96 || frame == 255))
        game::writeSnapshot(
            output / ("lifecycle-" + std::to_string(frame + 1) + ".snapshot"),
            state);
      for (auto byte : state.systems)
        digest = (digest ^ byte) * 1099511628211ull;
    }
  }
  check(sharedCredit && separateCredit,
        "shared and independent task accounting witnesses");
  check(active && multiple && split && merged, "source lifecycle witnesses");
  check(orderedActors.count(0) && orderedActors.count(1),
        "both friendly members execute orders");
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"frames\":512,\"restores\":" << restores
            << ",\"refusals\":" << refusals << ",\"active\":" << active
            << ",\"multiple_tasks\":" << multiple << ",\"splits\":" << split
            << ",\"merges\":" << merged << ",\"digest\":\"" << digest
            << "\"}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
