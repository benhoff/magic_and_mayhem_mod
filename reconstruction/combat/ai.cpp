#include "ai.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
std::int32_t bits(std::uint32_t n) {
  return std::int32_t(n <= 0x7fffffffU ? std::int64_t(n)
                                       : std::int64_t(n) - 0x100000000LL);
}
std::int32_t add(std::int32_t a, std::int32_t b) {
  return bits(std::uint32_t(a) + std::uint32_t(b));
}
std::int32_t mul(std::int32_t a, std::int32_t b) {
  return bits(std::uint32_t(a) * std::uint32_t(b));
}
std::int32_t negate(std::int32_t a) { return bits(0U - std::uint32_t(a)); }
std::int32_t divide(std::int32_t a, std::int32_t b) {
  if (!b || (a == std::numeric_limits<std::int32_t>::min() && b == -1))
    throw std::invalid_argument("original AI divide fault domain");
  return a / b;
}
bool ordinary(const AiCreature &a) {
  return a.kind != 0 && a.kind != 24 && a.kind != 25 && a.kind != 26;
}
void validate(const AiCreature &a, const AiWorld &w) {
  if (a.type >= 28 || a.owner < 0 || a.owner >= 8 || !a.maximumHealth ||
      w.creatures.size() > 256)
    throw std::invalid_argument("outside AI selector domain");
  for (const auto &b : w.creatures)
    if (b.type >= 28 || b.owner < 0 || b.owner >= 8)
      throw std::invalid_argument("invalid AI candidate");
}
bool hostile(const AiCreature &a, const AiCreature &b, const AiWorld &w) {
  return !w.relation[unsigned(a.owner)][unsigned(b.owner)];
}
bool enemy(const AiCreature &a, const AiCreature &b, const AiWorld &w) {
  return b.active && a.slot != b.slot && b.health && hostile(a, b, w) &&
         b.visible[unsigned(a.owner)];
}
std::int32_t radius(const AiCreature &a, const AiCreature &b) {
  return a.bloodLust ? 10 : a.enemyRadius[b.type];
}
bool rangedThreat(const AiCreature &a, const AiCreature &b) {
  return b.target == std::int32_t(a.slot) && !(b.type == 18 && b.flying) &&
         b.ranged && (b.behavior == 10 || (b.behavior == 3 && !b.motion));
}
std::int32_t expand(std::int32_t n) { return divide(mul(n, 3), 2); }
std::uint32_t unsignedDivide(std::uint32_t n, std::uint32_t d) {
  if (!d)
    throw std::invalid_argument("original AI zero unsigned denominator");
  return n / d;
}
const AiCreature *find(const AiWorld &w, std::int32_t slot) {
  for (const auto &a : w.creatures)
    if (std::int32_t(a.slot) == slot)
      return &a;
  return nullptr;
}
} // namespace
Selection selectEnemy(const AiCreature &a, const AiWorld &w, bool homeRadius,
                      const std::function<bool(std::uint32_t)> &admit) {
  validate(a, w);
  Selection best;
  best.value = -1;
  if (homeRadius ? !ordinary(a) : (!ordinary(a) && w.human[unsigned(a.owner)]))
    return best;
  for (const auto &b : w.creatures) {
    if (!enemy(a, b, w))
      continue;
    auto range = mul(homeRadius ? 2 : -2, radius(a, b));
    if (a.history == b.slot + 1)
      range = expand(range);
    if (rangedThreat(a, b))
      range = expand(range);
    if (homeRadius)
      range = divide(mul(std::int32_t(a.health), range),
                     std::int32_t(a.maximumHealth));
    if (creatureDistance(a.home, b.position, w.dimensions) > range)
      continue;
    auto score = add(
        100, negate(creatureDistance(a.position, b.position, w.dimensions)));
    if (!homeRadius) {
      auto ownHth = a.hth[b.type];
      if (b.physical && !a.physical)
        ownHth /= 3;
      auto otherHth = b.hth[a.type];
      if (a.physical && !b.physical)
        otherHth /= 3;
      auto difference =
          bits(unsignedDivide(b.health, std::uint32_t(ownHth) + 1U) -
               unsignedDivide(a.health, std::uint32_t(otherHth) + 1U));
      score = add(score, mul(difference, 2));
    }
    if (score <= best.value || !admit || !admit(b.slot))
      continue;
    best = {b.slot, score, range};
  }
  return best;
}
Selection selectCorpse(const AiCreature &a, const AiWorld &w, bool troll,
                       const std::function<bool(std::uint32_t)> &admit) {
  validate(a, w);
  Selection best;
  best.value = -1;
  if (a.type != (troll ? 21U : 10U))
    return best;
  for (const auto &b : w.creatures) {
    if (!b.active || b.health || (!b.corpseA && !b.corpseB) ||
        !b.visible[unsigned(a.owner)])
      continue;
    if (troll) {
      if (b.physical)
        continue;
    } else if (a.slot == b.slot || !hostile(a, b, w) || b.corpseClaim)
      continue;
    // Native callback retains original per-selector admission order.
    const auto distance = creatureDistance(a.home, b.position, w.dimensions);
    if (distance > 8 || !admit || !admit(b.slot))
      continue;
    auto score = add(
        100, negate(creatureDistance(a.position, b.position, w.dimensions)));
    if (score > best.value)
      best = {b.slot, score, 8};
  }
  return best;
}
Selection
selectAssist(const AiCreature &a, const AiWorld &w,
             const std::function<bool(std::uint32_t, std::uint32_t)> &assist,
             const std::function<bool(std::uint32_t)> &admit) {
  validate(a, w);
  Selection best;
  best.value = -1;
  if (!ordinary(a))
    return best;
  for (const auto &ally : w.creatures) {
    if (!ally.active || !ally.health || ally.owner != a.owner ||
        ally.slot == a.slot ||
        (!ally.engaged && !ally.incoming && (!ally.recent || a.age <= 80)))
      continue;
    const auto *b = find(w, ally.target);
    if (!b || !b->active || !b->health || b->slot == a.slot ||
        !hostile(a, *b, w) || !assist || !assist(ally.slot, b->slot))
      continue;
    auto range = add(mul(a.allyRadius[ally.type], 2), radius(a, *b) / 2);
    range = divide(mul(range, std::int32_t(a.health)),
                   std::int32_t(a.maximumHealth));
    if (creatureDistance(a.home, ally.position, w.dimensions) > range)
      continue;
    const auto distance = creatureDistance(a.home, b->position, w.dimensions),
               score = add(100, negate(distance));
    if (score <= best.value || !admit || !admit(b->slot))
      continue;
    best = {b->slot, score, distance};
  }
  return best;
}
Selection selectRetaliation(const AiCreature &a, const AiWorld &w,
                            const std::function<bool(std::uint32_t)> &admit) {
  validate(a, w);
  Selection best;
  best.value = -1;
  if ((!ordinary(a) && w.human[unsigned(a.owner)]) || !a.recent || a.age <= 80)
    return best;
  for (const auto &b : w.creatures) {
    if (!enemy(a, b, w) || (!rangedThreat(a, b) && a.recent != b.slot + 1))
      continue;
    const auto threshold = a.retaliateThreshold[b.type];
    const auto strength = unsignedDivide(a.health * 10U, a.maximumHealth);
    const auto magnitude = threshold < 0 ? std::uint32_t(negate(threshold))
                                         : std::uint32_t(threshold);
    if (strength > magnitude)
      continue;
    const unsigned substate = threshold >= 0 && admit && admit(b.slot) ? 1 : 2;
    const auto distance =
        creatureDistance(a.position, b.position, w.dimensions);
    const auto score = add(100, negate(distance));
    if (score <= best.value)
      continue;
    best = {b.slot, score, add(distance, int(substate * 2 + 2)), substate};
  }
  return best;
}
DecisionResult decide(AiCreature a, const DecisionInputs &p,
                      const std::function<std::uint32_t()> &random) {
  DecisionResult r{a, {}, {}};
  auto emit = [&](AiEffect k, int v = 0) { r.events.push_back({k, v}); };
  auto selector = [&](Selector s, bool result) {
    emit(AiEffect::selector, int(s));
    return result;
  };
  if ((a.behavior == 1 || a.behavior == 10) && !a.pending) {
    if (a.type != 22) {
      emit(AiEffect::immediate);
      if (p.immediate) {
        if (a.type == 27)
          emit(AiEffect::reset);
        else {
          r.actor.readiness = 0;
          emit(AiEffect::prepareInteraction);
          r.actor.marker = 0;
          emit(AiEffect::behavior, 9);
        }
      } else {
        emit(AiEffect::escapePreparation);
        if (!((p.cellFlags & 0x30) && !(p.cellFlags & 0x10)) && !a.gateFC) {
          int substate = 0;
          if (selector(Selector::priority, p.priority)) {
          } else if (selector(Selector::corpse21, p.corpse21))
            substate = 3;
          else if (selector(Selector::corpse10, p.corpse10))
            substate = 4;
          else if (selector(Selector::assist, p.assist))
            substate = 5;
          else if (selector(Selector::score, p.score))
            substate = 2;
          else if (selector(Selector::home, p.home))
            substate = 1;
          else {
            emit(AiEffect::hazard);
            if (p.hazard) {
              if (!random)
                throw std::invalid_argument("AI requires RNG");
              auto word = random();
              r.draws.push_back(word);
              if (add(add(a.bravery, mul(p.hazard, 2)),
                      int((word & 0x7fffffffU) % 35)) > 45)
                substate = 6;
            }
            if (!substate) {
              if (selector(Selector::interaction, p.interaction))
                substate = 8;
              else if (selector(Selector::spell, p.spell))
                substate = 9;
            }
          }
          if (substate)
            emit(AiEffect::substate, substate);
        }
        if (a.behavior == 1)
          r.actor.history = 0;
      }
    }
  } else if (a.behavior == 2 && a.type != 22 && p.ownerEligible &&
             !p.humanOwner && !a.flag704) {
    if (selector(Selector::interaction, p.interaction)) {
      emit(AiEffect::substate, 8);
      r.actor.dirty932 = 1;
    }
  }
  r.actor.age = add(a.age, -20);
  if (r.actor.age <= 0)
    r.actor.recent = 0;
  r.actor.flag815 = 0;
  emit(AiEffect::tailUpdate);
  if (a.behavior == 3 ||
      (a.behavior == 11 && (a.countdown == 5 || a.countdown == 1)))
    emit(AiEffect::attackResponse);
  return r;
}
} // namespace mnm::reconstruction::combat
