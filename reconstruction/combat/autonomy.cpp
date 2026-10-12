#include "autonomy.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace mnm::reconstruction::combat {
namespace {
std::int32_t signedBits(std::uint32_t n) {
  return n <= 0x7fffffffU ? std::int32_t(n)
                          : std::int32_t(std::int64_t(n) - 0x100000000LL);
}
std::int32_t add32(std::int32_t a, std::int32_t b) {
  return signedBits(std::uint32_t(a) + std::uint32_t(b));
}
std::int32_t multiply32(std::int32_t a, std::int32_t b) {
  return signedBits(std::uint32_t(a) * std::uint32_t(b));
}
std::int32_t wrap(std::int32_t n, std::int32_t size) {
  if (size <= 0)
    throw std::invalid_argument("AI map dimension");
  n %= size;
  return n < 0 ? n + size : n;
}
Cell canonical(Cell p, Extent e) {
  return {wrap(p.x, e.x), wrap(p.y, e.y), p.z};
}
Cell add(Cell a, Cell b) {
  return {add32(a.x, b.x), add32(a.y, b.y), add32(a.z, b.z)};
}
bool leader(std::uint32_t kind) {
  return kind == 0 || kind == 24 || kind == 25 || kind == 26;
}
void owner(const AutonomousCreature &a) {
  if (a.ai.owner < 0 || a.ai.owner >= 8)
    throw std::invalid_argument("AI owner");
}
bool hostile(const AutonomousCreature &a, const AutonomousCreature &b,
             const AutonomousWorld &w) {
  owner(a);
  owner(b);
  return w.relation[a.ai.owner][b.ai.owner] == 0;
}
bool rangedThreat(const AutonomousCreature &a) {
  return (a.ai.type == 18 && a.ai.flying) || a.ai.ranged ||
         a.animationKind == 2;
}
} // namespace

std::int32_t localHazard(const AutonomousCreature &a, Cell p, Extent e,
                         const AiGeometry &g) {
  p = canonical(p, e);
  bool dry = !g.hazardous(a, p);
  if (dry) {
    auto x = p.x, y = p.y;
    // The original advances X across rows, without resetting it at each row.
    for (std::uint32_t row = 0; row < a.width; ++row) {
      for (std::uint32_t col = 0; col < a.width; ++col) {
        if (g.cell({x, y, p.z}).tilePropertyA0)
          dry = false;
        x = wrap(x + 1, e.x);
      }
      y = wrap(y + 1, e.y);
    }
  }
  std::int32_t score = 0;
  for (unsigned i = 0; i < g.hazardOffsets.size(); ++i) {
    auto q = add(p, g.hazardOffsets[i]);
    if (q.z <= 0 || q.z >= e.z)
      continue;
    q = canonical(q, e);
    if (g.hazardous(a, q)) {
      if (!dry && a.hazardSensitive && !a.protection8c9)
        score += i == 0 ? 8 : 1;
    } else if (g.cell(q).flags & 0x30)
      score += i == 0 ? 10 : 1;
  }
  return score;
}

bool targetAdmitted(const AutonomousCreature &a, const AutonomousCreature &b,
                    Extent e, const AiGeometry &g) {
  auto p = canonical(a.ai.position, e), target = canonical(b.ai.position, e);
  if (a.ai.flying) {
    auto dx = p.x - target.x, dy = p.y - target.y;
    if (std::abs(dx) > e.x / 2)
      dx += dx > 0 ? -e.x : e.x;
    if (std::abs(dy) > e.y / 2)
      dy += dy > 0 ? -e.y : e.y;
    if (dx < 0)
      target.x = wrap(target.x - std::int32_t(a.width), e.x);
    else if (dx > 0)
      target.x = wrap(target.x + std::int32_t(b.width), e.x);
    else if (dy < 0)
      target.y = wrap(target.y - std::int32_t(a.width), e.y);
    else if (dy > 0)
      target.y = wrap(target.y + std::int32_t(b.width), e.y);
  }
  auto distance = creatureDistance(p, target, e);
  std::optional<Cell> previous;
  while (!(p == target)) {
    if (g.reach(a, b, p))
      return true;
    std::optional<Cell> best;
    auto bestDistance = distance;
    for (auto offset : g.routeOffsets) {
      auto q = canonical(add(p, offset), e);
      if (q.z < 0 || q.z >= e.z || (previous && q == *previous))
        continue;
      auto candidate = creatureDistance(q, target, e);
      if (candidate < bestDistance && g.legal(a, p, q, 1)) {
        best = q;
        bestDistance = candidate;
      }
    }
    if (!best)
      return false;
    previous = p;
    p = *best;
    distance = bestDistance;
  }
  return false;
}

