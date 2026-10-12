#include "creature_encounter.hpp"
#include <algorithm>
#include <array>
#include <deque>
#include <stdexcept>
namespace mnm::game {
namespace {
void need(bool b, const char *s) {
  if (!b)
    throw std::invalid_argument(s);
}
struct Writer {
  Bytes b = {'M', 'N', 'M', 'C', 'A', 'I', '0', '1'};
  void word(std::uint32_t n) {
    for (unsigned i = 0; i < 4; ++i)
      b.push_back(std::uint8_t(n >> (8 * i)));
  }
  void wide(std::uint64_t n) {
    word(unsigned(n));
    word(unsigned(n >> 32));
  }
  void point(Point p) {
    word(p.x);
    word(p.y);
    word(p.z);
  }
  void blob(const Bytes &v) {
    need(v.size() <= 16384, "creature blob limit");
    word(v.size());
    b.insert(b.end(), v.begin(), v.end());
  }
};
struct Reader {
  const Bytes &b;
  std::size_t at = 8;
  unsigned word() {
    need(at <= b.size() && b.size() - at >= 4, "truncated creature state");
    unsigned n = 0;
    for (unsigned i = 0; i < 4; ++i)
      n |= unsigned(b[at++]) << (8 * i);
    return n;
  }
  std::uint64_t wide() {
    auto lo = word(), hi = word();
    return std::uint64_t(hi) << 32 | lo;
  }
  Point point() {
    auto x = word(), y = word(), z = word();
    return {std::int32_t(x), std::int32_t(y), std::int32_t(z)};
  }
  Bytes blob() {
    auto n = word();
    need(n <= 16384 && at <= b.size() && n <= b.size() - at,
         "creature blob extent");
    Bytes out(b.begin() + at, b.begin() + at + n);
    at += n;
    return out;
  }
};
int wrap(int v, int n) {
  v %= n;
  return v < 0 ? v + n : v;
}
Point position(const Entity &e) { return {e.x, e.y, e.z}; }
constexpr std::array<Point, 8> neighbors{{{0, -1, 0},
                                          {1, -1, 0},
                                          {1, 0, 0},
                                          {1, 1, 0},
                                          {0, 1, 0},
                                          {-1, 1, 0},
                                          {-1, 0, 0},
                                          {-1, -1, 0}}};
Point next(Point p, Point d, Point extent) {
  return {wrap(p.x + d.x, extent.x), wrap(p.y + d.y, extent.y), p.z};
}
bool solid(const CreatureEncounter &s, int x, int y) {
  return s.walls[wrap(y, s.dimensions.y) * s.dimensions.x +
                 wrap(x, s.dimensions.x)] != 0;
}
bool free(Point p, unsigned self, const State &w, const CreatureEncounter &s,
          int width) {
  if (p.z != 1)
    return false;
  for (int y = 0; y < width; ++y)
    for (int x = 0; x < width; ++x) {
      auto px = wrap(p.x + x, s.dimensions.x),
           py = wrap(p.y + y, s.dimensions.y);
      if (solid(s, px, py))
        return false;
      for (unsigned j = 0; j < s.actors.size(); ++j) {
        if (j == self || !s.actors[j].health)
          continue;
        const auto &e = *w.slots[j].entity;
        for (int by = 0; by < width; ++by)
          for (int bx = 0; bx < width; ++bx)
            if (px == wrap(e.x + bx, s.dimensions.x) &&
                py == wrap(e.y + by, s.dimensions.y))
              return false;
      }
    }
  return true;
}
bool edge(Point from, Point to, unsigned self, const State &w,
          const CreatureEncounter &s, int width) {
  if (!free(to, self, w, s, width))
    return false;
  if (from.x != to.x && from.y != to.y)
    return free({to.x, from.y, from.z}, self, w, s, width) &&
           free({from.x, to.y, from.z}, self, w, s, width);
  return true;
}
// Native map policy: flat torus, footprint occupancy, blocked corner checks.
// This supplies legality to recovered reach; it is not 4f4ba0 equivalence.
bool diagonal(Point a, Point b, const CreatureEncounter &s) {
  int dx = b.x - a.x, dy = b.y - a.y;
  if (dx > s.dimensions.x / 2)
    dx -= s.dimensions.x;
  if (dx < -s.dimensions.x / 2)
    dx += s.dimensions.x;
  if (dy > s.dimensions.y / 2)
    dy -= s.dimensions.y;
  if (dy < -s.dimensions.y / 2)
    dy += s.dimensions.y;
  for (int i = 1, n = std::max(std::abs(dx), std::abs(dy)); i < n; ++i)
    if (solid(s, a.x + dx * i / n, a.y + dy * i / n))
      return false;
  return !solid(s, a.x + (dx > 0 ? 1 : -1), a.y) &&
         !solid(s, a.x, a.y + (dy > 0 ? 1 : -1));
}
std::optional<Point> route(unsigned self, Point target, const State &w,
                           const CreatureEncounter &s, const CreatureRules &r) {
  const auto origin = position(*w.slots[self].entity);
  int count = s.dimensions.x * s.dimensions.y;
  std::vector<int> parent(count, -1);
  auto index = [&](Point p) { return p.y * s.dimensions.x + p.x; };
  std::deque<Point> queue;
  queue.push_back(origin);
  parent[index(origin)] = index(origin);
  std::optional<Point> goal;
  while (!queue.empty()) {
    auto p = queue.front();
    queue.pop_front();
    if (p != origin && r.reach(p, target, s.dimensions, [&](Point a, Point b) {
          return diagonal(a, b, s);
        })) {
      goal = p;
      break;
    }
    for (auto d : neighbors) {
      auto q = next(p, d, s.dimensions);
      if (parent[index(q)] != -1 || !edge(p, q, self, w, s, r.width()))
        continue;
      parent[index(q)] = index(p);
      queue.push_back(q);
    }
  }
  if (!goal)
    return {};
  auto at = index(*goal);
  while (parent[at] != index(origin))
    at = parent[at];
  return Point{at % s.dimensions.x, at / s.dimensions.x, origin.z};
}
void cancel(CreatureActor &a, CreaturePhase phase) {
  a.cursor.clear();
  a.cooldown = 0;
  a.stunTimer = 0;
  a.phase = phase;
}
} // namespace
Bytes encodeCreatureEncounter(const CreatureEncounter &s) {
  Writer w;
  w.wide(s.rules);
  w.wide(s.healthRules);
  w.blob(s.random);
  w.word(s.quietTicks);
  w.point(s.dimensions);
  w.blob(s.walls);
  need(s.actors.size() <= 16, "creature actor count");
  w.word(s.actors.size());
  for (const auto &a : s.actors) {
    w.word(a.handle.slot);
    w.word(a.handle.generation);
    w.point(a.home);
    w.word(a.health);
    w.word(a.facing);
    w.word(a.cooldown);
    w.word(a.recent);
    w.word(a.age);
    w.word(a.target);
    w.word(a.stunTimer);
    w.word(unsigned(a.phase));
    w.blob(a.cursor);
  }
  return w.b;
}
CreatureEncounter decodeCreatureEncounter(const Bytes &b) {
  need(b.size() >= 8 && std::string(b.begin(), b.begin() + 8) == "MNMCAI01",
       "unsupported creature state");
  Reader r{b};
  CreatureEncounter s;
  s.rules = r.wide();
  s.healthRules = r.wide();
  s.random = r.blob();
  s.quietTicks = r.word();
  s.dimensions = r.point();
  s.walls = r.blob();
  auto count = r.word();
  need(count <= 16, "creature actor count");
  for (unsigned i = 0; i < count; ++i) {
    CreatureActor a;
    a.handle = {r.word(), r.word()};
    a.home = r.point();
    a.health = r.word();
    a.facing = r.word();
    a.cooldown = r.word();
    a.recent = r.word();
    a.age = r.word();
    a.target = std::int32_t(r.word());
    a.stunTimer = std::int32_t(r.word());
    a.phase = CreaturePhase(r.word());
    a.cursor = r.blob();
    s.actors.push_back(a);
  }
  need(r.at == b.size(), "trailing creature state");
  return s;
}
void CreatureEncounterSession::validate(const State &w,
                                        const CreatureEncounter &s) const {
  need(rules_ && health_ && s.rules == rules_->identity() &&
           s.healthRules == health_->identity(),
       "creature rules identity");
  need(s.quietTicks <= 160, "creature quiet counter");
  need(s.dimensions == Point{16, 16, 3} && s.walls.size() == 256,
       "creature map shape");
  need(std::all_of(s.walls.begin(), s.walls.end(),
                   [](auto n) { return n <= 1; }),
       "creature wall flag");
  need(s.actors.size() == 4 && w.slots.size() == 4 && w.pending.empty(),
       "creature capacity");
  need(!w.navigation && !w.animation, "unexpected creature navigation binding");
  for (unsigned i = 0; i < 4; ++i) {
    const auto &a = s.actors[i];
    need(a.handle.slot == i && a.handle.generation == w.slots[i].generation &&
             w.slots[i].entity.has_value(),
         "creature handle");
    const auto &e = *w.slots[i].entity;
    need(e.family == Family::creature && e.type == 17 && e.owner == i % 2 &&
             !e.motion && e.state.empty(),
         "creature entity shape");
    need(e.x >= 0 && e.x < 16 && e.y >= 0 && e.y < 16 && e.z == 1 &&
             a.home.x >= 0 && a.home.x < 16 && a.home.y >= 0 && a.home.y < 16 &&
             a.home.z == 1,
         "creature geometry");
    need(a.health <= rules_->maximumHealth() && unsigned(a.phase) <= 7 &&
             a.facing < 8 && a.cooldown <= 34 && a.stunTimer >= 0 &&
             a.stunTimer <= 39 && a.recent <= 4 && a.age <= 100,
         "creature control bounds");
    need((a.health == 0) == e.cleaned &&
             (a.health == 0) == (a.phase == CreaturePhase::dead),
         "creature lifecycle");
    need(a.target >= -1 && a.target < 4 && a.target != int(i),
         "creature target bound");
    if (a.target >= 0) {
      need(s.actors[a.target].health &&
               w.slots[a.target].entity->owner != e.owner,
           "creature target living enemy");
      need(e.target == std::optional<Handle>(s.actors[a.target].handle),
           "creature target identity");
    } else
      need(!e.target, "creature unexpected target");
    need(a.phase == CreaturePhase::ready ? a.cooldown > 0 : a.cooldown == 0,
         "creature cooldown phase");
    need(a.phase == CreaturePhase::stunned ? a.stunTimer >= 0
                                           : a.stunTimer == 0,
         "creature stun phase");
    if (a.phase == CreaturePhase::attacking)
      need(a.target >= 0, "attack requires target");
    if (a.health)
      need(free(position(e), i, w, s, rules_->width()),
           "creature overlapping footprint");
    rules_->validate(a, s.random);
  }
}
CreatureEncounterSession::CreatureEncounterSession(
    std::shared_ptr<const CreatureRules> r,
    std::shared_ptr<const HealthRules> h, Bytes profile, std::uint32_t seed)
    : world_(4), rules_(std::move(r)), health_(std::move(h)) {
  need(rules_ && health_, "creature rules missing");
  CreatureEncounter s;
  s.rules = rules_->identity();
  s.healthRules = health_->identity();
  s.random = rules_->random(seed);
  s.walls.resize(256);
  // Authored obstacle forces a turn; toroidal paths remain possible.
  for (int y = 5; y < 11; ++y)
    s.walls[y * 16 + 4] = 1;
  for (unsigned i = 0; i < 4; ++i) {
    Entity e;
    e.type = 17;
    e.owner = i % 2;
    e.x = i % 2 ? 6 : 2;
    e.y = i < 2 ? 3 : 6;
    e.z = 1;
    CreatureActor a;
    a.handle = world_.spawn(e);
    a.home = {4, 5, 1};
    a.health = rules_->maximumHealth();
    s.actors.push_back(a);
  }
  auto w = world_.state();
  w.map = "native-creature-control-lab";
  w.campaign = std::move(profile);
  w.systems = encodeCreatureEncounter(s);
  validate(w, s);
  world_.restore(w);
}
CreatureEncounterSession::CreatureEncounterSession(
    World w, std::shared_ptr<const CreatureRules> r,
    std::shared_ptr<const HealthRules> h)
    : world_(std::move(w)), rules_(std::move(r)), health_(std::move(h)) {
  validate(world_.state(), state());
}
bool CreatureEncounterSession::finished() const {
  auto s = state();
  bool teams[2] = {false, false};
  for (unsigned i = 0; i < s.actors.size(); ++i)
    if (s.actors[i].health)
      teams[i % 2] = true;
  return !teams[0] || !teams[1] || s.quietTicks == 160;
}
std::vector<CreatureEvent> CreatureEncounterSession::step(TickInput input) {
  if (!input.admitted || finished())
    return {};
  auto candidate = world_;
  candidate.step(input);
  auto w = candidate.state();
  auto s = state();
  std::vector<CreatureEvent> events;
  auto emit = [&](unsigned i, const char *k, unsigned n = 0) {
    events.push_back({i, s.actors[i].target, k, n});
  };
  if (!input.alternateMode)
    for (unsigned i = 0; i < 4; ++i) {
      auto &a = s.actors[i];
      auto &e = *w.slots[i].entity;
      if (!a.health)
        continue;
      if (a.age) {
        --a.age;
        if (!a.age)
          a.recent = 0;
      }
      if (a.phase == CreaturePhase::stunned) {
        auto ev = rules_->animation(a.cursor, true, a.facing);
        auto before = a.stunTimer;
        a.stunTimer = rules_->stunTick(a.stunTimer, ev);
        if (ev == 1) {
          if (before > 0)
            a.cursor = rules_->start(true, a.facing);
          else {
            cancel(a, CreaturePhase::idle);
            emit(i, "stun-ended");
          }
        }
        continue;
      }
      if (a.phase == CreaturePhase::knockback) {
        cancel(a, CreaturePhase::idle);
        continue;
      }
      if (a.phase == CreaturePhase::ready) {
        if (--a.cooldown)
          continue;
        cancel(a, CreaturePhase::idle);
      }
      if (a.target < 0 || !s.actors[a.target].health) {
        cancel(a, CreaturePhase::idle);
        a.target = rules_->select(i, w, s);
        e.target = a.target >= 0
                       ? std::optional<Handle>(s.actors[a.target].handle)
                       : std::nullopt;
        if (a.target >= 0)
          emit(i, "target");
      }
      if (a.target < 0)
        continue;
      auto &b = s.actors[a.target];
      auto &target = *w.slots[a.target].entity;
      bool reach = rules_->reach(
          position(e), position(target), s.dimensions,
          [&](Point from, Point to) { return diagonal(from, to, s); });
      if (!reach) {
        if (a.phase == CreaturePhase::attacking)
          emit(i, "attack-interrupted");
        cancel(a, CreaturePhase::pursuing);
        if (w.tick % 4 == 0 && !input.suppressSearch) {
          auto p = route(i, position(target), w, s, *rules_);
          if (p) {
            a.facing = rules_->facing(position(e), *p, s.dimensions);
            e.x = p->x;
            e.y = p->y;
            emit(i, "move");
          } else {
            a.phase = CreaturePhase::blocked;
            emit(i, "blocked");
          }
        }
        continue;
      }
      if (a.phase != CreaturePhase::attacking) {
        a.facing = rules_->facing(position(e), position(target), s.dimensions);
        a.phase = CreaturePhase::attacking;
        a.cursor = rules_->start(false, a.facing);
        emit(i, "attack-started");
      }
      auto ev = rules_->animation(a.cursor, false, a.facing);
      if (ev == 2) {
        auto roll = rules_->damage(s.random);
        auto after = roll.amount
                         ? health_->change(b.health, rules_->maximumHealth(),
                                           -int(roll.amount), true)
                         : b.health;
        auto applied = b.health - after;
        b.health = after;
        emit(i, "hit", applied);
        b.recent = i + 1;
        b.age = 100;
        if (roll.stun) {
          auto sr =
              rules_->stun(s.random, target, b, e, s.dimensions, [&](Point p) {
                return free(p, unsigned(a.target), w, s, rules_->width());
              });
          if (after) {
            if (sr.knockback) {
              target.x = sr.knockback->x;
              target.y = sr.knockback->y;
              target.z = sr.knockback->z;
              cancel(b, CreaturePhase::knockback);
              emit(unsigned(a.target), "knockback");
            } else {
              bool already = b.phase == CreaturePhase::stunned;
              if (!already)
                cancel(b, CreaturePhase::stunned);
              b.stunTimer = sr.duration;
              if (!already)
                b.cursor = rules_->start(true, b.facing);
              emit(unsigned(a.target), "stunned", unsigned(sr.duration));
            }
          }
        }
        if (!after) {
          const auto victim = a.target;
          cancel(b, CreaturePhase::dead);
          b.target = -1;
          target.cleaned = true;
          target.target.reset();
          emit(unsigned(a.target), "died");
          for (unsigned j = 0; j < 4; ++j)
            if (s.actors[j].target == victim) {
              auto &peer = s.actors[j];
              peer.target = -1;
              w.slots[j].entity->target.reset();
              if (peer.phase == CreaturePhase::attacking ||
                  peer.phase == CreaturePhase::pursuing ||
                  peer.phase == CreaturePhase::blocked)
                cancel(peer, CreaturePhase::idle);
            }
        }
      } else if (ev == 1) {
        a.phase = CreaturePhase::ready;
        a.cursor.clear();
        a.cooldown = rules_->cooldown(s.random);
        emit(i, "ready", a.cooldown);
      }
    }
  if (!input.alternateMode) {
    bool progress =
        std::any_of(events.begin(), events.end(),
                    [](const auto &e) { return e.kind != "blocked"; });
    s.quietTicks = progress ? 0 : std::min(160U, s.quietTicks + 1);
  }
  w.systems = encodeCreatureEncounter(s);
  validate(w, s);
  candidate.restore(w);
  world_ = std::move(candidate);
  return events;
}
} // namespace mnm::game
