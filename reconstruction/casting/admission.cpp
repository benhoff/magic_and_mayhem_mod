#include "admission.hpp"
#include "admission_tables.hpp"
#include <stdexcept>
namespace mnm::reconstruction::casting {
namespace {
int word(std::uint32_t u) { return u <= 0x7fffffffu ? int(u) : -1 - int(~u); }
Cell point(const CastEffectDescriptor &d, unsigned start) {
  return {word(d[start]), word(d[start + 1]), word(d[start + 2])};
}
void store(CastEffectDescriptor &d, unsigned start, Cell p) {
  d[start] = unsigned(p.x);
  d[start + 1] = unsigned(p.y);
  d[start + 2] = unsigned(p.z);
}
Cell gridTruncate(Cell p) { return {p.x / 32, p.y / 32, p.z / 16}; }
bool specialClass(unsigned kind) {
  return kind == 0 || kind == 24 || kind == 25 || kind == 26;
}
int normalize(int n, int size) {
  if (size <= 0)
    throw std::invalid_argument("cast coordinate extent");
  n %= size;
  return n < 0 ? n + size : n;
}
} // namespace
void admitCast(const CastDescriptor &input, AutonomousWorld &world,
               CastAdmissionState &state, const CastAdmissionServices &s) {
  CastEffectDescriptor effect{};
  for (unsigned i = 0; i < 10; ++i)
    effect[i] = UINT32_MAX;
  effect[10] = 1;
  effect[11] = UINT32_MAX;
  effect[12] = 7;
  const auto pool = s.pool();
  s.initializePath();
  auto &configuration = state.spells.at(input[0]);
  if (!configuration.cost || !pool)
    return;
  auto resolve = [&](unsigned slot) -> AutonomousCreature * {
    if (slot == UINT32_MAX)
      return nullptr;
    return &world.creatures.at(slot);
  };
  auto *source = resolve(input[7]);
  auto *target = resolve(input[8]);
  auto requireSource = [&]() -> AutonomousCreature & {
    if (!source)
      throw std::invalid_argument("cast arm needs source");
    return *source;
  };
  auto learn = [&] {
    auto profile = s.currentProfile();
    if (!profile || input[0] >= 124)
      return;
    auto &learning = state.profiles.at(*profile);
    if (learning.known[input[0]] >= 1)
      return;
    learning.known[input[0]] = 1;
    if (learning.notices.size() < 20)
      learning.notices.push_back({5, input[0]});
  };
  unsigned sourceSlot = UINT32_MAX;
  if (source) {
    sourceSlot = source->ai.slot;
    for (unsigned owner = 0; owner < 8; ++owner)
      if (source->ai.visible[owner]) {
        configuration.observed[owner].at(unsigned(source->ai.owner)) = 1;
        s.visibility(*source, owner, (0u - configuration.cost) << 8);
        if (world.localOwner == int(owner))
          learn();
      }
  }
  Cell from{word(input[1]), word(input[2]), word(input[3])};
  if (sourceSlot != UINT32_MAX) {
    const auto &a = requireSource();
    const auto offset =
        state.muzzle.at(a.ai.type).at(state.direction.at(input[7]));
    from = {word(unsigned(a.finePosition.x) + unsigned(a.aimXY) +
                 unsigned(offset.x)),
            word(unsigned(a.finePosition.y) + unsigned(a.aimXY) +
                 unsigned(offset.y)),
            word(unsigned(a.finePosition.z) + unsigned(a.aimZ) +
                 unsigned(offset.z))};
  }
  Cell to{word(input[4]), word(input[5]), word(input[6])};
  if (target) {
    if (target->ai.health)
      to = {word(unsigned(target->finePosition.x) + unsigned(target->aimXY)),
            word(unsigned(target->finePosition.y) + unsigned(target->aimXY)),
            word(unsigned(target->finePosition.z) + unsigned(target->aimZ))};
    else
      to = {word(unsigned(target->ai.position.x) * 32u + 16u),
            word(unsigned(target->ai.position.y) * 32u + 16u),
            word(unsigned(target->ai.position.z) * 16u + 8u)};
    if (world.localOwner >= 0 && world.localOwner < 8 &&
        target->ai.visible[unsigned(world.localOwner)])
      learn();
  } else if (input[7] == UINT32_MAX) {
    state.iteratorCursor = 0;
    while (auto index = s.nextActor()) {
      const auto &a = world.creatures.at(*index);
      if (a.ai.health && a.ai.owner == world.localOwner &&
          s.containsPoint(a, to)) {
        auto profile = s.currentProfile();
        if (profile) {
          if (input[0] < 124) {
            auto &learning = state.profiles.at(*profile);
            if (learning.known[input[0]] < 1) {
              learning.known[input[0]] = 1;
              if (learning.notices.size() < 20)
                learning.notices.push_back({5, input[0]});
            }
          }
          break;
        }
      }
    }
  }
  store(effect, 0, from);
  store(effect, 3, to);
  effect[6] = input[7];
  effect[7] = input[8];
  effect[8] = input[0];
  effect[9] = configuration.pathMode;
  effect[10] = input[9];
  effect[11] = input[10];
  if (sourceSlot != UINT32_MAX && s.chargeable(requireSource()) &&
      !source->block7c && effect[10] == 1) {
    if ((state.resource.at(input[7]) >> 8) < configuration.cost) {
      if (input[11] && s.ownerMessage(*source))
        s.insufficientOwnerMessage();
      else
        s.cue(0x3f3, gridTruncate(to));
      return;
    }
    s.resource(*source, (0u - configuration.cost) << 8);
    if (configuration.resourceMode == 0)
      state.resourceCounter -= configuration.resourceDelta;
    else if (configuration.resourceMode == 2)
      state.resourceCounter += configuration.resourceDelta;
  }
  int travel = configuration.travelKind;
  if (!target && (effect[10] == 0 || effect[10] == 4) && from == to)
    travel = 0;
  s.castCue(input[0], gridTruncate(from), input[11]);
  if (input[0] >= castArms.size())
    return;
  auto secondary = [&](unsigned kind) { s.secondary(*pool, kind, effect); };
  auto path = [&] {
    auto p = point(effect, 0), q = point(effect, 3);
    if (world.dimensions.x <= 0 || world.dimensions.x > 0x3ffffff ||
        world.dimensions.y <= 0 || world.dimensions.y > 0x3ffffff)
      throw std::invalid_argument("cast fine extent");
    p.x = normalize(p.x, world.dimensions.x * 32);
    p.y = normalize(p.y, world.dimensions.y * 32);
    q.x = normalize(q.x, world.dimensions.x * 32);
    q.y = normalize(q.y, world.dimensions.y * 32);
    return s.path(p, q, effect);
  };
  auto refund = [&] {
    auto &a = requireSource();
    if (specialClass(a.ai.kind) && !a.block7c)
      s.resource(a, state.spells.at(effect[8]).cost << 8);
  };
  switch (castArms[input[0]]) {
  case 6:
    effect[12] = 1;
    secondary(2);
    break;
  case 12:
    if (path())
      secondary(4);
    else if (effect[6] != UINT32_MAX)
      refund();
    break;
  case 4:
    if (path())
      secondary(5);
    break;
  case 8:
    effect[14] = input[7];
    effect[12] = 0;
    effect[34] = 1;
    secondary(3);
    break;
  case 2:
    effect[12] = 1;
    if (travel == 1 && effect[7] != effect[6])
      secondary(7);
    else if (travel == 2 && effect[7] != effect[6]) {
      if (path())
        secondary(6);
      else
        refund();
    } else if (travel == 3) {
      if (effect[7] != UINT32_MAX)
        secondary(30);
    } else if (effect[7] != UINT32_MAX)
      secondary(11);
    break;
  case 13:
    effect[11] = 2;
    secondary(34);
    break;
  case 15:
    effect[11] = 0;
    if (source)
      s.cue(0x180, source->ai.position);
    secondary(34);
    break;
  case 0:
  case 5:
    effect[12] = castArms[input[0]] == 5 ? 3 : 1;
    if (travel == 1)
      secondary(7);
    else if (travel == 2) {
      if (path())
        secondary(6);
      else if (effect[6] != UINT32_MAX)
        refund();
    } else
      secondary(11);
    break;
  case 7:
    effect[12] = 1;
    secondary(21);
    break;
  case 10:
    secondary(31);
    break;
  case 9:
    effect[13] = state.specialValue;
    effect[12] = 0;
    effect[14] = input[7];
    effect[34] = 1;
    secondary(48);
    break;
  case 14:
    effect[12] = 1;
    secondary(46);
    break;
  case 1:
    effect[12] = 1;
    secondary(31);
    break;
  case 3: {
    auto tile = [&](Cell p) {
      if (world.dimensions.x <= 0 || world.dimensions.y <= 0)
        throw std::invalid_argument("cast relocation extent");
      return Cell{int((unsigned(p.x) >> 5) % unsigned(world.dimensions.x)),
                  int((unsigned(p.y) >> 5) % unsigned(world.dimensions.y)),
                  int(unsigned(p.z) >> 4)};
    };
    if (effect[10] == 4) {
      if (!target)
        throw std::invalid_argument("cast relocation needs target");
      s.relocate(*target,
                 tile({word(input[4]), word(input[5]), word(input[6])}), true);
    } else {
      auto &a = requireSource();
      Cell destination = tile(point(effect, 3));
      CastMovementDescriptor descriptor{
          a.height,       a.width,   a.flag104,
          a.movementKind, a.ai.type, bool(a.block723 || a.ai.behavior == 14)};
      (void)s.adjust(a.ai.slot, descriptor, destination);
      s.relocate(a, destination, false);
    }
    break;
  }
  case 11:
    if (target && !specialClass(target->ai.kind))
      s.areaEffect(*target, 0, 20);
    break;
  default:
    break;
  }
}
} // namespace mnm::reconstruction::casting