std::vector<AutonomousEvent>
escapeLocal(const AutonomousCreature &a, Extent e, const AiGeometry &g,
            const std::function<bool(Cell)> &move) {
  std::optional<unsigned> best;
  std::int32_t score = 0;
  for (unsigned i = 0; i < g.escapeOffsets.size(); ++i) {
    auto q = canonical(add(a.ai.position, g.escapeOffsets[i]), e);
    if (q.z <= 0 || q.z >= e.z || (i && !g.legal(a, a.ai.position, q, 0)))
      continue;
    auto value = localHazard(a, q, e, g);
    if (!best || value < score) {
      best = i;
      score = value;
    }
  }
  if (!best || *best == 0)
    return {{AutonomousEffect::reset, {}, 0}};
  auto q = canonical(add(a.ai.position, g.escapeOffsets[*best]), e);
  std::vector<AutonomousEvent> events{{AutonomousEffect::move, q, 0}};
  if (move(q))
    events.push_back({AutonomousEffect::beginMotion, {}, 0});
  return events;
}

bool chooseInteraction(AutonomousCreature &a, const AutonomousWorld &w,
                       const std::vector<AiObject> &objects,
                       const AiGeometry &g) {
  owner(a);
  if (leader(a.ai.kind) && (w.human[a.ai.owner] || a.inventoryCount >= 42))
    return false;
  if (a.block7c || !(a.configFlags & 4) || !w.ownerEligible[a.ai.owner])
    return false;
  for (const auto &object : objects) {
    auto kind = object.kind;
    if (!(kind <= 4 || kind == 6 || kind == 19))
      continue;
    if (kind == 19 && !leader(a.ai.kind))
      continue;
    auto p = object.position;
    if (creatureDistance(a.ai.position, p, w.dimensions) > 10 ||
        g.cell(p).occupant != 0xffff || !g.objectReachable(a, a.ai.position, p))
      continue;
    if (kind != 19 &&
        std::any_of(w.creatures.begin(), w.creatures.end(), [&](const auto &b) {
          return b.ai.active && b.ai.health && b.ai.owner == a.ai.owner &&
                 b.destination == p;
        }))
      continue;
    a.chosenDestination = p;
    return true;
  }
  return false;
}

bool chooseDestination(AutonomousCreature &a, const AutonomousWorld &w,
                       const std::vector<AiObject> &objects,
                       const AiGeometry &g) {
  owner(a);
  if ((leader(a.ai.kind) && w.human[a.ai.owner]) || a.block7c ||
      !w.ownerEligible[a.ai.owner])
    return false;
  for (const auto &object : objects) {
    auto p = object.position;
    if (object.kind != 61 ||
        creatureDistance(a.ai.position, p, w.dimensions) > 10 ||
        g.cell(p).occupant != 0xffff || !g.objectReachable(a, a.ai.position, p))
      continue;
    bool admitted = true;
    for (const auto &b : w.creatures) {
      if (!b.ai.active || !b.ai.health)
        continue;
      if (b.ai.slot != a.ai.slot && hostile(a, b, w)) {
        if (!b.ai.visible[a.ai.owner])
          continue;
        auto distance = creatureDistance(b.ai.position, p, w.dimensions);
        auto fear = a.ai.enemyRadius.at(b.ai.type);
        if (fear < 0 && std::int64_t(distance) <= -2 * std::int64_t(fear)) {
          admitted = false;
          break;
        }
        auto evasion = a.ai.retaliateThreshold.at(b.ai.type);
        if (rangedThreat(b) && evasion < 0 &&
            distance <= (b.animationKind == 2 ? 40 : 2 * b.rangedDistance)) {
          admitted = false;
          break;
        }
      } else if ((b.ai.behavior == 2 ||
                  (b.ai.behavior == 11 && b.substate == 9)) &&
                 b.destination == p) {
        admitted = false;
        break;
      }
    }
    if (admitted) {
      a.chosenDestination = p;
      return true;
    }
  }
  return false;
}

