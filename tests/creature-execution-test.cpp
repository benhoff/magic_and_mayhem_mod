#include "../reconstruction/creature-ai/execution.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_EXECUTION_REFERENCE
#pragma push_macro("main")
#undef main
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#pragma pop_macro("main")
#endif
using namespace mnm::reconstruction::combat;
namespace {
struct Event {
  unsigned kind, slot, argument;
  Cell position;
  bool operator==(const Event &o) const {
    return kind == o.kind && slot == o.slot && argument == o.argument &&
           position == o.position;
  }
};
struct Fixture {
  AutonomousWorld world;
  std::vector<CreatureExecution> execution;
  CreatureScheduler scheduler;
  SchedulerEnvironment environment;
  unsigned number = 0;
};
Fixture *fixture;
std::vector<Event> events;
void record(unsigned kind, unsigned slot, unsigned argument = 0, Cell p = {}) {
  events.push_back({kind, slot, argument, p});
}
Fixture generate(unsigned n) {
  Fixture f;
  f.number = n;
  unsigned count = 1 + n % 128;
  f.world.creatures.resize(count);
  f.execution.resize(count);
  f.scheduler = {int(n % 80), int(n % count), int(n % 20), int(n % 90),
                 n % 3 == 0};
  f.environment = {n % 7 == 0, n % 2 == 0, n % 5 == 0, int(n % 8), {}};
  for (unsigned i = 0; i < 8; ++i)
    f.environment.ownerController[i] = (n + i) % 3 ? -1 : 4;
  for (unsigned i = 0; i < count; ++i) {
    auto &a = f.world.creatures[i];
    auto &e = f.execution[i];
    a.ai.slot = i;
    a.ai.owner = int((n + i) % 8);
    a.ai.type = (n + i) % 28;
    a.ai.active = (n + i) % 11 != 0;
    a.ai.health = (n + i) % 9 ? 400 : 0;
    constexpr unsigned kinds[] = {1, 2,  3,  4,  5,  6,  7, 8,
                                  9, 10, 11, 13, 14, 15, 16};
    a.ai.behavior = kinds[(n / 3 + i) % 15];
    a.ai.action = (n / 7 + i) % 25 + 1;
    a.ai.pending = n % 17 == 0 ? 8 : n % 16;
    a.ai.motion = (n + i) % 4 == 0;
    a.ai.flag704 = (n + i) % 3;
    a.ai.position = {int(n % 16), int(i % 16), int(n % 4)};
    a.ai.home = {2, 3, 1};
    a.ai.target = int((i + 1) % count);
    a.alternateTarget = int((i + 2) % count);
    a.ai.engaged = (n + i) % 3;
    a.ai.incoming = (n + i) % 5;
    a.mode = n % 31 == 0 ? 1 + n % 3 : 0;
    a.block784 = n % 37 == 0;
    a.blockDE3 = n % 41 == 0;
    a.block71c = n % 43 == 0;
    a.block723 = n % 47 == 0;
    a.cooldown = int(n % 23);
    a.ai.readiness = n % 5;
    a.substate = n % 10;
    a.routePhase = n % 3;
    a.destination = {5, 6, 2};
    e.absorptionTimer = n % 11;
    a.form = n % 53 == 0;
    e.mode10c = n % 4;
    a.ai.flying = n % 3;
    e.markerB73 = 5;
    a.ai.readiness = 7;
    e.block962 = 2;
    e.block963 = 9;
    e.block722 = 3;
    a.ai.dirty932 = 8;
    e.marker6a0 = 4;
    a.ai.marker = 6;
    e.routeRenewal = n % 2;
    e.commandMode = 1;
    e.follow = 3;
    e.marker68 = n % 2;
  }
  return f;
}
SchedulerServices schedulerServices() {
  SchedulerServices s;
  s.route = [](unsigned i, CreatureScheduler &p) {
    record(0, i);
    p.budget -= int(1 + (fixture->number + i) % 13);
    p.continuation = (fixture->number + i) % 4 == 0;
    return (fixture->number + i) % 3 == 0;
  };
  s.preDecision = [](unsigned i) {
    record(1, i);
    if (fixture->number % 2)
      fixture->world.creatures[i].ai.type = 22;
  };
  s.decision = [](unsigned i) { record(2, i); };
  s.valuation = [](unsigned i) { record(3, i); };
  s.region = [](unsigned i) {
    record(4, i);
    return int(i + 17);
  };
  s.ownerWork = [](unsigned i) { record(5, i); };
  return s;
}
ExecutionServices executionServices() {
  ExecutionServices s;
  s.behavior = [](auto &a, unsigned n) {
    record(6, a.ai.slot, n);
    ++a.cooldown;
  };
  s.action = [](auto &a, unsigned n) {
    record(7, a.ai.slot, n);
    a.ai.action = n;
  };
  s.restoreColor = [](unsigned n) { record(8, 0, n); };
  s.peerEngagement = [](auto &a, unsigned n) {
    record(9, a.ai.slot, n);
    a.ai.incoming = n;
  };
  s.releaseEffect = [](unsigned n) { record(10, 0, n); };
  s.movementRenewal = [](auto &a) {
    record(11, a.ai.slot);
    return fixture->number % 2 != 0;
  };
  s.orient = [](auto &a, Cell p) { record(12, a.ai.slot, 0, p); };
  s.deliverQueue = [](auto &a) { record(13, a.ai.slot); };
  s.updateQueue = [](auto &a) { record(14, a.ai.slot); };
  return s;
}
RouteExecutionServices routeServices() {
  RouteExecutionServices s;
  s.request = [](auto &a, Cell p, unsigned mode, unsigned flag) {
    if (flag != 1)
      throw std::runtime_error("route request flag");
    record(15, a.ai.slot, mode, p);
    return fixture->number % 3;
  };
  s.resumeDisplacement = [](auto &a) {
    record(16, a.ai.slot);
    return fixture->number % 17 == 0;
  };
  s.displacementPending = fixture->number % 2;
  s.queueFallback = [](auto &a) {
    record(17, a.ai.slot);
    return fixture->number % 2;
  };
  s.reach = [](const auto &a, const auto &b, Cell p) {
    record(18, a.ai.slot, b.ai.slot, p);
    return creatureDistance(p, b.ai.position, fixture->world.dimensions) <=
           int(fixture->number % 3);
  };
  s.adjust = [](const auto &a, const auto &, Cell &p) {
    record(19, a.ai.slot, 0, p);
    if (fixture->number % 3 == 0) {
      p.x = (p.x + 1) % fixture->world.dimensions.x;
      return true;
    }
    return false;
  };
  s.legal = [](const auto &a, Cell p) {
    record(20, a.ai.slot, 0, p);
    return (p.x + p.y + p.z + int(fixture->number)) % 3 != 0;
  };
  s.localReach = [](const auto &a, Cell p) {
    record(21, a.ai.slot, 0, p);
    return (p.x + p.y + p.z + int(fixture->number)) % 4 != 0;
  };
  s.beginMotion = [](auto &a) {
    record(22, a.ai.slot);
    a.routePhase = fixture->number % 2 ? 2 : 1;
  };
  s.reset = [](auto &a) { record(23, a.ai.slot); };
  s.action = executionServices().action;
  return s;
}

std::uint64_t digest = 1469598103934665603ull;
void mix(unsigned n) {
  for (unsigned i = 0; i < 4; ++i) {
    digest ^= (n >> (i * 8)) & 255;
    digest *= 1099511628211ull;
  }
}
#ifdef MNM_EXECUTION_REFERENCE
std::vector<std::pair<unsigned, Bytes>> guards;
std::vector<std::pair<unsigned, Bytes>> originalBodies;
Bytes actors, lists, schedulerRecord, configs;
unsigned read32(const void *p, unsigned o) {
  unsigned n;
  std::memcpy(&n, static_cast<const char *>(p) + o, 4);
  return n;
}
void write32(void *p, unsigned o, unsigned n) {
  std::memcpy(static_cast<char *>(p) + o, &n, 4);
}
void packActor(void *p, const AutonomousCreature &a,
               const CreatureExecution &e) {
  const std::pair<unsigned, unsigned> fields[] = {
      {0, a.ai.slot},
      {4, unsigned(a.ai.active)},
      {8, unsigned(a.ai.position.x)},
      {12, unsigned(a.ai.position.y)},
      {16, unsigned(a.ai.position.z)},
      {0xa8, a.ai.type},
      {0xe4, a.ai.health},
      {0x174, unsigned(a.ai.owner)},
      {0x5ec, a.ai.behavior},
      {0x5f0, a.ai.pending},
      {0x5fc, a.ai.action},
      {0x618, unsigned(a.ai.motion)},
      {0x64c, unsigned(a.ai.target)},
      {0x6a8, unsigned(a.alternateTarget)},
      {0x664, a.ai.engaged},
      {0x668, a.ai.incoming},
      {0x704, a.ai.flag704},
      {0x66c, unsigned(a.cooldown)},
      {0xbc3, unsigned(a.ai.home.x)},
      {0xbc7, unsigned(a.ai.home.y)},
      {0xbcb, unsigned(a.ai.home.z)},
      {0xa0, a.mode},
      {0x784, a.block784},
      {0xde3, a.blockDE3},
      {0x684, a.substate},
      {0x688, a.routePhase},
      {0x96b, unsigned(a.destination.x)},
      {0x96f, unsigned(a.destination.y)},
      {0x973, unsigned(a.destination.z)},
      {0x9c, e.absorptionTimer},
      {0x798, a.form},
      {0x10c, e.mode10c},
      {0x108, a.ai.flying},
      {0xb73, e.markerB73},
      {0x164, a.ai.readiness},
      {0x963, e.block963},
      {0x932, a.ai.dirty932},
      {0x6a0, e.marker6a0},
      {0xb8b, a.ai.marker},
      {0x6ac, e.routeRenewal},
      {0x698, e.commandMode},
      {0xb6f, e.routeChanged},
      {0x906, unsigned(e.follow)},
      {0xde7, unsigned(a.region)},
      {0x68, e.marker68},
      {0x6e8, e.routeEffect.value_or(0)},
      {0x902, e.queuedCount},
      {0x694, a.routeMode},
      {0x650, unsigned(a.chosenDestination.x)},
      {0x654, unsigned(a.chosenDestination.y)},
      {0x658, unsigned(a.chosenDestination.z)},
      {0x690, a.ai.corpseClaim}};
  for (auto v : fields)
    write32(p, v.first, v.second);
  for (auto v : {std::pair<unsigned, unsigned>{0x71c, a.block71c},
                 {0x723, a.block723},
                 {0x962, e.block962},
                 {0x722, e.block722}})
    static_cast<unsigned char *>(p)[v.first] =
        static_cast<unsigned char>(v.second);
}
void prepare() {
  unsigned count = fixture->world.creatures.size();
  actors.assign(count * 0xe4b, 0x9d);
  lists.assign(count * 16, 0);
  configs.assign(count * 0x5c9, 0);
  for (unsigned i = 0; i < count; ++i) {
    auto *p = actors.data() + i * 0xe4b;
    packActor(p, fixture->world.creatures[i], fixture->execution[i]);
    write32(p, 0x8f9, reinterpret_cast<std::uintptr_t>(lists.data() + i * 16));
    auto *c = configs.data() + i * 0x5c9;
    write32(p, 0xac, reinterpret_cast<std::uintptr_t>(c));
    const auto &v = fixture->world.creatures[i];
    write32(c, 0, v.ai.kind);
    write32(c, 8, v.width);
    write32(c, 12, v.height);
    write32(c, 0x34, v.rangedDistance);
    write32(c, 0x3c, v.formCapability);
    write32(c, 0x44, v.movementKind);
    write32(p, 0x5f4, 0);
    write32(p, 0x5f8, 0);
  }
  global(0x6def58, reinterpret_cast<std::uintptr_t>(actors.data()));
  global(0x6def5c, count);
  global(0x6c5494, fixture->world.dimensions.x);
  global(0x6c5498, fixture->world.dimensions.y);
  global(0x6c549c, fixture->world.dimensions.z);
  schedulerRecord.assign(0x268, 0);
  auto &s = fixture->scheduler;
  write32(schedulerRecord.data(), 0,
          reinterpret_cast<std::uintptr_t>(actors.data()));
  write32(schedulerRecord.data(), 4, count);
  for (auto v : {std::pair<unsigned, unsigned>{0x228, count},
                 {0x22c, unsigned(s.continuation)},
                 {0x230, unsigned(s.budget)},
                 {0x258, unsigned(s.cursor)},
                 {0x25c, unsigned(s.phase20)},
                 {0x264, unsigned(s.phase90)}})
    write32(schedulerRecord.data(), v.first, v.second);
  global(0x6e1f44, fixture->environment.suppressRoutes);
  global(0x5e1410, fixture->environment.updateRegions);
  global(0x68991c, fixture->environment.network);
  global(0x644520, fixture->environment.localOwner);
  for (unsigned i = 0; i < 8; ++i)
    global(0x643a80 + i * 0x154 + 12, fixture->environment.ownerController[i]);
}
unsigned slot(void *p) { return read32(p, 0); }
extern "C" unsigned __attribute__((thiscall)) routeStub(void *p) {
  unsigned i = slot(p);
  record(0, i);
  auto *q = schedulerRecord.data();
  write32(q, 0x230, read32(q, 0x230) - (1 + (fixture->number + i) % 13));
  write32(q, 0x22c, (fixture->number + i) % 4 == 0);
  return (fixture->number + i) % 3 == 0;
}
extern "C" void __attribute__((thiscall)) preStub(void *p) {
  unsigned i = slot(p);
  record(1, i);
  if (fixture->number % 2)
    write32(p, 0xa8, 22);
}
extern "C" void __attribute__((thiscall)) decisionStub(void *p) {
  record(2, slot(p));
}
extern "C" void __attribute__((thiscall)) valuationStub(void *p) {
  record(3, slot(p));
}
extern "C" void __attribute__((thiscall)) ownerStub(void *p) {
  record(5, slot(p));
}
struct RegionQuery {
  unsigned char bytes[84];
};
extern "C" unsigned __attribute__((thiscall))
regionStub(void *, int, int, int, RegionQuery, unsigned, unsigned) {
  unsigned i = events.back().slot;
  record(4, i);
  return i + 17;
}
extern "C" void *__attribute__((thiscall)) queryStub(void *self,
                                                     const RegionQuery *input) {
  std::memcpy(self, input, 84);
  return self;
}
extern "C" void __attribute__((thiscall)) behaviorStub(void *p) {
  record(6, slot(p), read32(p, 0x5ec));
  write32(p, 0x66c, read32(p, 0x66c) + 1);
}
extern "C" void __attribute__((thiscall)) actionStub(void *p, unsigned n) {
  record(7, slot(p), n);
  write32(p, 0x5fc, n);
}
extern "C" void __attribute__((thiscall)) colorStub(void *, unsigned n) {
  record(8, 0, n);
}
extern "C" void __attribute__((thiscall)) engagementStub(void *p, unsigned n) {
  record(9, slot(p), n);
  write32(p, 0x668, n);
}
extern "C" void __attribute__((thiscall)) releaseStub(void *p) {
  record(10, 0, reinterpret_cast<std::uintptr_t>(p));
}
extern "C" unsigned __attribute__((thiscall)) renewalStub(void *p) {
  record(11, slot(p));
  return fixture->number % 2;
}
extern "C" void __attribute__((thiscall)) orientStub(void *p, int x, int y) {
  record(12, slot(p), 0, {x, y, 0});
}
extern "C" void __attribute__((thiscall)) updateStub(void *p) {
  record(14, slot(p));
}
Bytes routeBody;
extern "C" unsigned __attribute__((thiscall))
requestStub(void *p, int x, int y, int z, unsigned mode, unsigned flag) {
  if (flag != 1)
    throw std::runtime_error("original route request flag");
  record(15, slot(p), mode, {x, y, z});
  return fixture->number % 3;
}
extern "C" unsigned __attribute__((thiscall)) resumeStub(void *p) {
  record(16, slot(p));
  return fixture->number % 17 == 0;
}
extern "C" unsigned __attribute__((thiscall)) fallbackStub(void *p) {
  record(17, slot(p));
  return fixture->number % 2;
}
extern "C" unsigned __attribute__((thiscall)) reachStub(void *p, void *b, int x,
                                                        int y, int z) {
  Cell q{int(read32(b, 8)), int(read32(b, 12)), int(read32(b, 16))};
  record(18, slot(p), slot(b), {x, y, z});
  return creatureDistance({x, y, z}, q, fixture->world.dimensions) <=
         int(fixture->number % 3);
}
extern "C" unsigned __attribute__((thiscall))
adjustStub(void *, unsigned actor, void *, int *x, int *y, int *z) {
  record(19, actor, 0, {*x, *y, *z});
  if (fixture->number % 3 == 0) {
    *x = (*x + 1) % fixture->world.dimensions.x;
    return 1;
  }
  return 0;
}
extern "C" unsigned __attribute__((thiscall)) legalStub(void *, int x, int y,
                                                        int z, void *) {
  record(20, 0, 0, {x, y, z});
  return (x + y + z + int(fixture->number)) % 3 != 0;
}
extern "C" unsigned __attribute__((thiscall)) localStub(void *p, int x, int y,
                                                        int z) {
  record(21, slot(p), 0, {x, y, z});
  return (x + y + z + int(fixture->number)) % 4 != 0;
}
extern "C" void __attribute__((thiscall)) beginStub(void *p) {
  record(22, slot(p));
  write32(p, 0x688, fixture->number % 2 ? 2 : 1);
}
extern "C" void __attribute__((thiscall)) resetStub(void *p) {
  record(23, slot(p));
}

void redirect(unsigned at, void *p) {
  auto *a = reinterpret_cast<unsigned char *>(at);
  a[0] = 0xe9;
  unsigned delta = reinterpret_cast<std::uintptr_t>(p) - at - 5;
  std::memcpy(a + 1, &delta, 4);
}
const unsigned behaviorAddresses[16] = {0x50acb0, 0x510010, 0x50afa0, 0x50ccd0,
                                        0x50d580, 0x50d840, 0x50d860, 0x50d850,
                                        0x50d870, 0x50d8f0, 0x51c510, 0,
                                        0x51c230, 0x521750, 0x521770, 0x521780};
const unsigned actionAddresses[25] = {
    0x50f090, 0x5104b0, 0x50f360, 0,        0,        0x50f090, 0,
    0x50e6f0, 0x50e670, 0x50e7a0, 0,        0,        0x50f3b0, 0x50f590,
    0x50f7c0, 0x50f090, 0x50f090, 0x50eba0, 0x50f090, 0x5170e0, 0x521790,
    0x50f090, 0x5217e0, 0x521dd0, 0x50f090};
void install(const char *path) {
  map_image(read(path));
  for (auto r : {std::pair<unsigned, unsigned>{0x50f090,0x50f329},{0x50f360,0x50f3ae},{0x50e7a0,0x50eb24},{0x5170e0,0x517427},{0x50eba0,0x50ec71},{0x50f590,0x50f7bb},{0x509e50,0x50a8d0},{0x50a8d0,0x50a92c},{0x510010,0x510480},{0x50d580,0x50d817},{0x50acb0,0x50ad03},
                 {0x50afa0,0x50b0b1},{0x50b0c0,0x50b1e3},
                 {0x50ccd0,0x50cdb4},{0x50d870,0x50d8f0},
                 {0x50d8f0,0x50d9e3},{0x51c230,0x51c398},
                 {0x50d840,0x50d84e},{0x50d850,0x50d85e},
                 {0x50d860,0x50d86e},{0x521750,0x521766},
                 {0x521770,0x521778},{0x521780,0x521788},
                 {0x523470,0x523858},{0x523860,0x523bc0},{0x50db60, 0x50e36b},
                 {0x5244f0, 0x5248e2},
                 {0x522ad0, 0x522dfe},
                 {0x505cf0, 0x505d18},
                 {0x50b2c0, 0x50b643},
                 {0x50e670, 0x50e6ea},
                 {0x50e6f0, 0x50e791},
                 {0x50f7c0, 0x50f84f},
                 {0x50ad10, 0x50af02},
                 {0x50ca30, 0x50cc0e},
                 {0x513850, 0x51397f},
                 {0x508b00, 0x508c95},
                 {0x523be0, 0x5240ae},
                 {0x524130, 0x524382},
                 {0x5243b0, 0x524460},
                 {0x520ad0, 0x520b24}})
    originalBodies.push_back(
        {r.first, Bytes(reinterpret_cast<unsigned char *>(r.first),
                        reinterpret_cast<unsigned char *>(r.second))});
  routeBody = Bytes(reinterpret_cast<unsigned char *>(0x519ae0),
                    reinterpret_cast<unsigned char *>(0x51a5b6));
  for (auto r : {std::pair<unsigned, unsigned>{0x52a780, 0x52a9ee},
                 {0x50a930, 0x50a9e2},
                 {0x50aa30, 0x50ab05},
                 {0x50ab30, 0x50ac91},
                 {0x524460, 0x5244ea},
                 {0x50fae0, 0x50fd4c},
                 {0x50da50, 0x50daf8}})
    guards.push_back(
        {r.first, Bytes(reinterpret_cast<unsigned char *>(r.first),
                        reinterpret_cast<unsigned char *>(r.second))});
  for(const auto &g:guards)originalBodies.push_back(g);
  redirect(0x519ae0, reinterpret_cast<void *>(routeStub));
  redirect(0x522ad0, reinterpret_cast<void *>(preStub));
  redirect(0x518420, reinterpret_cast<void *>(decisionStub));
  redirect(0x5244f0, reinterpret_cast<void *>(valuationStub));
  redirect(0x523470, reinterpret_cast<void *>(ownerStub));
  redirect(0x53d7d0, reinterpret_cast<void *>(regionStub));
  redirect(0x441560, reinterpret_cast<void *>(queryStub));
  for (auto a : behaviorAddresses)
    if (a)
      redirect(a, reinterpret_cast<void *>(behaviorStub));
  redirect(0x50db60, reinterpret_cast<void *>(actionStub));
  redirect(0x54dce0, reinterpret_cast<void *>(colorStub));
  redirect(0x50eb60, reinterpret_cast<void *>(engagementStub));
  redirect(0x4943f0, reinterpret_cast<void *>(releaseStub));
  redirect(0x516fd0, reinterpret_cast<void *>(renewalStub));
  redirect(0x5084f0, reinterpret_cast<void *>(orientStub));
  redirect(0x5120c0, reinterpret_cast<void *>(updateStub));
  redirect(0x5128a0, reinterpret_cast<void *>(requestStub));
  redirect(0x51a600, reinterpret_cast<void *>(resumeStub));
  redirect(0x50ed80, reinterpret_cast<void *>(fallbackStub));
  redirect(0x50bd60, reinterpret_cast<void *>(reachStub));
  redirect(0x4f5230, reinterpret_cast<void *>(adjustStub));
  redirect(0x4f4900, reinterpret_cast<void *>(legalStub));
  redirect(0x4f5a40, reinterpret_cast<void *>(localStub));
  redirect(0x510e80, reinterpret_cast<void *>(beginStub));
}
void checkOriginal(const Fixture &initial, const Fixture &expected,
                   unsigned operation, unsigned argument,
                   const std::vector<Event> &trace) {
  Fixture input = initial;
  fixture = &input;
  events.clear();
  prepare();
  auto before = actors;
  auto *a = actors.data();
  using Step = void(__attribute__((thiscall)) *)(void *);
  using Value = void(__attribute__((thiscall)) *)(void *, unsigned);
  if (operation == 0)
    reinterpret_cast<Step>(0x52a780)(schedulerRecord.data());
  else if (operation == 1)
    reinterpret_cast<Value>(0x50aa30)(a, argument);
  else if (operation == 2)
    reinterpret_cast<Step>(0x50ab30)(a);
  else if (operation == 3)
    reinterpret_cast<Value>(0x524460)(a, argument);
  else if (operation == 4) {
    using Move = void(__attribute__((thiscall)) *)(void *, int, int, int);
    reinterpret_cast<Move>(0x50fae0)(a, 5, 7, 2);
  }
  if (operation == 5) {
    std::memcpy(reinterpret_cast<void *>(0x519ae0), routeBody.data(), 5);
    redirect(0x50ab30, reinterpret_cast<void *>(resetStub));
    global(0x6df184, fixture->number % 2);
    using Route = unsigned(__attribute__((thiscall)) *)(void *);
    auto result = reinterpret_cast<Route>(0x519ae0)(a);
    if (result != argument)
      throw std::runtime_error("route return mismatch");
    if (std::memcmp(reinterpret_cast<void *>(0x519ae0), routeBody.data(),
                    routeBody.size()))
      throw std::runtime_error("original route text changed");
    for (const auto &g : guards)
      if (g.first == 0x50ab30)
        std::memcpy(reinterpret_cast<void *>(g.first), g.second.data(), 5);
    redirect(0x519ae0, reinterpret_cast<void *>(routeStub));
  }
  if (events != trace) {
    for (auto v : events)
      std::cerr << "actual " << v.kind << "," << v.slot << "," << v.argument
                << " point=" << v.position.x << "," << v.position.y << ","
                << v.position.z << "\n";
    for (auto v : trace)
      std::cerr << "expected " << v.kind << "," << v.slot << "," << v.argument
                << " point=" << v.position.x << "," << v.position.y << ","
                << v.position.z << "\n";
    throw std::runtime_error("execution helper ordering mismatch n=" +
                             std::to_string(initial.number) +
                             " op=" + std::to_string(operation));
  }
  for (unsigned i = 0; i < expected.world.creatures.size(); ++i)
    packActor(before.data() + i * 0xe4b, expected.world.creatures[i],
              expected.execution[i]);
  for (auto v : trace)
    if (v.kind == 6) {
      write32(before.data() + v.slot * 0xe4b, 0x5f4,
              behaviorAddresses[v.argument - 1]);
      write32(before.data() + v.slot * 0xe4b, 0x5f8, 0);
    }
  if (actors != before) {
    unsigned at = 0;
    while (at < actors.size() && actors[at] == before[at])
      ++at;
    throw std::runtime_error(
        "execution actor write mismatch n=" + std::to_string(initial.number) +
        " op=" + std::to_string(operation) +
        " offset=" + std::to_string(at % 0xe4b));
  }
  if (operation == 0) {
    auto &p = expected.scheduler;
    for (auto v : {std::pair<unsigned, unsigned>{0x230, unsigned(p.budget)},
                   {0x258, unsigned(p.cursor)},
                   {0x25c, unsigned(p.phase20)},
                   {0x264, unsigned(p.phase90)},
                   {0x22c, unsigned(p.continuation)}})
      if (read32(schedulerRecord.data(), v.first) != v.second)
        throw std::runtime_error("scheduler state mismatch");
  }
  for (auto &g : guards)
    if (std::memcmp(reinterpret_cast<void *>(g.first), g.second.data(),
                    g.second.size()))
      throw std::runtime_error("original tested text changed");
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_EXECUTION_REFERENCE
  if (argc != 2)
    throw std::runtime_error("expected pinned PE");
  install(argv[1]);
  using Resolve = std::uint64_t(__attribute__((thiscall)) *)(void *, unsigned);
  for (unsigned n = 0; n < 40; ++n) {
    auto b = reinterpret_cast<Resolve>(0x50a930)(nullptr, n);
    auto a = reinterpret_cast<Resolve>(0x50da50)(nullptr, n);
    auto bh = behaviorHandler(n), ah = actionHandler(n);
    if (b != (bh ? behaviorAddresses[*bh - 1] : 0u) ||
        a != (ah ? actionAddresses[*ah - 1] : 0u))
      throw std::runtime_error("dispatch mapping mismatch");
  }
#else
  (void)argc;
  (void)argv;
#endif
  unsigned cases = 0;
  for (unsigned n = 0; n < 4096; ++n)
    for (unsigned op = 0; op < 6; ++op) {
      auto initial = generate(n);
      if (op == 5) {
        auto &a = initial.world.creatures[0];
        a.ai.marker = n % 11 == 0;
        a.ai.action = 15;
        a.cooldown = int(n % 4);
        constexpr unsigned kinds[] = {2, 3, 4, 11, 13, 14, 1, 9, 10};
        a.ai.behavior = kinds[n % 9];
        a.substate = 1 + (n / 9) % 9;
        a.ai.flying = n % 2;
        a.chosenDestination = {5, 7, 2};
        a.width = 1 + n % 2;
      }
      auto expected = initial;
      fixture = &expected;
      events.clear();
      auto s = executionServices();
      auto &a = expected.world.creatures[0];
      auto &e = expected.execution[0];
      unsigned kind = 1 + n % 16;
      if (kind == 12)
        kind = 11;
      if (op == 0)
        scheduleCreatures(expected.world, expected.scheduler,
                          expected.environment, schedulerServices());
      else if (op == 1)
        changeBehavior(a, e, kind, s);
      else if (op == 2)
        resetBehavior(a, e, expected.world, s);
      else if (op == 3)
        setFollow(a, e, expected.world, n % 3 ? 2 : -1, s);
      else if (op == 4)
        orderMove(a, e, expected.world, {5, 7, 2}, s);
      else
        kind = continueCreatureRoute(a, e, expected.world, routeServices());
      auto trace = events;
      for (auto &b : expected.world.creatures) {
        mix(b.ai.behavior);
        mix(b.ai.pending);
        mix(b.ai.action);
        mix(unsigned(b.cooldown));
        mix(unsigned(b.region));
        mix(b.ai.incoming);
      }
      mix(unsigned(expected.scheduler.budget));
      mix(unsigned(expected.scheduler.cursor));
      mix(expected.scheduler.continuation);
      mix(unsigned(expected.scheduler.phase20));
      mix(unsigned(expected.scheduler.phase90));
      for (auto v : trace) {
        mix(v.kind);
        mix(v.slot);
        mix(v.argument);
        mix(unsigned(v.position.x));
        mix(unsigned(v.position.y));
        mix(unsigned(v.position.z));
      }
#ifdef MNM_EXECUTION_REFERENCE
      checkOriginal(initial, expected, op,
                    op == 3 ? (n % 3 ? 2u : 0xffffffffu) : kind, trace);
#endif
      ++cases;
    }
  std::cout << "{\"success\":true,\"cases\":" << cases
            << ",\"functions\":8,\"outcome_digest\":" << digest << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
