#include "sphere_runtime.hpp"
#include "../../reconstruction/casting/admission.hpp"
#include "../../reconstruction/casting/projectile.hpp"
#include "../../reconstruction/rendering/creature_footprint.hpp"
#include "../../reconstruction/rendering/effect_creature_collision.hpp"
#include "../../reconstruction/rendering/effect_motion_initializer.hpp"
#include "no_cd.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::skirmish::sphere {
namespace g = game;
namespace r = reconstruction;
namespace c = r::casting;
namespace {
constexpr unsigned capacity = 8, width = 8, height = 8, layers = 3;
void require(bool yes, const char *why) {
  if (!yes)
    throw std::invalid_argument(why);
}
const g::Fighter *fighter(const g::CombatState &s, g::Handle h) {
  for (const auto &f : s.fighters)
    if (f.actor == h)
      return &f;
  return nullptr;
}
std::array<unsigned, 3> fine(const g::Entity &e) {
  if (e.motion && e.motion->fine) {
    auto p = e.motion->fine->fine;
    return {unsigned(p.x), unsigned(p.y), unsigned(p.z)};
  }
  return {unsigned(e.x) * 32, unsigned(e.y) * 32, unsigned(e.z) * 16};
}
std::vector<assets::AnimationRecord> sequence() {
  return {{1, 1, {}}, {0, 0, {}}, {0, 1, {}}, {0, 2, {}}, {6, 0, {}}};
}
struct Item {
  std::optional<g::Handle> entity;
  g::Handle caster, aim;
  std::optional<g::Handle> excluded;
  c::ProjectileEffect effect;
  r::EffectCreatureTransitionState motion;
  std::array<unsigned, 3> startup{};
  r::AnimationState animation;
};
struct Tracking {
  unsigned generation = 0, attacker = 0, time = 0;
};
struct State {
  r::EffectPlacementPool pool{width, height, layers, capacity};
  std::array<Item, capacity> items{};
  std::array<Tracking, 32> tracking{};
  unsigned resourceCounter = 0;
};
struct Codec {
  g::Bytes bytes;
  bool reading = false;
  std::size_t at = 0;
  void field(unsigned &n) {
    if (reading) {
      require(at + 4 <= bytes.size(), "Sphere checkpoint truncated");
      n = 0;
      for (unsigned i = 0; i < 4; ++i)
        n |= unsigned(bytes[at++]) << (i * 8);
    } else
      for (unsigned i = 0; i < 4; ++i)
        bytes.push_back(std::uint8_t(n >> (i * 8)));
  }
  void field(bool &b) {
    unsigned n = b;
    field(n);
    require(n <= 1, "Sphere Boolean");
    b = n != 0;
  }
  void field(g::Handle &h) {
    field(h.slot);
    field(h.generation);
  }
  void field(std::optional<g::Handle> &h) {
    bool has = bool(h);
    field(has);
    if (has) {
      auto v = h.value_or(g::Handle{});
      field(v);
      h = v;
    } else
      h.reset();
  }
  void field(std::optional<unsigned> &n) {
    auto v = n.value_or(UINT32_MAX);
    field(v);
    n = v == UINT32_MAX ? std::nullopt : std::optional<unsigned>(v);
  }
  template <class... T> void fields(T &...v) { (field(v), ...); }
  template <class T> void array(T &a) {
    for (auto &v : a)
      field(v);
  }
};
void content(Codec &z, State &s) {
  unsigned version = 0x31504853;
  z.field(version);
  require(version == 0x31504853, "Sphere checkpoint version");
  z.field(s.resourceCounter);
  unsigned scan = s.pool.scan();
  z.field(scan);
  s.pool.setScan(scan);
  for (auto &t : s.tracking)
    z.fields(t.generation, t.attacker, t.time);
  for (auto &cell : s.pool.cells()) {
    unsigned head = cell.head, terrain = cell.terrain;
    z.fields(head, terrain, cell.flags);
    require(head == r::NoEffect || head < capacity, "Sphere cell head");
    require(terrain == 0 && (cell.flags & ~0x80u) == 0,
            "Sphere arena terrain policy");
    cell.head = std::uint16_t(head);
    cell.terrain = std::uint16_t(terrain);
  }
  for (unsigned i = 0; i < capacity; ++i) {
    auto &a = s.items[i];
    auto &e = a.effect;
    auto &p = s.pool.records()[i];
    auto &m = a.motion.movement;
    z.fields(a.entity, a.caster, a.aim, a.excluded, e.type, e.spell, e.source,
             e.target, e.attacker, e.attribution, e.remaining, e.audio);
    z.array(e.contributions);
    z.field(e.cachedActor);
    z.fields(p.active, p.type);
    z.array(p.parameters);
    z.array(p.position);
    z.array(p.units);
    z.array(p.initialPosition);
    z.array(p.sentinels);
    unsigned next = p.next;
    z.fields(next, p.cell, p.initialized);
    require(next == r::NoEffect || next < capacity, "Sphere cell next");
    p.next = std::uint16_t(next);
    z.array(m.motion.trajectory.words);
    z.array(m.motion.previousUnits);
    z.field(m.motion.changes);
    z.array(m.previousPosition);
    z.field(m.terrain);
    z.field(a.motion.candidate);
    z.array(a.startup);
    auto &ani = a.animation;
    unsigned pc = unsigned(ani.pc);
    std::optional<unsigned> displayed =
        ani.displayedRecord
            ? std::optional<unsigned>(unsigned(*ani.displayedRecord))
            : std::nullopt;
    z.fields(pc, displayed, ani.active, ani.delay, ani.elapsed, ani.repeats,
             ani.breakFlag);
    ani.pc = pc;
    ani.displayedRecord =
        displayed ? std::optional<std::size_t>(*displayed) : std::nullopt;
  }
}
State decode(const g::Bytes &b) {
  require(b.size() < 65536, "Sphere byte bound");
  Codec z;
  z.reading = true;
  z.bytes = b;
  State s;
  content(z, s);
  require(z.at == b.size(), "Sphere checkpoint trailing bytes");
  return s;
}
g::Bytes encode(State s) {
  Codec z;
  content(z, s);
  return z.bytes;
}
g::Bytes tag(unsigned slot) { return {'S', 'P', 'H', '1', std::uint8_t(slot)}; }
void release(State &s, unsigned i, g::CombatSession &combat) {
  auto &a = s.items[i];
  auto &p = s.pool.records()[i];
  if (!a.entity)
    return;
  auto &cell = s.pool.cells().at(p.cell);
  if (cell.head == i)
    cell.head = p.next;
  else {
    auto prev = cell.head;
    unsigned n = 0;
    while (prev != r::NoEffect && s.pool.records().at(prev).next != i &&
           ++n <= capacity)
      prev = s.pool.records().at(prev).next;
    require(prev != r::NoEffect && n <= capacity, "Sphere unlink");
    s.pool.records()[prev].next = p.next;
  }
  combat.releaseEffect(*a.entity);
  a = Item{};
  p = r::EffectPlacementRecord{};
  unsigned scan = s.pool.scan();
  while (scan && !s.pool.records()[scan - 1].active)
    --scan;
  s.pool.setScan(scan);
}
void validateState(const State &s, const g::CombatSession &combat,
                   const g::SkirmishState &meta) {
  std::array<bool, capacity> seen{};
  unsigned count = 0;
  for (unsigned cell = 0; cell < s.pool.cells().size(); ++cell) {
    unsigned steps = 0;
    for (unsigned i = s.pool.cells()[cell].head; i != r::NoEffect;
         i = s.pool.records().at(i).next) {
      require(i < capacity && ++steps <= capacity && !seen[i],
              "Sphere duplicate/cyclic membership");
      seen[i] = true;
      require(s.pool.records()[i].cell == cell, "Sphere cell mismatch");
    }
  }
  for (unsigned i = 0; i < capacity; ++i) {
    const auto &a = s.items[i];
    const auto &p = s.pool.records()[i];
    require(bool(a.entity) == p.active && bool(a.entity) == seen[i],
            "Sphere active/membership mismatch");
    if (!a.entity) {
      require(p.next == r::NoEffect, "inactive Sphere link");
      continue;
    }
    ++count;
    const auto *body = combat.world().find(*a.entity);
    const auto *source = combat.world().find(a.caster);
    const auto *aim = combat.world().find(a.aim);
    require(body && body->family == g::Family::missileEffect &&
                body->type == spell && body->owner == 0 && !body->motion &&
                !body->cleaned && body->state == tag(i) &&
                body->target == std::optional<g::Handle>(a.aim),
            "Sphere native identity");
    require(source && aim && fighter(combat.combat(), a.caster) &&
                fighter(combat.combat(), a.aim) &&
                a.caster == meta.wizards[0].actor,
            "Sphere actor binding");
    require(!a.excluded || (combat.world().find(*a.excluded) &&
                            fighter(combat.combat(), *a.excluded)),
            "Sphere exclusion generation");
    require(a.effect.type == 3 && a.effect.spell == spell &&
                a.effect.source ==
                    (a.excluded ? a.excluded->slot : UINT32_MAX) &&
                a.effect.attacker == a.caster.slot &&
                a.effect.attribution == 0 && a.effect.audio <= 1 &&
                (!a.effect.cachedActor || *a.effect.cachedActor < 32),
            "Sphere updater state");
    require(
        p.type == 3 && p.initialized && p.parameters[6] == a.effect.source &&
            p.parameters[7] == a.effect.target && p.parameters[8] == spell &&
            p.parameters[9] == speed && p.units[0] < 256 && p.units[1] < 256 &&
            p.units[2] < 48 &&
            p.position == std::array<unsigned, 3>{p.units[0] >> 5,
                                                  p.units[1] >> 5,
                                                  p.units[2] >> 4} &&
            std::equal(p.units.begin(), p.units.end(), p.parameters.begin()) &&
            p.cell == (p.position[2] * height + p.position[1]) * width +
                          p.position[0],
        "Sphere spatial state");
    require(body->x == int(p.position[0]) && body->y == int(p.position[1]) &&
                body->z == int(p.position[2]),
            "Sphere body position mismatch");
    require(a.motion.movement.terrain == 0, "Sphere terrain mismatch");
    require(a.motion.movement.motion.changes <= 8192, "Sphere counter bound");
    r::NoCdAnimationPlayer player(sequence());
    player.restore(a.animation);
    const auto roster = combat.effects().entities;
    require(std::count(roster.begin(), roster.end(), *a.entity) == 1,
            "Sphere roster missing");
  }
  const auto roster = combat.effects().entities;
  unsigned native = 0;
  for (auto h : roster)
    if (combat.world().find(h)->type == spell)
      ++native;
  require(native == count, "Sphere roster count");
  if (meta.outcome != g::Outcome::running)
    require(!count, "terminal Sphere");
}
} // namespace
g::Bytes initial() { return encode(State{}); }
void validate(const g::Bytes &b, const g::CombatSession &c,
              const g::SkirmishState &s) {
  validateState(decode(b), c, s);
}
g::SkirmishRefusal cast(g::Bytes &b, g::CombatSession &combat,
                        g::SkirmishState &meta, g::Handle source,
                        g::Handle target) {
  auto s = decode(b);
  const auto cstate = combat.combat();
  const auto *from = combat.world().find(source);
  const auto *to = combat.world().find(target);
  const auto *sf = fighter(cstate, source);
  const auto *tf = fighter(cstate, target);
  if (!(source == meta.wizards[0].actor) || !from || !to || !sf || !tf ||
      !sf->health || !tf->health || from->owner != 0 || to->owner != 1)
    return g::SkirmishRefusal::target;
  if (meta.wizards[0].mana < cost)
    return g::SkirmishRefusal::mana;
  unsigned slot = capacity;
  for (unsigned i = 0; i < capacity; ++i)
    if (!s.items[i].entity) {
      slot = i;
      break;
    }
  if (slot == capacity ||
      std::none_of(combat.world().state().slots.begin(),
                   combat.world().state().slots.end(),
                   [](const auto &v) { return !v.entity && v.generation; }))
    return g::SkirmishRefusal::capacity;
  c::AutonomousWorld world;
  world.dimensions = {8, 8, 3};
  world.creatures.resize(combat.world().state().slots.size());
  c::CastAdmissionState admission;
  admission.spells.resize(74);
  admission.spells[spell].cost = cost;
  admission.spells[spell].pathMode = speed;
  admission.resource.resize(world.creatures.size());
  admission.direction.resize(world.creatures.size());
  admission.resourceCounter = s.resourceCounter;
  for (const auto &f : cstate.fighters) {
    const auto &entity = *combat.world().find(f.actor);
    auto &a = world.creatures[f.actor.slot];
    auto p = fine(entity);
    a.ai.slot = f.actor.slot;
    a.ai.type = 17;
    a.ai.kind = f.actor == source ? 0 : 1;
    a.ai.health = f.health;
    a.ai.owner = int(entity.owner);
    a.ai.visible.fill(0);
    a.finePosition = {int(p[0]), int(p[1]), int(p[2])};
    a.aimXY = 16;
    a.aimZ = 8;
  }
  admission.resource[source.slot] = meta.wizards[0].mana * 256;
  c::CastAdmissionServices services;
  services.pool = [&] { return std::optional<unsigned>(slot); };
  services.initializePath = [] {};
  services.currentProfile = [] { return std::optional<unsigned>{}; };
  services.chargeable = [](const auto &a) { return a.ai.kind == 0; };
  services.resource = [&](auto &a, unsigned delta) {
    admission.resource.at(a.ai.slot) += delta;
  };
  services.castCue = [](unsigned, auto, unsigned) {};
  std::optional<c::CastEffectDescriptor> descriptor;
  services.secondary = [&](unsigned chosen, unsigned type, const auto &p) {
    require(chosen == slot && type == 3, "Sphere admission dispatch");
    descriptor = p;
  };
  c::CastDescriptor command{};
  command[0] = spell;
  command[7] = source.slot;
  command[8] = target.slot;
  command[9] = 1;
  admitCast(command, world, admission, services);
  require(bool(descriptor), "Sphere admitted without effect");
  auto p = *descriptor;
  auto &a = s.items[slot];
  a.caster = source;
  a.aim = target;
  a.excluded = source;
  a.effect.source = source.slot;
  a.effect.target = target.slot;
  a.effect.attacker = p[14];
  a.effect.attribution = from->owner;
  p[6] = UINT32_MAX;
  s.pool.place(slot, 3, p);
  s.pool.records()[slot].parameters[6] = source.slot;
  r::initializeEffectMotionRecord(s.pool.records()[slot], a.motion.movement,
                                  a.startup, {p[0], p[1], p[2]},
                                  {p[3], p[4], p[5]}, 8, 8, 3, s.pool.cells());
  a.motion.movement.previousPosition = s.pool.records()[slot].position;
  r::NoCdAnimationPlayer player(sequence());
  player.start();
  a.animation = player.state();
  const auto pos = s.pool.records()[slot].position;
  g::Entity entity;
  entity.family = g::Family::missileEffect;
  entity.type = spell;
  entity.owner = 0;
  entity.x = int(pos[0]);
  entity.y = int(pos[1]);
  entity.z = int(pos[2]);
  entity.target = target;
  entity.state = tag(slot);
  a.entity = combat.spawnEffect(std::move(entity));
  meta.wizards[0].mana = admission.resource[source.slot] / 256;
  s.resourceCounter = admission.resourceCounter;
  b = encode(s);
  return g::SkirmishRefusal::none;
}
g::CombatReport step(g::Bytes &b, g::CombatSession &combat) {
  auto s = decode(b);
  g::CombatReport report;
  for (unsigned i = 0; i < capacity; ++i) {
    auto &a = s.items[i];
    if (!a.entity)
      continue;
    r::EffectCreatureWorld world;
    world.cells.assign(8 * 8 * 3, r::NoEffect);
    world.creatures.resize(32);
    std::vector<c::ProjectileActor> actors(32);
    for (auto &v : world.creatures)
      v.status = 27;
    for (const auto &f : combat.combat().fighters) {
      const auto &entity = *combat.world().find(f.actor);
      require(f.actor.slot < 32, "Sphere actor capacity");
      auto &v = world.creatures[f.actor.slot];
      v.id = f.actor.slot;
      v.status = f.health ? 0 : 27;
      v.units = fine(entity);
      v.footprint = r::initializeCreatureFootprint(1, 0);
      auto &tracking = s.tracking[f.actor.slot];
      if (tracking.generation != f.actor.generation)
        tracking = {f.actor.generation, 0, 0};
      auto &actor = actors[f.actor.slot];
      actor.health = f.health;
      actor.recentAttacker = tracking.attacker;
      actor.recentTime = tracking.time;
      actor.visible[0] = 1;
      if (f.health) {
        auto pos = std::array<unsigned, 3>{v.units[0] >> 5, v.units[1] >> 5,
                                           v.units[2] >> 4};
        require(pos[0] < 8 && pos[1] < 8 && pos[2] < 3,
                "Sphere actor position");
        auto &cell = world.cells[(pos[2] * 8 + pos[1]) * 8 + pos[0]];
        require(cell == r::NoEffect, "Sphere overlapping occupants");
        cell = std::uint16_t(f.actor.slot);
      }
    }
    bool retired = false;
    c::ProjectileServices services;
    services.motion = [&](auto &effect) {
      const auto code =
          r::transitionEffectCreatureWorld(s.pool, i, a.motion, 8, 8, 3, world);
      effect.target = s.pool.records()[i].parameters[7];
      effect.cachedActor = a.motion.candidate == r::NoCreature
                               ? std::nullopt
                               : std::optional<unsigned>(a.motion.candidate);
      return code;
    };
    services.damage = [&](auto &actor, int amount, unsigned, const auto &) {
      require(amount > 0, "Sphere positive damage domain");
      auto slot = unsigned(&actor - actors.data());
      const auto &native = combat.world().state().slots.at(slot);
      require(native.entity && native.entity->family == g::Family::creature,
              "Sphere collision actor");
      g::Handle h{slot, native.generation};
      auto result = combat.damage(h, unsigned(amount), a.caster);
      report.events.insert(report.events.end(), result.events.begin(),
                           result.events.end());
      actor.health = fighter(combat.combat(), h)->health;
    };
    services.bounce = [&](auto &, bool) {
      retired = true;
    }; // Native arena stop-on-terrain policy, not recovered ricochet geometry.
    services.release = [&](auto &) { retired = true; };
    r::NoCdAnimationPlayer player(sequence());
    player.restore(a.animation);
    services.animationTick = [&] { return player.tick(); };
    services.animationRestart = [&] { player.start(); };
    services.visibleDamage = [](auto &) {};
    services.startAudio = [](auto &e) { e.audio = 1; };
    services.followAudio = [](auto &) {};
    c::updateProjectile(a.effect, actors, int(magnitude), 0, services);
    a.animation = player.state();
    a.motion.candidate = a.effect.cachedActor.value_or(r::NoCreature);
    for (unsigned slot = 0; slot < actors.size(); ++slot) {
      s.tracking[slot].attacker = actors[slot].recentAttacker;
      s.tracking[slot].time = actors[slot].recentTime;
    }
    if (retired) {
      release(s, i, combat);
      continue;
    }
    a.excluded =
        a.effect.source == UINT32_MAX
            ? std::nullopt
            : std::optional<g::Handle>(g::Handle{
                  a.effect.source,
                  combat.world().state().slots.at(a.effect.source).generation});
    auto &p = s.pool.records()[i];
    p.parameters[6] = a.effect.source;
    combat.relocateEffect(*a.entity, {int(p.position[0]), int(p.position[1]),
                                      int(p.position[2])});
  }
  b = encode(s);
  return report;
}
void cancel(g::Bytes &b, g::CombatSession &combat) {
  auto s = decode(b);
  for (unsigned i = 0; i < capacity; ++i)
    release(s, i, combat);
  b = encode(s);
}
void removed(g::Bytes &b, g::CombatSession &combat, g::Handle h) {
  auto s = decode(b);
  for (unsigned i = 0; i < capacity; ++i) {
    auto &a = s.items[i];
    if (!a.entity)
      continue;
    if (a.caster == h || a.aim == h ||
        a.excluded == std::optional<g::Handle>(h) ||
        a.effect.cachedActor == std::optional<unsigned>(h.slot))
      release(s, i, combat);
  }
  b = encode(s);
}
std::vector<View> views(const g::Bytes &b) {
  auto s = decode(b);
  std::vector<View> result;
  for (unsigned i = 0; i < capacity; ++i)
    if (s.items[i].entity) {
      auto p = s.pool.records()[i].units;
      result.push_back({*s.items[i].entity,
                        {int(p[0]), int(p[1]), int(p[2])},
                        s.items[i].effect.remaining});
    }
  return result;
}
} // namespace mnm::skirmish::sphere