std::vector<AutonomousEvent>
healFromInventory(AutonomousCreature &a, const AutonomousWorld &w,
                  const std::function<void(InventoryEntry &)> &use,
                  const std::function<void(std::uint32_t)> &refresh) {
  owner(a);
  if (!leader(a.ai.kind) || a.block7c || w.human[a.ai.owner] ||
      !a.inventoryCount || a.ai.health >= a.ai.maximumHealth)
    return {};
  for (unsigned i = 0; i < a.inventory.size(); ++i) {
    auto &item = a.inventory[i];
    if (!item.available || !(item.kind <= 4 || item.kind == 6))
      continue;
    auto sum = a.ai.health + item.magnitude * 4U;
    auto half = signedBits(a.ai.maximumHealth) / 2;
    if (sum > a.ai.maximumHealth && a.ai.health >= std::uint32_t(half))
      continue;
    if (!item.count || item.kind > 84 || !item.available)
      return {};
    std::vector<AutonomousEvent> events{
        {AutonomousEffect::useInventory, {}, std::int32_t(i)}};
    use(item);
    if (a.ai.owner == w.localOwner) {
      events.push_back(
          {AutonomousEffect::refreshInventory, {}, std::int32_t(i)});
      if (refresh)
        refresh(i);
    }
    if (!item.count && item.kind < 84) {
      item.available = 0;
      --a.inventoryCount;
    }
    return events;
  }
  return {};
}

bool immediateOpponent(AutonomousCreature &a, const AutonomousWorld &w,
                       const AiGeometry &g,
                       const std::function<std::uint32_t()> &random) {
  owner(a);
  if (a.mode || (a.form && !a.formCapability && a.animationKind != 3) ||
      a.blockDE3 || a.block71c || a.block723)
    return false;
  std::optional<std::uint32_t> best;
  std::int32_t score = 0;
  for (const auto &b : w.creatures) {
    if (!b.ai.active || b.ai.slot == a.ai.slot || !hostile(a, b, w) ||
        !b.ai.health)
      continue;
    if (!b.ai.visible[a.ai.owner] && !(a.specialVision && b.specialVision))
      continue;
    if (!g.reach(a, b, a.ai.position))
      continue;
    if (b.ai.motion &&
        (b.ai.behavior == 3 ||
         (b.ai.behavior == 11 && (b.substate == 1 || b.substate == 5))) &&
        b.ai.target == std::int32_t(a.ai.slot))
      continue;
    auto hth = a.ai.hth.at(b.ai.type);
    if (b.ai.physical && !a.ai.physical)
      hth /= 3;
    auto base =
        add32(std::max(0, signedBits(500U - b.ai.health)), multiply32(5, hth));
    if (leader(b.ai.kind))
      base = add32(base, 100);
    auto value = add32(base, std::int32_t((random() & 0x7fffffffU) % 50));
    if (value > score) {
      best = b.ai.slot;
      score = value;
    }
  }
  if (!best)
    return false;
  a.ai.target = std::int32_t(*best);
  return true;
}

