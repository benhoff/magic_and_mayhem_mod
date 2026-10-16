#include "../apps/cure-world/installed.hpp"
#include "arena.hpp"
#include "campaign.hpp"
#include "cure_runtime.hpp"
#include "health_rules.hpp"
#include "persistence/snapshot.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace mnm::game;
namespace app = mnm::skirmish;
namespace {
void need(bool yes, const char *why) {
  if (!yes)
    throw std::runtime_error(why);
}
template <class F> void refuses(F f) {
  bool yes = false;
  try {
    f();
  } catch (const std::exception &) {
    yes = true;
  }
  need(yes, "expected Cure battle refusal");
}
unsigned health(const SkirmishSession &s, Handle h = {2, 1}) {
  for (const auto &f : s.combat().combat().fighters)
    if (f.actor == h)
      return f.health;
  throw std::runtime_error("missing fighter");
}
Bytes bytes(const SkirmishSession &s) {
  return encodeSnapshot(s.world().state());
}
SkirmishSession edited(const SkirmishSession &s, CombatSession combat,
                       SkirmishState meta) {
  auto state = combat.world().state();
  state.campaign = encodeSkirmish(meta);
  World world(0);
  world.restore(std::move(state));
  return {std::move(world), app::arenaNavigation(), combat.healthRules(),
          s.effectsRuntime()};
}
SkirmishSession
ready(unsigned hp = 9,
      std::shared_ptr<const SkirmishEffects> runtime = app::cureRuntime()) {
  SkirmishSession s(app::arenaNavigation(), app::ordinaryHealthRules(),
                    std::move(runtime));
  auto meta = s.state();
  meta.aiWait = 8;
  meta.wizards[1].mana = 0;
  s = edited(s, s.combat(), meta);
  s.enqueue({SkirmishOrderKind::summon, {0, 1}, {}, Point{2, 5, 1}});
  for (unsigned i = 0; i < 3; ++i)
    s.step();
  auto combat = s.combat();
  if (hp < 24)
    combat.damage({2, 1}, 24 - hp, {1, 1});
  return edited(s, std::move(combat), s.state());
}
SkirmishSession restored(const SkirmishSession &s) {
  auto state = decodeSnapshot(bytes(s));
  auto runtime = app::resolveCureRuntime(state);
  World world(0);
  world.restore(std::move(state));
  return {std::move(world), app::arenaNavigation(), app::ordinaryHealthRules(),
          std::move(runtime)};
}
void cast(SkirmishSession &s, Handle target = {2, 1}) {
  s.enqueue({SkirmishOrderKind::cure, {0, 1}, target, {}});
}
unsigned continuation(SkirmishSession &s, bool alternate = false) {
  auto resumed = restored(s);
  unsigned healed = 0, ticks = 0;
  while (!app::cureViews(s.combat()).empty() && ticks++ < 256) {
    auto report = s.step({true, alternate, false});
    resumed.step({true, alternate, false});
    need(bytes(s) == bytes(resumed), "mid-ANI continuation differs");
    resumed = restored(resumed);
    for (const auto &e : report.combat.events)
      if (e.kind == CombatEventKind::healed)
        healed += e.amount;
  }
  need(app::cureViews(s.combat()).empty(), "Cure did not complete");
  return healed;
}
void contracts() {
  auto s = ready();
  auto mana = s.state().wizards[0].mana;
  s.enqueue({SkirmishOrderKind::move, {2, 1}, {}, Point{4, 5, 1}});
  cast(s);
  s.step();
  need(s.state().wizards[0].mana == mana - app::cureCost,
       "single authoritative mana debit");
  need(app::cureViews(s.combat()).size() == 1, "missing native Cure effect");
  auto old = app::cureViews(s.combat())[0].effect;
  s.step();
  auto v = app::cureViews(s.combat())[0];
  auto body = s.world().find({2, 1});
  need(v.position == Point{body->x, body->y, body->z} && body->x != 2,
       "Cure follows real movement");
  need(continuation(s) == 8 && health(s) == 17 && !s.world().find(old),
       "shared combat health/heal/release");
  for (auto hp : {1U, 17U, 23U, 24U}) {
    auto capped = ready(hp);
    cast(capped);
    capped.step();
    need(continuation(capped) == std::min(8U, 24 - hp) &&
             health(capped) == std::min(24U, hp + 8),
         "authoritative health cap");
  }
  auto poor = ready();
  auto poorMeta = poor.state();
  poorMeta.wizards[0].mana = 0;
  poorMeta.wizards[0].regeneration = 0;
  poor = edited(poor, poor.combat(), poorMeta);
  cast(poor);
  auto poorReport = poor.step();
  need(app::cureViews(poor.combat()).empty() &&
           poor.state().wizards[0].mana == 0 &&
           std::any_of(poorReport.events.begin(), poorReport.events.end(),
                       [](const auto &e) {
                         return e.refusal == SkirmishRefusal::mana;
                       }),
       "insufficient mana charged or allocated");
  auto twice = ready();
  auto doubleMana = twice.state().wizards[0].mana;
  cast(twice);
  cast(twice);
  twice.step();
  need(app::cureViews(twice.combat()).size() == 2 &&
           twice.state().wizards[0].mana == doubleMana - 2 * app::cureCost,
       "concurrent Cure admission");
  need(continuation(twice) == 15 && health(twice) == 24,
       "concurrent Cure health cap");
  auto hitDuring = ready(17);
  cast(hitDuring);
  hitDuring.step();
  auto damaged = hitDuring.combat();
  damaged.damage({2, 1}, 8, {1, 1});
  hitDuring = edited(hitDuring, std::move(damaged), hitDuring.state());
  need(continuation(hitDuring) == 8 && health(hitDuring) == 17,
       "Cure restored stale pre-damage health");
  auto melee = ready(23);
  auto fighters = melee.combat();
  fighters.holdAll();
  Entity foe;
  foe.type = app::arenaNavigation()->creatureType();
  foe.owner = 1;
  foe.x = 3;
  foe.y = 5;
  foe.z = 1;
  auto hostile = fighters.spawn(foe, {SkirmishRules::creatureHealth,
                                      SkirmishRules::creatureDamage, 2, 3});
  auto meleeMeta = melee.state();
  meleeMeta.units.push_back({hostile, UnitRole::sentinel});
  std::sort(
      meleeMeta.units.begin(), meleeMeta.units.end(),
      [](const auto &a, const auto &b) { return a.actor.slot < b.actor.slot; });
  melee = edited(melee, std::move(fighters), meleeMeta);
  cast(melee);
  melee.step();
  unsigned hits = 0, heals = 0;
  for (unsigned n = 0; n < 32 && !app::cureViews(melee.combat()).empty(); ++n) {
    auto r = melee.step();
    for (const auto &e : r.combat.events) {
      if (e.kind == CombatEventKind::hit && e.target == Handle{2, 1})
        ++hits;
      if (e.kind == CombatEventKind::healed)
        heals += e.amount;
    }
  }
  need(hits && heals == 8 && health(melee) > 0 && health(melee) < 24,
       "melee and Cure did not share live health");
  auto dead = ready();
  cast(dead);
  dead.step();
  auto paid = dead.state().wizards[0].mana;
  auto combat = dead.combat();
  combat.damage({2, 1}, 1000, {1, 1});
  dead = edited(dead, std::move(combat), dead.state());
  dead.step({true, true, false});
  need(!health(dead) && app::cureViews(dead.combat()).empty() &&
           dead.state().wizards[0].mana == paid,
       "death wins without resurrection/refund");
  auto full = ready();
  auto state = full.world().state();
  state.slots.resize(3);
  World small(0);
  small.restore(state);
  full = {std::move(small), app::arenaNavigation(), app::ordinaryHealthRules(),
          full.effectsRuntime()};
  paid = full.state().wizards[0].mana;
  cast(full);
  auto report = full.step();
  need(app::cureViews(full.combat()).empty() &&
           full.state().wizards[0].mana == paid &&
           std::any_of(report.events.begin(), report.events.end(),
                       [](const auto &e) {
                         return e.refusal == SkirmishRefusal::capacity;
                       }),
       "native pool exhaustion must not charge");
  auto removed = ready();
  cast(removed);
  removed.step();
  old = app::cureViews(removed.combat())[0].effect;
  paid = removed.state().wizards[0].mana;
  auto tick = removed.world().state().tick;
  removed.removeCreature({2, 1});
  need(app::cureViews(removed.combat()).empty() && !removed.world().find(old) &&
           removed.world().state().tick == tick &&
           removed.state().wizards[0].mana == paid,
       "removal cancels without clock/refund");
  auto before = bytes(removed);
  refuses([&] { removed.removeCreature({2, 1}); });
  refuses([&] { removed.removeCreature({0, 1}); });
  need(before == bytes(removed), "invalid removal changed battle");
  for (unsigned n = 0; n < 40; ++n) {
    auto meta = removed.state();
    meta.wizards[0].mana = 20;
    meta.wizards[1].mana = 0;
    meta.aiWait = 8;
    removed = edited(removed, removed.combat(), meta);
    removed.enqueue({SkirmishOrderKind::summon, {0, 1}, {}, Point{2, 5, 1}});
    for (unsigned j = 0; j < 3; ++j)
      removed.step();
    auto h = removed.state().units[2].actor;
    need(h.slot == 2 && h.generation > 1, "creature generation reuse");
    refuses([&] { cast(removed, {2, 1}); });
    cast(removed, h);
    removed.step();
    need(app::cureViews(removed.combat()).size() == 1,
         "new generation not enrolled");
    removed.removeCreature(h);
  }
  auto pending = ready();
  cast(pending);
  auto frozen = bytes(pending);
  pending.step({false, false, false});
  need(bytes(pending) == frozen, "unadmitted cast must freeze");
  pending.step();
  auto forged = pending.world().state();
  auto parts = decodeBattleSystems(forged.systems);
  parts.effects.data.pop_back();
  forged.systems = encodeBattleSystems(parts);
  World bad(0);
  bad.restore(forged);
  refuses([&] {
    SkirmishSession invalid(std::move(bad), app::arenaNavigation(),
                            app::ordinaryHealthRules(), app::cureRuntime());
  });
  forged = pending.world().state();
  auto effect = app::cureViews(pending.combat())[0].effect;
  forged.slots[effect.slot].entity->state[0] ^= 1;
  bad = World(0);
  bad.restore(forged);
  refuses([&] {
    SkirmishSession invalid(std::move(bad), app::arenaNavigation(),
                            app::ordinaryHealthRules(), app::cureRuntime());
  });
  refuses([&] { cast(pending, {0, 1}); });
  refuses([&] { cast(pending, {1, 1}); });
  // Prior campaign profile remains available under its exact original identity.
  auto legacy = app::newCampaign(false);
  legacy.startMission();
  need(!legacy.battle()->effectsRuntime(), "legacy profile acquired new rules");
  auto restoredLegacy = app::newCampaign(false);
  restoredLegacy.restore(legacy.checkpoint());
  need(
      encodeCampaignSnapshot(legacy.checkpoint(), legacy.progress().identity) ==
          encodeCampaignSnapshot(restoredLegacy.checkpoint(),
                                 restoredLegacy.progress().identity),
      "legacy campaign compatibility");
}
} // namespace
int main(int argc, char **argv) try {
  if (argc == 3 && std::string(argv[1]) == "seed-legacy-campaign") {
    auto campaign = app::newCampaign(false);
    campaign.startMission();
    need(campaign.save(argv[2]).durable, "legacy fixture publication");
    return 0;
  }
  if (argc == 3 && std::string(argv[1]) == "seed-campaign") {
    auto campaign = app::newCampaign();
    campaign.startMission();
    auto snapshot = campaign.checkpoint();
    auto prepared = ready(9, app::battleRuntime());
    snapshot.world = prepared.world().state();
    campaign.restore(snapshot);
    need(campaign.save(argv[2]).durable, "campaign fixture publication");
    return 0;
  }
  if (argc == 4 && std::string(argv[1]) == "resume-campaign") {
    auto campaign = app::loadCampaign(argv[2]);
    unsigned healed = 0;
    while (campaign.battle() &&
           !app::cureViews(campaign.battle()->combat()).empty()) {
      auto r = campaign.step();
      if (r.battle)
        for (const auto &e : r.battle->combat.events)
          if (e.kind == CombatEventKind::healed)
            healed += e.amount;
    }
    need(campaign.battle() && health(*campaign.battle()) == 17,
         "resumed campaign did not heal");
    need(campaign.save(argv[3]).durable, "resumed publication");
    std::cout << "{\"success\":true,\"health\":17,\"healed\":" << healed
              << "}\n";
    return 0;
  }
  if (argc == 3 && std::string(argv[1]) == "installed") {
    auto store = mnm::assets::AssetStore::create(argv[2]);
    auto *assets = std::get_if<mnm::assets::AssetStore>(&store);
    need(assets, "installed assets unavailable");
    auto definition = mnm::cure::loadInstalledCureAnimation(*assets);
    auto s = ready(9, app::cureRuntime(definition));
    cast(s);
    s.step();
    need(continuation(s, true) == 8 && health(s) == 17,
         "installed ANI integrated healing");
    std::cout << "{\"success\":true,\"installed\":true,\"health\":17}\n";
    return 0;
  }
  need(argc == 1, "invalid test arguments");
  contracts();
  std::cout << "{\"success\":true,\"moving_cure\":true,\"shared_health\":true,"
               "\"checkpoint_continuation\":true,\"generation_reenrollment\":"
               "40,\"legacy_campaign\":true}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
