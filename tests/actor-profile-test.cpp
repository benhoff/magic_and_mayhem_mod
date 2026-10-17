#include "../apps/creature-lab/profile.hpp"
#include "creature_runtime.hpp"
#include "map_campaign.hpp"
#include "persistence/snapshot.hpp"
#include "simulation/occupancy.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <iostream>
#include <stdexcept>
namespace g = mnm::game;
namespace app = mnm::skirmish;
void check(bool ok, const char *why) {
  if (!ok)
    throw std::runtime_error(why);
}
g::Bytes bytes(const g::CampaignSession &s) {
  return g::encodeCampaignSnapshot(s.checkpoint(), s.progress().identity);
}
g::Entity actor(g::Point p, unsigned owner, unsigned type = 17) {
  g::Entity e;
  e.type = type;
  e.owner = owner;
  e.x = p.x;
  e.y = p.y;
  e.z = p.z;
  return e;
}
g::CampaignSession seed(const std::filesystem::path &root,
                        bool pursuit = false) {
  auto s = app::newMapCampaign(
      app::mapCampaignResources(root, true, true, true, true));
  s.startMission();
  auto c = s.battle()->combat();
  auto m = s.battle()->state();
  auto friendly = c.spawn(actor({8, 2, 7}, 0), c.driver()->creatureStats());
  auto enemy =
      c.spawn(actor({8, pursuit ? 6 : 4, 7}, 1), c.driver()->creatureStats());
  m.units.push_back({friendly, g::UnitRole::sentinel});
  m.units.push_back({enemy, g::UnitRole::sentinel});
  m.wizards[1].mana = 0;
  m.wizards[1].regeneration = 0;
  m.aiWait = 8;
  c.damage(friendly, 40, m.wizards[0].actor);
  auto snapshot = s.checkpoint();
  snapshot.world = c.world().state();
  snapshot.world.campaign = g::encodeSkirmish(m);
  s.restore(snapshot);
  return s;
}
struct Events {
  unsigned hits = 0, heals = 0, healed = 0, spheres = 0, damage = 0, moves = 0;
  bool playerMoved = false, enemyMoved = false, pursued = false;
};
Events advance(g::CampaignSession &s, unsigned count) {
  Events out;
  for (unsigned i = 0; i < count; ++i) {
    auto before = s.battle()->world().state();
    auto r = s.step();
    check(s.battle() && r.battle.has_value(), "unexpected early result");
    for (const auto &f : s.battle()->combat().combat().fighters) {
      const auto &a = *before.slots.at(f.actor.slot).entity;
      const auto &b = *s.battle()->world().find(f.actor);
      const bool moved = a.x != b.x || a.y != b.y;
      out.moves += moved;
      if (f.actor.slot == 2)
        out.playerMoved |= moved;
      if (f.actor.slot == 3) {
        out.enemyMoved |= moved;
        out.pursued |= moved && f.action == g::CombatAction::approaching;
      }
    }
    for (auto e : r.battle->combat.events) {
      if (e.kind == g::CombatEventKind::hit && e.actor.slot >= 2)
        ++out.hits;
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
  std::cout << "{\"success\":true,\"melee_hits\":" << e.hits
            << ",\"cure_heals\":" << e.heals << ",\"healed\":" << e.healed
            << ",\"sphere_impacts\":" << e.spheres
            << ",\"sphere_damage\":" << e.damage << ",\"moves\":" << e.moves
            << ",\"player_moved\":" << (e.playerMoved ? "true" : "false")
            << ",\"enemy_moved\":" << (e.enemyMoved ? "true" : "false")
            << ",\"enemy_pursued\":" << (e.pursued ? "true" : "false");
  if (contract)
    std::cout
        << ",\"continuations\":24,\"wizard_body\":[1,1,4],\"creature_body\":[2,"
           "2,2],\"profile_refusals\":true,\"mixed_reservations\":true";
  std::cout << "}\n";
}
int main(int argc, char **argv) try {
  if (argc == 4 && (std::string(argv[1]) == "seed" ||
                    std::string(argv[1]) == "seed-pursuit")) {
    auto s = seed(argv[2], std::string(argv[1]) == "seed-pursuit");
    check(s.save(argv[3]).durable, "seed save");
    print({});
    return 0;
  }
  if (argc == 6 && std::string(argv[1]) == "resume") {
    auto s = app::loadMapCampaign(std::filesystem::path(argv[2]), argv[3]);
    auto count = std::stoul(argv[5]);
    check(count <= 128, "resume bound");
    auto e = advance(s, count);
    check(s.save(argv[4]).durable, "resume save");
    print(e);
    return 0;
  }
  check(argc == 2, "usage: actor-profile-test INSTALLATION");
  auto resources = app::mapCampaignResources(argv[1], true, true, true, true);
  auto s = seed(argv[1]);
  const auto &nav = *s.battle()->combat().navigation();
  const auto meta = s.battle()->state();
  const auto wizard = *s.battle()->world().find(meta.wizards[0].actor);
  check(wizard.type == 0 && nav.actorFootprint(wizard) == g::Point{1, 1, 4},
        "installed wizard body");
  check(nav.actorFootprint(*s.battle()->world().find({2, 1})) ==
            g::Point{2, 2, 2},
        "installed Crocodile body");
  auto resolve = [&](const g::Entity &e) { return nav.actorFootprint(e); };
  g::Occupancy occupied(s.battle()->world().state(), nav.binding().dimensions,
                        resolve);
  check(occupied.owner({wizard.x, wizard.y, wizard.z + 3}) ==
            std::optional<g::Handle>(meta.wizards[0].actor),
        "wizard upper body absent");
  check(!occupied.owner({wizard.x + 1, wizard.y, wizard.z}),
        "wizard inherited Crocodile width");
  // Find a supported Crocodile placement beside the narrow wizard. It would
  // overlap the historical wide placeholder and must now be independently
  // legal.
  bool adjacent = false;
  for (auto p : s.battle()->mission().standing)
    if (p.x == wizard.x + 1 && p.y == wizard.y &&
        s.battle()->combat().driver()->canSpawn(s.battle()->combat(), p))
      adjacent = true;
  check(adjacent, "narrow wizard did not release adjacent standing space");
  auto c = s.battle()->combat();
  auto unchanged = g::encodeSnapshot(c.world().state());
  bool refused = false;
  try {
    c.spawn(actor({wizard.x, wizard.y, wizard.z}, 0),
            c.driver()->creatureStats());
  } catch (const std::invalid_argument &) {
    refused = true;
  }
  check(refused && unchanged == g::encodeSnapshot(c.world().state()),
        "overlap spawn rollback");
  refused = false;
  try {
    c.spawn(actor({7, 1, 7}, 0, 1), c.driver()->creatureStats());
  } catch (const std::invalid_argument &) {
    refused = true;
  }
  check(refused && unchanged == g::encodeSnapshot(c.world().state()),
        "unknown type spawn rollback");
  auto original = bytes(s);
  auto bad = s.checkpoint();
  bad.world.slots.at(0).entity->type = 17;
  refused = false;
  try {
    s.restore(bad);
  } catch (const std::exception &) {
    refused = true;
  }
  check(refused && bytes(s) == original, "wizard role/type spoof accepted");
  bad = s.checkpoint();
  bad.world.slots.at(2).entity->type = 0;
  refused = false;
  try {
    s.restore(bad);
  } catch (const std::exception &) {
    refused = true;
  }
  check(refused && bytes(s) == original, "creature role/type spoof accepted");
  bad = s.checkpoint();
  auto parts = g::decodeBattleSystems(bad.world.systems);
  const g::Bytes marker{'S', 'H', 'P', '4'};
  auto it = std::search(parts.effects.data.begin(), parts.effects.data.end(),
                        marker.begin(), marker.end());
  check(it != parts.effects.data.end(), "mixed Sphere wire absent");
  *(it + 3) = '3';
  bad.world.systems = g::encodeBattleSystems(parts);
  refused = false;
  try {
    s.restore(bad);
  } catch (const std::exception &) {
    refused = true;
  }
  check(refused && bytes(s) == original,
        "global Sphere footprint accepted for mixed actors");
  auto old =
      app::newMapCampaign(app::mapCampaignResources(argv[1], true, true, true));
  refused = false;
  try {
    old.restore(s.checkpoint());
  } catch (const std::exception &) {
    refused = true;
  }
  check(refused, "legacy content binding accepted mixed campaign");
  auto terrain = *app::creatureTerrain(*s.battle()->combat().driver());
  auto document = QJsonDocument::fromJson(
      QByteArray(reinterpret_cast<const char *>(terrain.bytes.data() + 8),
                 terrain.bytes.size() - 8));
  const auto raw = QByteArray::fromBase64(
      document.object()["wizard_frozen"].toString().toLatin1());
  const auto word = [](const auto &data, std::size_t at) {
    check(at + 4 <= std::size_t(data.size()), "test word bound");
    unsigned n = 0;
    for (unsigned i = 0; i < 4; ++i)
      n |= unsigned(std::uint8_t(data[at + i])) << (8 * i);
    return n;
  };
  const auto bundle = [&](QByteArray wizard) {
    auto obj = document.object();
    obj["wizard_frozen"] = QString::fromLatin1(wizard.toBase64());
    auto json = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    g::Bytes result{'M', 'N', 'M', 'T', 'E', 'R', '0', '2'};
    result.insert(result.end(), json.begin(), json.end());
    return result;
  };
  auto altered = raw;
  const auto cells = 92 + word(raw, 64) + word(raw, 68);
  altered[cells + 8] = char(std::uint8_t(altered[cells + 8]) ^ 1);
  refused = false;
  try {
    app::decodeTerrainBattle(bundle(altered));
  } catch (const std::exception &) {
    refused = true;
  }
  check(refused, "wizard navigation accepted a different physical world");
  unsigned object = 92;
  for (unsigned i = 0; i < 4; ++i)
    object += word(raw, 64 + i * 4);
  altered = raw;
  altered[object + 0xd07 + 16] =
      char(std::uint8_t(altered[object + 0xd07 + 16]) ^ 1);
  auto forged = app::decodeTerrainBattle(bundle(altered));
  refused = false;
  try {
    app::validateTerrainCreatureProfile(
        forged, mnm::creature_lab::loadProfile(argv[1]).bytes);
  } catch (const std::exception &) {
    refused = true;
  }
  check(refused, "wizard configuration mismatch accepted");
  bad = s.checkpoint();
  parts = g::decodeBattleSystems(bad.world.systems);
  auto &rules = parts.rules.data;
  const auto terrainLength = 8 + word(rules, 4);
  const auto terrainSize = word(rules, terrainLength);
  rules.erase(rules.begin() + terrainLength + 4,
              rules.begin() + terrainLength + 4 + terrainSize);
  for (unsigned i = 0; i < 4; ++i)
    rules[terrainLength + i] = 0;
  bad.world.systems = g::encodeBattleSystems(parts);
  refused = false;
  try {
    app::resolveCreatureRuntime(bad.world);
  } catch (const std::exception &) {
    refused = true;
  }
  check(refused, "empty CRB3 terrain profile accepted");

  // Save continuation resolves both actor resources solely from owned bytes.
  auto state = s.battle()->world().state();
  auto driver = app::resolveCreatureRuntime(state);
  check(app::creatureTerrain(*driver)->mixedActors,
        "owned mixed profile resolution");
  s.enqueue(
      {g::SkirmishOrderKind::cure, meta.wizards[0].actor, g::Handle{2, 1}, {}});
  s.enqueue({g::SkirmishOrderKind::sphere,
             meta.wizards[0].actor,
             g::Handle{3, 1},
             {}});
  Events total;
  for (unsigned i = 0; i < 24; ++i) {
    auto restored = app::newMapCampaign(resources);
    restored.restore(s.checkpoint());
    auto e = advance(s, 1);
    advance(restored, 1);
    check(bytes(s) == bytes(restored), "mixed profile continuation differs");
    total.hits += e.hits;
    total.heals += e.heals;
    total.healed += e.healed;
    total.spheres += e.spheres;
    total.damage += e.damage;
    total.moves += e.moves;
  }
  check(total.hits > 0 && total.heals == 1 && total.healed == 8 &&
            total.spheres == 1 && total.damage == 20,
        "mixed spell/melee journey");
  // Swept claims must reserve each mover's complete body and preserve identity.
  g::State synthetic;
  synthetic.slots = {{1, actor({1, 1, 1}, 0, 0)}, {7, actor({3, 1, 1}, 1)}};
  g::MovementReservations claims(synthetic, {8, 8, 8}, resolve);
  check(claims.tryReserve({0, 1}, {1, 1, 1}, {2, 1, 1}), "wizard step refused");
  check(claims.owner({2, 1, 4}) == std::optional<g::Handle>({0, 1}),
        "wizard sweep height missing");
  check(!claims.tryReserve({1, 7}, {3, 1, 1}, {2, 1, 1}),
        "conflicting mixed sweep accepted");
  check(!claims.owner({2, 2, 2}), "refused sweep left partial claims");
  check(claims.tryReserve({1, 7}, {3, 1, 1}, {4, 1, 1}),
        "separate Crocodile step refused");
  check(claims.owner({5, 2, 2}) == std::optional<g::Handle>({1, 7}),
        "Crocodile sweep width missing");
  bool stale = false;
  try {
    claims.tryReserve({1, 6}, {3, 1, 1}, {4, 1, 1});
  } catch (const std::invalid_argument &) {
    stale = true;
  }
  check(stale, "stale generation reservation accepted");
  // Force bounded authored results to test mission-specific resource retention,
  // not original mission balance or victory scheduling.
  auto terminal = s.checkpoint();
  auto fighters = g::decodeCombat(terminal.world.systems);
  for (auto &f : fighters.fighters)
    if (f.actor == g::Handle{1, 1})
      f.health = 1;
  auto sections = g::decodeBattleSystems(terminal.world.systems);
  sections.combat = g::encodeCombat(fighters);
  terminal.world.systems = g::encodeBattleSystems(sections);
  s.restore(terminal);
  s.enqueue({g::SkirmishOrderKind::bolt,
             meta.wizards[0].actor,
             meta.wizards[1].actor,
             {}});
  unsigned wins = 0;
  for (unsigned i = 0; i < 20 && s.battle(); ++i)
    wins += s.step().result.has_value();
  check(wins == 1 && !s.battle() && !s.step().result,
        "mixed victory settlement");
  auto next = app::newMapCampaign(resources);
  next.restore(s.checkpoint());
  next.startMission();
  check(next.battle() &&
            next.battle()->mission().map == g::arenaMission().map &&
            next.battle()->combat().navigation()->wizardType() ==
                next.battle()->combat().navigation()->creatureType(),
        "arena finale did not retain its explicit legacy profile");
  auto defeat = seed(argv[1]);
  auto failed = defeat.checkpoint();
  fighters = g::decodeCombat(failed.world.systems);
  for (auto &f : fighters.fighters)
    if (f.actor == g::Handle{0, 1})
      f.health = 1;
  sections = g::decodeBattleSystems(failed.world.systems);
  sections.combat = g::encodeCombat(fighters);
  failed.world.systems = g::encodeBattleSystems(sections);
  auto fm = g::decodeSkirmish(failed.world.campaign);
  fm.wizards[1].mana = 20;
  fm.aiWait = 0;
  failed.world.campaign = g::encodeSkirmish(fm);
  defeat.restore(failed);
  unsigned losses = 0;
  for (unsigned i = 0; i < 20 && defeat.battle(); ++i)
    losses += defeat.step().result.has_value();
  check(losses == 1 && !defeat.battle() &&
            defeat.progress().lastResult->outcome == g::CampaignOutcome::defeat,
        "mixed defeat settlement");
  auto retry = app::newMapCampaign(resources);
  retry.restore(defeat.checkpoint());
  retry.startMission();
  check(retry.battle() &&
            app::creatureTerrain(*retry.battle()->combat().driver())
                ->mixedActors &&
            retry.battle()->world().find({0, 1})->type == 0,
        "retry lost mixed actor resources");
  print(total, true);
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
