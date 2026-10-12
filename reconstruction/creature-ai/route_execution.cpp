#include "../combat/autonomy_tables.hpp"
#include "execution.hpp"
#include <cstdlib>

namespace mnm::reconstruction::combat {
namespace {
int normalize(int n, int size) {
  n %= size;
  return n < 0 ? n + size : n;
}
int difference(int a, int b, int size) {
  int n = a - b;
  if (std::abs(n) > size / 2)
    n += n > 0 ? -size : size;
  return n;
}
Cell targetEdge(const AutonomousCreature &a, const AutonomousCreature &b,
                Extent d, bool wrapAfter) {
  Cell p = b.ai.position;
  int x = difference(a.ai.position.x, p.x, d.x);
  int y = difference(a.ai.position.y, p.y, d.y);
  if (x < 0)
    p.x -= int(a.width);
  else if (x > 0)
    p.x += int(b.width);
  else if (y < 0)
    p.y -= int(a.width);
  else if (y > 0)
    p.y += int(b.width);
  if (wrapAfter) {
    p.x = normalize(p.x, d.x);
    p.y = normalize(p.y, d.y);
  }
  return p;
}
} // namespace
bool continueCreatureRoute(AutonomousCreature &a, CreatureExecution &e,
                           AutonomousWorld &w,
                           const RouteExecutionServices &s) {
  if (a.ai.behavior == 1 || a.ai.behavior == 9 || a.ai.behavior == 10 ||
      a.ai.marker || !a.routePhase || a.ai.action != 15) {
    a.routePhase = 0;
    return false;
  }
  if (a.cooldown > 1)
    return false;
  const unsigned phase = a.routePhase;
  if (phase == 2 && s.resumeDisplacement(a))
    return s.displacementPending;
  a.routeMode = a.routePhase == 1 ? 1 : 0;
  auto finish = [&] {
    a.routeMode = 0;
    if (a.routePhase && (phase == a.routePhase || phase == 2))
      a.cooldown = 10;
    return false;
  };
  auto request = [&](Cell p) {
    unsigned result = s.request(a, p, a.routeMode, 1);
    a.routeMode = 0;
    if (result != 2)
      a.routePhase = 0;
    return result;
  };
  auto begin = [&] {
    if (a.cooldown <= 1)
      s.beginMotion(a);
  };
  auto retreat = [&] {
    a.ai.marker = 0;
    s.action(a, 15);
    a.routePhase = 1;
    a.substate = 7;
  };
  switch (a.ai.behavior) {
  case 2: {
    unsigned result = request(a.destination);
    if (result == 2)
      return true;
    if (result == 1) {
      if (a.cooldown <= 1) {
        s.beginMotion(a);
        if (phase == 2 && a.routePhase == 2)
          e.routeRenewal = 0;
      }
    } else if (!s.queueFallback(a))
      s.reset(a);
    break;
  }
  case 3: {
    auto &b = w.creatures.at(unsigned(a.ai.target));
    if (s.reach(a, b, a.ai.position)) {
      a.routePhase = 0;
      break;
    }
    Cell p = a.chosenDestination;
    if (a.ai.owner >= 0 && a.ai.owner < 8 &&
        b.ai.visible[unsigned(a.ai.owner)]) {
      p = targetEdge(a, b, w.dimensions, true);
      bool adjusted = s.adjust(a, b, p);
      if (!adjusted || !s.reach(a, b, p)) {
        bool found = false;
        for (auto offset : originalRouteOffsets) {
          Cell q{normalize(b.ai.position.x + offset.x +
                               (b.width == 2 && offset.x > 0 ? 1 : 0),
                           w.dimensions.x),
                 normalize(b.ai.position.y + offset.y +
                               (b.width == 2 && offset.y > 0 ? 1 : 0),
                           w.dimensions.y),
                 b.ai.position.z + offset.z};
          if (q.z < 0 || q.z >= w.dimensions.z)
            continue;
          if (s.legal(a, q) && s.localReach(a, q) && s.reach(a, b, q)) {
            p = q;
            found = true;
            break;
          }
        }
        if (!found)
          p = b.ai.position;
      }
    }
    unsigned result = request(p);
    if (result == 2)
      return true;
    if (result == 1)
      begin();
    else
      s.reset(a);
    break;
  }
  case 4:
  case 13: {
    unsigned result = request(a.destination);
    if (result == 2)
      return true;
    if (result == 1)
      begin();
    else {
      const bool copyPosition = a.ai.behavior == 13;
      s.reset(a);
      if (copyPosition)
        a.destination = a.ai.position;
    }
    break;
  }
  case 11: {
    switch (a.substate) {
    case 1:
    case 3:
    case 4:
    case 5: {
      if (a.ai.target == -1) {
        retreat();
        break;
      }
      auto &b = w.creatures.at(unsigned(a.ai.target));
      if (s.reach(a, b, a.ai.position)) {
        a.routePhase = 0;
        break;
      }
      Cell p = b.ai.position;
      if (a.ai.flying) {
        p = targetEdge(a, b, w.dimensions, true);
        if (!s.adjust(a, b, p) || !s.reach(a, b, p))
          p = b.ai.position;
      }
      unsigned result = request(p);
      if (result == 2)
        return true;
      if (result == 1)
        begin();
      else {
        if (a.substate == 4)
          w.creatures.at(unsigned(a.ai.target)).ai.corpseClaim = 1;
        retreat();
      }
      break;
    }
    case 7: {
      if (a.ai.position == a.ai.home) {
        a.routePhase = 0;
        s.reset(a);
        break;
      }
      unsigned result = request(a.ai.home);
      if (result == 2)
        return true;
      if (result == 1)
        begin();
      else
        s.reset(a);
      break;
    }
    case 8:
    case 9: {
      unsigned result = request(a.chosenDestination);
      if (result == 2)
        return true;
      if (result == 1)
        begin();
      else {
        retreat();
        a.ai.dirty932 = 0;
      }
      break;
    }
    default:
      break;
    }
    break;
  }
  default:
    break;
  }
  return finish();
}
} // namespace mnm::reconstruction::combat
