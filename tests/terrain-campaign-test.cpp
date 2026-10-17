#include "creature_runtime.hpp"
#include "cure_runtime.hpp"
#include "health_rules.hpp"
#include "map_campaign.hpp"
#include "persistence/snapshot.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
namespace g = mnm::game;
namespace app = mnm::skirmish;
void check(bool b, const char *m) {
  if (!b)
    throw std::runtime_error(m);
}
g::Bytes bytes(const g::CampaignSession &s) {
  return g::encodeCampaignSnapshot(s.checkpoint(), s.progress().identity);
}
g::Entity actor(g::Point p, unsigned owner) {
  g::Entity e;
  e.type = 17;
  e.owner = owner;
  e.x = p.x;
  e.y = p.y;
  e.z = p.z;
  return e;
}
g::CampaignSession seed(const std::filesystem::path &root,
                        bool pursuit = false) {
  auto resources = app::mapCampaignResources(root, true, true, true);
  auto campaign = app::newMapCampaign(resources);
  campaign.startMission();
  auto combat = campaign.battle()->combat();
  auto meta = campaign.battle()->state();
  auto terrain = *app::creatureTerrain(*combat.driver());
  auto friendly = combat.spawn(actor({8, 2, int(terrain.plane)}, 0),
                               combat.driver()->creatureStats());
  auto enemy = combat.spawn(actor({8, pursuit ? 6 : 4, int(terrain.plane)}, 1),
                            combat.driver()->creatureStats());
  meta.units.push_back({friendly, g::UnitRole::sentinel});
  meta.units.push_back({enemy, g::UnitRole::sentinel});
  meta.wizards[1].mana = 0;
  meta.wizards[1].regeneration = 0;
  meta.aiWait = 8;
  combat.damage(friendly, 40, meta.wizards[0].actor);
  auto checkpoint = campaign.checkpoint();
  checkpoint.world = combat.world().state();
  checkpoint.world.campaign = g::encodeSkirmish(meta);
  campaign.restore(checkpoint);
  return campaign;
}
struct Events {
  unsigned melee = 0, heals = 0, healed = 0, spheres = 0, damage = 0;
  bool moved = false, pursued = false, playerMoved = false;
};
Events advance(g::CampaignSession &s, unsigned ticks) {
  Events out;
  const auto phase = [&] {
    for (auto v : app::creatureBattleViews(s.battle()->combat()))
      if (v.actor == g::Handle{3, 1})
        return v.phase;
    return g::CreaturePhase::idle;
  };
  for (unsigned i = 0; i < ticks; ++i) {
    auto before = *s.battle()->world().find({3, 1}),
         playerBefore = *s.battle()->world().find({2, 1});
    const auto previousPhase = phase();
    auto r = s.step();
    check(r.battle.has_value() && s.battle(), "terrain campaign ended early");
    const auto after = *s.battle()->world().find({3, 1}),
               playerAfter = *s.battle()->world().find({2, 1});
    const bool moved = before.x != after.x || before.y != after.y;
    out.moved |= moved;
    out.pursued |= moved && (previousPhase == g::CreaturePhase::pursuing ||
                             phase() == g::CreaturePhase::pursuing);
    out.playerMoved |=
        playerBefore.x != playerAfter.x || playerBefore.y != playerAfter.y;
    for (auto e : r.battle->combat.events) {
      if (e.kind == g::CombatEventKind::hit && e.actor.slot >= 2)
        ++out.melee;
      if (e.kind == g::CombatEventKind::healed) {
        ++out.heals;
        out.healed += e.amount;
      }
      if (e.kind == g::CombatEventKind::hit && e.actor == g::Handle{0, 1} &&
          e.target == g::Handle{3, 1}) {
        ++out.spheres;
        out.damage += e.amount;
      }
    }
  }
  return out;
}
void print(Events e, bool contract = false) {
  std::cout << "{\"success\":true,\"melee_hits\":" << e.melee
            << ",\"cure_heals\":" << e.heals << ",\"healed\":" << e.healed
            << ",\"sphere_impacts\":" << e.spheres
            << ",\"sphere_damage\":" << e.damage
            << ",\"enemy_moved\":" << (e.moved ? "true" : "false")
            << ",\"enemy_pursued\":" << (e.pursued ? "true" : "false")
            << ",\"player_moved\":" << (e.playerMoved ? "true" : "false");
  if (contract)
    std::cout << ",\"campaign_continuations\":20,\"terrain_retired\":true,"
                 "\"bad_footprint_refused\":true,\"next_retry\":true";
  std::cout << "}\n";
}
int main(int argc, char **argv) try {
  if (argc == 4 && (std::string(argv[1]) == "seed" ||
                    std::string(argv[1]) == "seed-pursuit")) {
    auto s = seed(argv[2], std::string(argv[1]) == "seed-pursuit");
    check(s.save(argv[3]).durable, "terrain seed publication");
    const auto enemy = *s.battle()->world().find({3, 1});
    std::cout
        << "{\"success\":true,\"plane\":7,\"friendly\":[8,2,7],\"enemy\":["
        << enemy.x << ',' << enemy.y << ',' << enemy.z
        << "],\"wizard_secondary\":[6,2,7]}\n";
    return 0;
  }
  if (argc == 6 && std::string(argv[1]) == "resume") {
    auto s = app::loadMapCampaign(std::filesystem::path(argv[2]), argv[3]);
    auto count = std::stoul(argv[5]);
    check(count <= 128 && s.battle(), "terrain resume bounds");
    auto e = advance(s, count);
    check(s.save(argv[4]).durable, "terrain resume publication");
    print(e);
    return 0;
  }
  check(argc == 2, "usage: terrain-campaign-test INSTALLATION");
  auto resources = app::mapCampaignResources(argv[1], true, true, true);
  auto s = seed(argv[1]);
  const auto meta = s.battle()->state();
  s.enqueue(
      {g::SkirmishOrderKind::cure, meta.wizards[0].actor, g::Handle{2, 1}, {}});
  s.enqueue({g::SkirmishOrderKind::sphere,
             meta.wizards[0].actor,
             g::Handle{3, 1},
             {}});
  Events total;
  for (unsigned i = 0; i < 20; ++i) {
    auto checkpoint = s.checkpoint();
    auto resumed = app::newMapCampaign(resources);
    resumed.restore(checkpoint);
    auto r = advance(s, 1);
    advance(resumed, 1);
    check(bytes(s) == bytes(resumed),
          "terrain campaign fresh continuation differs");
    total.melee += r.melee;
    total.heals += r.heals;
    total.healed += r.healed;
    total.spheres += r.spheres;
    total.damage += r.damage;
    total.moved |= r.moved;
    total.pursued |= r.pursued;
    total.playerMoved |= r.playerMoved;
  }
  check(total.melee > 0 && total.heals == 1 && total.healed == 8 &&
            total.spheres == 1 && total.damage == 20,
        "terrain mixed combat/spell events");
  check(app::sphereViews(s.battle()->combat()).empty(),
        "terrain projectile did not retire before lifetime");
  auto checkpoint = s.checkpoint();
  auto original = bytes(s);
  auto bad = checkpoint;
  bad.world.navigation->fingerprint ^= 1;
  bool refused = false;
  try {
    s.restore(bad);
  } catch (const std::exception &) {
    refused = true;
  }
  check(refused && bytes(s) == original,
        "terrain campaign fingerprint refusal rollback");
  // The source profile binds dimensions and projectile footprint as well as map
  // bytes.
  bad = checkpoint;
  auto sections = g::decodeBattleSystems(bad.world.systems);
  const g::Bytes marker{'S', 'H', 'P', '3'};
  auto found =
      std::search(sections.effects.data.begin(), sections.effects.data.end(),
                  marker.begin(), marker.end());
  check(found != sections.effects.data.end(), "wide Sphere wire missing");
  auto at = std::size_t(found - sections.effects.data.begin());
  check(sections.effects.data.at(at + 4) == 2 &&
            sections.effects.data.at(at + 8) == 2,
        "wide Sphere input bytes differ");
  sections.effects.data.at(at + 4) = 1;
  sections.effects.data.at(at + 8) = 1;
  bad.world.systems = g::encodeBattleSystems(sections);
  refused = false;
  try {
    s.restore(bad);
  } catch (const std::exception &) {
    refused = true;
  }
  check(refused && bytes(s) == original,
        "terrain projectile footprint mismatch rollback");
  // Explicit native progression: forcing a result tests once-only settlement,
  // installed driver retention on next/retry, not recovered original missions.
  auto terminal = checkpoint;
  auto c = g::decodeCombat(terminal.world.systems);
  for (auto &f : c.fighters)
    if (f.actor == meta.wizards[1].actor)
      f.health = 1;
  auto parts = g::decodeBattleSystems(terminal.world.systems);
  parts.combat = g::encodeCombat(c);
  terminal.world.systems = g::encodeBattleSystems(parts);
  s.restore(terminal);
  s.enqueue({g::SkirmishOrderKind::bolt,
             meta.wizards[0].actor,
             meta.wizards[1].actor,
             {}});
  unsigned settlements = 0;
  for (unsigned i = 0; i < 20 && s.battle(); ++i)
    if (s.step().result)
      ++settlements;
  check(settlements == 1 && !s.battle(), "terrain victory did not settle once");
  check(!s.step().result, "terrain result settled again");
  s.startMission();
  check(s.battle()->world().state().map == g::arenaMission().map &&
            s.battle()->combat().driver(),
        "terrain finale lost installed driver");
  auto defeat = seed(argv[1]);
  auto failed = defeat.checkpoint();
  auto fc = g::decodeCombat(failed.world.systems);
  for (auto &f : fc.fighters)
    if (f.actor == g::Handle{0, 1})
      f.health = 1;
  auto fp = g::decodeBattleSystems(failed.world.systems);
  fp.combat = g::encodeCombat(fc);
  failed.world.systems = g::encodeBattleSystems(fp);
  auto fm = g::decodeSkirmish(failed.world.campaign);
  fm.wizards[1].mana = 20;
  fm.aiWait = 0;
  failed.world.campaign = g::encodeSkirmish(fm);
  defeat.restore(failed);
  unsigned losses = 0;
  for (unsigned i = 0; i < 20 && defeat.battle(); ++i)
    if (defeat.step().result)
      ++losses;
  check(losses == 1 && !defeat.battle() &&
            defeat.progress().lastResult->outcome == g::CampaignOutcome::defeat,
        "terrain defeat settlement");
  auto retry = app::newMapCampaign(resources);
  retry.restore(defeat.checkpoint());
  retry.startMission();
  check(
      retry.battle() &&
          retry.battle()->world().state().map == app::terrainBattleMap &&
          app::creatureTerrain(*retry.battle()->combat().driver()).has_value(),
      "terrain retry lost geometry/profile");
  print(total, true);
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
