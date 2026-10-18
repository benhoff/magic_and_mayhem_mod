#include "../apps/command-session/native_owner_sources.hpp"
#include "../apps/command-session/native_regional.hpp"
#include "../apps/native-encounter/encounter.hpp"
#include "../game/persistence/snapshot.hpp"
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
  unsigned checks = 0, restores = 0, refusals = 0, pending = 0,
           secondMember = 0, decisions = 0, splits = 0, merges = 0, active = 0;
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
    check(failed, "expected automatic source refusal");
    ++refusals;
  };
  std::uint64_t digest = 1469598103934665603ull;
  for (unsigned journey = 0; journey < 3; ++journey) {
    auto p = en::decodeInputs(en::loadInputs(argv[1], {0, 0, 32, 32}, 8));
    en::enableRegionalNavigation(p);
    en::enableOwnerPlanner(p, std::filesystem::path(argv[1]) / "AI/Brain.dat",
                           17);
    if (journey == 1)
      cs::splitNativeOwnerSource(p.initial, 2, {0});
    cs::enableAutomaticOwnerSources(p.initial);
    if (journey == 2) {
      p.initial.native.ownerSources->planningAllocation = 1;
      auto &c = p.initial;
      for (auto &cell : c.native.cells)
        cell.occupant = 0xffff;
      cs::NativeCreatureGeometry geometry(c, p.native);
      auto &member = c.world.creatures[1];
      int best = -1, distance = -1;
      const auto origin = c.world.creatures[0].ai.position;
      for (unsigned i = 0; i < c.native.regions.size(); ++i) {
        const auto point = c.native.regions[i].position;
        if (!c.native.regions[i].parameter ||
            int(i) == c.world.creatures[0].region ||
            !geometry.legal(member, point) || !geometry.ground(member, point))
          continue;
        int dx = std::abs(point.x - origin.x),
            dy = std::abs(point.y - origin.y);
        dx = std::min(dx, c.world.dimensions.x - dx);
        dy = std::min(dy, c.world.dimensions.y - dy);
        if (dx * dx + dy * dy > distance) {
          distance = dx * dx + dy * dy;
          best = int(i);
        }
      }
      check(best >= 0 && distance >= 64, "independent member region placement");
      member.ai.position = c.native.regions[unsigned(best)].position;
      member.region = best;
      member.ai.home = member.destination = member.ai.position;
      c.execution[1].roamHome = member.ai.position;
      cs::prepareNativeContinuation(c, p.native);
    }
    auto initial = cs::encode(p.initial);
    check(initial[4] == 18, "CMD1v18 automatic source state");
    check(cs::encode(cs::decode(initial)) == initial,
          "automatic source initial roundtrip");
    reject([&] { (void)cs::encode(p.initial, 17); });
    auto session = en::create(p, 17);
    const auto prefix = journey == 2 ? "sliced-"
                        : journey    ? "split-"
                                     : "group-";
    game::writeSnapshot(output / (std::string(prefix) + "0.snapshot"),
                        session.world().state());
    for (unsigned frame = 1; frame <= 256; ++frame) {
      session.step({});
      auto state = session.world().state();
      const auto c = cs::decode(state.systems);
      const auto &s = *c.native.ownerSources;
      check(s.automatic, "automatic source state lost");
      check(cs::encode(c) == state.systems, "automatic state exact roundtrip");
      if (s.search) {
        ++pending;
        secondMember += s.search->scratch.grouped->memberCursor > 0;
        if (pending == 1) {
          auto bad = c;
          bad.native.ownerSources->search->decision.stage = 9;
          reject([&] { (void)cs::encode(bad); });
          bad = c;
          bad.native.ownerSources->search->scratch.grouped->memberCursor = 9;
          reject([&] { (void)cs::encode(bad); });
          bad = c;
          bad.native.ownerSources->search->inputs.members = {2};
          reject([&] { (void)cs::encode(bad); });
          bad = c;
          bad.native.ownerSources->automatic = false;
          reject([&] { (void)cs::encode(bad); });
        }
      }
      for (const auto &row : s.planners)
        if (row.planner && row.planner->active) {
          ++active;
          check(row.planner->grouped->perMember,
                "automatic task uses member paths");
        }
      if (frame <= 8 || frame % 16 == 0) {
        game::World restored(0);
        restored.restore(state);
        game::CreatureRuleSession resumed(std::move(restored), en::runtime(p));
        auto expected = session;
        expected.step({});
        resumed.step({});
        check(expected.world().state().systems ==
                  resumed.world().state().systems,
              "automatic sliced resume");
        ++restores;
      }
      if (frame == 1 || frame == 3 || frame == 16 || frame == 64 ||
          frame == 128 || frame == 256)
        game::writeSnapshot(output / (std::string(prefix) +
                                      std::to_string(frame) + ".snapshot"),
                            state);
      for (auto byte : state.systems)
        digest = (digest ^ byte) * 1099511628211ull;
      if (frame == 256) {
        decisions += s.decisions;
        splits += s.splits;
        merges += s.merges;
      }
    }
  }
  check(decisions && pending && active && secondMember && splits && merges,
        ("automatic decision/task witnesses decisions=" +
         std::to_string(decisions) + " pending=" + std::to_string(pending) +
         " second=" + std::to_string(secondMember) + " splits=" +
         std::to_string(splits) + " merges=" + std::to_string(merges))
            .c_str());
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"restores\":" << restores << ",\"refusals\":" << refusals
            << ",\"pending\":" << pending
            << ",\"second_member\":" << secondMember
            << ",\"decisions\":" << decisions << ",\"splits\":" << splits
            << ",\"merges\":" << merges << ",\"active\":" << active
            << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
