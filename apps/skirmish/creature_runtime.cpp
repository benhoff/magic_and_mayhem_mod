#include "creature_runtime.hpp"
#include "../creature-lab/profile.hpp"
#include "health_rules.hpp"
#include "simulation/skirmish.hpp"
#include <algorithm>
#include <array>
#include <deque>
#include <stdexcept>
namespace mnm::skirmish {
namespace {
namespace g = game;
void need(bool yes, const char *why) {
  if (!yes)
    throw std::invalid_argument(why);
}
struct Actor {
  g::CreatureActor state;
  std::optional<g::Handle> target, recent;
};
struct Stored {
  g::Bytes profile, random, terrain;
  std::vector<Actor> actors;
};
struct Codec {
  g::Bytes bytes;
  bool read = false;
  std::size_t at = 0;
  void word(unsigned &n) {
    if (read) {
      need(at <= bytes.size() && bytes.size() - at >= 4,
           "creature battle truncation");
      n = 0;
      for (unsigned i = 0; i < 4; ++i)
        n |= unsigned(bytes[at++]) << (8 * i);
    } else
      for (unsigned i = 0; i < 4; ++i)
        bytes.push_back(std::uint8_t(n >> (8 * i)));
  }
  void signedWord(int &v) {
    auto n = unsigned(v);
    word(n);
    v = n <= 0x7fffffffU ? int(n) : -1 - int(~n);
  }
  void handle(g::Handle &h) {
    word(h.slot);
    word(h.generation);
  }
  void optional(std::optional<g::Handle> &h) {
    unsigned present = bool(h);
    word(present);
    need(present <= 1, "creature handle flag");
    g::Handle v = h.value_or(g::Handle{});
    handle(v);
    need(present || v == g::Handle{}, "empty creature handle");
    h = present ? std::optional<g::Handle>(v) : std::nullopt;
  }
  void blob(g::Bytes &b, unsigned max) {
    auto n = unsigned(b.size());
    word(n);
    need(n <= max, "creature battle blob limit");
    if (read) {
      need(at <= bytes.size() && n <= bytes.size() - at,
           "creature battle blob truncation");
      b.assign(bytes.begin() + at, bytes.begin() + at + n);
      at += n;
    } else
      bytes.insert(bytes.end(), b.begin(), b.end());
  }
  void content(Stored &s) {
    unsigned tag = s.terrain.empty() ? 0x31425243 : s.terrain.at(7)=='2' ? 0x33425243 : 0x32425243;
    word(tag);
    need(tag == 0x31425243 || tag == 0x32425243 || tag == 0x33425243, "creature battle version");
    blob(s.profile, 2 * 1024 * 1024);
    if (tag == 0x32425243 || tag == 0x33425243) {
      blob(s.terrain, 2 * 1024 * 1024);
      need(!s.terrain.empty(), "creature terrain profile missing");
    }
    need(s.terrain.empty() || (s.terrain.size()>8 && ((tag==0x33425243)==(s.terrain[7]=='2'))),"creature actor profile version mismatch");
    blob(s.random, 1008);
    unsigned n = unsigned(s.actors.size());
    word(n);
    need(n <= 30, "creature battle actor limit");
    if (read)
      s.actors.resize(n);
    for (auto &a : s.actors) {
      auto &v = a.state;
      handle(v.handle);
      signedWord(v.home.x);
      signedWord(v.home.y);
      signedWord(v.home.z);
      word(v.facing);
      word(v.cooldown);
      word(v.age);
      signedWord(v.stunTimer);
      unsigned phase = unsigned(v.phase);
      word(phase);
      need(phase <= 7, "creature battle phase");
      v.phase = g::CreaturePhase(phase);
      optional(a.target);
      optional(a.recent);
      blob(v.cursor, 28);
    }
  }
};
g::Bytes encode(Stored s) {
  Codec c;
  c.content(s);
  return c.bytes;
}
Stored decode(const g::Bytes &b) {
  need(b.size() <= 3 * 1024 * 1024, "creature battle extent");
  Codec c{b, true, 0};
  Stored s;
  c.content(s);
  need(c.at == b.size(), "creature battle trailing bytes");
  return s;
}
g::Fighter *fighter(g::CombatState &s, g::Handle h) {
  for (auto &f : s.fighters)
    if (f.actor == h)
      return &f;
  return nullptr;
}
const g::Entity *entity(const g::State &s, g::Handle h) {
  if (h.slot >= s.slots.size())
    return nullptr;
  const auto &v = s.slots[h.slot];
  return v.generation == h.generation && v.entity ? &*v.entity : nullptr;
}
g::Point position(const g::Entity &e) { return {e.x, e.y, e.z}; }
void cancel(Actor &a, g::CreaturePhase phase) {
  a.state.phase = phase;
  a.state.cursor.clear();
  a.state.cooldown = 0;
  a.state.stunTimer = 0;
}
bool managed(const g::State &w, g::Handle h) {
  if (w.campaign.empty())
    return false;
  auto meta = g::decodeSkirmish(w.campaign);
  return !(h == meta.wizards[0].actor) && !(h == meta.wizards[1].actor);
}
void sync(Stored &s, const g::State &w, g::CombatState c) {
  s.actors.erase(std::remove_if(s.actors.begin(), s.actors.end(),
                                [&](const auto &a) {
                                  return !entity(w, a.state.handle) ||
                                         !fighter(c, a.state.handle);
                                }),
                 s.actors.end());
  for (auto &a : s.actors) {
    auto *f = fighter(c, a.state.handle);
    if (!f->health) {
      cancel(a, g::CreaturePhase::dead);
      a.target.reset();
      a.recent.reset();
      a.state.age = 0;
    }
    if (a.target) {
      auto *t = fighter(c, *a.target);
      if (!t || !t->health) {
        a.target.reset();
        if (f->health && a.state.phase != g::CreaturePhase::stunned &&
            a.state.phase != g::CreaturePhase::knockback)
          cancel(a, g::CreaturePhase::idle);
      }
    }
    if (a.recent && (!fighter(c, *a.recent) || !entity(w, *a.recent))) {
      a.recent.reset();
      a.state.age = 0;
    }
    if (f->health && (f->control == g::CombatControl::hold ||
                      f->control == g::CombatControl::move)) {
      if (a.state.phase != g::CreaturePhase::stunned &&
          a.state.phase != g::CreaturePhase::knockback)
        cancel(a, g::CreaturePhase::idle);
      a.target.reset();
    }
  }
}
// Native 2x2 standing/edge policy over the existing arena navigation. Wizards
// occupy one cell; effects and corpses do not block. No original routing claim.
bool free(g::Point p, g::Handle self, const g::State &w, g::CombatState c,
          const g::Navigation &nav) {
  const bool terrain = w.map == terrainBattleMap;
  const bool mixed = nav.wizardType()!=nav.creatureType();
  if (!terrain && p.z != 1)
    return false;
  if (terrain)
    try {
      nav.validateSpawnPosition(p);
    } catch (const std::invalid_argument &) {
      return false;
    }
  for (int y = 0; y < 2; ++y)
    for (int x = 0; x < 2; ++x) {
      g::Point q{p.x + x, p.y + y, p.z};
      if (!terrain)
        try {
          nav.validateSpawnPosition(q);
        } catch (const std::invalid_argument &) {
          return false;
        }
      for (const auto &f : c.fighters) {
        if (f.actor == self || !f.health)
          continue;
        const auto *e = entity(w, f.actor);
        if (!e)
          continue;
        int width = terrain || managed(w, f.actor) ? 2 : 1;
        if(mixed) width=nav.actorFootprint(*e).x;
        if (q.x >= e->x && q.x < e->x + width && q.y >= e->y &&
            q.y < e->y + width && q.z == e->z)
          return false;
      }
    }
  return true;
}
constexpr std::array<g::Point, 8> directions{{{0, -1, 0},
                                              {1, -1, 0},
                                              {1, 0, 0},
                                              {1, 1, 0},
                                              {0, 1, 0},
                                              {-1, 1, 0},
                                              {-1, 0, 0},
                                              {-1, -1, 0}}};
bool edge(g::Point a, g::Point b, g::Handle h, const g::State &w,
          g::CombatState c, const g::Navigation &nav) {
  if (w.map == terrainBattleMap) {
    g::Entity probe;
    probe.type = nav.creatureType();
    probe.x = a.x;
    probe.y = a.y;
    probe.z = a.z;
    probe.motion = g::CreatureMotion{};
    probe.motion->origin = a;
    probe.motion->destination = b;
    auto direction = std::find_if(directions.begin(), directions.end(),
                                  [&](auto d) {
                                    return d.x == b.x - a.x && d.y == b.y - a.y;
                                  }) -
                     directions.begin();
    if (direction == 8)
      return false;
    bool legal = false;
    for (int category : {0, 4})
      legal |= nav.accepts(probe, {b, int(direction), 0, category, 0});
    if (!legal)
      return false;
  }
  return free(b, h, w, c, nav) && (a.x == b.x || a.y == b.y ||
                                   (free({a.x, b.y, a.z}, h, w, c, nav) &&
                                    free({b.x, a.y, a.z}, h, w, c, nav)));
}
bool reach(const g::CreatureRules &r, g::Point a, g::Point b,
           const g::Navigation &nav) {
  return r.reach(a, b, nav.binding().dimensions, [&](g::Point x, g::Point y) {
    try {
      nav.validateSpawnPosition({x.x, y.y, x.z});
      nav.validateSpawnPosition({y.x, x.y, x.z});
      return true;
    } catch (const std::exception &) {
      return false;
    }
  });
}
std::optional<g::Point> route(const Actor &a, g::Point target,
                              const g::State &w, g::CombatState c,
                              const g::Navigation &nav,
                              const g::CreatureRules &rules) {
  auto origin = position(*entity(w, a.state.handle));
  const auto dims = nav.binding().dimensions;
  std::vector<int> parent(dims.x * dims.y, -1);
  auto index = [dims](g::Point p) { return p.y * dims.x + p.x; };
  std::deque<g::Point> queue{origin};
  parent.at(index(origin)) = index(origin);
  while (!queue.empty()) {
    auto p = queue.front();
    queue.pop_front();
    if (p != origin && reach(rules, p, target, nav)) {
      int at = index(p);
      while (parent[at] != index(origin))
        at = parent[at];
      return g::Point{at % dims.x, at / dims.x, origin.z};
    }
    for (auto d : directions) {
      g::Point q{p.x + d.x, p.y + d.y, p.z};
      if (q.x < 0 || q.x >= dims.x || q.y < 0 || q.y >= dims.y ||
          parent[index(q)] != -1 || !edge(p, q, a.state.handle, w, c, nav))
        continue;
      parent[index(q)] = index(p);
      queue.push_back(q);
    }
  }
  return {};
}
// The existing recovered selector adapter uses compact ordinal indices. This
// disposable projection maps those ordinals back to complete native handles.
g::CreatureEncounter projection(const Stored &s, const g::State &w,
                                g::CombatState c, g::State &compact) {
  g::CreatureEncounter out;
  out.dimensions = w.navigation->dimensions;
  compact.slots.clear();
  for (const auto &a : s.actors) {
    auto v = a.state;
    v.health = fighter(c, v.handle)->health;
    v.target = -1;
    v.recent = 0;
    for (unsigned i = 0; i < s.actors.size(); ++i) {
      if (a.target == std::optional<g::Handle>(s.actors[i].state.handle))
        v.target = int(i);
      if (a.recent == std::optional<g::Handle>(s.actors[i].state.handle))
        v.recent = i + 1;
    }
    out.actors.push_back(v);
    compact.slots.push_back(w.slots.at(v.handle.slot));
  }
  return out;
}
class Driver final : public g::CombatDriver {
  creature_lab::Profile profile_;
  std::optional<TerrainBattle> terrain_;

public:
  explicit Driver(creature_lab::Profile p, std::optional<TerrainBattle> t = {})
      : profile_(std::move(p)), terrain_(std::move(t)) {
    if (terrain_)
      validateTerrainCreatureProfile(*terrain_, profile_.bytes);
  }
  std::uint64_t identity() const override { return creatureBattleIdentity; }
  g::Bytes initial() const override {
    return encode({profile_.bytes,
                   profile_.rules->random(1),
                   terrain_ ? terrain_->bytes : g::Bytes{},
                   {}});
  }
  g::CombatStats creatureStats() const override {
    return {profile_.rules->maximumHealth(), 1, 2, 3};
  }
  bool canSpawn(const g::CombatSession &c, g::Point p) const override {
    return free(p, {UINT32_MAX, 0}, c.world().state(), c.combat(),
                *c.navigation());
  }
  void synchronize(g::CombatSession &c) const override {
    auto s = decode(c.rules().data);
    sync(s, c.world().state(), c.combat());
    c.updateRules(encode(s));
  }
  void validate(const g::CombatSession &c) const override {
    if (terrain_) {
      need(c.world().state().map == terrainBattleMap &&
               c.navigation()->binding() == terrain_->navigation->binding(),
           "creature terrain binding");
      if (!c.world().state().campaign.empty()) {
        auto m = g::decodeSkirmish(c.world().state().campaign);
        for (unsigned owner = 0; owner < 2; ++owner) {
          const auto *wizard = c.world().find(m.wizards[owner].actor);
          need(wizard && position(*wizard) == terrain_->wizards[owner],
               "creature terrain wizard placement");
        }
      }
    } else
      need(c.world().state().map == "native-skirmish-arena-v1" &&
               c.navigation()->binding().dimensions == g::Point{8, 8, 3},
           "creature arena binding");
    need(c.healthRules()->identity() == ordinaryHealthIdentity,
         "creature ordinary health binding");
    need(c.rules().identity == identity(), "creature battle runtime binding");
    auto s = decode(c.rules().data);
    need(s.profile == profile_.bytes &&
             s.terrain == (terrain_ ? terrain_->bytes : g::Bytes{}),
         "creature battle profile/terrain mismatch");
    auto combat = c.combat();
    if(terrain_ && terrain_->mixedActors && !c.world().state().campaign.empty()) {
      const auto meta=g::decodeSkirmish(c.world().state().campaign);
      for(const auto &u:meta.units) {
        const auto *e=c.world().find(u.actor);
        need(e && e->type==(u.role==g::UnitRole::wizard?0U:17U),"battle actor role/species mismatch");
      }
    }
    for (const auto &f : combat.fighters)
      if (managed(c.world().state(), f.actor)) {
        const auto expected = creatureStats();
        need(f.stats.maximumHealth == expected.maximumHealth &&
                 f.stats.damage == expected.damage &&
                 f.stats.windupTicks == expected.windupTicks &&
                 f.stats.recoveryTicks == expected.recoveryTicks &&
                 !c.world().find(f.actor)->motion->fine,
             "creature battle installed profile");
        if (f.health)
          need(free(position(*c.world().find(f.actor)), f.actor,
                    c.world().state(), combat, *c.navigation()),
               "creature battle footprint overlap");
      }
    for (unsigned i = 0; i < s.actors.size(); ++i) {
      const auto &a = s.actors[i];
      auto *f = fighter(combat, a.state.handle);
      const auto *e = entity(c.world().state(), a.state.handle);
      need(f && e && a.state.handle.generation && a.state.facing < 8 &&
               a.state.age <= 100 &&
               g::contains(a.state.home, c.navigation()->binding()) &&
               a.state.home.z == (terrain_ ? int(terrain_->plane) : 1),
           "creature battle actor");
      need(i == 0 || s.actors[i - 1].state.handle.slot < a.state.handle.slot,
           "creature battle actor order");
      need(f->stats.maximumHealth == creatureStats().maximumHealth &&
               !e->motion->fine,
           "creature battle installed health/grid profile");
      need((a.state.phase == g::CreaturePhase::dead) == (!f->health),
           "creature battle death mismatch");
      need(f->target == a.target &&
               (a.state.phase != g::CreaturePhase::attacking || a.target),
           "creature battle control target mismatch");
      need(bool(a.recent) == (a.state.age != 0),
           "creature battle recent attacker age");
      if (a.recent)
        need(std::any_of(s.actors.begin(), s.actors.end(),
                         [&](const auto &b) {
                           return b.state.handle == *a.recent &&
                                  !(b.state.handle == a.state.handle);
                         }),
             "creature battle recent attacker generation");
      need(a.state.phase == g::CreaturePhase::ready ? a.state.cooldown > 0
                                                    : !a.state.cooldown,
           "creature battle cooldown");
      need(a.state.phase == g::CreaturePhase::stunned ? a.state.stunTimer >= 0
                                                      : a.state.stunTimer == 0,
           "creature battle stun timer");
      if (a.state.phase == g::CreaturePhase::stunned ||
          a.state.phase == g::CreaturePhase::knockback)
        need(f->control != g::CombatControl::move && !f->destination &&
                 !g::active(e->motion->action),
             "interrupted creature retains movement");
      if (a.target) {
        auto *t = fighter(combat, *a.target);
        need(t && t->health &&
                 entity(c.world().state(), *a.target)->owner != e->owner &&
                 std::any_of(s.actors.begin(), s.actors.end(),
                             [&](const auto &b) {
                               return b.state.handle == *a.target;
                             }),
             "creature battle target generation");
      }
      auto value = a.state;
      value.health = f->health;
      profile_.rules->validate(value, s.random);
      if (f->health)
        need(free(position(*e), f->actor, c.world().state(), combat,
                  *c.navigation()),
             "creature battle footprint overlap");
    }
    // Even an empty continuation must validate every RNG word/cursor layout.
    if (s.actors.empty()) {
      g::CreatureActor a;
      profile_.rules->validate(a, s.random);
    }
  }
  g::CombatReport step(g::CombatSession &combat,
                       g::TickInput input) const override {
    if (input.alternateMode)
      return combat.controlledStep(
          input, [](auto &, auto &, auto &) {},
          [](auto &, auto &, auto &, const auto &) {});
    auto s = decode(combat.rules().data);
    auto c = combat.combat();
    sync(s, combat.world().state(), c);
    for (const auto &f : c.fighters)
      if (managed(combat.world().state(), f.actor) &&
          std::none_of(s.actors.begin(), s.actors.end(), [&](const auto &a) {
            return a.state.handle == f.actor;
          })) {
        Actor a;
        a.state.handle = f.actor;
        a.state.home = position(*combat.world().find(f.actor));
        a.state.phase =
            f.health ? g::CreaturePhase::idle : g::CreaturePhase::dead;
        s.actors.push_back(a);
      }
    std::sort(s.actors.begin(), s.actors.end(),
              [](const auto &a, const auto &b) {
                return a.state.handle.slot < b.state.handle.slot;
              });
    const auto &rules = *profile_.rules;
    const auto &nav = *combat.navigation();
    auto report = combat.controlledStep(
        input,
        [&](g::MovementSession &movement, g::CombatState &state,
            g::CombatReport &out) {
          auto findActor = [&](g::Handle h) -> Actor * {
            for (auto &a : s.actors)
              if (a.state.handle == h)
                return &a;
            return nullptr;
          };
          for (const auto &o : state.pending) {
            auto *a = findActor(o.subject);
            auto *f = fighter(state, o.subject);
            auto *target = o.target ? findActor(*o.target) : nullptr;
            if (!a || !f || !f->health ||
                ((a->state.phase == g::CreaturePhase::stunned ||
                  a->state.phase == g::CreaturePhase::knockback) &&
                 o.kind != g::CombatOrderKind::stop) ||
                (o.kind == g::CombatOrderKind::attack &&
                 (!target || !fighter(state, *o.target)->health ||
                  movement.world().find(*o.target)->owner ==
                      movement.world().find(o.subject)->owner)) ||
                (o.destination &&
                 !free(*o.destination, o.subject, movement.world().state(),
                       state, nav))) {
              ++out.rejectedOrders;
              continue;
            }
            if (a->state.phase != g::CreaturePhase::stunned &&
                a->state.phase != g::CreaturePhase::knockback)
              cancel(*a, g::CreaturePhase::idle);
            a->target.reset();
            f->target.reset();
            f->action = g::CombatAction::idle;
            f->remaining = 0;
            f->destination.reset();
            movement.cancelQueuedMoves(o.subject);
            movement.stop(o.subject);
            if (o.kind == g::CombatOrderKind::move) {
              f->control = g::CombatControl::move;
              f->destination = o.destination;
              movement.move(o.subject, *o.destination);
            } else if (o.kind == g::CombatOrderKind::stop)
              f->control = g::CombatControl::hold;
            else {
              f->control = g::CombatControl::attack;
              a->target = o.target;
              f->target = o.target;
              f->action = g::CombatAction::approaching;
            }
          }
          state.pending.clear();
          for (unsigned i = 0; i < s.actors.size(); ++i) {
            auto &a = s.actors[i];
            auto &v = a.state;
            auto *f = fighter(state, v.handle);
            if (!f->health)
              continue;
            if (v.age && !--v.age)
              a.recent.reset();
            const auto *own = movement.world().find(v.handle);
            if (f->control == g::CombatControl::move) {
              const auto &m = *own->motion;
              if (m.next < m.route.size()) {
                const auto &p = m.route[m.next];
                g::Point q = p.position;
                if (!edge(position(*own), q, v.handle, movement.world().state(),
                          state, nav)) {
                  movement.cancelQueuedMoves(v.handle);
                  movement.stop(v.handle);
                  f->control = g::CombatControl::hold;
                  f->destination.reset();
                }
              }
              continue;
            }
            if (f->control == g::CombatControl::hold ||
                v.phase == g::CreaturePhase::stunned ||
                v.phase == g::CreaturePhase::knockback ||
                v.phase == g::CreaturePhase::ready)
              continue;
            if (!a.target) {
              g::State compact;
              auto view =
                  projection(s, movement.world().state(), state, compact);
              int chosen = rules.select(i, compact, view);
              if (chosen >= 0) {
                need(unsigned(chosen) < s.actors.size(),
                     "creature selector result");
                a.target = s.actors[chosen].state.handle;
                out.events.push_back(
                    {g::CombatEventKind::acquired, v.handle, *a.target, 0});
              }
            }
            if (!a.target)
              continue;
            f->target = a.target;
            f->action = g::CombatAction::approaching;
            f->remaining = 0;
            auto destination = position(*movement.world().find(*a.target));
            if (reach(rules, position(*own), destination, nav)) {
              if (g::active(own->motion->action))
                movement.stop(v.handle);
            } else {
              if (v.phase == g::CreaturePhase::attacking)
                cancel(a, g::CreaturePhase::pursuing);
              else
                v.phase = g::CreaturePhase::pursuing;
              if (!input.suppressSearch && !g::active(own->motion->action) &&
                  movement.world().state().tick % 4 == 0) {
                auto p = route(a, destination, movement.world().state(), state,
                               nav, rules);
                if (p) {
                  movement.move(v.handle, *p);
                  out.events.push_back(
                      {g::CombatEventKind::approach, v.handle, *a.target, 0});
                } else
                  v.phase = g::CreaturePhase::blocked;
              }
            }
          }
        },
        [&](g::State &w, g::CombatState &state, g::CombatReport &out,
            const g::HealthRules &health) {
          for (unsigned i = 0; i < s.actors.size(); ++i) {
            sync(s, w, state);
            auto &a = s.actors[i];
            auto &v = a.state;
            auto *f = fighter(state, v.handle);
            auto &own = *w.slots[v.handle.slot].entity;
            if (!f->health)
              continue;
            if (f->control == g::CombatControl::move) {
              if (!g::active(own.motion->action)) {
                f->control = own.motion->action == g::Action::arrived
                                 ? g::CombatControl::automatic
                                 : g::CombatControl::hold;
                f->destination.reset();
              }
              continue;
            }
            if (v.phase == g::CreaturePhase::stunned) {
              auto ev = rules.animation(v.cursor, true, v.facing);
              int before = v.stunTimer;
              v.stunTimer = rules.stunTick(v.stunTimer, ev);
              if (ev == 1) {
                if (before > 0)
                  v.cursor = rules.start(true, v.facing);
                else
                  cancel(a, g::CreaturePhase::idle);
              }
              continue;
            }
            if (v.phase == g::CreaturePhase::knockback) {
              cancel(a, g::CreaturePhase::idle);
              continue;
            }
            if (f->control == g::CombatControl::hold)
              continue;
            if (v.phase == g::CreaturePhase::ready) {
              if (--v.cooldown)
                continue;
              cancel(a, g::CreaturePhase::idle);
            }
            if (!a.target)
              continue;
            auto *victim = fighter(state, *a.target);
            auto &target = *w.slots[a.target->slot].entity;
            if (!victim || !victim->health ||
                !reach(rules, position(own), position(target), nav)) {
              cancel(a, g::CreaturePhase::pursuing);
              continue;
            }
            g::stopCombatMotion(own);
            if (v.phase != g::CreaturePhase::attacking) {
              v.facing = rules.facing(position(own), position(target),
                                      nav.binding().dimensions);
              v.phase = g::CreaturePhase::attacking;
              v.cursor = rules.start(false, v.facing);
              out.events.push_back(
                  {g::CombatEventKind::windup, v.handle, *a.target, 0});
            }
            auto ev = rules.animation(v.cursor, false, v.facing);
            if (ev == 2) {
              auto roll = rules.damage(s.random);
              g::damageCombat(w, state, *a.target, roll.amount, v.handle,
                              out.events, health);
              auto &b = *std::find_if(
                  s.actors.begin(), s.actors.end(),
                  [&](const auto &x) { return x.state.handle == *a.target; });
              b.recent = v.handle;
              b.state.age = 100;
              if (roll.stun) {
                auto projected = b.state;
                projected.health = victim->health;
                auto stun =
                    rules.stun(s.random, target, projected, own,
                               nav.binding().dimensions, [&](g::Point p) {
                                 return free(p, victim->actor, w, state, nav);
                               });
                if (victim->health) {
                  g::stopCombatMotion(target);
                  if (victim->control == g::CombatControl::move)
                    victim->control = g::CombatControl::automatic;
                  victim->destination.reset();
                  if (stun.knockback) {
                    target.x = stun.knockback->x;
                    target.y = stun.knockback->y;
                    target.z = stun.knockback->z;
                    g::stopCombatMotion(target);
                    cancel(b, g::CreaturePhase::knockback);
                  } else {
                    bool already = b.state.phase == g::CreaturePhase::stunned;
                    if (!already)
                      cancel(b, g::CreaturePhase::stunned);
                    b.state.stunTimer = stun.duration;
                    if (!already)
                      b.state.cursor = rules.start(true, b.state.facing);
                  }
                }
              }
            } else if (ev == 1) {
              cancel(a, g::CreaturePhase::ready);
              v.cooldown = rules.cooldown(s.random);
              need(v.cooldown > 0, "creature ready cooldown");
            }
          }
          sync(s, w, state);
          for (auto &a : s.actors) {
            auto *f = fighter(state, a.state.handle);
            if (!f->health || !a.target) {
              f->target.reset();
              f->action =
                  f->health ? g::CombatAction::idle : g::CombatAction::dead;
              f->remaining = 0;
              if (f->control == g::CombatControl::attack)
                f->control = g::CombatControl::hold;
            } else if (f->control == g::CombatControl::automatic ||
                       f->control == g::CombatControl::attack) {
              f->target = a.target;
              f->action = g::CombatAction::approaching;
              f->remaining = 0;
            }
          }
        },
        [&] { return encode(s); });
    validate(combat);
    return report;
  }
};
} // namespace
std::shared_ptr<const game::CombatDriver>
installedCreatureRuntime(const std::filesystem::path &p) {
  return std::make_shared<Driver>(creature_lab::loadProfile(p));
}
std::shared_ptr<const game::CombatDriver>
resolveCreatureRuntime(const game::State &w) {
  auto r = game::decodeBattleSystems(w.systems).rules;
  if (!r.identity)
    return {};
  need(r.identity == creatureBattleIdentity, "unknown battle creature runtime");
  auto stored = decode(r.data);
  return std::make_shared<Driver>(
      creature_lab::decodeProfile(stored.profile),
      stored.terrain.empty()
          ? std::nullopt
          : std::optional<TerrainBattle>(decodeTerrainBattle(stored.terrain)));
}
std::shared_ptr<const game::CombatDriver>
installedTerrainCreatureRuntime(const std::filesystem::path &root,bool mixedActors) {
  return std::make_shared<Driver>(creature_lab::loadProfile(root),
                                  installedTerrainBattle(root,mixedActors));
}
std::optional<TerrainBattle> creatureTerrain(const game::CombatDriver &driver) {
  need(driver.identity() == creatureBattleIdentity,
       "unknown creature terrain driver");
  auto stored = decode(driver.initial());
  return stored.terrain.empty() ? std::nullopt
                                : std::optional<TerrainBattle>(
                                      decodeTerrainBattle(stored.terrain));
}
std::vector<CreatureBattleView>
creatureBattleViews(const game::CombatSession &c) {
  std::vector<CreatureBattleView> out;
  if (!c.rules().identity)
    return out;
  need(c.rules().identity == creatureBattleIdentity,
       "unknown creature battle view");
  for (const auto &a : decode(c.rules().data).actors)
    out.push_back({a.state.handle, a.state.phase, a.target});
  return out;
}
} // namespace mnm::skirmish
