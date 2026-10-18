#pragma push_macro("main")
#undef main
#define main unused_execution_fixture_main
#include "creature-execution-test.cpp"
#undef main
#pragma pop_macro("main")
#include "../reconstruction/creature-ai/death_behavior.hpp"
namespace {
struct LifeFixture {
  Fixture base;
  std::vector<CreatureMaintenance> maintenance;
  std::vector<MovementBehaviorState> movement;
  std::vector<unsigned short> corpseFlags, corpseMarkers;
};
LifeFixture *life;
unsigned cellIndex(Cell p) { return unsigned((p.z * 32 + p.y) * 32 + p.x); }
unsigned pollCode(MaintenanceAnimation slot) { return 110 + unsigned(slot); }
MaintenanceEnvironment maintenanceEnvironment() {
  MaintenanceEnvironment e;
  e.tick = 1000;
  e.upkeepInterval = 60;
  e.upkeepCost = 2;
  e.statusDamage = 7;
  e.statusInterval = 23;
  e.poisonInterval = 17;
  e.powerInterval = 11;
  e.powerMaximum = 20;
  e.restoreMode = 42;
  e.drainQuantity = 101;
  e.drainDivisor = int(fixture->number % 5) - 1;
  e.effectDamage = 13;
  e.localOwner = int(fixture->number % 8);
  e.diagnostic = fixture->number % 2;
  return e;
}
void plain(unsigned kind, AutonomousCreature &a) { record(kind, a.ai.slot); }
void applyHealth(AutonomousCreature &a, int amount, int source, bool flag) {
  record(103, a.ai.slot, flag, {amount, source, 0});
  if (fixture->number % 29 == 0) {
    a.ai.health = 0;
    a.ai.active = false;
  }
}
MaintenanceServices maintenanceServices() {
  MaintenanceServices s;
  s.repairAnimation = [](auto &a) {
    plain(90, a);
    plain(91, a);
  };
  s.refresh = [](auto &a) { plain(93, a); };
  s.expireStatus = [](auto &a) {
    plain(94, a);
    if (fixture->number % 31 == 0)
      a.ai.health = 0;
  };
  s.bonusWork = [](auto &a) { plain(95, a); };
  s.expireForm = [](auto &a) { plain(96, a); };
  s.removeFromWorld = [](auto &a) { plain(97, a); };
  s.remove = [](auto &a) { plain(98, a); };
  s.repairFlight = [](auto &a) {
    plain(99, a);
    if (fixture->number % 37 == 0)
      a.ai.active = false;
  };
  s.action = [](auto &a) { plain(100, a); };
  s.activate = [](auto &a) { plain(101, a); };
  s.diagnostic = [](auto &a) { plain(102, a); };
  s.repairStatus = [](auto &a) { plain(106, a); };
  s.random = [] {
    record(107, 0);
    return 123u;
  };
  s.upkeep = [](auto &) {
    throw std::runtime_error("unconfigured upkeep domain");
  };
  s.color = [](auto &a, MaintenanceColor mode, unsigned key) {
    record(108, a.ai.slot, unsigned(mode), {int(key), 0, 0});
  };
  s.health = applyHealth;
  s.drain = [](auto &a, unsigned n) { record(104, a.ai.slot, n); };
  s.restoreMode = [](auto &a, unsigned n) { record(109, a.ai.slot, n); };
  s.ownerEvent = [](auto &a, unsigned n) { record(117, a.ai.slot, n); };
  s.poll = [](auto &a, MaintenanceAnimation slot) {
    record(pollCode(slot), a.ai.slot);
    return int((fixture->number + unsigned(slot)) % 4);
  };
  s.effect = [](auto &a, std::optional<unsigned> &token, unsigned kind) {
    record(118, a.ai.slot, kind);
    if (kind != 0x2f1)
      token = kind;
  };
  s.releaseEffect = [](unsigned) {
  }; // Legacy disabled-display domain; compare final ownership clear.
  s.repairReference = [](unsigned peer) { record(119, peer, 0); };
  return s;
}
MovementBehaviorServices movementServices() {
  MovementBehaviorServices s;
  s.renew = [](auto &a, const auto &args) {
    record(120, a.ai.slot, args[0]);
    if (fixture->number % 3)
      a.block723 = 1;
  };
  s.move = [](auto &a, Cell p) { record(121, a.ai.slot, 0, p); };
  s.releaseOccupancy = [](auto &a, Cell p) { record(122, a.ai.slot, 0, p); };
  s.relocate = [](auto &a, Cell p) { record(123, a.ai.slot, a.ai.type, p); };
  s.action = executionServices().action;
  s.beginMotion = routeServices().beginMotion;
  s.reset = [](auto &a) { plain(23, a); };
  s.clearOverlay = [](auto &a) { plain(124, a); };
  s.invalidate = [](auto &a) { plain(125, a); };
  s.settle = [](auto &a) { plain(126, a); };
  s.refresh = [](auto &a) { plain(127, a); };
  s.remove = [](auto &a) { plain(98, a); };
  s.fallback = [](auto &a) {
    record(128, a.ai.slot);
    return fixture->number % 7 == 0;
  };
  s.landingHealth = [](auto &a, unsigned kind, int z) {
    record(129, a.ai.slot, kind, {z, 0, 0});
    return 77u;
  };
  s.setHealth = [](auto &a, unsigned health) {
    record(130, a.ai.slot, health);
  };
  s.adjust = [](const auto &a, Cell &p) {
    record(131, a.ai.slot, 0, p);
    if (fixture->number % 5 == 0)
      return false;
    p.x = (p.x + 1) % 32;
    return true;
  };
  s.releaseEffect = [](unsigned) { record(132, 0); };
  return s;
}
DeathEnvironment deathEnvironment() {
  return {123, 42, 17, 3, fixture->number % 2 != 0};
}
DeathServices deathServices() {
  DeathServices s;
  s.action = executionServices().action;
  s.explode = [](auto &a, int power, unsigned mode) {
    record(133, a.ai.slot, mode, {power, 0, 0});
  };
  s.specialRemoval = [](auto &a) { plain(134, a); };
  s.remove = [](auto &a) { plain(98, a); };
  s.releaseUpkeep = [](auto &a) { plain(135, a); };
  s.registerCorpse = [](auto &a) {
    plain(136, a);
    life->corpseFlags.at(cellIndex(a.ai.position)) &= 0xff7fu;
    life->corpseMarkers.at(cellIndex(a.ai.position)) |= 0x2000u;
  };
  s.releaseReference = [](auto &a) { plain(137, a); };
  s.releaseOccupancy = movementServices().releaseOccupancy;
  return s;
}
void nativeLife(LifeFixture &f, unsigned op) {
  auto &a = f.base.world.creatures[0];
  auto &e = f.base.execution[0];
  auto &m = f.maintenance[0];
  auto &v = f.movement[0];
  if (op == 0)
    maintainCreature(a, e, m, f.base.world, maintenanceEnvironment(),
                     maintenanceServices());
  else if (op == 1)
    movementBehavior(a, e, m, v, f.base.world.dimensions, movementServices());
  else if (op == 2)
    deathBehavior(a, m, v, deathEnvironment(), deathServices());
}
#ifdef MNM_EXECUTION_REFERENCE
Bytes effectsLife, displayLife, terrainLife, listLife;
unsigned effectPointer(std::optional<unsigned> token) {
  return token
             ? reinterpret_cast<std::uintptr_t>(effectsLife.data() + *token * 4)
             : 0;
}
void lifeExtra(void *p, const AutonomousCreature &a, const CreatureExecution &e,
               const CreatureMaintenance &m, const MovementBehaviorState &v) {
  write32(p, 0x678, 0);
  write32(p, 0x67c, 0);
  for (auto f : {std::pair<unsigned, unsigned>{0x98, m.random98},
                 {0x100, m.upkeepDeadline},
                 {0xfc, a.ai.gateFC},
                 {0x114, m.status114},
                 {0x118, m.status118},
                 {0x744, m.status744},
                 {0x748, m.status748},
                 {0x74c, m.status74c},
                 {0x110, m.flight110},
                 {0x778, m.status778},
                 {0x77c, m.status77c},
                 {0x780, m.status780},
                 {0x821, m.status821},
                 {0x8c5, m.status8c5},
                 {0x78c, m.status78c},
                 {0x790, m.status790},
                 {0x819, m.status819},
                 {0x81d, m.status81d},
                 {0x8d5, m.status8d5},
                 {0x8d9, m.status8d9},
                 {0x8dd, m.status8dd},
                 {0x8e1, m.powerDeadline},
                 {0x889, m.animation889},
                 {0x8bd, m.animation8bd},
                 {0x809, m.flight809},
                 {0x94, m.timer94},
                 {0x750, m.marker750},
                 {0x660, m.timer660},
                 {0x6b0, m.recent6b0},
                 {0xcff, unsigned(m.lifespan)},
                 {0xddf, unsigned(m.suspension)},
                 {0x8cd, unsigned(m.status8cd)},
                 {0x8c, a.specialVision},
                 {0x90, e.mitigation90},
                 {0x92e, e.dirtyValue},
                 {0x124, e.bonusMode124},
                 {0x104, a.flag104},
                 {0x7cd, a.animationKind},
                 {0x8c9, a.protection8c9},
                 {0x84, e.power84},
                 {0x855, e.block855},
                 {0x120, effectPointer(m.statusEffect)},
                 {0x794, effectPointer(m.damageEffect)},
                 {0x805, effectPointer(m.flightEffect)},
                 {0xa4, v.landingKind},
                 {0x977, 0},
                 {0x97f, unsigned(v.nextRouteCell.x)},
                 {0x983, unsigned(v.nextRouteCell.y)},
                 {0x987, unsigned(v.nextRouteCell.z)},
                 {0xf0, a.ai.corpseA},
                 {0xf4, a.ai.corpseB},
                 {0xf8, a.ai.physical},
                 {0x7c, a.block7c}})
    write32(p, f.first, f.second);
  static_cast<unsigned char *>(p)[0x7cc] = m.animation7cc;
  static_cast<unsigned char *>(p)[0x71d] = m.block71d;
  for (unsigned i = 0; i < 8; ++i) {
    write32(p, 0xe0b + i * 4, m.visibilityAge[i]);
    write32(p, 0x724 + i * 4, v.renewedOrder[i]);
    write32(p, 0x24 + i * 4, a.ai.visible[i]);
  }
}
unsigned actorFrom(void *p, unsigned offset) {
  return unsigned((static_cast<unsigned char *>(p) - actors.data() - offset) /
                  0xe4b);
}
extern "C" unsigned __attribute__((thiscall)) lifeFrameStub(void *, unsigned) {
  record(90, 0);
  return 0;
}
extern "C" unsigned __attribute__((thiscall)) lifeValidStub(void *, unsigned,
                                                            unsigned) {
  record(91, 0);
  return 1;
}
#define PLAIN_STUB(name, kind)                                                 \
  extern "C" void __attribute__((thiscall)) name(void *p) {                    \
    record(kind, slot(p));                                                     \
  }
PLAIN_STUB(lifeRefreshStub, 93)
PLAIN_STUB(lifeBonusStub, 95)
PLAIN_STUB(lifeFormExpiredStub, 96)
PLAIN_STUB(lifeRemoveStub, 98)
PLAIN_STUB(lifeActionStub, 100)
PLAIN_STUB(lifeStatusRepairStub, 106)
PLAIN_STUB(lifeSpecialRemovalStub, 134)
PLAIN_STUB(lifeSettleStub, 126)
PLAIN_STUB(lifeMoveRefreshStub, 127)
PLAIN_STUB(lifeInvalidateStub, 125)
#undef PLAIN_STUB
extern "C" void __attribute__((thiscall)) lifeStatusExpiredStub(void *p) {
  record(94, slot(p));
  if (fixture->number % 31 == 0)
    write32(p, 0xe4, 0);
}
extern "C" void __attribute__((thiscall)) lifeFlightRepairStub(void *p) {
  record(99, slot(p));
  if (fixture->number % 37 == 0)
    write32(p, 4, 0);
}
extern "C" void __attribute__((thiscall)) lifeInvalidateArgStub(void *p,
                                                                unsigned n) {
  if (n)
    throw std::runtime_error("invalidate flags");
  record(125, slot(p));
}
extern "C" void __attribute__((thiscall)) lifeRemoveWorldStub(void *p,
                                                              unsigned n) {
  if (n)
    throw std::runtime_error("removal flags");
  record(fixture->number == 0 ? 97 : 97, slot(p));
}
extern "C" void __attribute__((thiscall)) lifeRemoveDisplayStub(void *,
                                                                void *p) {
  record(97, slot(p));
}
extern "C" void __attribute__((thiscall)) lifeActivateStub(void *, unsigned n) {
  record(101, n);
}
extern "C" unsigned __attribute__((thiscall)) lifeRandomStub(void *) {
  record(107, 0);
  return 123;
}
extern "C" void __attribute__((thiscall))
lifeDiagnosticStub(void *, const char *, unsigned) {
  record(102, 0);
}
extern "C" void __attribute__((thiscall)) lifeColorAddStub(void *p,
                                                           unsigned n) {
  record(108, actorFrom(p, 0xbf3), unsigned(MaintenanceColor::add),
         {int(n), 0, 0});
}
extern "C" void __attribute__((thiscall)) lifeColorClearStub(void *p,
                                                             unsigned n) {
  record(108, actorFrom(p, 0xbf3), unsigned(MaintenanceColor::clear),
         {int(n), 0, 0});
}
extern "C" void __attribute__((thiscall)) lifeColorRemoveStub(void *p,
                                                              unsigned n) {
  record(108, actorFrom(p, 0xbf3), unsigned(MaintenanceColor::remove),
         {int(n), 0, 0});
}
extern "C" void __attribute__((thiscall)) lifeColorTickStub(void *p) {
  record(108, actorFrom(p, 0xbf3), unsigned(MaintenanceColor::tick), {0, 0, 0});
}
extern "C" void __attribute__((thiscall))
lifeHealthStub(void *p, int amount, int source, unsigned zero, unsigned flag) {
  if (zero)
    throw std::runtime_error("health flags");
  record(103, slot(p), flag, {amount, source, 0});
  if (fixture->number % 29 == 0) {
    write32(p, 4, 0);
    write32(p, 0xe4, 0);
  }
}
extern "C" void __attribute__((thiscall)) lifeDrainStub(void *p, unsigned n) {
  record(104, slot(p), n);
}
extern "C" void __attribute__((thiscall)) lifeRestoreModeStub(void *p,
                                                              unsigned n) {
  record(109, slot(p), n);
}
extern "C" void __attribute__((thiscall)) lifeOwnerStub(void *p, unsigned n) {
  record(117, slot(p), n);
}
extern "C" unsigned __attribute__((thiscall)) lifePollStub(void *p) {
  auto offset =
      unsigned(static_cast<unsigned char *>(p) - actors.data()) % 0xe4b;
  MaintenanceAnimation kind;
  switch (offset) {
  case 0x12c:
    kind = MaintenanceAnimation::bonus;
    break;
  case 0x79c:
    kind = MaintenanceAnimation::form;
    break;
  case 0x7d5:
    kind = MaintenanceAnimation::flight;
    break;
  case 0x825:
    kind = MaintenanceAnimation::response;
    break;
  case 0x859:
    kind = MaintenanceAnimation::auxiliary;
    break;
  case 0x88d:
    kind = MaintenanceAnimation::owner;
    break;
  default:
    throw std::runtime_error("poll offset");
  }
  unsigned id = actorFrom(p, offset);
  record(pollCode(kind), id);
  return (fixture->number + unsigned(kind)) % 4;
}
extern "C" void __attribute__((thiscall))
lifeEffectUpdateStub(void *, unsigned *ref, int, int, int) {
  auto delta = reinterpret_cast<unsigned char *>(ref) - actors.data();
  unsigned kind = delta % 0xe4b == 0x120   ? 0x2c9
                  : delta % 0xe4b == 0x794 ? 0x2cf
                                           : 0x2ee;
  record(118, unsigned(delta / 0xe4b), kind);
  *ref = effectPointer(kind);
}
extern "C" void __attribute__((thiscall))
lifeEffectCreateStub(void *, unsigned kind, int, int, int, unsigned zero,
                     unsigned *ref, unsigned) {
  if (zero)
    throw std::runtime_error("effect flags");
  record(118, 0, kind);
  if (ref)
    *ref = effectPointer(kind);
}
extern "C" void __attribute__((thiscall)) lifeReferenceStub(void *p,
                                                            unsigned n) {
  record(119, slot(p), n);
}
extern "C" void __attribute__((thiscall))
lifeRenewStub(void *p, unsigned a, unsigned, unsigned, unsigned, unsigned,
              unsigned, unsigned, unsigned) {
  record(120, slot(p), a);
  if (fixture->number % 3)
    static_cast<unsigned char *>(p)[0x723] = 1;
}
extern "C" void __attribute__((thiscall)) lifeMoveStub(void *p, int x, int y,
                                                       int z) {
  record(121, slot(p), 0, {x, y, z});
}
extern "C" void __attribute__((thiscall)) lifeOccupancyStub(void *p, int x,
                                                            int y, int z) {
  record(122, slot(p), 0, {x, y, z});
}
extern "C" void __attribute__((thiscall))
lifeRelocateStub(void *p, int x, int y, int z, unsigned type) {
  record(123, slot(p), type, {x, y, z});
}
extern "C" void __attribute__((thiscall)) lifeClearOverlayStub(void *p) {
  record(124, actorFrom(p, 0x7d5));
}
extern "C" void __attribute__((thiscall))
lifeReleaseDisplayStub(void *, unsigned zero, unsigned one) {
  if (zero || one != 1)
    throw std::runtime_error("display release flags");
  record(132, 0);
}
extern "C" unsigned __attribute__((thiscall)) lifeFallbackStub(void *p) {
  record(128, slot(p));
  return fixture->number % 7 == 0;
}
extern "C" unsigned __attribute__((thiscall))
lifeLandingHealthStub(void *p, unsigned kind, int z) {
  record(129, slot(p), kind, {z, 0, 0});
  return 77;
}
extern "C" void __attribute__((thiscall))
lifeSetHealthStub(void *p, unsigned n, unsigned invalid, unsigned zero) {
  if (invalid != unsigned(-1) || zero)
    throw std::runtime_error("set health flags");
  record(130, slot(p), n);
}
extern "C" unsigned __attribute__((thiscall))
lifeAdjustStub(void *, unsigned id, const void *, int *x, int *y, int *z) {
  record(131, id, 0, {*x, *y, *z});
  if (fixture->number % 5 == 0)
    return false;
  *x = (*x + 1) % 32;
  return true;
}
extern "C" void __attribute__((thiscall)) lifeExplodeStub(void *p, int power,
                                                          unsigned mode) {
  record(133, slot(p), mode, {power, 0, 0});
}
extern "C" unsigned __attribute__((fastcall))
lifeScoreStub(unsigned type, unsigned zero2, unsigned a, unsigned b, unsigned c,
              float ratio) {
  if (type != fixture->world.creatures[0].ai.type || zero2 || a || b || c ||
      ratio != float(2.0L / 60.0L))
    throw std::runtime_error("death score ABI");
  record(135, 0);
  return 23;
}
extern "C" void __attribute__((thiscall)) lifeCorpseStub(void *, void *, void *,
                                                         unsigned *id) {
  record(136, *id);
}
extern "C" void __attribute__((thiscall)) lifeCorpseReleaseStub(void *,
                                                                unsigned id) {
  record(137, id);
}
void installLife() {
  for (auto v : {std::pair<unsigned, void *>{
                     0x464b30, reinterpret_cast<void *>(lifeFrameStub)},
                 {0x464c10, reinterpret_cast<void *>(lifeValidStub)},
                 {0x508040, reinterpret_cast<void *>(lifeRefreshStub)},
                 {0x507cd0, reinterpret_cast<void *>(lifeStatusExpiredStub)},
                 {0x514c60, reinterpret_cast<void *>(lifeBonusStub)},
                 {0x520e70, reinterpret_cast<void *>(lifeFormExpiredStub)},
                 {0x506d30, reinterpret_cast<void *>(lifeRemoveStub)},
                 {0x51f5e0, reinterpret_cast<void *>(lifeRemoveWorldStub)},
                 {0x49d400, reinterpret_cast<void *>(lifeRemoveDisplayStub)},
                 {0x51ef60, reinterpret_cast<void *>(lifeFlightRepairStub)},
                 {0x49d070, reinterpret_cast<void *>(lifeActivateStub)},
                 {0x485b10, reinterpret_cast<void *>(lifeDiagnosticStub)},
                 {0x514040, reinterpret_cast<void *>(lifeStatusRepairStub)},
                 {0x54e200, reinterpret_cast<void *>(lifeRandomStub)},
                 {0x54dad0, reinterpret_cast<void *>(lifeColorAddStub)},
                 {0x54dce0, reinterpret_cast<void *>(lifeColorClearStub)},
                 {0x54dba0, reinterpret_cast<void *>(lifeColorRemoveStub)},
                 {0x54dd60, reinterpret_cast<void *>(lifeColorTickStub)},
                 {0x5076f0, reinterpret_cast<void *>(lifeHealthStub)},
                 {0x520c70, reinterpret_cast<void *>(lifeDrainStub)},
                 {0x5142b0, reinterpret_cast<void *>(lifeRestoreModeStub)},
                 {0x518140, reinterpret_cast<void *>(lifeOwnerStub)},
                 {0x464ec0, reinterpret_cast<void *>(lifePollStub)},
                 {0x571460, reinterpret_cast<void *>(lifeEffectUpdateStub)},
                 {0x571310, reinterpret_cast<void *>(lifeEffectCreateStub)},
                 {0x508b00, reinterpret_cast<void *>(lifeReferenceStub)},
                 {0x51f270, reinterpret_cast<void *>(lifeRenewStub)},
                 {0x50fae0, reinterpret_cast<void *>(lifeMoveStub)},
                 {0x509230, reinterpret_cast<void *>(lifeMoveStub)},
                 {0x5093e0, reinterpret_cast<void *>(lifeOccupancyStub)},
                 {0x5070e0, reinterpret_cast<void *>(lifeRelocateStub)},
                 {0x464c80, reinterpret_cast<void *>(lifeClearOverlayStub)},
                 {0x4de090, reinterpret_cast<void *>(lifeReleaseDisplayStub)},
                 {0x50ed80, reinterpret_cast<void *>(lifeFallbackStub)},
                 {0x513c20, reinterpret_cast<void *>(lifeLandingHealthStub)},
                 {0x514860, reinterpret_cast<void *>(lifeSetHealthStub)},
                 {0x4f5230, reinterpret_cast<void *>(lifeAdjustStub)},
                 {0x51a8f0, reinterpret_cast<void *>(lifeSettleStub)},
                 {0x50f850, reinterpret_cast<void *>(lifeMoveRefreshStub)},
                 {0x51aa30, reinterpret_cast<void *>(lifeInvalidateStub)},
                 {0x520100, reinterpret_cast<void *>(lifeExplodeStub)},
                 {0x5141e0, reinterpret_cast<void *>(lifeSpecialRemovalStub)},
                 {0x416d80, reinterpret_cast<void *>(lifeScoreStub)},
                 {0x52ccc0, reinterpret_cast<void *>(lifeCorpseStub)},
                 {0x52af60, reinterpret_cast<void *>(lifeCorpseReleaseStub)},
                 {0x50ab30, reinterpret_cast<void *>(resetStub)}})
    redirect(v.first, v.second);
}
// Additional owned fields can be compared while executing the complete parent
// body. The ordinary lifecycle suite keeps its original domain and defaults.
struct LifeCompareExtension {
  std::function<void()> install;
  std::function<void(Bytes &, bool)> pack;
  std::function<void()> verify;
};
void compareLife(const LifeFixture &input, const LifeFixture &expected,
                 unsigned op, bool ready, const std::vector<Event> &trace,
                 const LifeCompareExtension *extension = nullptr) {
  auto f = input;
  life = &f;
  fixture = &f.base;
  events.clear();
  prepare();
  installLife();
  auto env = maintenanceEnvironment();
  effectsLife.assign(4096, 0);
  displayLife.assign(32, 0);
  listLife.assign(4, 0);
  terrainLife.assign(32 * 32 * 4 * 12, 0);
  for (unsigned i = 0; i < f.corpseFlags.size(); ++i) {
    terrainLife[i * 12 + 8] = f.corpseFlags[i] & 255;
    terrainLife[i * 12 + 9] = f.corpseFlags[i] >> 8;
    terrainLife[i * 12 + 10] = f.corpseMarkers[i] & 255;
    terrainLife[i * 12 + 11] = f.corpseMarkers[i] >> 8;
  }
  for (unsigned i = 0; i < f.base.world.creatures.size(); ++i) {
    auto *p = actors.data() + i * 0xe4b;
    lifeExtra(p, f.base.world.creatures[i], f.base.execution[i],
              f.maintenance[i], f.movement[i]);
    write32(p, 0x600, reinterpret_cast<std::uintptr_t>(lifeActionStub));
    write32(p, 0x604, 0);
    write32(p, 0x160, 1000 + f.base.number % 3);
    auto *c = configs.data() + i * 0x5c9;
    write32(c, 0x28, env.powerMaximum);
    write32(c, 0x54, f.base.number % 2);
    write32(c, 0x5b1, unsigned(env.drainDivisor));
    global(0x6a652d + f.base.world.creatures[i].ai.type * 0x5c9,
           unsigned(env.effectDamage));
  }
  global(0x6f437c, reinterpret_cast<std::uintptr_t>(displayLife.data()));
  global(0x6defec, reinterpret_cast<std::uintptr_t>(listLife.data()));
  global(0x6c54dc, reinterpret_cast<std::uintptr_t>(terrainLife.data()));
  for (unsigned i = 0; i < 32; ++i)
    global(0x6cb942 + i * 4, i * 32);
  for (unsigned i = 0; i < 4; ++i)
    global(0x6cb8c2 + i * 4, i * 32 * 32);
  global(0x6df180, f.base.world.creatures.size());
  global(0x6c4830, env.tick);
  global(0x5e1904, 60);
  global(0x5e1908, 2);
  global(0x6e1f30, env.diagnostic);
  global(0x6bb743, env.statusDamage);
  global(0x6bf3c4, env.restoreMode);
  global(0x6bf519, env.drainQuantity);
  global(0x6bf248, env.statusInterval);
  global(0x6de830, env.poisonInterval);
  global(0x6a49dc, env.powerInterval);
  global(0x5e4e6c, 123);
  global(0x6bf244, 17);
  global(0x6bf268, 3);
  global(0x6bf0f3, 42);
  if (op == 1)
    redirect(0x51f5e0, reinterpret_cast<void *>(lifeInvalidateArgStub));
  if (extension) {
    extension->install();
    extension->pack(actors, false);
  }
  auto before = actors;
  unsigned entry = op == 0   ? 0x509e50
                   : op == 1 ? 0x510010
                   : op == 2 ? 0x50d580
                             : 0x50a8d0;
  const Bytes *body = nullptr;
  for (const auto &g : originalBodies)
    if (g.first == entry)
      body = &g.second;
  if (!body)
    throw std::runtime_error("life guard absent");
  std::memcpy(reinterpret_cast<void *>(entry), body->data(), body->size());
  if (op == 3) {
    using F = unsigned(__attribute__((thiscall)) *)(void *);
    if (bool(reinterpret_cast<F>(entry)(actors.data())) != ready)
      throw std::runtime_error("ready return");
  } else {
    using F = void(__attribute__((thiscall)) *)(void *);
    reinterpret_cast<F>(entry)(actors.data());
  }
  if (trace != events) {
    unsigned i = 0;
    while (i < trace.size() && i < events.size() && trace[i] == events[i])
      ++i;
    std::cerr << "life trace i=" << i << " actual=" << events.size()
              << " expected=" << trace.size() << '\n';
    if (i < events.size())
      std::cerr << "actual " << events[i].kind << "," << events[i].slot << ","
                << events[i].argument << " xyz=" << events[i].position.x << ","
                << events[i].position.y << "," << events[i].position.z << '\n';
    if (i < trace.size())
      std::cerr << "expected " << trace[i].kind << "," << trace[i].slot << ","
                << trace[i].argument << " xyz=" << trace[i].position.x << ","
                << trace[i].position.y << "," << trace[i].position.z << '\n';
    throw std::runtime_error(
        "life trace n=" + std::to_string(input.base.number) +
        " op=" + std::to_string(op));
  }
  for (unsigned i = 0; i < expected.base.world.creatures.size(); ++i) {
    packActor(before.data() + i * 0xe4b, expected.base.world.creatures[i],
              expected.base.execution[i]);
    lifeExtra(before.data() + i * 0xe4b, expected.base.world.creatures[i],
              expected.base.execution[i], expected.maintenance[i],
              expected.movement[i]);
  }
  if (extension)
    extension->pack(before, true);
  if (before != actors) {
    unsigned i = 0;
    while (i < actors.size() && before[i] == actors[i])
      ++i;
    throw std::runtime_error(
        "life write n=" + std::to_string(input.base.number) +
        " op=" + std::to_string(op) + " offset=" + std::to_string(i % 0xe4b) +
        " actual=" + std::to_string(actors[i]) +
        " expected=" + std::to_string(before[i]));
  }
  for (unsigned i = 0; i < expected.corpseFlags.size(); ++i)
    if (unsigned(terrainLife[i * 12 + 8]) +
                (unsigned(terrainLife[i * 12 + 9]) << 8) !=
            expected.corpseFlags[i] ||
        unsigned(terrainLife[i * 12 + 10]) +
                (unsigned(terrainLife[i * 12 + 11]) << 8) !=
            expected.corpseMarkers[i])
      throw std::runtime_error("corpse flag write");
  if (std::memcmp(reinterpret_cast<void *>(entry), body->data(), body->size()))
    throw std::runtime_error("life text modified");
  if (extension)
    extension->verify();
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_EXECUTION_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE expected");
  install(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  unsigned cases = 0;
  for (unsigned n = 0; n < 2048; ++n)
    for (unsigned op = 0; op < 4; ++op) {
      LifeFixture input;
      input.base = generate(n);
      auto count = input.base.world.creatures.size();
      input.maintenance.resize(count);
      input.movement.resize(count);
      input.corpseFlags.assign(4096, 0x80);
      input.corpseMarkers.assign(4096, 0);
      for (unsigned i = 0; i < count; ++i) {
        auto &a = input.base.world.creatures[i];
        auto &e = input.base.execution[i];
        auto &m = input.maintenance[i];
        auto &v = input.movement[i];
        auto value = [&](unsigned shift) {
          return unsigned((n >> shift) + i) % 4;
        };
        a.ai.kind = (n + i) % 28;
        a.ai.physical = (n + i) % 2;
        a.block7c = (n + i) % 5 == 0;
        a.specialVision = (n + i) % 23;
        e.mitigation90 = value(1);
        m.timer94 = value(2);
        a.ai.bloodLust = m.timer94 != 0;
        a.protection8c9 = value(3);
        a.form = value(4);
        a.formCapability = value(5);
        a.animationKind = value(6);
        a.flag104 = value(7);
        a.ai.flying = value(8);
        a.block71c = value(2) != 0;
        m.block71d = value(4) != 0;
        a.block723 = value(3) != 0;
        e.routeChanged = value(1) != 0;
        a.ai.marker = value(0) != 0;
        a.destination = n % 4 == 0   ? a.ai.position
                        : n % 4 == 1 ? Cell{a.ai.position.x + 1,
                                            a.ai.position.y, a.ai.position.z}
                                     : Cell{6, 7, 1};
        m.random98 = value(0);
        m.status114 = value(1);
        m.status118 = n % 3 ? 999 : 1001;
        m.lifespan = int(n % 50) - 5;
        m.status744 = value(2);
        m.status748 = value(3);
        m.status74c = 2;
        m.suspension = int(n % 7) - 3;
        m.flight110 = value(1);
        m.status778 = value(2);
        m.status77c = value(3);
        m.status780 = value(4);
        m.animation7cc = value(5) != 0;
        m.status821 = value(6);
        m.status8c5 = value(7);
        m.status8cd = int(n % 5) - 1;
        m.status78c = value(8);
        m.status790 = n % 3 ? 999 : 1001;
        m.status81d = value(1);
        m.status819 = value(2);
        m.timer660 = value(3);
        m.flight809 = value(4);
        m.status8d5 = value(5);
        m.status8d9 = n % 3 ? 999 : 1001;
        m.status8dd = 3;
        e.power84 = n % 23;
        m.powerDeadline = n % 3 ? 999 : 1001;
        m.recent6b0 = value(6);
        e.bonusMode124 = value(2);
        e.block855 = value(1);
        m.animation889 = value(2);
        m.animation8bd = value(3);
        m.statusEffect = n % 2 ? std::optional<unsigned>(0x2c9) : std::nullopt;
        m.damageEffect = n % 2 ? std::optional<unsigned>(0x2cf) : std::nullopt;
        m.flightEffect = n % 2 ? std::optional<unsigned>(0x2ee) : std::nullopt;
        for (unsigned j = 0; j < 8; ++j) {
          m.visibilityAge[j] = unsigned(-1) + (n + j) % 3;
          v.renewedOrder[j] = j + 10;
          a.ai.visible[j] = (n + i + j) % 7;
        }
        v.landingKind = 8;
        v.nextRouteCell = {3, 4, 1};
        a.ai.corpseA = value(1);
        a.ai.corpseB = value(2);
        a.rangedDistance = n % 3 ? 3 : 0;
        e.rangedDeadline = 1000 + n % 3;
        a.ai.readiness = value(2);
        a.ai.gateFC = op == 2 ? value(4) : 0;
      }
      auto expected = input;
      life = &expected;
      fixture = &expected.base;
      events.clear();
      bool ready = rangedReady(expected.base.world.creatures[0],
                               expected.base.execution[0], 1000);
      if (op != 3)
        nativeLife(expected, op);
      auto trace = events;
      mix(ready);
      for (const auto &v : trace) {
        mix(v.kind);
        mix(v.slot);
        mix(v.argument);
        mix(unsigned(v.position.x));
        mix(unsigned(v.position.y));
        mix(unsigned(v.position.z));
      }
      for (unsigned i = 0; i < count; ++i) {
        auto &a = expected.base.world.creatures[i];
        auto &m = expected.maintenance[i];
        mix(a.ai.health);
        mix(a.form);
        mix(m.timer94);
        mix(m.status8d5);
        mix(m.flight809);
        mix(m.visibilityAge[0]);
      }
#ifdef MNM_EXECUTION_REFERENCE
      compareLife(input, expected, op, ready, trace);
#endif
      ++cases;
    }
  std::cout << "{\"success\":true,\"cases\":" << cases
            << ",\"functions\":4,\"outcome_digest\":" << digest << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
