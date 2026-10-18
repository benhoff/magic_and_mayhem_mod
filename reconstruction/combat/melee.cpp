#include "melee.hpp"
#include <stdexcept>

namespace mnm::reconstruction::combat {
namespace {
std::int32_t signedBits(std::uint32_t n) {
  return static_cast<std::int32_t>(
      n <= 0x7fffffffU ? std::int64_t(n) : std::int64_t(n) - 0x100000000LL);
}
std::int32_t add(std::int32_t a, std::int32_t b) {
  return signedBits(std::uint32_t(a) + std::uint32_t(b));
}
std::int32_t mul(std::int32_t a, std::int32_t b) {
  return signedBits(std::uint32_t(a) * std::uint32_t(b));
}
} // namespace
RandomState seedRandom(std::uint32_t seed) {
  // Same initialization recovered in rendering/terrain_selection.cpp;
  // retained here as a standalone owned simulation projection.
  RandomState s;
  for (unsigned i = 250; i-- > 0;) {
    const auto product = std::uint64_t(seed) * 0x41c64e6dU;
    const auto low = std::uint32_t(product);
    seed = low + 0x3039U;
    const auto high =
        std::uint32_t((product >> 32) << 16) + 0xffffU + (seed < low);
    s.words[i] = (seed >> 16) | (high & 0xffff0000U);
  }
  std::uint32_t bit = 0x80000000U, keep = 0xffffffffU;
  for (unsigned i = 3; bit; i += 7, bit >>= 1, keep >>= 1)
    s.words[i] = (s.words[i] & keep) | bit;
  return s;
}
std::uint32_t nextRandom(RandomState &s) {
  if (s.left >= 250 || s.right >= 250 || (s.right + 250 - s.left) % 250 != 103)
    throw std::invalid_argument("invalid original RNG continuation");
  const auto n = s.words[s.left] ^ s.words[s.right];
  s.words[s.left] = n;
  s.left = (s.left + 1) % 250;
  s.right = (s.right + 1) % 250;
  return n;
}
MeleeResult resolveMelee(const MeleeInputs &p,
                         const std::function<std::uint32_t()> &next,
                         const MeleeDispatch &dispatch) {
  if (!next || p.rank > 2)
    throw std::invalid_argument("unsupported ordinary melee inputs");
  MeleeResult r;
  auto emit=[&](MeleeEffect kind,int value=0){MeleeEvent event{kind,value};r.events.push_back(event);if(dispatch.event)dispatch.event(event);};
  auto draw = [&](unsigned modulus) {
    const auto word = next();
    r.draws.push_back(word);
    return static_cast<std::int32_t>((word & 0x7fffffffU) % modulus);
  };
  auto base = p.targetPhysical && !p.attackerPhysical ? p.base / 3 : p.base;
  if (p.power)
    base = add(base, mul(p.power, base) / 100);
  if (p.bloodLust)
    base = add(base, base);
  auto modifier =
      add(p.height > 0   ? p.heightBonus
          : p.height < 0 ? signedBits(0U - std::uint32_t(p.heightBonus))
                         : 0,
          p.responseModifier);
  emit(MeleeEffect::responseQuery);
  int charge=0;
  if(p.charge){
    if(p.chargeScalar>1000)charge=mul(p.chargeBonus,4);
    else if(p.chargeScalar>750)charge=add(mul(p.chargeBonus,3),mul(add(p.chargeScalar,-750),p.chargeBonus)/250);
    else if(p.chargeScalar>500)charge=add(mul(p.chargeBonus,2),mul(add(p.chargeScalar,-500),p.chargeBonus)/250);
    else if(p.chargeScalar>200)charge=add(p.chargeBonus,mul(add(p.chargeScalar,-200),p.chargeBonus)/300);
    modifier=add(modifier,charge);
  }
  if (p.targetResponse)
    modifier = add(modifier, 50);
  if (modifier)
    base = add(base, mul(modifier, base) / 256);
  if (base < 0)
    base = 0;
  auto amount = mul(draw(257), base) / 128;
  if (amount <= 0)
    return r; // No stun/sound draw when the pre-rank amount is nonpositive.
  if (p.rank)
    amount = add(amount, mul(p.rankBonus, amount) / 100);
  if (p.quarterFlag)
    amount = mul(amount, 4);
  r.amount = amount;
  emit(MeleeEffect::damage,amount);
  emit(MeleeEffect::hostileCredit,amount);
  const auto stun = draw(100);
  if (stun <= p.stunThreshold) {
    const auto argument=add(stun,signedBits(0U-std::uint32_t(p.stunThreshold)));
    emit(MeleeEffect::stunQuery,argument);
    if (dispatch.tryStun?dispatch.tryStun(argument):p.stunHandled)
      emit(MeleeEffect::stunHandled);
    else
      emit(MeleeEffect::stunFallback,draw(20)+20);
  }
  emit(MeleeEffect::attackSound);
  if (charge>200 || p.bloodLust || p.quarterFlag || p.targetResponse)
    emit(MeleeEffect::effectAdmission);
  return r;
}
MeleeActionResult meleeEvent(MeleeActionState s, std::int32_t event,
                             bool present, bool living, bool blocked, bool slow,
                             std::uint32_t actor,
                             const std::function<std::uint32_t()> &next) {
  MeleeActionResult r{s, false, false};
  if (event == 2) {
    if (!present)
      return r;
    r.state.engaged = 0;
    const auto decremented = signedBits(s.incoming - 1);
    r.state.incoming = decremented < 0 ? 0 : std::uint32_t(decremented);
    r.hit = !blocked;
    if (living)
      r.state.recentAttacker = actor + 1;
  } else if (event == 1) {
    if (s.engaged) {
      if (!present)
        throw std::invalid_argument(
            "event1 engaged target missing outside original safe domain");
      r.state.engaged = 0;
      const auto n = signedBits(s.incoming - 1);
      r.state.incoming = n < 0 ? 0 : std::uint32_t(n);
    }
    r.state.ready = true;
    r.state.cooldown = 2 + ((next() & 0x7fffffffU) % 3) + (slow ? 30 : 0);
  } else if (event == 3)
    r.sound = true;
  return r;
}
} // namespace mnm::reconstruction::combat
