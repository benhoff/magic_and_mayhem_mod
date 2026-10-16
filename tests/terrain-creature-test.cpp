#include "../apps/creature-lab/profile.hpp"
#include "creature_runtime.hpp"
#include "health_rules.hpp"
#include "persistence/snapshot.hpp"
#include "simulation/skirmish.hpp"
#include <iostream>
#include <stdexcept>
namespace g = mnm::game;
namespace s = mnm::skirmish;
void check(bool b, const char *m) {
  if (!b)
    throw std::runtime_error(m);
}
g::Entity entity(g::Point p, unsigned owner) {
  g::Entity e;
  e.type = 17;
  e.owner = owner;
  e.x = p.x;
  e.y = p.y;
  e.z = p.z;
  return e;
}
int main(int argc, char **argv) try {
  check(argc == 2, "usage: terrain-creature-test INSTALLATION");
  auto driver = s::installedTerrainCreatureRuntime(argv[1]);
  auto terrain = *s::creatureTerrain(*driver);
  auto profile = mnm::creature_lab::loadProfile(argv[1]);
  s::validateTerrainCreatureProfile(terrain, profile.bytes);
  auto mismatch = profile.bytes;
  mismatch.back() ^= 1;
  bool profileRefused = false;
  try {
    s::validateTerrainCreatureProfile(terrain, mismatch);
  } catch (const std::invalid_argument &) {
    profileRefused = true;
  }
  check(profileRefused, "mixed terrain/melee inputs accepted");
  g::World world(32);
  auto w = world.state();
  w.map = s::terrainBattleMap;
  w.navigation = terrain.navigation->binding();
  world.restore(w);
  g::CombatSession battle(std::move(world), terrain.navigation,
                          s::ordinaryHealthRules(), driver);
  g::SkirmishState meta;
  for (unsigned owner = 0; owner < 2; ++owner) {
    auto h = battle.spawn(entity(terrain.wizards[owner], owner),
                          {g::SkirmishRules::wizardHealth, 1, 2, 3}, false,
                          g::CombatControl::hold);
    meta.wizards[owner].actor = h;
    meta.units.push_back({h, g::UnitRole::wizard});
  }
  auto friendly = battle.spawn(entity({7, 1, int(terrain.plane)}, 0),
                               driver->creatureStats());
  meta.units.push_back({friendly, g::UnitRole::sentinel});
  auto enemy = battle.spawn(entity({8, 4, int(terrain.plane)}, 1),
                            driver->creatureStats());
  meta.units.push_back({enemy, g::UnitRole::sentinel});
  w = battle.world().state();
  w.campaign = g::encodeSkirmish(meta);
  g::World enrolled(0);
  enrolled.restore(w);
  battle = g::CombatSession(std::move(enrolled), terrain.navigation,
                            s::ordinaryHealthRules(), driver);
  battle.enqueue({g::CombatOrderKind::move,
                  friendly,
                  {},
                  g::Point{8, 2, int(terrain.plane)}});
  unsigned hits = 0, moves = 0, interruptions = 0;
  for (unsigned tick = 0; tick < 100; ++tick) {
    auto before = battle.world().state();
    auto report = battle.step();
    for (auto e : report.events)
      if (e.kind == g::CombatEventKind::hit)
        ++hits;
    for (auto h : {friendly, enemy}) {
      auto a = before.slots[h.slot].entity.value();
      auto b = *battle.world().find(h);
      if (a.x != b.x || a.y != b.y)
        ++moves;
    }
    for (auto a : s::creatureBattleViews(battle))
      if (a.phase == g::CreaturePhase::stunned ||
          a.phase == g::CreaturePhase::knockback)
        ++interruptions;
    auto snapshot = battle.world().state();
    auto rebound = s::resolveCreatureRuntime(snapshot);
    auto nav = s::creatureTerrain(*rebound)->navigation;
    g::World fresh(0);
    fresh.restore(snapshot);
    g::CombatSession resumed(std::move(fresh), nav, s::ordinaryHealthRules(),
                             rebound);
    auto direct = battle;
    direct.step();
    resumed.step();
    check(g::encodeSnapshot(direct.world().state()) ==
              g::encodeSnapshot(resumed.world().state()),
          "terrain creature RNG/ANI/movement continuation");
  }
  check(hits > 0 && moves > 0, "terrain pursuit and melee not exercised");
  auto saved = g::encodeSnapshot(battle.world().state());
  bool refused = false;
  try {
    g::World wrong(0);
    wrong.restore(battle.world().state());
    g::CombatSession bad(std::move(wrong), terrain.navigation,
                         s::ordinaryHealthRules(),
                         s::installedCreatureRuntime(argv[1]));
  } catch (const std::invalid_argument &) {
    refused = true;
  }
  check(refused && saved == g::encodeSnapshot(battle.world().state()),
        "wrong terrain driver rollback");
  std::cout << "{\"success\":true,\"melee_hits\":" << hits
            << ",\"moves\":" << moves << ",\"interruptions\":" << interruptions
            << ",\"continuations\":100,\"missing_terrain_refused\":true,"
               "\"mismatched_profile_refused\":true}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
