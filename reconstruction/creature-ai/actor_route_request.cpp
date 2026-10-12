#include "actor_route_request.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int bits(unsigned u) { return u <= 0x7fffffffu ? int(u) : -1 - int(~u); }
int subtract(int a, int b) { return bits(unsigned(a) - unsigned(b)); }
int add(int a, int b) { return bits(unsigned(a) + unsigned(b)); }
int absolute(int a) { return a < 0 ? bits(0u - unsigned(a)) : a; }
int normalize(int v, int dimension) {
  if (dimension <= 0)
    throw std::invalid_argument("owned actor route dimension");
  v %= dimension;
  return v < 0 ? v + dimension : v;
}
int hint(int target, int from, int dimension) {
  if (target > from)
    return subtract(target, from) > dimension / 2 ? -1 : 1;
  if (target < from)
    return subtract(from, target) > dimension / 2 ? 1 : -1;
  return 0;
}
void applySnapshot(AutonomousCreature &a, CreatureExecution &e,
                   MotionActionState &m, SegmentCompletionState &r,
                   const ActorRouteSnapshot &snapshot) {
  if (snapshot.count > r.route.size())
    throw std::invalid_argument("owned actor route snapshot count");
  a.destination = snapshot.destination;
  r.cursor = snapshot.cursor;
  r.count = snapshot.count;
  r.route = snapshot.points;
  const auto &v = snapshot.motionWords;
  m.progress = bits(v[0]);
  m.step = bits(v[1]);
  m.travelX = bits(v[2]);
  m.travelY = bits(v[3]);
  m.deltaX = bits(v[4]);
  m.deltaY = bits(v[5]);
  m.heightDelta = bits(v[6]);
  m.heightStep = bits(v[7]);
  m.heightOrigin = bits(v[8]);
  e.moveVariant = v[9];
  m.duration = bits(v[10]);
  m.accumulator = bits(v[11]);
  e.routeChanged = v[12];
  e.markerB73 = v[13];
}
} // namespace
unsigned requestCreatureRoute(AutonomousCreature &a, CreatureExecution &e,
                              MotionActionState &m, SegmentCompletionState &r,
                              ActorRouteMetadata &meta, ActorRouteContext &c,
                              const SegmentCompletionEnvironment &env,
                              Extent dimensions, Cell target, unsigned mode,
                              bool regionRequest,
                              const ActorRouteRequestServices &s) {
  if (dimensions.x <= 0 || dimensions.y <= 0)
    throw std::invalid_argument("owned actor route dimensions");
  if (c.active && !r.regionPending && target == c.destination && mode == c.mode)
    regionRequest = false;
  const Cell desired = target;
  auto region = [&](int id) -> const SegmentRegion & {
    if (id < 0)
      throw std::out_of_range("owned actor region reference");
    return env.regions.at(unsigned(id));
  };
  auto path = [&](unsigned i) {
    if (i < 50)
      return r.regionPath[i];
    // The original inline array aliases its cursor and destination after row49.
    if (i == 50)
      return bits(r.regionCursor);
    if (i == 51)
      return meta.regionDestination.x;
    throw std::out_of_range("owned actor region lookahead");
  };
  auto prior = [&](unsigned i) {
    return i ? path(i - 1) : int(r.regionPending);
  };
  auto shortPair = [&](unsigned i) {
    const int first = s.regionDistance(prior(i), path(i));
    const int second = s.regionDistance(path(i), path(i + 1));
    return add(first, second) < 180;
  };
  auto candidate = [&] {
    const auto &v = region(path(r.regionCursor));
    target = v.position;
    s.regionCandidate(a, target, v.parameter, desired);
  };
  if (c.useRegions && regionRequest) {
    if (r.regionPending && meta.regionDestination == target) {
      bool direct = false;
      if (c.active) {
        if (bits(r.regionCursor) < 50 && path(r.regionCursor) != -1) {
          const auto &v = region(path(r.regionCursor));
          Cell p = v.position;
          s.regionCandidate(a, p, v.parameter, desired);
          if (p == c.destination) {
            target = p;
            direct = true;
          }
        } else
          c.active = 0;
      }
      if (!direct) {
        if (bits(r.regionCursor) >= 50 || path(r.regionCursor) == -1 ||
            (bits(r.regionCursor) < 49 && path(r.regionCursor + 1) == -1 &&
             s.regionDistance(prior(r.regionCursor), path(r.regionCursor)) <
                 180)) {
          r.regionPending = false;
        } else {
          if (path(r.regionCursor + 1) != -1 &&
              path(r.regionCursor + 2) != -1 && shortPair(r.regionCursor))
            ++r.regionCursor;
          candidate();
        }
      }
    } else {
      r.regionPending = false;
      ActorRegionQuery query{a.ai.slot, a.ai.owner, e.commandMode,
                             meta.marker69c};
      const int start = s.regionAt(
          a.ai.position, query, hint(target.x, a.ai.position.x, dimensions.x),
          hint(target.y, a.ai.position.y, dimensions.y));
      Cell endpoint{normalize(target.x, dimensions.x),
                    normalize(target.y, dimensions.y), target.z};
      if (!s.legal(a, endpoint))
        s.adjustLegal(a, endpoint);
      const int end = s.regionAt(endpoint, query, 0, 0);
      if (start != -1 && end != -1 &&
          s.regionRoute(start, end, query, r.regionPath, 700, true) &&
          path(1) != -1 && path(2) != -1) {
        r.regionPending = true;
        meta.regionDestination = desired;
        r.regionCursor = 1;
        if (path(2) != -1 && shortPair(1)) {
          if (path(r.regionCursor + 2) != -1)
            r.regionCursor = 2;
          else
            r.regionPending = false;
        }
        if (r.regionPending)
          candidate();
      }
    }
  }
  const bool exact = c.active && target == c.destination && mode == c.mode &&
                     a.ai.position == c.origin;
  if (!exact) {
    c.scratchDirty = true;
    c.requestBudget = !mode && !regionRequest ? 70 : 210;
  }
  bool coarse = false;
  if (!exact && mode) {
    ActorRouteSnapshot snapshot;
    Cell normalized{normalize(target.x, dimensions.x),
                    normalize(target.y, dimensions.y), target.z};
    coarse = s.coarse(a, normalized, snapshot);
    if (coarse)
      c.scratch = snapshot;
  }
  if (!coarse) {
    const int offered = std::min(c.budget, c.requestBudget);
    int remaining = offered;
    Cell normalized{normalize(target.x, dimensions.x),
                    normalize(target.y, dimensions.y), target.z};
    s.search(a, mode, normalized, c, remaining);
    const int spent = subtract(offered, remaining);
    c.requestBudget = subtract(c.requestBudget, spent);
    c.budget = subtract(c.budget, spent);
  }
  if (c.budget <= 0 && c.requestBudget > 0) {
    c.active = 1;
    c.destination = target;
    c.mode = mode;
    c.origin = a.ai.position;
    meta.marker69c = 0;
    return 2;
  }
  applySnapshot(a, e, m, r, c.scratch);
  if (c.useRegions && regionRequest && r.regionPending)
    a.destination = desired;
  a.ai.marker = r.count ? 1 : 0;
  c.active = 0;
  const unsigned result = a.ai.marker;
  if (r.regionPending && regionRequest && r.count >= 4 &&
      absolute(subtract(a.ai.position.z, target.z)) < 5)
    r.count -= 3;
  meta.marker69c = 0;
  return result;
}
} // namespace mnm::reconstruction::combat
