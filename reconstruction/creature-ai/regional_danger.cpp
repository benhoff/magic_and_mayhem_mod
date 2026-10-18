#include "regional_danger.hpp"
#include <algorithm>
#include <cmath>
#include <set>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int bits(unsigned n) { return n <= INT32_MAX ? int(n) : -1 - int(~n); }
int add(int a, int b) { return bits(unsigned(a) + unsigned(b)); }
int subtract(int a, int b) { return bits(unsigned(a) - unsigned(b)); }
int product(int a, int b) { return bits(unsigned(a) * unsigned(b)); }
int quotient(int a, int b) {
  if (!b || (a == INT32_MIN && b == -1))
    throw std::invalid_argument("danger division");
  return a / b;
}
int axis(int a, int b, int n) {
  a %= n;
  b %= n;
  if (a < 0)
    a += n;
  if (b < 0)
    b += n;
  int d = std::abs(a - b);
  return d > n / 2 ? n - d : d;
}
unsigned square(Cell a, Cell b, Extent d, bool fine) {
  unsigned x = unsigned(axis(a.x, b.x, d.x)), y = unsigned(axis(a.y, b.y, d.y));
  unsigned z = unsigned(a.z) - unsigned(b.z);
  if (fine) {
    x <<= 5;
    y <<= 5;
    z <<= 4;
    return (x * x + y * y + z * z) >> 10;
  }
  return x * x + y * y + z * z;
}
bool special(const RegionalDangerActor &a) {
  return a.typeClass == 0 || a.typeClass == 24 || a.typeClass == 25 ||
         a.typeClass == 26;
}
bool broad(const RegionalDangerActor &a) {
  return special(a) || ((a.type != 18 || !a.flying) && a.ranged);
}
CreatureMovementParameters movement(const RegionalDangerActor &a) {
  return {int(a.height),
          int(a.width),
          int(a.flag104),
          int(a.movementKind),
          0,
          int(a.type),
          std::uint8_t(a.block723 || a.behavior14)};
}
const RegionalDangerActor *actor(const RegionalDangerInputs &in, int slot) {
  auto it = std::find_if(in.actors.begin(), in.actors.end(),
                         [&](auto &a) { return a.slot == slot; });
  return it == in.actors.end() ? nullptr : &*it;
}
std::set<int> vicinity(const std::vector<NavigationRegion> &g, int start,
                       const CreatureMovementParameters &p, unsigned depth) {
  std::set<int> out{start}, front{start};
  for (unsigned n = 0; n < depth; ++n) {
    std::set<int> next;
    for (int id : front)
      for (auto &e : g.at(unsigned(id)).connections)
        if (regionConnectionAdmitted(e, p)) {
          out.insert(e.destination);
          next.insert(e.destination);
        }
    front = std::move(next);
  }
  return out;
}
void accumulate(std::uint8_t &v, int amount) {
  v = std::uint8_t(std::clamp(add(int(v), amount), 0, 255));
}
bool near(Cell a, Cell b, int radius, Extent d) {
  return bits(square(a, b, d, true)) <= product(radius, radius);
}
void effects(const std::vector<NavigationRegion> &g, RegionalDangerState &s,
             const RegionalDangerInputs &in, int region, int owner,
             bool deduction) {
  auto &row =
      (deduction ? s.pendingDeduction : s.pendingDanger).at(unsigned(region));
  for (auto &e : in.effects) {
    if (!near(g.at(unsigned(region)).position, e.position, e.radius,
              in.dimensions))
      continue;
    const bool allied =
        in.allied.at(unsigned(owner)).at(unsigned(e.owner)) != 0;
    if (deduction) {
      if (allied && e.kind == 0x45)
        accumulate(row[3], 30);
      if (allied && e.kind == 0x46)
        accumulate(row[4], 20);
      if (allied && e.kind == 0x5b)
        accumulate(row[5], 25);
    } else {
      if (e.kind == 0x41)
        accumulate(row[2], 10);
      if (!allied && e.kind == 0x46)
        accumulate(row[4], 20);
      if (!allied && e.kind == 0x5b)
        accumulate(row[5], 5);
      if (!allied && e.kind == 0x5c)
        accumulate(row[6], 20);
    }
  }
}
void initialize(const std::vector<NavigationRegion> &g, RegionalDangerState &s,
                const RegionalDangerInputs &in) {
  if (s.pendingDanger.empty())
    s.pendingDanger.resize(g.size());
  if (s.pendingDeduction.empty())
    s.pendingDeduction.resize(g.size());
  if (s.pendingBase.empty())
    s.pendingBase.resize(g.size());
  if (s.actorSupport.empty())
    s.actorSupport.resize(in.actors.size());
  validateRegionalDanger(g, s, in);
}
int distance(Cell a, Cell b, Extent d) {
  return int(std::sqrt(double(square(a, b, d, true))));
}
int available(const RegionalDangerActor &a) {
  return bits(a.resource + a.inventoryResource);
}
bool connected(const std::vector<NavigationRegion> &g,
               const RegionalDangerActor &a,
               const RegionalDangerActor *commander, Extent d, bool wide) {
  int center;
  return commander && commander->region >= 0 && a.region >= 0 &&
         regionalDangerCenter(
             g, a.region, commander->region,
             wide ? CreatureMovementParameters{1, 1, 1, 0, 0, 0, 0}
                  : movement(a),
             d, center);
}
void hostile(const std::vector<NavigationRegion> &g, RegionalDangerState &s,
             const RegionalDangerInputs &in,
             const RegionalDangerMemory &memory) {
  const auto &a = memory.actor;
  if (!a.health || a.region < 0)
    return;
  int power =
      std::max(bits(a.maximumResource * 64u),
               a.owner == s.dangerOwner
                   ? available(a)
                   : bits(a.estimatedResource.at(unsigned(s.dangerOwner))));
  power = add(power, a.threat);
  int denominator =
      add(10000, product(subtract(in.tick, memory.observedTick), a.aging));
  for (auto p : in.observationPoints)
    if (near(a.position, p, 5, in.dimensions)) {
      denominator = 10000;
      break;
    }
  const int amount = quotient(product(power, 20), denominator);
  const auto *commander = actor(in, s.commander);
  const int range =
      commander ? distance(a.position, commander->position, in.dimensions) : 0;
  if (broad(a) || a.animationKind == 2 || a.status114) {
    const auto nodes = vicinity(g, a.region, {1, 1, 1, 0, 0, 0, 0}, 2);
    if (broad(a)) {
      for (int id : nodes) {
        auto &row = s.pendingDanger.at(unsigned(id));
        accumulate(row[0], amount);
        accumulate(row[1], amount);
      }
      if (connected(g, a, commander, in.dimensions, true)) {
        int reach = special(a) ? 0 : std::max(a.attackRange, 0);
        if (!special(a) && a.animationKind == 2)
          reach = std::max(reach, in.animationRange);
        s.dangerTotal =
            add(s.dangerTotal,
                quotient(product(amount, 10),
                         add(std::max(subtract(range, reach), 0), 1)));
      }
    }
    if (a.status114) {
      for (int id : nodes)
        accumulate(s.pendingDanger.at(unsigned(id))[2], in.firePower);
      if (connected(g, a, commander, in.dimensions, true)) {
        const unsigned divisor =
            unsigned(add(std::max(subtract(range, in.fireRange), 0), 1));
        s.dangerTotal = add(
            s.dangerTotal, bits(unsigned(product(in.firePower, 10)) / divisor));
      }
    }
  }
  if (!broad(a) && a.animationKind != 2) {
    for (int id : vicinity(g, a.region, movement(a), 1)) {
      auto &row = s.pendingDanger.at(unsigned(id));
      accumulate(row[0], amount);
      if (!g.at(unsigned(id)).parameter || a.flag104)
        accumulate(row[1], amount);
    }
    if (connected(g, a, commander, in.dimensions, false))
      s.dangerTotal =
          add(s.dangerTotal, quotient(product(amount, 10), add(range, 1)));
  }
}
void friendly(const std::vector<NavigationRegion> &g, RegionalDangerState &s,
              const RegionalDangerInputs &in, const RegionalDangerActor &a) {
  const unsigned amount =
      ((a.resource & 0xffffff00u) + unsigned(a.threat)) * 20u / 10000u;
  const bool wide =
      (special(a) && !a.block7c) || ((a.type != 18 || !a.flying) && a.ranged);
  if (wide || a.status114) {
    for (int id : vicinity(g, a.region, {1, 1, 1, 0, 0, 0, 0}, 2)) {
      auto &row = s.pendingDeduction.at(unsigned(id));
      if (wide) {
        accumulate(row[0], bits(amount));
        accumulate(row[1], bits(amount));
      }
      if (a.status114)
        accumulate(row[2], in.firePower);
    }
  }
  if (!wide)
    for (int id : vicinity(g, a.region, movement(a), 1)) {
      auto &row = s.pendingDeduction.at(unsigned(id));
      accumulate(row[0], bits(amount));
      if (!g.at(unsigned(id)).parameter || a.flag104)
        accumulate(row[1], bits(amount));
    }
  // 529e40 uses the subject's owner relation toward the queried owner.
  const auto *commander =
      actor(in, in.commander.at(unsigned(s.deductionOwner)));
  int support = 0;
  if (in.allied.at(unsigned(s.deductionOwner)).at(unsigned(a.owner)) &&
      connected(g, a, commander, in.dimensions, broad(a))) {
    const unsigned numerator =
        ((a.resource & 0xffffff00u) + unsigned(a.threat)) * 20u;
    support =
        int(double(numerator) /
            (std::sqrt(double(square(g.at(unsigned(a.region)).position,
                                     g.at(unsigned(commander->region)).position,
                                     in.dimensions, true))) *
                 2304.0 +
             10000.0));
    if (a.block7c)
      support /= 2;
    support = product(support, 10);
  }
  auto at = std::find_if(in.actors.begin(), in.actors.end(),
                         [&](auto &v) { return v.slot == a.slot; });
  s.actorSupport.at(unsigned(at - in.actors.begin())) = support;
  s.deductionTotal = add(s.deductionTotal, support);
}
} // namespace
bool regionalDangerCenter(const std::vector<NavigationRegion> &g, int start,
                          int end, const CreatureMovementParameters &p,
                          Extent d, int &center) {
  center = -1;
  std::vector<int> path{start};
  while (start != end) {
    unsigned best = square(g.at(unsigned(start)).position,
                           g.at(unsigned(end)).position, d, false);
    int next = -1;
    for (auto &e : g.at(unsigned(start)).connections)
      if (regionConnectionAdmitted(e, p)) {
        unsigned value = square(g.at(unsigned(e.destination)).position,
                                g.at(unsigned(end)).position, d, false);
        if (bits(value) < bits(best)) {
          best = value;
          next = e.destination;
        }
      }
    if (next < 0)
      return false;
    start = next;
    path.push_back(start);
  }
  center = path[(path.size() - 1) / 2];
  return true;
}
void updateRegionalDanger(std::vector<NavigationRegion> &g,
                          RegionalDangerState &s,
                          const RegionalDangerInputs &in, bool blocked) {
  if (blocked)
    return;
  initialize(g, s, in);
  if (s.resetDanger) {
    s.dangerOwner = -1;
    s.dangerRegion = 0;
    s.dangerTotal = 0;
    std::fill(s.pendingDanger.begin(), s.pendingDanger.end(), DangerRow{});
    std::fill(s.pendingBase.begin(), s.pendingBase.end(), 0);
  }
  unsigned owners = 0, records = 0;
  for (;;) {
    if (!s.resetDanger && s.dangerOwner >= 0 &&
        in.enabled.at(unsigned(s.dangerOwner))) {
      const auto &memories = in.memories.at(unsigned(s.dangerOwner));
      auto it = s.memoryKey < 0
                    ? memories.end()
                    : std::lower_bound(
                          memories.begin(), memories.end(), s.memoryKey,
                          [](auto &v, int key) { return v.key < key; });
      if (it != memories.end()) {
        hostile(g, s, in, *it);
        s.memoryKey = it->key + 1;
        if (++records == 4)
          return;
        continue;
      }
      unsigned regions = 0;
      while (unsigned(s.dangerRegion) < g.size()) {
        if (regions++ == 32)
          return;
        effects(g, s, in, s.dangerRegion++, s.dangerOwner, false);
      }
      s.dangerRegion = 0;
      for (unsigned i = 0; i < g.size(); ++i)
        g[i].danger.at(unsigned(s.dangerOwner)) = s.pendingDanger[i];
      s.dangerTotals.at(unsigned(s.dangerOwner)) = s.dangerTotal;
      s.dangerTotal = 0;
      std::fill(s.pendingDanger.begin(), s.pendingDanger.end(), DangerRow{});
    }
    s.resetDanger = false;
    for (;;) {
      if (owners++ == 8)
        return;
      if (++s.dangerOwner == 8) {
        s.dangerOwner = 7;
        unsigned regions = 0;
        while (unsigned(s.dangerRegion) < g.size()) {
          if (regions++ == 32)
            return;
          const auto at = unsigned(s.dangerRegion++);
          for (auto &v : in.hazards)
            if (near(g[at].position, v.position, v.radius, in.dimensions))
              accumulate(s.pendingBase[at], bits(v.amount));
        }
        s.dangerRegion = 0;
        for (unsigned i = 0; i < g.size(); ++i)
          g[i].dangerBase = s.pendingBase[i];
        std::fill(s.pendingBase.begin(), s.pendingBase.end(), 0);
        s.dangerOwner = 0;
      }
      const auto &m = in.memories.at(unsigned(s.dangerOwner));
      s.memoryKey = m.empty() ? -1 : m.front().key;
      if (in.enabled.at(unsigned(s.dangerOwner))) {
        s.commander = in.commander.at(unsigned(s.dangerOwner));
        break;
      }
    }
  }
}
void updateRegionalDeduction(std::vector<NavigationRegion> &g,
                             RegionalDangerState &s,
                             const RegionalDangerInputs &in, bool blocked) {
  if (blocked)
    return;
  initialize(g, s, in);
  if (s.resetDeduction) {
    s.deductionOwner = -1;
    s.deductionRegion = 0;
    s.deductionTotal = 0;
    std::fill(s.pendingDeduction.begin(), s.pendingDeduction.end(),
              DangerRow{});
  }
  unsigned owners = 0, processed = 0;
  for (;;) {
    if (!s.resetDeduction && s.deductionOwner >= 0 &&
        in.enabled.at(unsigned(s.deductionOwner))) {
      while (s.actorCursor < 125) {
        const auto *a = actor(in, s.actorCursor++);
        if (!a || !a->active || !a->health || a->region < 0)
          continue;
        if (a->owner != s.deductionOwner &&
            (!in.shared.at(unsigned(a->owner)).at(unsigned(s.deductionOwner)) ||
             !in.allied.at(unsigned(a->owner)).at(unsigned(s.deductionOwner))))
          continue;
        friendly(g, s, in, *a);
        if (++processed == 4)
          return;
      }
      unsigned regions = 0;
      while (unsigned(s.deductionRegion) < g.size()) {
        if (regions++ == 32)
          return;
        effects(g, s, in, s.deductionRegion++, s.deductionOwner, true);
      }
      s.deductionRegion = 0;
      for (unsigned i = 0; i < g.size(); ++i)
        g[i].deductedDanger.at(unsigned(s.deductionOwner)) =
            s.pendingDeduction[i];
      s.deductionTotals.at(unsigned(s.deductionOwner)) = s.deductionTotal;
      s.deductionTotal = 0;
      std::fill(s.pendingDeduction.begin(), s.pendingDeduction.end(),
                DangerRow{});
    }
    s.resetDeduction = false;
    do {
      if (owners++ == 8)
        return;
      if (++s.deductionOwner == 8)
        s.deductionOwner = 0;
    } while (!in.enabled.at(unsigned(s.deductionOwner)));
    s.actorCursor = 0;
  }
}
void validateRegionalDanger(const std::vector<NavigationRegion> &g,
                            const RegionalDangerState &s,
                            const RegionalDangerInputs &in) {
  if (in.dimensions.x <= 0 || in.dimensions.y <= 0 || in.dimensions.z <= 0 ||
      g.empty() || g.size() > 4096 || in.actors.size() > 125 ||
      in.effects.size() > 128 || in.hazards.size() > 128 ||
      in.observationPoints.size() > 128 || s.dangerOwner < -1 ||
      s.dangerOwner > 7 || s.deductionOwner < -1 || s.deductionOwner > 7 ||
      s.memoryKey < -1 || s.dangerRegion < 0 ||
      unsigned(s.dangerRegion) > g.size() || s.deductionRegion < 0 ||
      unsigned(s.deductionRegion) > g.size() || s.actorCursor < 0 ||
      s.actorCursor > 125 || s.pendingDanger.size() != g.size() ||
      s.pendingDeduction.size() != g.size() ||
      s.pendingBase.size() != g.size() ||
      s.actorSupport.size() != in.actors.size())
    throw std::invalid_argument("regional danger state domain");
  const auto check = [&](const RegionalDangerActor &a) {
    if (a.slot < 0 || a.slot >= 125 || a.owner < 0 || a.owner >= 8 ||
        a.region < -1 || (a.region >= 0 && unsigned(a.region) >= g.size()) ||
        a.width < 1 || a.width > 2 || a.height < 1 || a.height > 5 ||
        a.movementKind > 3 || a.type >= 28)
      throw std::invalid_argument("regional danger actor domain");
  };
  std::set<int> slots;
  for (auto &a : in.actors) {
    check(a);
    if (!slots.insert(a.slot).second)
      throw std::invalid_argument("danger duplicate actor");
  }
  if (s.commander != -1 && !slots.count(s.commander))
    throw std::invalid_argument("danger pending commander reference");
  for (unsigned i = 0; i < 8; ++i) {
    if (in.commander[i] != -1 && !slots.count(in.commander[i]))
      throw std::invalid_argument("danger commander reference");
    if (in.enabled[i] && in.commander[i] < 0)
      throw std::invalid_argument("danger enabled owner has no commander");
  }
  for (auto &h : in.hazards)
    if (h.radius < 0)
      throw std::invalid_argument("danger hazard domain");
  for (auto &memories : in.memories) {
    int last = -1;
    if (memories.size() > 125)
      throw std::invalid_argument("danger memory bound");
    for (auto &m : memories) {
      check(m.actor);
      if (m.key <= last || m.key == INT32_MAX)
        throw std::invalid_argument("danger memory order");
      last = m.key;
    }
  }
  for (auto &e : in.effects)
    if (e.owner < 0 || e.owner >= 8 || e.radius < 0)
      throw std::invalid_argument("danger effect domain");
  for (auto &node : g)
    for (auto &e : node.connections)
      if (e.cost && (e.destination < 0 || unsigned(e.destination) >= g.size()))
        throw std::invalid_argument("danger edge reference");
}
} // namespace mnm::reconstruction::combat
