#include "../apps/command-session/native_geometry.hpp"
#include "../apps/native-encounter/encounter.hpp"
#include "../game/persistence/snapshot.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
namespace {
using namespace mnm;
void need(bool v, const char *s) {
  if (!v)
    throw std::runtime_error(s);
}
game::Bytes snapshot(const game::CreatureRuleSession &s) {
  return game::encodeSnapshot(s.world().state());
}
void word(std::ostream &out, unsigned n) {
  for (unsigned i = 0; i < 4; ++i)
    out.put(char(n >> (8 * i)));
}
void blob(std::ostream &out, const game::Bytes &b) {
  word(out, unsigned(b.size()));
  out.write(reinterpret_cast<const char *>(b.data()),
            std::streamsize(b.size()));
}
template <class F> void refuses(F fn) {
  bool failed = false;
  try {
    fn();
  } catch (const std::exception &) {
    failed = true;
  }
  need(failed, "expected encounter refusal");
}
} // namespace
int main(int argc, char **argv) try {
  using namespace mnm;
  need(argc == 2 || argc == 3,
       "expected installation and optional new trace file");
  auto p =
      encounter::decodeInputs(encounter::loadInputs(argv[1], {0, 0, 12, 12}));
  need(p.maximum == 360 && p.melee.base == 18 && p.melee.stunThreshold == 9,
       "installed combat configuration changed");
  need(p.native.types[17].width == 2 && p.native.types[17].height == 2 &&
           p.native.types[17].movementKind == 3 &&
           p.native.types[17].scalar.type_10 == 10,
       "installed movement configuration changed");
  std::ofstream trace;
  if (argc == 3) {
    need(!std::filesystem::exists(argv[2]), "trace destination exists");
    trace.open(argv[2], std::ios::binary);
    trace.write("MNMETR01", 8);
    blob(trace, p.bytes);
    word(trace, 1024);
  }
  unsigned restored = 0, hits = 0, stuns = 0, knockbacks = 0, deaths = 0,
           movement = 0, acquisition = 0, removed = 0;
  std::uint64_t digest = 1469598103934665603ull;
  for (unsigned seed : {17u, 42u, 91u}) {
    auto live = encounter::create(p, seed);
    auto paused = snapshot(live);
    live.step({false, false, false});
    need(snapshot(live) == paused, "paused encounter changed");
    for (unsigned tick = 0; tick < 1024; ++tick) {
      auto before = command_session::decode(live.world().state().systems);
      game::World w(0);
      w.restore(game::decodeSnapshot(snapshot(live)));
      game::CreatureRuleSession resumed(std::move(w), encounter::runtime(p));
      live.step();
      resumed.step();
      need(snapshot(live) == snapshot(resumed),
           "encounter one-tick resume diverged");
      ++restored;
      auto after = command_session::decode(live.world().state().systems);
      for (unsigned i = 0; i < 2; ++i) {
        movement += before.world.creatures[i].finePosition ==
                            after.world.creatures[i].finePosition
                        ? 0
                        : 1;
        acquisition += before.world.creatures[i].ai.target < 0 &&
                       after.world.creatures[i].ai.target >= 0;
      }
      if (seed == 17 && trace) {
        blob(trace, command_session::encode(before));
        blob(trace, command_session::encode(after));
      }
    }
    auto c = command_session::decode(live.world().state().systems);
    auto stats = encounter::statistics(c);
    hits += stats.hits;
    stuns += stats.stuns;
    knockbacks += stats.knockbacks;
    deaths += stats.deaths;
    need(stats.hits && stats.deaths <= 1, "encounter fight/lifecycle bounds");
    for (const auto &a : c.world.creatures) {
      need(a.ai.health <= 360, "encounter health exceeded installed maximum");
      if (!a.ai.health) {
        need(!a.ai.active, "death animation did not remove actor");
        ++removed;
        for (const auto &cell : c.native.cells)
          need(cell.occupant != a.ai.slot + 1, "dead actor retained occupancy");
      }
    }
    for (auto b : snapshot(live))
      digest = (digest ^ b) * 1099511628211ull;
  }
  need(stuns && knockbacks && movement && acquisition && deaths > 0 &&
           removed == deaths,
       "encounter branch corpus incomplete");
  if (trace) {
    trace.close();
    need(bool(trace), "trace write failed");
  }
  auto fault = std::make_shared<bool>(true);
  auto lower = encounter::consequences(p);
  auto bad = std::make_shared<command_session::Runtime>(
      p.initial, p.bytes,
      command_session::nativeProviders(p.native, [fault, lower](auto &c) {
        auto f = lower(c);
        auto hit = f.callbacks.hit;
        f.callbacks.hit = [fault, hit](auto &a) {
          hit(a);
          if (*fault)
            throw std::runtime_error("injected staged combat failure");
        };
        return f;
      }));
  game::CreatureRuleSession test(bad, p.bytes, 17, 2);
  auto control = encounter::create(p, 17);
  bool checked = false;
  for (unsigned tick = 0; tick < 200 && !checked; ++tick) {
    auto old = snapshot(test);
    try {
      test.step();
      control.step();
      need(snapshot(test) == snapshot(control), "rollback lead-in diverged");
    } catch (const std::runtime_error &e) {
      need(std::string(e.what()) == "injected staged combat failure",
           "unexpected encounter failure");
      need(snapshot(test) == old,
           "failed health/control callback published state");
      *fault = false;
      test.step();
      control.step();
      need(snapshot(test) == snapshot(control), "combat retry diverged");
      checked = true;
    }
  }
  need(checked, "combat rollback injection was not reached");
  auto changed = p;
  changed.bytes.back() ^= 1;
  auto incompatible = std::make_shared<command_session::Runtime>(
      changed.initial, changed.bytes,
      command_session::nativeProviders(changed.native,
                                       encounter::consequences(changed)));
  auto prior = snapshot(test);
  refuses([&] { incompatible->validate(test.world().state()); });
  need(snapshot(test) == prior, "incompatible profile admission changed state");
  auto corrupted = p.bytes;
  corrupted[0] ^= 1;
  refuses([&] { encounter::decodeInputs(corrupted); });
  std::cout << "{\"success\":true,\"restored_ticks\":" << restored
            << ",\"hits\":" << hits << ",\"stuns\":" << stuns
            << ",\"knockbacks\":" << knockbacks << ",\"deaths\":" << deaths
            << ",\"movement_samples\":" << movement
            << ",\"acquisitions\":" << acquisition << ",\"removed\":" << removed
            << ",\"combat_rollback\":true,\"profile_refusal\":true,\"digest\":"
            << digest << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