std::vector<AutonomousEvent> orientToThreat(const AutonomousCreature &a,
                                            const AutonomousWorld &w,
                                            const AiGeometry &g) {
  owner(a);
  if (a.ai.behavior != 1 || a.ai.type == 22)
    return {};
  const AutonomousCreature *best = nullptr;
  std::int32_t distance = 16;
  for (const auto &b : w.creatures) {
    if (!b.ai.active || b.ai.slot == a.ai.slot || !hostile(a, b, w) ||
        !b.ai.health || !b.ai.visible[a.ai.owner])
      continue;
    auto d = creatureDistance(a.ai.position, b.ai.position, w.dimensions);
    if (d >= 16)
      continue;
    auto aim = add(b.finePosition, {b.aimXY, b.aimXY, b.aimZ});
    if (g.lineOfSight(a, aim) && d < distance) {
      best = &b;
      distance = d;
    }
  }
  if (!best)
    return {};
  auto aim = add(best->finePosition, {best->aimXY, best->aimXY, 0});
  return {{AutonomousEffect::orient, {aim.x, aim.y, 0}, 0}};
}
bool assistAdmitted(const AutonomousCreature &ally,
                    const AutonomousCreature &enemy, Extent e) {
  auto dx = ally.ai.position.x - enemy.ai.position.x,
       dy = ally.ai.position.y - enemy.ai.position.y;
  if (std::abs(dx) > e.x / 2)
    dx += dx > 0 ? -e.x : e.x;
  if (std::abs(dy) > e.y / 2)
    dy += dy > 0 ? -e.y : e.y;
  if (std::abs(ally.ai.position.z - enemy.ai.position.z) > 1)
    return false;
  auto lo = ally.width == 2 ? -2 : -1, hi = enemy.width == 2 ? 2 : 1;
  return dx >= lo && dx <= hi && dy >= lo && dy <= hi;
}
void attackResponse(AutonomousCreature &a, const AutonomousWorld &w) {
  if (a.ai.target == -1)
    return;
  if (a.ai.target < 0 || std::size_t(a.ai.target) >= w.creatures.size())
    throw std::invalid_argument("AI target");
  auto &b = w.creatures[a.ai.target];
  auto d = creatureDistance(a.destination, b.ai.position, w.dimensions);
  if (d > 3 && std::int64_t(d) * 3 >=
                   creatureDistance(a.ai.position, b.ai.position, w.dimensions))
    a.ai.marker = 0;
}
namespace {
AiWorld targetWorld(const AutonomousCreature &a, const AutonomousWorld &w) {
  AiWorld result;
  result.dimensions = w.dimensions;
  result.relation = w.relation;
  result.human = w.human;
  for (const auto &b : w.creatures)
    result.creatures.push_back(b.ai.slot == a.ai.slot ? a.ai : b.ai);
  return result;
}
const AutonomousCreature &actor(const AutonomousWorld &w, std::uint32_t slot) {
  if (slot >= w.creatures.size() || w.creatures[slot].ai.slot != slot)
    throw std::invalid_argument("AI slot");
  return w.creatures[slot];
}
bool reserve(AutonomousCreature &a, const Selection &selection) {
  if (!selection.target)
    return false;
  a.ai.target = std::int32_t(*selection.target);
  a.range = selection.range;
  return true;
}
bool edgeAdmitted(const AutonomousCreature &a, const AiRegionEdge &edge) {
  if (!edge.active || a.width > 2 || a.height > 127)
    return false;
  auto bound = [&](unsigned at) {
    return std::int32_t(edge.bounds.at(at + a.width));
  };
  if ((edge.bounds[8] && bound(8) >= std::int32_t(a.height)) ||
      (a.flag104 && edge.bounds[11] && bound(11) >= std::int32_t(a.height)))
    return true;
  switch (a.movementKind) {
  case 3:
    return edge.bounds[17] && bound(17) >= std::int32_t(a.height);
  case 2:
    return edge.bounds[14] && bound(14) >= std::int32_t(a.height);
  case 1:
    return edge.bounds[20] && bound(20) >= std::int32_t(a.height) &&
           bound(22) <= std::int32_t(a.height);
  default:
    return false;
  }
}
} // namespace
void setAutonomousSubstate(AutonomousCreature &a, AutonomousWorld &w,
                           std::uint32_t state, const AutonomousServices &s) {
  if (a.ai.type == 27) {
    s.reset(a);
    return;
  }
  if (a.ai.type == 22 || a.ai.pending == 8 || a.ai.behavior == 15 ||
      a.ai.behavior == 6 || a.ai.behavior == 8 || a.ai.behavior == 16 ||
      a.mode || a.block784 || a.blockDE3 ||
      (a.form && !a.formCapability && a.animationKind != 3) || a.block71c ||
      a.block723)
    return;
  a.ai.readiness = 0;
  if (a.ai.engaged && (a.ai.action == 10 || a.ai.action == 20)) {
    auto target = a.ai.action == 10 ? a.ai.target : a.alternateTarget;
    if (target < 0 || std::size_t(target) >= w.creatures.size())
      throw std::invalid_argument("engaged AI target");
    auto &b = w.creatures[target];
    a.ai.engaged = 0;
    auto incoming = signedBits(b.ai.incoming - 1U);
    b.ai.incoming = incoming < 0 ? 0 : std::uint32_t(incoming);
  }
  a.ai.marker = 0;
  a.substate = state;
  a.ai.countdown = state;
  s.behavior(a, 11);
}
bool priorityResponse(AutonomousCreature &a, AutonomousWorld &w,
                      const AutonomousServices &s) {
  owner(a);
  if (leader(a.ai.kind) && w.human[a.ai.owner])
    return false;
  if (a.ai.recent && a.ai.age > 80) {
    auto selection =
        selectRetaliation(a.ai, targetWorld(a, w), [&](unsigned slot) {
          return targetAdmitted(a, actor(w, slot), w.dimensions, s.geometry);
        });
    if (!reserve(a, selection))
      return false;
    setAutonomousSubstate(a, w, selection.substate, s);
    return true;
  }
  if (!a.ai.flag815)
    return false;
  for (const auto &threat : s.threats) {
    if (threat.spell != 70 || !s.geometry.threatReaches(a, threat))
      continue;
    if (threat.owner < 0 || threat.owner >= 8)
      throw std::invalid_argument("AI threat owner");
    if (w.relation[threat.owner][a.ai.owner] || a.region == -1)
      continue;
    if (a.region < 0 || std::size_t(a.region) >= s.regions.size())
      throw std::invalid_argument("AI region");
    const AiRegion *best = nullptr;
    std::int32_t distance = 0;
    for (const auto &edge : s.regions[a.region].edges) {
      if (!edgeAdmitted(a, edge))
        continue;
      if (edge.destination < 0 ||
          std::size_t(edge.destination) >= s.regions.size())
        throw std::invalid_argument("AI region edge");
      const auto &region = s.regions[edge.destination];
      auto d = creatureDistance(region.position, threat.position, w.dimensions);
      if (d > distance) {
        best = &region;
        distance = d;
      }
    }
    if (best) {
      s.move(a, best->position);
      return true;
    }
  }
  return false;
}
void autonomousDecision(AutonomousCreature &a, AutonomousWorld &w,
                        const AutonomousServices &s) {
  owner(a);
  const auto &g = s.geometry;
  auto admission = [&](unsigned slot) {
    return targetAdmitted(a, actor(w, slot), w.dimensions, g);
  };
  auto apply = [&](Selection result, unsigned state) {
    if (!reserve(a, result))
      return false;
    setAutonomousSubstate(a, w, state, s);
    return true;
  };
  if ((a.ai.behavior == 1 || a.ai.behavior == 10) && !a.ai.pending) {
    if (a.ai.type != 22) {
      if (immediateOpponent(a, w, g, s.random)) {
        if (a.ai.type == 27)
          s.reset(a);
        else {
          a.ai.readiness = 0;
          s.prepareInteraction(a);
          a.ai.marker = 0;
          s.behavior(a, 9);
        }
      } else {
        for (auto event : orientToThreat(a, w, g))
          s.orient(event.position);
        auto flags = g.cell(a.ai.position).flags;
        if (!((flags & 0x30) && !(flags & 0x10)) && !a.ai.gateFC) {
          if (!priorityResponse(a, w, s) &&
              !apply(selectCorpse(a.ai, targetWorld(a, w), true, admission),
                     3) &&
              !apply(selectCorpse(a.ai, targetWorld(a, w), false, admission),
                     4) &&
              !apply(selectAssist(
                         a.ai, targetWorld(a, w),
                         [&](unsigned ally, unsigned enemy) {
                           return assistAdmitted(actor(w, ally),
                                                 actor(w, enemy), w.dimensions);
                         },
                         admission),
                     5) &&
              !apply(selectEnemy(a.ai, targetWorld(a, w), false, admission),
                     2) &&
              !apply(selectEnemy(a.ai, targetWorld(a, w), true, admission),
                     1)) {
            auto hazardScore = localHazard(a, a.ai.position, w.dimensions, g);
            if (hazardScore &&
                add32(add32(a.ai.bravery, multiply32(hazardScore, 2)),
                      int((s.random() & 0x7fffffffU) % 35)) > 45)
              setAutonomousSubstate(a, w, 6, s);
            else if (chooseInteraction(a, w, s.objects, g))
              setAutonomousSubstate(a, w, 8, s);
            else if (chooseDestination(a, w, s.objects, g))
              setAutonomousSubstate(a, w, 9, s);
          }
        }
        if (a.ai.behavior == 1)
          a.ai.history = 0;
      }
    }
  } else if (a.ai.behavior == 2 && a.ai.type != 22 &&
             w.ownerEligible[a.ai.owner] && !w.human[a.ai.owner] &&
             !a.ai.flag704) {
    if (chooseInteraction(a, w, s.objects, g)) {
      setAutonomousSubstate(a, w, 8, s);
      a.ai.dirty932 = 1;
    }
  }
  a.ai.age = signedBits(std::uint32_t(a.ai.age) - 20U);
  if (a.ai.age <= 0)
    a.ai.recent = 0;
  a.ai.flag815 = 0;
  healFromInventory(
      a, w, [&](InventoryEntry &entry) { s.useInventory(a, entry); },
      s.refreshInventory);
  if (a.ai.behavior == 3 ||
      (a.ai.behavior == 11 && (a.substate == 1 || a.substate == 5)))
    attackResponse(a, w);
}
namespace {
void prepareRoute(AutonomousCreature &a, const AutonomousServices &s) {
  if (a.ai.marker)
    s.beginMotion(a);
  else {
    s.action(a, 15);
    if (!a.routePhase)
      a.routePhase = 1;
  }
}
void returnHome(AutonomousCreature &a, const AutonomousServices &s) {
  a.ai.marker = 0;
  s.action(a, 15);
  a.routePhase = 1;
  a.substate = 7;
  a.ai.countdown = 7;
}
void attackAtHome(AutonomousCreature &a, AutonomousWorld &w,
                  const AutonomousServices &s, bool assist) {
  owner(a);
  if (a.ai.target != -1) {
    const auto &b = actor(w, unsigned(a.ai.target));
    if (b.ai.owner == a.ai.owner || !b.ai.visible[a.ai.owner] || !b.ai.health)
      a.ai.target = -1;
  }
  if (a.ai.target == -1) {
    auto admit = [&](unsigned slot) {
      return targetAdmitted(a, actor(w, slot), w.dimensions, s.geometry);
    };
    auto selection =
        assist ? selectAssist(
                     a.ai, targetWorld(a, w),
                     [&](unsigned ally, unsigned enemy) {
                       return assistAdmitted(actor(w, ally), actor(w, enemy),
                                             w.dimensions);
                     },
                     admit)
               : selectEnemy(a.ai, targetWorld(a, w), true, admit);
    if (reserve(a, selection))
      a.ai.marker = 0;
  }
  if (a.ai.target == -1) {
    returnHome(a, s);
    return;
  }
  auto &b = w.creatures.at(unsigned(a.ai.target));
  if (creatureDistance(a.ai.home, b.ai.position, w.dimensions) > a.range) {
    returnHome(a, s);
    return;
  }
  if (s.geometry.reach(a, b, a.ai.position)) {
    if (a.ai.incoming) {
      s.action(a, 15);
      a.cooldown = 1;
      return;
    }
    s.faceTarget(a, b);
    if (b.ai.motion) {
      b.targetMovingAttacker = a.ai.slot + 1;
      s.action(a, 15);
      a.cooldown = 1;
    } else
      s.action(a, 10);
  } else if (!(a.ai.type == 18 && a.ai.flying) && a.rangedDistance) {
    a.castSpell = a.selectedSpell;
    a.castSource = std::int32_t(a.ai.slot);
    a.castTarget = a.ai.target;
    s.rangedAttack(a);
  } else
    prepareRoute(a, s);
}
void corpseBehavior(AutonomousCreature &a, AutonomousWorld &w,
                    const AutonomousServices &s, bool troll) {
  auto choose = [&] {
    return reserve(
        a, selectCorpse(a.ai, targetWorld(a, w), troll, [&](unsigned slot) {
          return targetAdmitted(a, actor(w, slot), w.dimensions, s.geometry);
        }));
  };
  if (a.ai.target == -1) {
    if (choose())
      a.ai.marker = 0;
  } else if (troll && actor(w, unsigned(a.ai.target)).ai.health) {
    a.ai.target = -1;
    if (choose())
      a.ai.marker = 0;
  }
  if (a.ai.target == -1) {
    returnHome(a, s);
    return;
  }
  auto &b = w.creatures.at(unsigned(a.ai.target));
  if (!s.geometry.reach(a, b, a.ai.position)) {
    prepareRoute(a, s);
    return;
  }
  auto action = troll ? 17U : 16U;
  if (a.ai.action != action) {
    s.orient({b.finePosition.x + b.aimXY, b.finePosition.y + b.aimXY, 0});
    s.action(a, action);
    return;
  }
  auto percent = troll ? 5U : 25U;
  auto numerator = signedBits(a.ai.maximumHealth * percent);
  s.changeHealth(a, numerator / 100);
  if (troll)
    b.ai.corpseA = signedBits(b.ai.corpseA) > 60 ? b.ai.corpseA - 60U : 1U;
  else
    b.ai.corpseClaim = 1;
  s.reset(a);
}
} // namespace
bool retreatLocal(AutonomousCreature &a, const AutonomousWorld &w,
                  const AutonomousServices &s) {
  const auto &b = actor(w, unsigned(a.ai.target));
  auto distance = creatureDistance(a.ai.position, b.ai.position, w.dimensions);
  auto limit = distance > 16 ? distance + 4 : 20;
  std::int32_t current = 0, bestDistance = 0, bestScore = 0;
  std::optional<unsigned> best;
  for (unsigned i = 0; i < s.geometry.retreatOffsets.size(); ++i) {
    auto offset = s.geometry.retreatOffsets[i];
    auto p = canonical(add(a.ai.position, offset), w.dimensions);
    if (p.z <= 0 || p.z >= w.dimensions.z)
      continue;
    if (i && (!s.geometry.legal(a, a.ai.position, p, 0) ||
              (s.geometry.cell(p).flags & 0x30)))
      continue;
    auto nearest = limit;
    for (const auto &enemy : w.creatures) {
      if (!enemy.ai.active || enemy.ai.slot == a.ai.slot ||
          !hostile(a, enemy, w) || !enemy.ai.health ||
          !enemy.ai.visible[a.ai.owner])
        continue;
      auto old =
          creatureDistance(a.ai.position, enemy.ai.position, w.dimensions);
      if (old > limit || old > distance * 2)
        continue;
      nearest = std::min(nearest,
                         creatureDistance(p, enemy.ai.position, w.dimensions));
    }
    if (!i) {
      current = nearest;
      continue;
    }
    auto axes = int(offset.x != 0) + int(offset.y != 0) + int(offset.z != 0);
    auto score = (nearest - current) * 10;
    if (axes >= 1 && axes <= 3)
      score /= (axes + 1);
    if (!best || score > bestScore) {
      best = i;
      bestScore = score;
      bestDistance = nearest;
    }
  }
  if (!best || bestDistance <= current)
    return false;
  auto p = canonical(add(a.ai.position, s.geometry.retreatOffsets[*best]),
                     w.dimensions);
  if (!s.routeMove(a, p))
    return false;
  s.beginMotion(a);
  return true;
}
void autonomousBehavior(AutonomousCreature &a, AutonomousWorld &w,
                        const AutonomousServices &s) {
  switch (a.substate) {
  case 1:
    attackAtHome(a, w, s, false);
    break;
  case 2: {
    if (a.ai.target == -1 &&
        reserve(a,
                selectEnemy(a.ai, targetWorld(a, w), false, [&](unsigned slot) {
                  return targetAdmitted(a, actor(w, slot), w.dimensions,
                                        s.geometry);
                })))
      a.ai.marker = 0;
    if (a.ai.target == -1 ||
        creatureDistance(a.ai.position,
                         actor(w, unsigned(a.ai.target)).ai.position,
                         w.dimensions) > a.range ||
        !retreatLocal(a, w, s))
      s.reset(a);
    break;
  }
  case 3:
    corpseBehavior(a, w, s, true);
    break;
  case 4:
    corpseBehavior(a, w, s, false);
    break;
  case 5:
    attackAtHome(a, w, s, true);
    break;
  case 6: {
    // Call consequences in original order; movement failure does not reset.
    auto result = escapeLocal(a, w.dimensions, s.geometry,
                              [&](Cell p) { return s.routeMove(a, p); });
    for (auto event : result) {
      if (event.kind == AutonomousEffect::reset)
        s.reset(a);
      else if (event.kind == AutonomousEffect::beginMotion)
        s.beginMotion(a);
    }
    break;
  }
  case 7:
    if (a.ai.position == a.ai.home)
      s.reset(a);
    else
      prepareRoute(a, s);
    break;
  case 8:
  case 9: {
    const bool interaction = a.substate == 8;
    if (a.ai.position == a.chosenDestination) {
      if (interaction) {
        returnHome(a, s);
        a.ai.dirty932 = 0;
      } else
        s.reset(a);
      break;
    }
    auto occupant = s.geometry.cell(a.chosenDestination).occupant;
    if (occupant != 0xffff && occupant != a.ai.slot) {
      returnHome(a, s);
      if (interaction)
        a.ai.dirty932 = 0;
    } else
      prepareRoute(a, s);
    break;
  }
  default:
    break;
  }
}
} // namespace mnm::reconstruction::combat
