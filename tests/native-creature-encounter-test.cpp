#include "../apps/skirmish/health_rules.hpp"
#include "persistence/snapshot.hpp"
#include "profile.hpp"
#include <iostream>
#include <set>
namespace g = mnm::game;
namespace lab = mnm::creature_lab;
void check(bool b, const char *s) {
  if (!b)
    throw std::runtime_error(s);
}
g::CreatureEncounterSession clone(const g::CreatureEncounterSession &s,
                                  const lab::Profile &p) {
  auto state = g::decodeSnapshot(g::encodeSnapshot(s.world().state()));
  check(state.campaign == p.bytes, "clone profile changed");
  g::World w(0);
  w.restore(std::move(state));
  return {std::move(w), p.rules, mnm::skirmish::ordinaryHealthRules()};
}
int main(int argc, char **argv) try {
  check(argc == 2, "installation argument required");
  auto p = lab::loadProfile(argv[1]);
  unsigned ticks = 0, clones = 0, stuns = 0, knocks = 0, moves = 0, hits = 0,
           deaths = 0, retargets = 0;
  std::set<unsigned> facings;
  for (unsigned seed : {1U, 2U, 3U, 17U, 99U, 1000U, 0xffffffffU}) {
    auto s = lab::create(p, seed);
    auto before = g::encodeSnapshot(s.world().state());
    check(s.step({false, false, false}).empty() &&
              g::encodeSnapshot(s.world().state()) == before,
          "unadmitted tick changed state");
    auto alternate = clone(s, p);
    alternate.step({true, true, false});
    check(alternate.world().state().systems == s.world().state().systems,
          "alternate tick changed AI control");
    for (unsigned t = 0; t < 8192 && !s.finished(); ++t) {
      auto resumed = clone(s, p);
      auto events = s.step();
      resumed.step();
      check(g::encodeSnapshot(resumed.world().state()) ==
                g::encodeSnapshot(s.world().state()),
            "one-tick checkpoint continuation diverged");
      ++clones;
      ++ticks;
      for (const auto &e : events) {
        if (e.kind == "stunned")
          ++stuns;
        if (e.kind == "knockback")
          ++knocks;
        if (e.kind == "move")
          ++moves;
        if (e.kind == "hit")
          ++hits;
        if (e.kind == "died")
          ++deaths;
        if (e.kind == "target" && t > 20)
          ++retargets;
      }
      for (const auto &a : s.state().actors)
        if (a.phase == g::CreaturePhase::attacking)
          facings.insert(a.facing);
    }
    check(s.finished(), "autonomous encounter failed to finish");
    std::cerr << "seed " << seed << " tick " << s.world().state().tick
              << " quiet " << s.state().quietTicks << "\n";
    auto terminal = g::encodeSnapshot(s.world().state());
    check(s.step().empty() && g::encodeSnapshot(s.world().state()) == terminal,
          "terminal tick advanced");
  }
  // Damaged-enemy fixture leaves the attacker healthy enough to acquire the
  // second enemy after a lethal hit. No installed balance values are changed.
  {
    auto scenario = lab::create(p, 11);
    auto w = scenario.world().state();
    auto v = scenario.state();
    v.actors[1].health = 1;
    w.slots[0].entity->x = 4;
    v.actors[0].home = {6, 6, 1};
    v.actors[0].phase = g::CreaturePhase::attacking;
    v.actors[0].facing = 2;
    v.actors[0].cursor = p.rules->start(false, 2);
    v.actors[1].phase = g::CreaturePhase::stunned;
    v.actors[1].facing = 6;
    v.actors[1].stunTimer = 39;
    v.actors[1].cursor = p.rules->start(true, 6);
    v.actors[0].target = 1;
    w.slots[0].entity->target = v.actors[1].handle;
    w.systems = g::encodeCreatureEncounter(v);
    g::World world(0);
    world.restore(w);
    g::CreatureEncounterSession fixture(std::move(world), p.rules,
                                        mnm::skirmish::ordinaryHealthRules());
    bool killed = false, reacquired = false;
    for (unsigned t = 0; t < 2048 && !fixture.finished(); ++t) {
      auto restored = clone(fixture, p);
      auto events = fixture.step();
      restored.step();
      check(g::encodeSnapshot(restored.world().state()) ==
                g::encodeSnapshot(fixture.world().state()),
            "retarget continuation diverged");
      ++clones;
      ++ticks;
      for (const auto &e : events) {
        if (e.kind == "died" && e.actor == 1)
          killed = true;
        if (killed && e.kind == "target" && e.actor == 0 && e.target == 3) {
          reacquired = true;
          ++retargets;
        }
      }
    }
    check(killed && reacquired,
          "lethal target was not replaced by the surviving enemy");
  }
  auto s = lab::create(p);
  auto base = s.world().state();
  unsigned rejections = 0;
  for (unsigned fault = 0; fault < 8; ++fault) {
    auto bad = base;
    auto v = s.state();
    switch (fault) {
    case 0:
      v.actors[0].facing = 8;
      break;
    case 1:
      v.actors[0].health = 9999;
      break;
    case 2:
      v.actors[0].target = 0;
      break;
    case 3:
      v.actors[0].cursor = {1};
      break;
    case 4:
      v.random[4] = 0;
      break;
    case 5:
      v.walls[0] = 2;
      break;
    case 6:
      v.actors[0].stunTimer = 100;
      break;
    case 7:
      bad.slots[0].entity->x = bad.slots[1].entity->x;
      break;
    }
    bad.systems = g::encodeCreatureEncounter(v);
    try {
      g::World w(0);
      w.restore(bad);
      g::CreatureEncounterSession invalid(std::move(w), p.rules,
                                          mnm::skirmish::ordinaryHealthRules());
      throw std::runtime_error("corrupt creature continuation accepted");
    } catch (const std::invalid_argument &) {
      ++rejections;
    }
    check(g::encodeSnapshot(s.world().state()) == g::encodeSnapshot(base),
          "invalid restore changed source session");
  }
  std::cerr << "branches " << stuns << " stun " << knocks << " knock " << moves
            << " move " << hits << " hit " << deaths << " dead " << retargets
            << " retarget " << facings.size() << " facing\n";
  check(stuns && knocks && moves && hits && deaths && retargets &&
            facings.size() > 1,
        "encounter missed control branches");
  std::cout << "{\"success\":true,\"ticks\":" << ticks
            << ",\"checkpoint_clones\":" << clones << ",\"stuns\":" << stuns
            << ",\"knockbacks\":" << knocks << ",\"moves\":" << moves
            << ",\"hits\":" << hits << ",\"deaths\":" << deaths
            << ",\"retargets\":" << retargets
            << ",\"facings\":" << facings.size()
            << ",\"semantic_rejections\":" << rejections << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
