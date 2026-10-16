#include "owner_visibility.hpp"
#include "../rendering/effect_trajectory_initializer.hpp"
#include <cmath>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int signedWord(unsigned v) { return v <= 0x7fffffffu ? int(v) : -1 - int(~v); }
int add(int a, int b) { return signedWord(unsigned(a) + unsigned(b)); }
unsigned magnitude(unsigned n) { return n < 0x80000000u ? n : 0u - n; }
unsigned wrapOnce(unsigned n, unsigned period) {
  if (signedWord(n) < 0)
    n += period;
  if (signedWord(n) >= int(period))
    n -= period;
  return n;
}
bool shared(const VisionRelations &r, const VisionRelations &s, int a,
            unsigned b) {
  return r.at(unsigned(a)).at(b) && s.at(unsigned(a)).at(b);
}
} // namespace
bool sightRay(Cell from, Cell to, int range, bool special, Extent d,
              unsigned attenuation, const SightSampler &sample) {
  if (d.x <= 0 || d.y <= 0 || d.z <= 0 || d.x > 4096 || d.y > 4096 ||
      range < 0 || range > 4096)
    throw std::invalid_argument("sight ray admitted dimensions/range");
  std::array<unsigned, 3> a{{wrapOnce(unsigned(from.x), unsigned(d.x) * 32),
                             wrapOnce(unsigned(from.y), unsigned(d.y) * 32),
                             unsigned(from.z)}},
      b{{wrapOnce(unsigned(to.x), unsigned(d.x) * 32),
         wrapOnce(unsigned(to.y), unsigned(d.y) * 32), unsigned(to.z)}};
  auto units = a;
  for (auto &n : units)
    n >>= 2;
  for (auto &n : b)
    n >>= 2;
  if (units == b)
    return true;
  Cell cell{int(a[0] >> 5), int(a[1] >> 5), int(a[2] >> 4)};
  reconstruction::EffectTrajectory trajectory;
  unsigned counter = 0;
  reconstruction::initializeEffectTrajectory(
      trajectory, units, b, {{unsigned(d.x), unsigned(d.y)}}, 8, counter);
  const unsigned budget = unsigned(range) * 16;
  if (!budget)
    return false;
  do {
    trajectory.step(units, counter);
    units[0] = wrapOnce(units[0], unsigned(d.x) * 8);
    units[1] = wrapOnce(units[1], unsigned(d.y) * 8);
    Cell next{int(units[0] >> 3), int(units[1] >> 3), int(units[2] >> 2)};
    const bool changed =
        next.x != cell.x || next.y != cell.y || next.z != cell.z;
    auto v = sample(next, units[0] & 7, units[1] & 7, units[2] & 3);
    if (changed && !special && v.attenuating)
      counter += attenuation;
    cell = next;
    // The endpoint succeeds before testing its occupied bit or budget.
    if (units == b)
      return true;
    if (v.occupied && v.blocked)
      return false;
  } while (signedWord(counter) < int(budget));
  return false;
}
bool withinSightRange(const VisibilityActor &a, Cell target, Extent d) {
  int values[2] = {target.x, target.y};
  const int from[2] = {a.position.x, a.position.y};
  const int dims[2] = {d.x, d.y};
  for (unsigned i = 0; i < 2; ++i) {
    const unsigned delta = unsigned(from[i]) - unsigned(values[i]);
    const auto direct = magnitude(delta);
    if (signedWord(magnitude(delta + unsigned(dims[i]))) < signedWord(direct))
      values[i] = add(values[i], -dims[i]);
    if (signedWord(magnitude(delta - unsigned(dims[i]))) < signedWord(direct))
      values[i] = add(values[i], dims[i]);
    if (values[i] < add(from[i], -a.definition.range) ||
        values[i] > add(from[i], a.definition.range))
      return false;
  }
  return true;
}
bool creatureSight(const VisibilityActor &a, const VisibilityActor &b, Extent d,
                   unsigned attenuation, const SightSampler &sample) {
  if (!withinSightRange(a, b.position, d))
    return false;
  const auto offset = a.definition.offsets.at(a.direction);
  Cell from{add(add(a.finePosition.x, a.definition.aimXY), offset.x),
            add(add(a.finePosition.y, a.definition.aimXY), offset.y),
            add(add(a.finePosition.z, a.definition.aimZ), offset.z)};
  Cell to{add(b.finePosition.x, b.definition.aimXY),
          add(b.finePosition.y, b.definition.aimXY),
          add(b.finePosition.z, b.definition.aimZ)};
  return sightRay(from, to, a.definition.range, a.type == 22, d, attenuation,
                  sample);
}
bool pointSight(const VisibilityActor &a, Cell target, Extent d,
                unsigned attenuation, const SightSampler &sample) {
  if (!withinSightRange(
          a, {int(unsigned(target.x) >> 5), int(unsigned(target.y) >> 5), 0},
          d))
    return false;
  const auto offset = a.definition.offsets.at(a.direction);
  Cell from{add(add(a.finePosition.x, a.definition.aimXY), offset.x),
            add(add(a.finePosition.y, a.definition.aimXY), offset.y),
            add(add(a.finePosition.z, a.definition.aimZ), offset.z)};
  return sightRay(from, target, a.definition.range, a.type == 22, d,
                  attenuation, sample);
}
std::optional<unsigned> fineCoordinateFacing(Cell from, int aimXY, Cell target,
                                             Extent d) {
  int delta[2]{};
  const int source[2] = {add(from.x, aimXY), add(from.y, aimXY)};
  const int values[2] = {target.x, target.y}, dimensions[2] = {d.x, d.y};
  for (unsigned i = 0; i < 2; ++i) {
    const auto period = unsigned(dimensions[i]) * 32;
    const auto direct = magnitude(unsigned(source[i]) - unsigned(values[i]));
    int value = values[i];
    if (signedWord(magnitude(unsigned(source[i]) + period -
                             unsigned(values[i]))) < signedWord(direct))
      value = add(value, -int(period));
    if (signedWord(magnitude(unsigned(source[i]) - period -
                             unsigned(values[i]))) < signedWord(direct))
      value = add(value, int(period));
    delta[i] = add(value, signedWord(0u - unsigned(source[i])));
  }
  const int x = delta[0], y = delta[1];
  if (!x && !y)
    return {};
  if (!x)
    return y > 0 ? 4u : 0u;
  if (!y)
    return x > 0 ? 2u : 6u;
  int degrees =
      std::abs(int(std::atan(double(y) / double(x)) * 57.29577951308232));
  if ((x < 0) == (y < 0))
    degrees = 90 - degrees;
  unsigned direction = degrees < 23 ? 2 : degrees < 68 ? 1 : 0;
  if (x > 0 && y > 0)
    direction += 2;
  else if (x < 0 && y > 0)
    direction += 4;
  else if (x < 0 && y < 0)
    direction += 6;
  return direction & 7;
}
void updateOwnerVisibility(
    VisibilityState &state, std::vector<VisibilityActor> &actors,
    const VisionRelations &relations, const VisionRelations &sharing,
    const std::function<bool(const VisibilityActor &, const VisibilityActor &)>
        &sight) {
  const auto phases = state.capacity / 10;
  if (state.capacity < 10 || state.capacity > 125 ||
      actors.size() > state.capacity || state.phase >= phases)
    throw std::invalid_argument("visibility pool/cadence domain");
  for (const auto &a : actors)
    if (a.owner < 0 || a.owner >= 8 || a.direction >= 8)
      throw std::invalid_argument("visibility actor domain");
  const auto phase = state.phase;
  for (auto &a : actors)
    if (a.active)
      for (unsigned owner = 0; owner < 8; ++owner)
        if (int(owner) != a.owner &&
            !shared(relations, sharing, a.owner, owner)) {
          if (signedWord(a.visible[owner]) > 0)
            --a.visible[owner];
          if (!a.visible[owner])
            a.observer[owner] = -1;
        }
  if (++state.phase >= phases)
    state.phase = 0;
  for (unsigned i = 0; i < actors.size(); ++i) {
    const auto &a = actors[i];
    if (!a.active || !a.health || i % phases != phase)
      continue;
    for (auto &b : actors) {
      if (!b.active || a.owner == b.owner ||
          signedWord(b.visible[unsigned(a.owner)]) > 15 || b.type == 27)
        continue;
      if (b.hidden && !b.engaged && !b.incoming && !b.readiness && a.type != 22)
        continue;
      const bool locked =
          (a.engaged || a.incoming || a.readiness) && a.target == b.slot;
      if (!locked && !sight(a, b))
        continue;
      b.visible[unsigned(a.owner)] = 30;
      b.observer[unsigned(a.owner)] = -1;
      for (unsigned owner = 0; owner < 8; ++owner)
        if (shared(relations, sharing, a.owner, owner)) {
          b.visible[owner] = 30;
          b.observer[owner] = -1;
        }
      b.observer[unsigned(a.owner)] = a.slot;
    }
  }
}
} // namespace mnm::reconstruction::combat
