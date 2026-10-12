#include "control.hpp"
#include <algorithm>
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
std::int32_t sub(std::int32_t a, std::int32_t b) {
  return bits(std::uint32_t(a) - std::uint32_t(b));
}
std::int32_t mul(std::int32_t a, std::int32_t b) {
  return bits(std::uint32_t(a) * std::uint32_t(b));
}
std::int32_t absolute(std::int32_t n) {
  return n < 0 ? bits(0U - std::uint32_t(n)) : n;
}
void dimensions(Extent e) {
  if (e.x < 1 || e.y < 1 || e.z < 1)
    throw std::invalid_argument("invalid creature dimensions");
}
std::int32_t difference(std::int32_t a, std::int32_t b, std::int32_t extent) {
  auto d = sub(a, b);
  if (absolute(d) > extent / 2)
    d = add(d, d > 0 ? -extent : extent);
  return d;
}
std::int32_t wrap(std::int32_t n, std::int32_t extent) {
  auto a = std::int64_t(n) % extent;
  return std::int32_t(a < 0 ? a + extent : a);
}
void event(StunResult &r, ControlEffect k, std::int32_t a = 0) {
  r.events.push_back({k, a});
}
void disengage(StunState &s) {
  if (s.engaged && (s.action == 10 || s.action == 20)) {
    s.engaged = 0;
    s.incoming = std::uint32_t(std::max(0, bits(s.incoming - 1)));
  }
}
void release(StunResult &r) {
  auto &s = r.state;
  if (s.attachment) {
    event(r, ControlEffect::releaseAttachment);
    s.attachment = false;
  }
  if (s.queueEffects) {
    event(r, ControlEffect::releaseQueue);
    s.queueEffects = false;
  }
  if (s.surface)
    event(r, ControlEffect::surfaceRepair);
}
} // namespace
ReachResult meleeReach(const ReachActor &a, const ReachActor &b, Extent e,
                       const std::function<bool(Cell, Cell)> &diagonal) {
  dimensions(e);
  ReachResult r;
  Cell pos = b.endpoint ? *b.endpoint : b.position;
  auto fine =
      b.endpoint ? add(mul(pos.z, 16), b.endpointTerrainHeight) : b.fineHeight;
  const auto dx = difference(pos.x, a.position.x, e.x),
             dy = difference(pos.y, a.position.y, e.y),
             dz = sub(pos.z, a.position.z);
  if (std::uint32_t(a.position.z) >
          std::uint32_t(add(b.position.z, b.height)) ||
      std::uint32_t(add(a.position.z, a.height)) < std::uint32_t(b.position.z))
    return r;
  if (absolute(dz) > 1 &&
      std::uint32_t(absolute(sub(fine, a.fineHeight))) > 24) {
    if (a.height == 1) {
      if (a.position.z < b.position.z ||
          std::uint32_t(a.position.z) >=
              std::uint32_t(add(b.position.z, b.height)))
        return r;
    } else if (b.height != 1 || b.position.z < a.position.z ||
               std::uint32_t(b.position.z) >=
                   std::uint32_t(add(a.position.z, a.height)))
      return r;
  }
  const int aw = a.width == 2 ? 2 : 1, bw = b.width == 2 ? 2 : 1;
  const bool boundary = ((dx == -bw || dx == aw) && dy >= -bw && dy <= aw) ||
                        (dx > -bw && dx < aw && (dy == -bw || dy == aw));
  if (!boundary)
    return r;
  r.admitted = true;
  if (dx && dy) {
    r.diagonalChecked = true;
    r.admitted = diagonal && diagonal(pos, a.position);
  }
  return r;
}
std::int32_t relativeDirection(Cell a, Cell b, std::uint32_t facing, Extent e) {
  dimensions(e);
  if (facing > 7)
    throw std::invalid_argument("invalid facing");
  const auto x = difference(a.x, b.x, e.x), y = difference(a.y, b.y, e.y);
  const auto x2 = mul(x, 2), y2 = mul(y, 2), p = sub(y, x2), q = add(x2, y),
             t = add(y2, x), u = sub(y2, x);
  int sector;
  if (p < 0 && q <= 0)
    sector = 0;
  else if (q > 0 && add(x, y2) <= 0)
    sector = 1;
  else if (t > 0 && u <= 0)
    sector = 2;
  else if (u > 0 && p <= 0)
    sector = 3;
  else if ((u > 0 || p > 0) && q >= 0)
    sector = 4;
  else if ((p > 0 || q < 0) && t >= 0)
    sector = 5;
  else if ((q < 0 || t < 0) && u >= 0)
    sector = 6;
  else
    sector = 7;
  auto n = sector - int(facing);
  return n < 0 ? n + 8 : n;
}
std::int32_t creatureDistance(Cell a, Cell b, Extent e) {
  dimensions(e);
  auto x = absolute(sub(a.x, b.x)), y = absolute(sub(a.y, b.y)),
       z = absolute(sub(a.z, b.z));
  if (x > e.x / 2)
    x = sub(e.x, x);
  if (y > e.y / 2)
    y = sub(e.y, y);
  std::array<std::int32_t, 3> d{{x, y, z}};
  std::sort(d.begin(), d.end());
  return add(add(mul(d[2], 2), d[1]), d[0]);
}
StunResult beginStun(StunState s, std::int32_t duration, std::uint32_t mode,
                     bool forced) {
  StunResult r{s, {}};
  auto &v = r.state;
  if (v.type == 27) {
    event(r, ControlEffect::reset);
    return r;
  }
  if (!v.health || v.mode == 3 ||
      (!forced && (v.behavior == 15 || v.behavior == 16)))
    return r;
  if (mode != 3 && duration < 1)
    duration = 1;
  if (v.mode && mode) {
    v.timer = duration;
    return r;
  }
  if (v.form && mode) {
    v.mode = mode;
    v.form = 1;
    v.timer = duration;
    return r;
  }
  if (v.flight && mode) {
    v.marker = 0;
    v.savedHeight = std::uint32_t(v.position.z);
    v.blockDE3 = v.block962 = v.block963 = v.readiness = 0;
    disengage(v);
    event(r, ControlEffect::behavior, 16);
    v.mode = mode;
    v.timer = duration;
    return r;
  }
  v.marker = v.readiness = 0;
  disengage(v);
  if (!v.mode)
    v.mode = mode;
  if (v.behavior != 8 || v.timer < duration)
    v.timer = duration;
  if (v.behavior != 8) {
    if (v.behavior == 2 || v.behavior == 6 || v.behavior == 3)
      v.pending = v.behavior;
    event(r, ControlEffect::behavior, 8);
  }
  release(r);
  return r;
}
StunResult tickStun(StunState s, std::int32_t animationEvent) {
  StunResult r{s, {}};
  auto &v = r.state;
  if (v.animationKind == 1)
    event(r, ControlEffect::animation, 1);
  if (v.mode != 3) {
    event(r, ControlEffect::animation, 0);
    if (animationEvent == 1) {
      if (v.timer > 0) {
        auto mode = v.mode;
        v.mode = 0;
        v.behavior = 1;
        auto again = beginStun(v, v.timer, mode);
        again.events.insert(again.events.begin(), r.events.begin(),
                            r.events.end());
        return again;
      }
      if (v.pending == 2 || v.pending == 6) {
        event(r, ControlEffect::behavior, int(v.pending));
        v.pending = 0;
        release(r);
      } else if (v.pending == 3 && v.target != -1) {
        event(r, ControlEffect::behavior, 3);
        v.pending = 0;
      } else {
        v.pending = 0;
        event(r, ControlEffect::reset);
      }
      if (v.form) {
        auto n = v.form;
        v.form = 0;
        event(r, ControlEffect::formRestore, int(n));
      }
    } else if (animationEvent == 100) {
      if (v.mode == 1)
        event(r, ControlEffect::colorSet, 4);
      else if (v.mode == 2)
        event(r, ControlEffect::colorSet, 1);
    } else if (animationEvent == 3)
      event(r, ControlEffect::stunAudio);
  }
  if (v.timer <= 0) {
    if (v.mode == 3)
      return r;
    v.timer = 0;
    v.animationDirty = 1;
    if (!v.mode)
      return r;
  } else {
    --v.timer;
    if (v.timer > 0)
      return r;
    v.timer = 0;
    v.animationDirty = 1;
    if (!v.mode)
      return r;
  }
  if (v.mode == 1)
    event(r, ControlEffect::colorClear, 4);
  else if (v.mode == 2)
    event(r, ControlEffect::colorClear, 1);
  v.mode = 0;
  v.timer = 0;
  return r;
}
StunResult knockbackResponse(StunState s) {
  StunResult r{s, {}};
  auto &v = r.state;
  if (v.type == 27) {
    event(r, ControlEffect::reset);
    return r;
  }
  if (v.type == 22 || v.pending == 8 || v.behavior == 15 || v.behavior == 6 ||
      v.behavior == 8 || v.behavior == 16 || v.mode ||
      (v.form && !v.configFlag && v.animationKind != 3) || v.block71c ||
      v.block723)
    return r;
  v.readiness = 0;
  disengage(v);
  if (v.health && v.behavior != 8) {
    v.pending = v.behavior;
    event(r, ControlEffect::behavior, 14);
  }
  return r;
}
KnockbackResult admitKnockback(const KnockbackInput &p, std::uint32_t word,
                               const std::function<bool(Cell)> &legal,
                               const std::function<bool(Cell)> &move) {
  dimensions(p.dimensions);
  KnockbackResult r;
  const auto &v = p.actor;
  if ((word & 0x7fffffffU) % 100 <= 50 || v.type == 22 || v.mode || !v.health ||
      v.behavior == 6 || (v.behavior == 8 && v.pending == 6) ||
      v.behavior == 14 || p.motion ||
      (v.form && !v.configFlag && v.animationKind == 3) ||
      (p.response != 0 && p.response != 1 && p.response != 7))
    return r;
  if (p.facing > 7)
    throw std::invalid_argument("invalid knockback facing");
  for (unsigned attempt = 0; attempt < 3; ++attempt) {
    if (attempt && (p.facing & 1))
      continue;
    const auto direction = (p.facing + (attempt == 0   ? 4
                                        : attempt == 1 ? 3
                                                       : 5)) %
                           8;
    const auto offset = p.offsets[direction];
    const auto x = wrap(add(v.position.x, offset.x), p.dimensions.x),
               y = wrap(add(v.position.y, offset.y), p.dimensions.y);
    for (int layer : {0, -1, 1}) {
      Cell dest{x, y, add(v.position.z, layer)};
      if (dest.z < 0 || dest.z >= p.dimensions.z)
        continue;
      r.checks.push_back(dest);
      if (legal && legal(dest)) {
        Cell request{x, y, dest.z};
        r.requested = request;
        r.admitted = move && move(request);
        return r;
      }
    }
  }
  return r;
}
ReadyResult readyStep(std::int32_t cooldown, std::int32_t animationEvent,
                      const std::function<void(std::int32_t &)> &behavior) {
  ReadyResult r{cooldown, false, 0};
  if (animationEvent == 1) {
    if (r.cooldown > 0)
      r.restartIdle = true;
    else {
      if (behavior)
        behavior(r.cooldown);
      ++r.behaviorCalls;
    }
  }
  if (r.cooldown > 0)
    --r.cooldown;
  if (r.cooldown <= 0) {
    if (behavior)
      behavior(r.cooldown);
    ++r.behaviorCalls;
  }
  return r;
}
} // namespace mnm::reconstruction::combat
