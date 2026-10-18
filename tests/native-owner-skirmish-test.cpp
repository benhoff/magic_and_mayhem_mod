#include "../apps/command-session/native_owner_sources.hpp"
#include "../apps/native-encounter/encounter.hpp"
#include "../game/persistence/snapshot.hpp"
#include "owner-task-trace-wire.hpp"
#include <fstream>
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
  unsigned checks = 0, restores = 0, deaths = 0, pending = 0, decisions = 0,
           executions = 0, retired = 0, fallbacks = 0, frames = 0, refusals = 0,
           rollbacks = 0, taskCalls = 0;
  std::vector<std::array<unsigned, 4>> journeys;
  std::uint64_t digest = 1469598103934665603ull;
  auto check = [&](bool b, const char *why) {
    if (!b)
      throw std::runtime_error(why);
    ++checks;
  };
  auto reject = [&](auto operation) {
    bool refused = false;
    try {
      operation();
    } catch (const std::exception &) {
      refused = true;
    }
    check(refused, "expected skirmish admission refusal");
    ++refusals;
  };
  for (unsigned seed : {17u, 31u}) {
    auto p = en::decodeInputs(en::loadInputs(argv[1], {0, 0, 32, 32}, 9));
    en::enableRegionalNavigation(p);
    en::enableOwnerPlanner(p, std::filesystem::path(argv[1]) / "AI/Brain.dat",
                           seed);
    check(p.initial.native.ownerSources->driver.has_value(),
          "skirmish driver absent");
    check(cs::encode(p.initial).at(4) == 19, "skirmish CMD1 version");
    reject([&] { (void)cs::encode(p.initial, 18); });
    auto bad = p.initial;
    bad.native.ownerSources->driver->phase = 9;
    reject([&] { (void)cs::encode(bad); });
    bad = p.initial;
    bad.native.ownerSources->driver->clock = 1;
    reject([&] { (void)cs::encode(bad); });
    bad = p.initial;
    bad.native.ownerSources->worldAllocation = 4097;
    reject([&] { (void)cs::encode(bad); });
    bad = p.initial;
    bad.native.ownerSources->retirements = bad.native.ownerSources->nextSerial;
    reject([&] { (void)cs::encode(bad); });
    bad = p.initial;
    for (auto &row : bad.native.ownerSources->planners)
      if (row.planner) {
        row.planner->selection.weighted = true;
        break;
      }
    reject([&] { (void)cs::encode(bad); });
    bad = p.initial;
    for (auto &row : bad.native.ownerSources->planners)
      if (row.planner) {
        auto &value = row.planner->model.layers[0].outside;
        value = value == 0 ? 1 : 0;
        break;
      }
    reject([&] { (void)cs::encode(bad); });
    std::ofstream calls(output / (std::to_string(seed) + ".tasktrace"),
                        std::ios::binary);
    calls.write("MNMOT001", 8);
    bool collecting = true, injectFault = false, visited = false,
         rollbackChecked = false;
    cs::NativeOwnerTaskVerifier verifier =
        [&](const cs::NativeOwnerTaskExecution &e) {
          visited = true;
          if (injectFault)
            throw std::runtime_error("injected owner task verifier failure");
          if (collecting) {
            ++taskCalls;
            mnm::test::owner_trace::append(
                calls, {e.before, e.after, e.knowledge, e.world, e.members,
                        e.raw, e.net, e.threat, e.tick, e.owner, e.percent,
                        e.initialBudget, e.finalBudget, e.result});
          }
        };
    auto session = en::create(p, seed, verifier);
    std::ofstream trace(output / (std::to_string(seed) + ".jsonl"));
    game::writeSnapshot(output / (std::to_string(seed) + "-0.snapshot"),
                        session.world().state());
    unsigned deadBefore = 0, wonAt = 0;
    for (unsigned frame = 1; frame <= 4096; ++frame) {
      try {
        session.step({});
      } catch (const std::exception &e) {
        throw std::runtime_error("seed=" + std::to_string(seed) + " frame=" +
                                 std::to_string(frame) + ": " + e.what());
      }
      const auto state = session.world().state();
      const auto c = cs::decode(state.systems);
      const auto &s = *c.native.ownerSources;
      std::array<unsigned, 2> living{};
      unsigned dead = 0;
      trace << "{\"frame\":" << frame << ",\"clock\":" << s.driver->clock
            << ",\"phase\":" << s.driver->phase << ",\"health\":[";
      for (unsigned i = 0; i < c.world.creatures.size(); ++i) {
        const auto &a = c.world.creatures[i].ai;
        if (a.health)
          ++living.at(unsigned(a.owner));
        else
          ++dead;
        trace << (i ? "," : "") << a.health;
      }
      trace << "],\"decisions\":" << s.decisions
            << ",\"executions\":" << s.executions << ",\"tasks\":[";
      bool comma = false;
      std::set<unsigned> seen;
      for (const auto &row : s.planners)
        if (row.planner) {
          const auto &v = *row.planner;
          check(v.members.size() <= 3, "oversized source");
          for (auto actor : v.members)
            check(c.world.creatures.at(actor).ai.owner == int(v.owner),
                  "foreign owner merge");
          if (v.activeSerial) {
            check(seen.insert(v.activeSerial).second, "duplicate task serial");
            const auto &t = *v.grouped->replanning->previous;
            trace << (comma ? "," : "") << "[" << row.source << ","
                  << v.activeSerial << ","
                  << int(v.grouped->fallbackActive ? 0 : 6) << "," << t.started
                  << "," << t.expires << "," << t.successes << "," << t.attempts
                  << "]";
            comma = true;
          }
        }
      trace << "]}\n";
      pending += s.search.has_value();
      check(unsigned(s.driver->clock) == c.native.tick,
            "skirmish clock differs");
      check(cs::encode(c) == state.systems, "skirmish roundtrip differs");
      if (frame <= 8 || frame % 128 == 0 || dead > deadBefore) {
        collecting = false;
        game::World world(0);
        world.restore(state);
        game::CreatureRuleSession resumed(std::move(world), en::runtime(p));
        auto expected = session;
        expected.step({});
        resumed.step({});
        check(expected.world().state().systems ==
                  resumed.world().state().systems,
              "skirmish sliced resume differs");
        ++restores;
        collecting = true;
      }
      if (!rollbackChecked && frame <= 32) {
        collecting = false;
        visited = false;
        auto probe = session;
        probe.step({});
        if (visited) {
          injectFault = true;
          bool failed = false;
          try {
            session.step({});
          } catch (const std::exception &e) {
            if (std::string(e.what()).find(
                    "injected owner task verifier failure") ==
                std::string::npos)
              throw;
            failed = true;
          }
          injectFault = false;
          check(failed, "task verifier did not refuse frame");
          check(game::encodeSnapshot(session.world().state()) ==
                    game::encodeSnapshot(state),
                "failed verifier did not roll back complete encounter");
          ++rollbacks;
          rollbackChecked = true;
        }
        collecting = true;
      }
      if (frame == 1 || frame == 3 || frame == 16 || frame == 64 ||
          frame == 256 || frame == 1024 || dead > deadBefore)
        game::writeSnapshot(output / (std::to_string(seed) + "-" +
                                      std::to_string(frame) + ".snapshot"),
                            state);
      if (dead > deadBefore) {
        deaths += dead - deadBefore;
        deadBefore = dead;
      }
      for (auto b : state.systems)
        digest = (digest ^ b) * 1099511628211ull;
      if (!wonAt && (!living[0] || !living[1]))
        wonAt = frame;
      if (frame % 256 == 0)
        std::cerr << "seed=" << seed << " frame=" << frame
                  << " living=" << living[0] << ":" << living[1]
                  << " decisions=" << s.decisions << " tasks=" << seen.size()
                  << "\n";
      if (wonAt && frame >= wonAt + 32) {
        decisions += s.decisions;
        executions += s.executions;
        retired += s.retirements;
        fallbacks += s.fallbacks;
        frames += frame;
        check(rollbackChecked, "battle did not exercise verifier rollback");
        journeys.push_back({seed, frame, wonAt, deadBefore});
        game::writeSnapshot(output / (std::to_string(seed) + "-end.snapshot"),
                            state);
        break;
      }
      check(frame < 4096, "autonomous battle did not finish");
    }
  }
  check(deaths >= 6 && pending && decisions && executions && retired &&
            fallbacks,
        "skirmish lifecycle witnesses missing");
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"frames\":" << frames << ",\"restores\":" << restores
            << ",\"deaths\":" << deaths << ",\"pending\":" << pending
            << ",\"decisions\":" << decisions
            << ",\"executions\":" << executions
            << ",\"retirements\":" << retired << ",\"fallbacks\":" << fallbacks
            << ",\"refusals\":" << refusals << ",\"rollbacks\":" << rollbacks
            << ",\"task_calls\":" << taskCalls << ",\"digest\":\"" << digest
            << "\",\"journeys\":[";
  for (unsigned i = 0; i < journeys.size(); ++i) {
    auto row = journeys[i];
    std::cout << (i ? "," : "") << "{\"seed\":" << row[0]
              << ",\"frames\":" << row[1] << ",\"won_at\":" << row[2]
              << ",\"deaths\":" << row[3] << "}";
  }
  std::cout << "]}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
