#include "arena.hpp"
#include "creature_runtime.hpp"
#include "cure_runtime.hpp"
#include "health_rules.hpp"
#include "persistence/snapshot.hpp"
#include <algorithm>
#include <iostream>
#include <set>
#include <stdexcept>
using namespace mnm::game;
namespace app = mnm::skirmish;
namespace {
void need(bool b, const char *s) {
  if (!b)
    throw std::runtime_error(s);
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
Bytes bytes(const SkirmishSession &s) {
  return encodeSnapshot(s.world().state());
}
SkirmishSession edit(const SkirmishSession &s, CombatSession c,
                     SkirmishState m) {
  auto w = c.world().state();
  w.campaign = encodeSkirmish(m);
  World world(0);
  world.restore(std::move(w));
  return {std::move(world), app::arenaNavigation(), c.healthRules(),
          s.effectsRuntime(), c.driver()};
}
SkirmishSession resume(const SkirmishSession &s) {
  auto w = decodeSnapshot(bytes(s));
  auto rules = app::resolveCreatureRuntime(w);
  auto effects = app::resolveCureRuntime(w);
  World world(0);
  world.restore(std::move(w));
  return {std::move(world), app::arenaNavigation(), app::ordinaryHealthRules(),
          effects, rules};
}
unsigned hp(const SkirmishSession &s, Handle h) {
  for (const auto &f : s.combat().combat().fighters)
    if (f.actor == h)
      return f.health;
  throw std::runtime_error("missing actor");
}
SkirmishSession pair(std::shared_ptr<const CombatDriver> driver,
                     Point a = {2, 3, 1}, Point b = {4, 3, 1}) {
  auto s = app::newSession(driver);
  auto c = s.combat();
  auto m = s.state();
  Entity e;
  e.type = app::arenaNavigation()->creatureType();
  for (unsigned owner = 0; owner < 2; ++owner) {
    auto p = owner ? b : a;
    e.owner = owner;
    e.x = p.x;
    e.y = p.y;
    e.z = p.z;
    auto h = c.spawn(e, driver->creatureStats());
    m.units.push_back({h, UnitRole::sentinel});
  }
  m.wizards[1].mana = 0;
  m.aiWait = 8;
  return edit(s, std::move(c), m);
}
class Fault final : public CombatDriver {
  std::shared_ptr<const CombatDriver> inner_;

public:
  mutable bool fail = true;
  explicit Fault(std::shared_ptr<const CombatDriver> p)
      : inner_(std::move(p)) {}
  std::uint64_t identity() const override { return inner_->identity(); }
  Bytes initial() const override { return inner_->initial(); }
  void validate(const CombatSession &c) const override { inner_->validate(c); }
  void synchronize(CombatSession &c) const override { inner_->synchronize(c); }
  CombatStats creatureStats() const override { return inner_->creatureStats(); }
  bool canSpawn(const CombatSession &c, Point p) const override {
    return inner_->canSpawn(c, p);
  }
  CombatReport step(CombatSession &c, TickInput i) const override {
    auto r = inner_->step(c, i);
    if (fail)
      throw std::runtime_error("injected rules failure");
    return r;
  }
};
void tests(const std::shared_ptr<const CombatDriver> &driver) {
  auto s = pair(driver);
  need(driver->creatureStats().maximumHealth == 360,
       "installed Crocodile health");
  auto frozen = bytes(s);
  s.step({false, false, false});
  need(bytes(s) == frozen, "unadmitted rules advanced");
  auto rules = s.combat().rules().data;
  s.step({true, true, false});
  need(s.combat().rules().data == rules, "alternate rules advanced");
  auto c = s.combat();
  auto original = c.rules().data;
  c.damage({2, 1}, 40, {0, 1});
  need(c.rules().data == original, "rules store a second health copy");
  s = edit(s, std::move(c), s.state());
  s.enqueue({SkirmishOrderKind::cure, {0, 1}, Handle{2, 1}, {}});
  s.enqueue({SkirmishOrderKind::sphere, {0, 1}, Handle{3, 1}, {}});
  unsigned hits = 0, sphereHits = 0, heals = 0, interruptions = 0;
  bool recovered = false;
  std::set<unsigned> interrupted;
  for (unsigned i = 0; i < 16 && s.state().outcome == Outcome::running; ++i) {
    auto copy = resume(s);
    auto report = s.step();
    copy.step();
    need(bytes(s) == bytes(copy), "creature fresh continuation mismatch");
    for (auto ev : report.combat.events) {
      if (ev.kind == CombatEventKind::hit && ev.actor.slot >= 2)
        ++hits;
      if (ev.kind == CombatEventKind::hit && ev.actor == Handle{0, 1} &&
          ev.target == Handle{3, 1}) {
        ++sphereHits;
        need(ev.amount == 20, "Sphere damage changed");
      }
      if (ev.kind == CombatEventKind::healed) {
        ++heals;
        need(ev.amount == 8, "Cure amount changed");
      }
    }
    for (auto v : app::creatureBattleViews(s.combat())) {
      if (v.phase == CreaturePhase::stunned ||
          v.phase == CreaturePhase::knockback) {
        ++interruptions;
        interrupted.insert(v.actor.slot);
      } else if (interrupted.count(v.actor.slot) &&
                 v.phase != CreaturePhase::dead)
        recovered = true;
    }
  }
  need(hits > 0 && heals == 1 && sphereHits >= 1,
       "Cure/Sphere shared battle effects missing");
  // Keep the authored wizard AI quiescent in this isolated control fixture so
  // a long installed stun can expire before an unrelated wizard defeat.
  auto ordinary = pair(driver);
  for (unsigned i = 0; i < 120; ++i) {
    auto quiet = ordinary.state();
    quiet.wizards[1].mana = 0;
    quiet.aiWait = 8;
    ordinary = edit(ordinary, ordinary.combat(), quiet);
    auto copy = resume(ordinary);
    auto r = ordinary.step();
    copy.step();
    need(bytes(ordinary) == bytes(copy), "stun/ANI/RNG continuation mismatch");
    for (auto ev : r.combat.events)
      if (ev.kind == CombatEventKind::hit && ev.actor.slot >= 2)
        ++hits;
    for (auto v : app::creatureBattleViews(ordinary.combat())) {
      if (v.phase == CreaturePhase::stunned ||
          v.phase == CreaturePhase::knockback) {
        ++interruptions;
        interrupted.insert(v.actor.slot);
      } else if (interrupted.count(v.actor.slot) &&
                 v.phase != CreaturePhase::dead)
        recovered = true;
    }
  }
  need(interruptions > 0 && recovered, "stun/knockback recovery absent");
  auto moving = pair(driver);
  auto acquiredReport = moving.step();
  unsigned acquired = 0;
  for (auto ev : acquiredReport.combat.events)
    if (ev.kind == CombatEventKind::acquired)
      ++acquired;
  moving.enqueue({SkirmishOrderKind::move, {2, 1}, {}, Point{1, 3, 1}});
  bool moved = false;
  for (unsigned i = 0; i < 24; ++i) {
    moving.step();
    auto *e = moving.world().find({3, 1});
    moved = moved || e->x != 4 || e->y != 3;
  }
  need(acquired >= 2 && moved,
       "creatures did not acquire/pursue moving target");
  auto death = pair(driver, {2, 4, 1}, {4, 4, 1});
  c = death.combat();
  c.damage({3, 1}, 359, {0, 1});
  Entity e;
  e.type = app::arenaNavigation()->creatureType();
  e.owner = 1;
  e.x = 2;
  e.y = 2;
  e.z = 1;
  auto next = c.spawn(e, driver->creatureStats());
  auto m = death.state();
  m.units.push_back({next, UnitRole::sentinel});
  death = edit(death, std::move(c), m);
  bool killed = false, retargeted = false;
  for (unsigned i = 0; i < 60 && death.state().outcome == Outcome::running;
       ++i) {
    auto report = death.step();
    for (auto ev : report.combat.events) {
      if (ev.kind == CombatEventKind::died && ev.actor == Handle{3, 1})
        killed = true;
      if (killed && ev.kind == CombatEventKind::acquired &&
          ev.actor == Handle{2, 1} && ev.target == next)
        retargeted = true;
    }
  }
  need(killed && retargeted && hp(death, {3, 1}) == 0,
       "death/retarget missing");
  auto reuse = pair(driver);
  reuse.step();
  reuse.removeCreature({3, 1});
  c = reuse.combat();
  e.x = 4;
  e.y = 3;
  auto newTarget = c.spawn(e, driver->creatureStats());
  need(newTarget == Handle{3, 2}, "generation reuse");
  m = reuse.state();
  m.units.push_back({newTarget, UnitRole::sentinel});
  reuse = edit(reuse, std::move(c), m);
  reuse.step();
  for (auto v : app::creatureBattleViews(reuse.combat()))
    need(!(v.actor == Handle{3, 1}) &&
             !(v.target == std::optional<Handle>({3, 1})),
         "stale creature identity survived");
  auto admission = app::newSession(driver);
  need(!driver->canSpawn(admission.combat(), {1, 5, 1}) &&
           !driver->canSpawn(admission.combat(), {6, 3, 1}),
       "2x2 wizard/wall admission");
  admission.enqueue({SkirmishOrderKind::summon, {0, 1}, {}, Point{2, 5, 1}});
  for (unsigned i = 0; i < 3; ++i)
    admission.step();
  need(hp(admission, {2, 1}) == 360, "summon did not bind installed health");
  c = admission.combat();
  auto spawnBefore = encodeSnapshot(c.world().state());
  Entity overlap;
  overlap.type = app::arenaNavigation()->creatureType();
  overlap.x = 3;
  overlap.y = 5;
  overlap.z = 1;
  refuses([&] { c.spawn(overlap, driver->creatureStats()); });
  need(encodeSnapshot(c.world().state()) == spawnBefore,
       "spawn refusal leaked state");
  auto held = pair(driver);
  held.step();
  held.enqueue({SkirmishOrderKind::stop, {2, 1}, {}, {}});
  held.step();
  auto heldViews = app::creatureBattleViews(held.combat());
  need(heldViews[0].phase == CreaturePhase::idle && !heldViews[0].target,
       "stop retained attack ANI/target");
  auto fault = std::make_shared<Fault>(driver);
  auto rollback = pair(fault);
  rollback.enqueue({SkirmishOrderKind::cure, {0, 1}, Handle{2, 1}, {}});
  auto before = bytes(rollback);
  refuses([&] { rollback.step(); });
  need(bytes(rollback) == before,
       "rules failure leaked health/mana/commands/movement");
  fault->fail = false;
  rollback.step();
  auto w = pair(driver).world().state();
  World missing(0);
  missing.restore(w);
  refuses([&] {
    SkirmishSession wrong(missing, app::arenaNavigation(),
                          app::ordinaryHealthRules(), app::battleRuntime());
  });
  auto sections = decodeBattleSystems(w.systems);
  auto valid = encodeBattleSystems(sections);
  need(valid[7] == '2' &&
           encodeBattleSystems(decodeBattleSystems(valid)) == valid,
       "v2 envelope roundtrip");
  for (auto n : {std::size_t(8), std::size_t(20), valid.size() - 1})
    refuses(
        [&] { decodeBattleSystems(Bytes(valid.begin(), valid.begin() + n)); });
  sections.rules.data.pop_back();
  w.systems = encodeBattleSystems(sections);
  refuses([&] { app::resolveCreatureRuntime(w); });
  std::cout << "{\"success\":true,\"installed_maximum\":360,\"melee_hits\":"
            << hits << ",\"interruptions\":" << interruptions
            << ",\"recovery\":true,\"sphere_impacts\":" << sphereHits
            << ",\"cure_heals\":" << heals
            << ",\"pursuit\":true,\"death_retarget\":true,\"generation_reuse\":"
               "true,\"rollback\":true,\"fresh_continuation\":true}\n";
}
} // namespace
int main(int argc, char **argv) try {
  need(argc == 2, "Use creature-battle-test INSTALLATION");
  tests(app::installedCreatureRuntime(argv[1]));
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
