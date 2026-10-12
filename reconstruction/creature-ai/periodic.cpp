#include "execution.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int bits(unsigned n) { return std::int32_t(n); }
int add(int a, int b) { return bits(unsigned(a) + unsigned(b)); }
int mul(int a, int b) { return bits(unsigned(a) * unsigned(b)); }
} // namespace
void periodicTerrain(AutonomousCreature &a, CreatureExecution &e,
                     unsigned resistance, const PeriodicEnvironment &env,
                     const PeriodicServices &s) {
  auto onTerrain = [&] {
    return s.terrain(a.ai.position) ||
           s.terrain({a.ai.position.x, a.ai.position.y,
                      a.ai.position.z + int(a.height) - 1});
  };
  auto randomRegion = [&]() -> Cell {
    if (env.regionCapacity <= 1)
      throw std::invalid_argument(
          "original region RNG divisor must be positive");
    int index = int(s.random() & 0x7fffffffu) % (env.regionCapacity - 1);
    return env.regions.at(unsigned(index));
  };
  if (e.terrainRelocation) {
    if (!onTerrain()) {
      e.terrainRelocation = a.blockDE3 = e.block962 = e.block963 = 0;
      if (a.ai.behavior != 8)
        s.reset(a);
      else
        a.ai.pending = 0;
      return;
    }
    if (a.ai.behavior == 1) {
      s.move(a, randomRegion());
      a.blockDE3 = e.routeRenewal = 1;
    } else if (a.ai.behavior != 8 && (s.random() & 0x7fffffffu) % 100 < 25) {
      s.move(a, randomRegion());
      a.blockDE3 = e.routeRenewal = 1;
    }
    return;
  }
  if (!env.terrainHasRegions || a.ai.kind == 0 || a.ai.kind == 24 ||
      a.ai.kind == 25 || a.ai.kind == 26 || a.ai.behavior == 8 ||
      a.ai.behavior == 5 || !onTerrain())
    return;
  unsigned chance = (s.random() & 0x7fffffffu) % 100;
  unsigned divisor = resistance + env.relocationStrength;
  if (!divisor)
    throw std::invalid_argument("original relocation divisor is zero");
  unsigned threshold = env.relocationStrength * 100u / divisor;
  if (chance >= threshold)
    return;
  Cell region = randomRegion();
  e.terrainRelocation = 1;
  a.block784 = 0;
  s.move(a, region);
  a.blockDE3 = e.routeRenewal = 1;
}
void updateCreatureValue(AutonomousCreature &a, CreatureExecution &e,
                         const ValuationEnvironment &env) {
  if (!e.dirtyValue)
    return;
  int sum = 0, activeSum = 0, count = 0;
  for (unsigned i = 0; i < 28; ++i) {
    int value = a.ai.hth[i];
    if (env.configured[i] && !a.ai.physical)
      value /= 3;
    value = add(value, mul(bits(e.valueScale8ed), value) / 100);
    if (a.ai.bloodLust)
      value = add(value, value);
    if (e.bonusMode124 == 1 || e.bonusMode124 == 2)
      value = add(value, mul(env.statusBonus[e.bonusMode124 - 1], value) / 100);
    if (a.animationKind == 1)
      value = add(value, env.hthBonus);
    sum = add(sum, value);
    if (env.configured[i]) {
      activeSum = add(activeSum, value);
      ++count;
    }
  }
  e.combatValue[0] = mul(bits(unsigned(sum) / 28u), bits(a.ai.health));
  e.combatValue[2] = count ? mul(activeSum / count, bits(a.ai.health)) : 0;
  sum = activeSum = count = 0;
  for (unsigned i = 0; i < 28; ++i) {
    int value = (a.ai.type == 8 || a.ai.type == 20) ? 10 : env.response[i];
    if (a.ai.type != 8 && a.ai.type != 20 && env.configured[i] &&
        !a.ai.physical)
      value /= 3;
    if (e.bonusMode124 == 1 || e.bonusMode124 == 2)
      value = add(value, mul(env.statusBonus[e.bonusMode124 - 1], value) / 100);
    if (a.animationKind == 2)
      value = add(value, env.responseBonus);
    sum = add(sum, value);
    if (env.configured[i]) {
      activeSum = add(activeSum, value);
      ++count;
    }
  }
  e.combatValue[1] = mul(bits(unsigned(sum) / 28u), bits(a.ai.health));
  e.combatValue[3] = count ? mul(activeSum / count, bits(a.ai.health)) : 0;
  e.combatValue[4] = bits(a.ai.health);
  if (e.mitigation90)
    e.combatValue[4] = mul(e.combatValue[4], 3);
  if (a.specialVision)
    e.combatValue[4] = mul(e.combatValue[4], 2);
  int total = add(add(e.combatValue[1], e.combatValue[4]),
                  add(std::max(e.combatValue[0], e.combatValue[1]),
                      std::max(e.combatValue[2], e.combatValue[3])));
  // Original x87 intermediates retain extended precision. Constants below are
  // exact float/double values from the pinned image, not authored percentages.
  // Hexadecimal constants preserve those bits even with i386 excess precision.
  long double multiplier = a.flag104 ? 0x1.3333340000000p+0L : 1.0L;
  long double divisor =
      1.0L - static_cast<long double>(mul(env.factor5a9, env.factor5ad)) *
                 0x1.a36e2eb1c432dp-15L;
  multiplier *=
      1.0L + static_cast<long double>(e.power84) * 0x1.0624dd2f1a9fcp-10L;
  multiplier *= 1.0L + static_cast<long double>(unsigned(a.ai.bravery)) *
                           0x1.5d867c3ece2a5p-12L;
  multiplier *=
      1.0L + static_cast<long double>(env.factor5c5) * 0x1.47ae147ae147bp-7L;
  multiplier *=
      1.0L + static_cast<long double>(env.factor589) * 0x1.a36e2eb1c432dp-14L;
  multiplier = divisor * multiplier;
  if (a.ai.type == 8 || a.ai.type == 20)
    multiplier *= 0x1.19999a0000000p+0L;
  else if (a.ai.type == 10)
    multiplier *= 0x1.051eb80000000p+0L;
  else if (a.ai.type == 12 || a.ai.type == 21)
    multiplier *= 0x1.0ccccc0000000p+0L;
  long double result = static_cast<long double>(total) * multiplier;
  // 59bee0 truncates to signed64 and returns EAX, the low DWORD. Masked
  // invalid conversions produce the x87 integer-indefinite value, whose low
  // DWORD is zero. Preserve that behavior without an undefined host cast.
  constexpr long double limit = 0x1p63L;
  e.combatValue[5] = std::isfinite(result) && result >= -limit && result < limit
                         ? std::int32_t(std::uint32_t(std::int64_t(result)))
                         : 0;
  e.dirtyValue = 0;
}
} // namespace mnm::reconstruction::combat
