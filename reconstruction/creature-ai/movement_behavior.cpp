#include "movement_behavior.hpp"
#include <cstdlib>
namespace mnm::reconstruction::combat {
namespace {
int normalize(int n, int extent) {
  n %= extent;
  return n < 0 ? n + extent : n;
}
} // namespace
void movementBehavior(AutonomousCreature &a, CreatureExecution &e,
                      CreatureMaintenance &m, const MovementBehaviorState &v,
                      Extent extent, const MovementBehaviorServices &s) {
  if (e.routeChanged) {
    e.routeChanged = 0;
    e.markerB73 = 1;
    unsigned savedForm = a.form, savedBlock = a.blockDE3, saved71c = a.block71c;
    a.form = a.blockDE3 = a.block71c = 0;
    if (a.block723) {
      a.block723 = 0;
      s.renew(a, v.renewedOrder);
      a.block71c = saved71c;
      a.form = savedForm;
      a.blockDE3 = savedBlock;
      if (a.block723)
        return;
    } else {
      if (s.renewMove) s.renewMove(a, a.destination);
      else s.move(a, a.destination);
      a.block71c = saved71c;
      a.form = savedForm;
      a.blockDE3 = savedBlock;
      return;
    }
  }
  if (a.ai.marker) {
    s.beginMotion(a);
    return;
  }
  auto finish = [&] {
    a.blockDE3 = e.block962 = e.block963 = 0;
    s.reset(a);
    if (a.ai.type == 27) {
      s.invalidate(a);
      s.remove(a);
    }
  };
  auto clearRequested = [&] {
    if (a.block723) {
      a.block723 = 0;
      e.block722 = 0;
    }
  };
  if (a.ai.position == a.destination) {
    finish();
    clearRequested();
    return;
  }
  if (std::abs(a.ai.position.x - a.destination.x) > 1 ||
      std::abs(a.ai.position.y - a.destination.y) > 1 ||
      std::abs(a.ai.position.z - a.destination.z) > 1) {
    s.action(a, 15);
    if (!a.routePhase) {
      a.routePhase = e.markerB73 = 1;
    }
    return;
  }
  if (a.routePhase) {
    s.action(a, 15);
    clearRequested();
    return;
  }
  if (s.fallback(a)) {
    clearRequested();
    return;
  }
  if (a.block71c) {
    if (m.flight110 || m.block71d) {
      a.flag104 = 0;
      if (a.animationKind == 3 && m.block71d) {
        a.animationKind = m.flight809 = 0;
        s.clearOverlay(a);
        if (m.flightEffect) {
          s.releaseEffect(*m.flightEffect);
          m.flightEffect.reset();
        }
      }
      m.block71d = 0;
    }
    unsigned health = a.ai.type == 15
                          ? a.ai.health
                          : s.landingHealth(a, v.landingKind, a.ai.position.z);
    s.setHealth(a, health);
    Cell p{normalize(a.ai.position.x, extent.x),
           normalize(a.ai.position.y, extent.y), a.ai.position.z};
    if (s.adjust(a, p)) {
      s.releaseOccupancy(a, a.ai.position);
      if (a.ai.motion)
        s.releaseOccupancy(a, v.nextRouteCell);
      s.relocate(a, p);
      s.move(a, p);
      s.settle(a);
      s.refresh(a);
      s.invalidate(a);
      if (!a.ai.active)
        return;
      a.ai.flying = 0;
    }
    a.block71c = 0;
  }
  finish();
  clearRequested();
}
} // namespace mnm::reconstruction::combat
