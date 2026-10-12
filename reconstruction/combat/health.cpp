#include "health.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::reconstruction::combat {
namespace {
std::int32_t signedBits(std::uint32_t n) {
  return static_cast<std::int32_t>(
      n <= 0x7fffffffU ? std::int64_t(n) : std::int64_t(n) - 0x100000000LL);
}
struct Work {
  Result result;
  const Inputs &p;
  void emit(Effect k, std::uint32_t a = 0, std::uint32_t b = 0,
            std::uint32_t c = 0) {
    result.events.push_back({k, a, b, c});
  }
  bool damage(std::uint32_t amount) {
    auto &s = result.state;
    if (!s.health || !amount)
      return true;
    for (unsigned i = 0; i < 8; ++i)
      if (p.visibleMask & (1U << i)) {
        s.incoming[i] = amount;
        s.ages[i] = 0;
      }
    const auto before = s.health;
    if (amount >= before) {
      s.marker = 0;
      emit(Effect::scenarioQuery);
      emit(Effect::killTracker);
      s.cacheA = s.cacheB = 0;
      emit(Effect::definitionQuery);
      emit(Effect::spatialCleanup);
      s.health = 0;
      s.gate9c = s.expiry = s.timer = s.rank = 0;
      s.target = 0xffffffffU;
      emit(Effect::detachActor, p.slot);
      s.flag104 = 0;
      emit(Effect::behavior, 5);
      emit(Effect::repair);
      // Cleanup already wrote health zero. With zero mana/no multiplier,
      // the original feedback boundary receives four zero stack words.
      emit(Effect::feedback, 0, 0, 0);
    } else {
      emit(Effect::feedback, amount, 0, 0);
      s.health -= amount;
      if (signedBits(s.cooldown) <= 0 && p.owner == p.selectedOwner) {
        emit(Effect::pain, 1);
        s.cooldown = 250;
      }
    }
    if (p.contributions) {
      emit(Effect::creditA, p.contributionIds[0]);
      if (p.creditAvailable[0])
        s.credit[0] += p.feedbackValue;
      emit(Effect::creditB, p.contributionIds[1]);
      if (p.creditAvailable[1])
        s.credit[1] += p.feedbackValue;
    }
    return amount < before;
  }
  void signedHealth(std::int32_t delta) {
    auto &s = result.state;
    if (p.globalGate || !s.health)
      return;
    if (delta < 0) {
      const auto amount = 0U - static_cast<std::uint32_t>(delta);
      damage(amount);
      if (p.attributionOwner != -1)
        emit(Effect::attribution, amount, p.owner,
             static_cast<std::uint32_t>(p.attributionOwner));
    } else {
      const auto before = s.health;
      s.health =
          std::min(s.health + static_cast<std::uint32_t>(delta), maximum(s, p));
      if (p.notify)
        emit(Effect::feedback, s.health - before, 0, 0);
    }
    emit(Effect::selectedQuery, p.selectedOwner);
    const auto cap = maximum(s, p);
    s.bar = cap ? std::min((s.health * 15U) / cap, 14U) : 0;
    emit(Effect::barSet, s.health);
    emit(Effect::barNotify);
    s.dirty = 1;
  }
};
} // namespace
std::uint32_t maximum(const State &s, const Inputs &p) {
  if (s.rank == 1)
    return p.baseMaximum + (p.baseMaximum * p.bonusOne) / 100U;
  if (s.rank == 2)
    return p.baseMaximum + (p.baseMaximum * p.bonusTwo) / 100U;
  return p.baseMaximum;
}
Result apply(State state, const Inputs &p, Entry entry, std::int32_t value) {
  if (p.owner > 7 || p.selectedOwner > 7 || p.attributionOwner < -1 ||
      p.attributionOwner > 7 || p.absorption > 2 || p.response > 1 ||
      state.rank > 2)
    throw std::invalid_argument("outside declared ordinary health domain");
  if ((entry == Entry::directDamage ||
       (entry == Entry::signedHealth && value < 0) ||
       entry == Entry::defendedDamage) &&
      state.rank)
    throw std::invalid_argument("ranked lethal cleanup remains outside scope");
  Work w{{state, {}, 0}, p};
  if (entry == Entry::maximum) {
    w.result.returned = maximum(state, p);
    return w.result;
  }
  if (entry == Entry::directDamage) {
    w.result.returned = w.damage(static_cast<std::uint32_t>(value));
    return w.result;
  }
  if (entry == Entry::signedHealth) {
    w.signedHealth(value);
    return w.result;
  }
  if (entry != Entry::defendedDamage)
    throw std::invalid_argument("unknown health entry");
  if (value <= 0)
    return w.result;
  w.emit(Effect::status);
  if (p.status == 8 || p.blocked)
    return w.result;
  auto amount = static_cast<std::uint32_t>(value);
  if (p.mitigation) {
    const auto kept = ((100U - p.mitigationPercent) * amount) / 100U;
    w.emit(Effect::mitigated, amount - kept);
    amount = kept;
    if (signedBits(amount) <= 0)
      return w.result;
  }
  if (p.quarter)
    amount /= 4U;
  if (p.absorption) {
    w.emit(Effect::response, amount);
    if (p.response)
      amount = state.health;
    else {
      w.emit(Effect::mitigated, amount);
      if (p.absorption == 2)
        w.signedHealth(signedBits(0U - amount));
      return w.result;
    }
  }
  if (signedBits(amount) > 0) {
    w.emit(Effect::damageNotice, amount);
    w.signedHealth(signedBits(0U - amount));
  }
  return w.result;
}
} // namespace mnm::reconstruction::combat
