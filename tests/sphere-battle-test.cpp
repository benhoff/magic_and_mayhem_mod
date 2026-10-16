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
void need(bool b, const char *why) {
  if (!b)
    throw std::runtime_error(why);
}
template <class F> void refuses(F f) {
  bool yes = false;
  try {
    f();
  } catch (const std::exception &) {
    yes = true;
  }
  need(yes, "expected refusal");
}
unsigned health(const SkirmishSession &s, Handle h = {2, 1}) {
  for (const auto &f : s.combat().combat().fighters)
    if (f.actor == h)
      return f.health;
  throw std::runtime_error("missing actor");
}
Bytes bytes(const SkirmishSession &s) {
  return encodeSnapshot(s.world().state());
}
SkirmishSession edit(const SkirmishSession &s, CombatSession combat,
                     SkirmishState meta) {
  auto state = combat.world().state();
  state.campaign = encodeSkirmish(meta);
  World w(0);
  w.restore(std::move(state));
  return {std::move(w), app::arenaNavigation(), combat.healthRules(),
          s.effectsRuntime()};
}
SkirmishSession resume(const SkirmishSession &s) {
  auto state = decodeSnapshot(bytes(s));
  auto runtime = app::resolveCureRuntime(state);
  World w(0);
  w.restore(std::move(state));
  return {std::move(w), app::arenaNavigation(), app::ordinaryHealthRules(),
          runtime};
}
SkirmishSession ready(bool moving = true, int row = 5) {
  auto s = app::newSession();
  auto combat = s.combat();
  Entity e;
  e.type = app::arenaNavigation()->creatureType();
  e.owner = 1;
  e.x = 5;
  e.y = row;
  e.z = 1;
  auto target =
      combat.spawn(e, {24, 6, 2, 3}, moving,
                   moving ? CombatControl::automatic : CombatControl::hold);
  auto meta = s.state();
  meta.units.push_back({target, UnitRole::sentinel});
  meta.aiWait = 8;
  meta.wizards[1].mana = 0;
  return edit(s, std::move(combat), meta);
}
void cast(SkirmishSession &s, Handle target = {2, 1}) {
  s.enqueue({SkirmishOrderKind::sphere, {0, 1}, target, {}});
}
void tests() {
  auto s = ready();
  const auto initial = bytes(s);
  cast(s);
  auto first = s.step();
  need(app::sphereViews(s.combat()).size() == 1, "Sphere not created");
  need(s.state().wizards[0].mana == 12, "Sphere must debit8 once");
  bool moved = false;
  auto effect = app::sphereViews(s.combat())[0].effect;
  auto restored = resume(s);
  unsigned impacts = 0, amount = 0;
  for (unsigned n = 0; n < 8; ++n) {
    auto result = s.step();
    moved = moved || s.world().find({2, 1})->x != 5 ||
            s.world().find({2, 1})->y != 5;
    restored.step();
    need(bytes(s) == bytes(restored), "mid-flight restart differs");
    restored = resume(restored);
    for (const auto &event : result.combat.events)
      if (event.kind == CombatEventKind::hit && event.target == Handle{2, 1} &&
          event.actor == Handle{0, 1}) {
        ++impacts;
        amount += event.amount;
      }
  }
  need(moved, "target did not move during flight");
  need(impacts == 1 && amount == 20 && health(s) == 4,
       "moving target must receive exactly one20-point Sphere impact");
  need(s.world().find(effect),
       "hit must retain original travelling projectile");
  auto frozen = bytes(s);
  s.step({false, false, false});
  need(bytes(s) == frozen, "unadmitted Sphere advanced");
  // Alternate admission advances effect life without creature movement/mana/AI.
  auto p = app::sphereViews(s.combat())[0].fine;
  auto position = s.world().find({2, 1})->x;
  auto mana = s.state().wizards[0].mana;
  s.step({true, true, false});
  need(app::sphereViews(s.combat())[0].fine != p &&
           s.world().find({2, 1})->x == position &&
           s.state().wizards[0].mana == mana,
       "alternate Sphere clock");
  unsigned ticks = 0;
  while (!app::sphereViews(s.combat()).empty() && ticks++ < 128)
    s.step({true, true, false});
  need(app::sphereViews(s.combat()).empty() && !s.world().find(effect),
       "Sphere timeout/release");
  auto evading = ready(true, 6);
  cast(evading);
  unsigned misses = 0;
  for (unsigned i = 0; i < 9; ++i) {
    auto report = evading.step();
    for (const auto &event : report.combat.events)
      if (event.kind == CombatEventKind::hit && event.target == Handle{2, 1} &&
          event.actor == Handle{0, 1})
        ++misses;
  }
  need(!misses && health(evading) == 24,
       "Sphere must not home onto an evading target");
  auto lethal = app::newSession();
  auto lethalCombat = lethal.combat();
  lethalCombat.damage({1, 1}, 28, {0, 1});
  auto lethalMeta = lethal.state();
  lethalMeta.aiWait = 8;
  lethalMeta.wizards[1].mana = 0;
  lethal = edit(lethal, std::move(lethalCombat), lethalMeta);
  cast(lethal, {1, 1});
  lethal.step();
  unsigned settled = 0;
  for (unsigned n = 0; n < 32 && lethal.state().outcome == Outcome::running;
       ++n) {
    auto result = lethal.step({true, true, false});
    for (auto event : result.events)
      settled += event.kind == SkirmishEventKind::finished;
  }
  need(lethal.state().outcome == Outcome::victory && settled == 1 &&
           lethal.combat().effects().entities.empty(),
       "alternate lethal projectile must settle/cancel exactly once");
  auto terminal = bytes(lethal);
  lethal.step({true, true, false});
  need(bytes(lethal) == terminal, "terminal effect clock advanced");
  auto poor = ready();
  auto meta = poor.state();
  meta.wizards[0].mana = 0;
  poor = edit(poor, poor.combat(), meta);
  cast(poor);
  auto refusal = poor.step();
  need(app::sphereViews(poor.combat()).empty() &&
           poor.state().wizards[0].mana == 0,
       "poor Sphere charged/allocated");
  need(std::any_of(refusal.events.begin(), refusal.events.end(),
                   [](auto e) { return e.refusal == SkirmishRefusal::mana; }),
       "missing mana refusal");
  auto removed = ready();
  cast(removed);
  removed.step();
  auto old = app::sphereViews(removed.combat())[0].effect;
  auto beforeTick = removed.world().state().tick;
  auto paid = removed.state().wizards[0].mana;
  removed.removeCreature({2, 1});
  need(!removed.world().find(old) &&
           app::sphereViews(removed.combat()).empty() &&
           removed.world().state().tick == beforeTick &&
           removed.state().wizards[0].mana == paid,
       "removed target cancellation/refund/clock");
  refuses([&] {
    removed.enqueue({SkirmishOrderKind::sphere, {0, 1}, Handle{2, 1}, {}});
  });
  auto reused = removed.combat();
  Entity replacement;
  replacement.type = app::arenaNavigation()->creatureType();
  replacement.owner = 1;
  replacement.x = 5;
  replacement.y = 5;
  replacement.z = 1;
  auto newActor = reused.spawn(replacement, {24, 6, 2, 3});
  need(newActor.slot == 2 && newActor.generation == 2, "target slot reuse");
  auto reusedMeta = removed.state();
  reusedMeta.units.push_back({newActor, UnitRole::sentinel});
  removed = edit(removed, std::move(reused), reusedMeta);
  removed.step({true, true, false});
  need(health(removed, newActor) == 24, "old Sphere hit reused identity");
  auto burst = ready(false);
  auto burstCombat = burst.combat();
  auto burstMeta = burst.state();
  for (unsigned i = 0; i < 8; ++i) {
    burstMeta.wizards[0].mana = 20;
    need(burst.effectsRuntime()->castSpell(burstCombat, burstMeta,
                                           Spell::sphere, {0, 1},
                                           {2, 1}) == SkirmishRefusal::none,
         "Sphere pool admission");
  }
  burstMeta.wizards[0].mana = 20;
  auto beforePool = encodeSnapshot(burstCombat.world().state());
  need(burst.effectsRuntime()->castSpell(burstCombat, burstMeta, Spell::sphere,
                                         {0, 1},
                                         {2, 1}) == SkirmishRefusal::capacity &&
           burstMeta.wizards[0].mana == 20 &&
           encodeSnapshot(burstCombat.world().state()) == beforePool,
       "Sphere component pool refused nonatomically");
  burst = edit(burst, std::move(burstCombat), burstMeta);
  need(app::sphereViews(burst.combat()).size() == 8, "Sphere pool count");
  auto full = ready();
  auto state = full.world().state();
  state.slots.resize(3);
  World tiny(0);
  tiny.restore(state);
  full = SkirmishSession(std::move(tiny), app::arenaNavigation(),
                         app::ordinaryHealthRules(), app::battleRuntime());
  cast(full);
  auto noSpace = full.step();
  need(app::sphereViews(full.combat()).empty() &&
           full.state().wizards[0].mana == 20,
       "full pool charged Sphere");
  (void)noSpace;
  auto invalid = ready();
  cast(invalid);
  invalid.step();
  auto corrupted = invalid.world().state();
  const auto identity = app::sphereViews(invalid.combat())[0].effect;
  corrupted.slots[identity.slot].entity->state[0] ^= 0x80;
  World bad(0);
  bad.restore(corrupted);
  refuses([&] {
    SkirmishSession rejected(std::move(bad), app::arenaNavigation(),
                             app::ordinaryHealthRules(), app::battleRuntime());
  });
  // Cure and Sphere share one authoritative combat transaction and checkpoint.
  auto mixed = ready();
  auto c = mixed.combat();
  Entity friendUnit;
  friendUnit.type = app::arenaNavigation()->creatureType();
  friendUnit.owner = 0;
  friendUnit.x = 2;
  friendUnit.y = 5;
  friendUnit.z = 1;
  auto friendly =
      c.spawn(friendUnit, {24, 6, 2, 3}, false, CombatControl::hold);
  c.damage(friendly, 15, {1, 1});
  auto mixedMeta = mixed.state();
  mixedMeta.units.push_back({friendly, UnitRole::sentinel});
  mixed = edit(mixed, std::move(c), mixedMeta);
  cast(mixed);
  mixed.enqueue({SkirmishOrderKind::cure, {0, 1}, friendly, {}});
  mixed.step();
  need(app::cureViews(mixed.combat()).size() == 1 &&
           app::sphereViews(mixed.combat()).size() == 1 &&
           mixed.state().wizards[0].mana == 9,
       "Cure/Sphere shared admission");
  auto same = resume(mixed);
  for (unsigned n = 0; n < 16; ++n) {
    mixed.step();
    same.step();
    need(bytes(mixed) == bytes(same), "mixed effect resume");
  }
  (void)initial;
  (void)first;
}
void seed(const std::filesystem::path &path) {
  auto campaign = app::newCampaign();
  campaign.startMission();
  auto checkpoint = campaign.checkpoint();
  checkpoint.world = ready().world().state();
  campaign.restore(checkpoint);
  need(campaign.save(path).durable, "Sphere campaign seed publication");
}
} // namespace
int main(int argc, char **argv) try {
  if (argc == 3 && std::string(argv[1]) == "seed-campaign") {
    seed(argv[2]);
    return 0;
  }
  if (argc == 3 && std::string(argv[1]) == "seed-cure-only") {
    auto c = app::newCampaign(true, false);
    c.startMission();
    need(c.save(argv[2]).durable, "Cure-only seed publication");
    return 0;
  }
  if (argc == 4 && std::string(argv[1]) == "resume-campaign") {
    auto campaign = app::loadCampaign(argv[2]);
    need(campaign.battle(), "resumed Sphere battle missing");
    unsigned impacts = 0, damage = 0;
    for (unsigned n = 0; n < 8; ++n) {
      auto report = campaign.step();
      need(report.battle.has_value(), "Sphere campaign ended early");
      for (const auto &event : report.battle->combat.events)
        if (event.kind == CombatEventKind::hit && event.actor == Handle{0, 1} &&
            event.target == Handle{2, 1}) {
          ++impacts;
          damage += event.amount;
        }
    }
    auto hp = health(*campaign.battle());
    need(impacts == 1 && damage == 20 && hp == 4,
         "resumed Sphere duplicated or lost impact");
    need(campaign.save(argv[3]).durable, "resumed Sphere publication");
    std::cout << "{\"success\":true,\"impacts\":" << impacts
              << ",\"damage\":" << damage << ",\"health\":" << hp << "}\n";
    return 0;
  }
  if (argc == 1) {
    tests();
    std::cout << "{\"success\":true,\"moving_target_impacts\":1,\"damage\":20,"
                 "\"health\":4,\"generation_reuse\":true,\"mixed_cure\":true,"
                 "\"timeout\":true,\"alternate_lethal\":true}\n";
    return 0;
  }
  throw std::runtime_error("invalid Sphere test command");
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
