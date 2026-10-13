#include "../reconstruction/casting/admission.hpp"
#include "../reconstruction/casting/admission_tables.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_CAST_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#include <asm/ldt.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif
using namespace mnm::reconstruction::casting;
namespace {
struct Context {
  AutonomousWorld world;
  CastAdmissionState state;
  CastDescriptor command{};
  unsigned number = 0;
};
Context *current;
std::vector<std::uint32_t> trace;
bool original = false;
std::array<unsigned, 19> serviceCounts{};
void event(unsigned kind, std::initializer_list<unsigned> args = {}) {
  if (kind >= 500 && kind < 519)
    ++serviceCounts[kind - 500];
  trace.push_back(kind);
  trace.push_back(unsigned(args.size()));
  trace.insert(trace.end(), args.begin(), args.end());
}
void descriptorEvent(unsigned kind, const CastEffectDescriptor &d) {
  event(kind, {63});
  trace.insert(trace.end(), d.begin(), d.end());
}
unsigned bits(int n) { return unsigned(n); }
int signedBits(unsigned n) { return n <= 0x7fffffffu ? int(n) : -1 - int(~n); }
bool profilePresent() { return current->number % 17 != 0; }
bool pathResult() { return current->number % 4 != 0; }
bool pathWork(Cell a, Cell b, CastEffectDescriptor &d) {
  event(509, {bits(a.x), bits(a.y), bits(a.z), bits(b.x), bits(b.y), bits(b.z),
              d[9], d[6], d[7], d[8], 1});
  descriptorEvent(510, d);
  d[62] = current->number ^ 0x57b710u;
  return pathResult();
}
#ifdef MNM_CAST_REFERENCE
Bytes actors, configs;
unsigned load(const void *p, unsigned offset) {
  unsigned n;
  std::memcpy(&n, static_cast<const unsigned char *>(p) + offset, 4);
  return n;
}
void put(void *p, unsigned offset, unsigned n) {
  std::memcpy(static_cast<unsigned char *>(p) + offset, &n, 4);
}
unsigned slot(const void *p) {
  auto distance = static_cast<const unsigned char *>(p) - actors.data();
  if (distance < 0 || unsigned(distance) >= actors.size() || distance % 0xe4b)
    throw std::runtime_error("cast actor ABI");
  return unsigned(distance) / 0xe4b;
}
#endif
void resourceWork(unsigned i, unsigned delta) {
  event(506, {i, delta});
#ifdef MNM_CAST_REFERENCE
  if (original) {
    auto *p = actors.data() + i * 0xe4b;
    put(p, 0xe8, load(p, 0xe8) + delta);
    return;
  }
#endif
  current->state.resource.at(i) += delta;
}
void relocationWork(unsigned i, Cell p, bool special) {
  event(513, {i, bits(p.x), bits(p.y), bits(p.z), unsigned(special)});
#ifdef MNM_CAST_REFERENCE
  if (original) {
    auto *a = actors.data() + i * 0xe4b;
    for (auto x : {std::pair<unsigned, unsigned>{8, bits(p.x)},
                   {12, bits(p.y)},
                   {16, bits(p.z)},
                   {20, bits(p.x) * 32u},
                   {24, bits(p.y) * 32u},
                   {28, bits(p.z) * 16u}})
      put(a, x.first, x.second);
    return;
  }
#endif
  auto &a = current->world.creatures.at(i);
  a.ai.position = p;
  a.finePosition = {signedBits(bits(p.x) * 32u), signedBits(bits(p.y) * 32u),
                    signedBits(bits(p.z) * 16u)};
}
void areaWork(unsigned i, unsigned a, unsigned b) {
  event(514, {i, a, b});
#ifdef MNM_CAST_REFERENCE
  if (original) {
    put(actors.data() + i * 0xe4b, 0x66c, b);
    return;
  }
#endif
  current->world.creatures.at(i).cooldown = int(b);
}
bool adjustWork(unsigned i, const CastMovementDescriptor &d, Cell &p) {
  event(512, {i, d.height, d.width, d.flight, d.movementKind, 0, d.creatureType,
              unsigned(d.restricted), bits(p.x), bits(p.y), bits(p.z)});
  p.x = (p.x + 3) % current->world.dimensions.x;
  return current->number % 2 != 0;
}
CastAdmissionServices services() {
  CastAdmissionServices s;
  s.pool = []() -> std::optional<unsigned> {
    event(500);
    return current->number % 11 ? std::optional<unsigned>(0x9001)
                                : std::nullopt;
  };
  s.initializePath = [] { event(501); };
  s.currentProfile = []() -> std::optional<unsigned> {
    event(502);
    return profilePresent() ? std::optional<unsigned>(0) : std::nullopt;
  };
  s.nextActor = []() -> std::optional<unsigned> {
    event(503);
    auto &i = current->state.iteratorCursor;
    if (i >= current->world.creatures.size())
      return std::nullopt;
    return i++;
  };
  s.visibility = [](auto &a, unsigned owner, unsigned delta) {
    event(504, {a.ai.slot, owner, delta});
  };
  s.containsPoint = [](const auto &a, Cell p) {
    event(505, {a.ai.slot, bits(p.x), bits(p.y), bits(p.z)});
    return (current->number + a.ai.slot) % 3 == 0;
  };
  s.chargeable = [](const auto &a) {
    event(507, {a.ai.slot});
    return current->number % 4 != 0;
  };
  s.ownerMessage = [](const auto &a) {
    event(508, {a.ai.slot});
    return current->number % 7 != 0;
  };
  s.resource = [](auto &a, unsigned delta) { resourceWork(a.ai.slot, delta); };
  s.insufficientOwnerMessage = [] { event(515, {0x3f3}); };
  s.cue = [](unsigned kind, Cell p) {
    event(516, {kind, bits(p.x), bits(p.y), bits(p.z)});
  };
  s.castCue = [](unsigned spell, Cell p, unsigned flags) {
    event(517, {spell, bits(p.x), bits(p.y), bits(p.z), flags});
  };
  s.path = pathWork;
  s.secondary = [](unsigned pool, unsigned kind, const auto &d) {
    event(511, {pool, kind});
    descriptorEvent(518, d);
  };
  s.adjust = adjustWork;
  s.relocate = [](auto &a, Cell p, bool b) { relocationWork(a.ai.slot, p, b); };
  s.areaEffect = [](auto &a, unsigned n, unsigned mode) {
    areaWork(a.ai.slot, n, mode);
  };
  return s;
}
Context generate(unsigned n) {
  Context f;
  f.number = n;
  f.world.dimensions = {16, 16, 4};
  f.world.localOwner = int(n % 10) - 1;
  unsigned count = 1 + n % 8;
  f.world.creatures.resize(count);
  f.state.resource.resize(count);
  f.state.direction.resize(count);
  for (unsigned i = 0; i < count; ++i) {
    auto &a = f.world.creatures[i];
    a.ai.slot = i;
    a.ai.owner = int((n + i) % 8);
    a.ai.type = (n / 8 + i) % 28;
    a.ai.kind = (n / 16 + i) % 28;
    a.ai.health = (n + i) % 3 ? 400 : 0;
    a.ai.position = {int((n + i) % 16), int((n / 16 + i) % 16),
                     int((n + i) % 4)};
    a.finePosition = {int((n + i * 23) % 550) - 35,
                      int((n + i * 37) % 530) - 15, int((n + i) % 64)};
    a.aimXY = int(n % 17) - 8;
    a.aimZ = int(n % 13) - 6;
    a.block7c = n % 5 == 0;
    a.width = 1 + n % 2;
    a.height = 1 + n % 4;
    a.flag104 = n % 3;
    a.movementKind = n % 8;
    a.block723 = n % 2;
    a.ai.behavior = n % 16 + 1;
    a.cooldown = 71;
    for (unsigned j = 0; j < 8; ++j)
      a.ai.visible[j] = (n + i + j) % 4;
    f.state.resource[i] = (n % 23) * 256u;
    f.state.direction[i] = (n / 28 + i) % 8;
  }
  f.state.spells.resize(104);
  for (unsigned i = 0; i < 104; ++i) {
    auto &c = f.state.spells[i];
    c.cost = (n / 124 + i) % 19 == 0 ? 0 : 1 + (n + i) % 17;
    c.pathMode = 29 + i;
    c.travelKind = int(n / 124) % 5;
    c.resourceMode = n % 4;
    c.resourceDelta = n % 17 == 0 ? 0x80000001u : 11 + n;
    for (unsigned j = 0; j < 8; ++j)
      for (unsigned k = 0; k < 8; ++k)
        c.observed[j][k] = (n + j + k) % 3;
  }
  f.state.profiles.resize(1);
  for (unsigned i = 0; i < 124; ++i)
    f.state.profiles[0].known[i] = (n + i) % 4;
  for (unsigned i = 0; i < std::min(20u, n % 23); ++i)
    f.state.profiles[0].notices.push_back({9, i});
  for (unsigned t = 0; t < 28; ++t)
    for (unsigned d = 0; d < 8; ++d)
      f.state.muzzle[t][d] = {int(t % 7) - 3, int(d) - 4, int(t + d) % 9 - 4};
  f.state.resourceCounter = 0x7fffff00u + n;
  f.state.specialValue = 0xfffffff0u + n;
  f.state.iteratorCursor = 73;
  f.command = {n % 104,
               unsigned(int(n % 600) - 50),
               n % 600 + 10,
               unsigned(int(n % 120) - 20),
               n % 700,
               n % 700 + 10,
               n % 90,
               n % 13 ? 0u : UINT32_MAX,
               n % 5 ? (count > 1 ? 1u : 0u) : UINT32_MAX,
               n / 620 % 5,
               n ^ 0x512u,
               n % 3};
  unsigned arm = f.command[0] < 103 ? castArms[f.command[0]] : 16;
  if (arm == 2 && f.state.spells[f.command[0]].travelKind == 2)
    f.command[7] = 0;
  if (f.command[0] == 55) {
    f.command[7] = 0;
    f.command[8] = count > 1 ? 1 : 0;
    f.command[4] = 130;
    f.command[5] = 210;
    f.command[6] = 33;
    for (auto &a : f.world.creatures) {
      a.finePosition.x = 100;
      a.finePosition.y = 150;
      a.aimXY = 3;
    }
  }
  return f;
}
unsigned refusalCases() {
  for (unsigned n = 0; n < 8; ++n) {
    auto input = generate(1);
    input.command[0] = n >= 5 && n != 6 ? 62 : 0;
    input.command[7] = 0;
    input.command[8] = 1;
    input.command[9] = 0;
    input.state.spells[input.command[0]].cost = 1;
    switch (n) {
    case 0:
      input.state.spells.clear();
      break;
    case 1:
      input.command[7] = unsigned(input.world.creatures.size());
      break;
    case 2:
      input.command[8] = unsigned(input.world.creatures.size());
      break;
    case 3:
      input.world.creatures[0].ai.owner = 8;
      break;
    case 4:
      input.state.direction[0] = 8;
      break;
    case 5:
      input.world.dimensions.x = 0;
      break;
    case 6:
      input.command[0] = 55;
      input.command[7] = UINT32_MAX;
      input.command[9] = 1;
      break;
    case 7:
      input.world.dimensions.x = 0x4000000;
      break;
    }
    auto staged = input;
    current = &staged;
    original = false;
    bool refused = false;
    try {
      admitCast(staged.command, staged.world, staged.state, services());
    } catch (const std::logic_error &) {
      refused = true;
    }
    if (!refused)
      throw std::runtime_error("cast native refusal missing " +
                               std::to_string(n));
  }
  return 8;
}

#ifdef MNM_CAST_REFERENCE
std::array<unsigned char, 0xf21> testedBody{};
void *pathObject = nullptr;
void redirect(unsigned a, void *target) {
  auto *p = reinterpret_cast<unsigned char *>(a);
  p[0] = 0xe9;
  unsigned delta = reinterpret_cast<std::uintptr_t>(target) - a - 5;
  std::memcpy(p + 1, &delta, 4);
}
struct PrivateFs {
  std::array<unsigned, 8> data{};
  unsigned short saved = 0;
  user_desc descriptor{};
  PrivateFs() {
    data.fill(0xa5a5a5a5);
    data[0] = 0xffffffffu;
    descriptor.entry_number = unsigned(-1);
    descriptor.base_addr = reinterpret_cast<std::uintptr_t>(data.data());
    descriptor.limit = 31;
    descriptor.seg_32bit = 1;
    descriptor.useable = 1;
    if (syscall(SYS_set_thread_area, &descriptor))
      throw std::runtime_error("private cast FS allocation");
    asm volatile("mov %%fs,%0" : "=rm"(saved));
    unsigned short selector =
        static_cast<unsigned short>((descriptor.entry_number << 3) | 3);
    asm volatile("mov %0,%%fs" ::"rm"(selector) : "memory");
  }
  ~PrivateFs() {
    asm volatile("mov %0,%%fs" ::"rm"(saved) : "memory");
    user_desc empty{};
    empty.entry_number = descriptor.entry_number;
    empty.seg_not_present = 1;
    empty.read_exec_only = 1;
    (void)syscall(SYS_set_thread_area, &empty);
  }
  void verify() {
    if (data[0] != UINT32_MAX ||
        std::any_of(data.begin() + 1, data.end(),
                    [](unsigned n) { return n != 0xa5a5a5a5; }))
      throw std::runtime_error("cast FS chain/guard changed");
  }
};
void packActor(void *p, const AutonomousCreature &a, unsigned resource,
               unsigned direction) {
  for (auto x : {std::pair<unsigned, unsigned>{0, a.ai.slot},
                 {4, unsigned(a.ai.active)},
                 {8, bits(a.ai.position.x)},
                 {12, bits(a.ai.position.y)},
                 {16, bits(a.ai.position.z)},
                 {20, bits(a.finePosition.x)},
                 {24, bits(a.finePosition.y)},
                 {28, bits(a.finePosition.z)},
                 {0x7c, a.block7c},
                 {0xa8, a.ai.type},
                 {0xe4, a.ai.health},
                 {0xe8, resource},
                 {0x104, a.flag104},
                 {0x174, bits(a.ai.owner)},
                 {0x5ec, a.ai.behavior},
                 {0x608, direction},
                 {0x66c, bits(a.cooldown)}})
    put(p, x.first, x.second);
  static_cast<unsigned char *>(p)[0x723] = a.block723;
  for (unsigned j = 0; j < 8; ++j)
    put(p, 0x24 + j * 4, a.ai.visible[j]);
}
Bytes profileBytes(const CastLearningState &p) {
  Bytes b(0x580, 0xb6);
  for (unsigned i = 0; i < 124; ++i)
    b[0x26c + i] = p.known[i];
  for (unsigned i = 0; i < p.notices.size(); ++i) {
    put(b.data(), 0x4d8 + i * 8, p.notices[i][0]);
    put(b.data(), 0x4dc + i * 8, p.notices[i][1]);
  }
  put(b.data(), 0x578, p.notices.size());
  return b;
}
void packSpells(void *base, const CastAdmissionState &s) {
  for (unsigned i = 0; i < s.spells.size(); ++i) {
    auto *p = static_cast<unsigned char *>(base) + i * 721;
    const auto &c = s.spells[i];
    for (auto x : {std::pair<unsigned, unsigned>{0, c.cost},
                   {0x14d, c.pathMode},
                   {0x151, bits(c.travelKind)},
                   {0x15d, c.resourceMode},
                   {0x161, c.resourceDelta}})
      put(p, x.first, x.second);
    for (unsigned j = 0; j < 8; ++j)
      for (unsigned k = 0; k < 8; ++k)
        put(p, 0x17d + (j * 8 + k) * 4, c.observed[j][k]);
  }
}
Bytes profile;
extern "C" void *__attribute__((thiscall)) castPool(void *p) {
  if (p != reinterpret_cast<void *>(0x689498))
    throw std::runtime_error("pool receiver");
  event(500);
  return current->number % 11 ? reinterpret_cast<void *>(0xc001) : nullptr;
}
extern "C" void *__attribute__((thiscall)) castPathInit(void *p) {
  event(501);
  pathObject = p;
  std::memset(p, 0, 72);
  return p;
}
extern "C" unsigned castProfile() {
  event(502);
  return 0;
}
extern "C" void *__attribute__((thiscall)) castIterator(void *p) {
  if (p != reinterpret_cast<void *>(0x6def58))
    throw std::runtime_error("iterator receiver");
  event(503);
  unsigned i = load(reinterpret_cast<void *>(0x6def60), 0);
  if (i >= current->world.creatures.size())
    return nullptr;
  global(0x6def60, i + 1);
  return actors.data() + i * 0xe4b;
}
extern "C" void __attribute__((thiscall)) castVisible(void *p, unsigned owner,
                                                      unsigned delta) {
  event(504, {slot(p), owner, delta});
}
extern "C" unsigned __attribute__((thiscall)) castContains(void *p, int x,
                                                           int y, int z) {
  unsigned i = slot(p);
  event(505, {i, bits(x), bits(y), bits(z)});
  return (current->number + i) % 3 == 0;
}
extern "C" unsigned __attribute__((thiscall)) castChargeable(void *p) {
  auto distance = static_cast<unsigned char *>(p) - configs.data();
  if (distance < 0 || unsigned(distance) >= configs.size() || distance % 0x5c9)
    throw std::runtime_error("chargeable receiver");
  event(507, {unsigned(distance) / 0x5c9});
  return current->number % 4 != 0;
}
extern "C" unsigned __attribute__((thiscall)) castOwnerMessage(void *p) {
  event(508, {slot(p)});
  return current->number % 7 != 0;
}
extern "C" void __attribute__((thiscall)) castResource(void *p,
                                                       unsigned delta) {
  resourceWork(slot(p), delta);
}
extern "C" void __attribute__((thiscall)) castMessage(void *p, unsigned code,
                                                      int a, int b, int c,
                                                      unsigned d, int g,
                                                      int h) {
  if (p != reinterpret_cast<void *>(0x6b0198) || code != 0x3f3 || a || b || c ||
      d || g != -1 || h != -1)
    throw std::runtime_error("cast owner-message ABI");
  event(515, {code});
}
extern "C" void __attribute__((thiscall)) castCue(void *p, unsigned code, int x,
                                                  int y, int z, unsigned a,
                                                  unsigned b, unsigned c) {
  if (p != reinterpret_cast<void *>(0x658178) || a || b || c)
    throw std::runtime_error("cast cue ABI");
  event(516, {code, bits(x), bits(y), bits(z)});
}
extern "C" void __attribute__((thiscall))
castSound(void *p, unsigned code, int x, int y, int z, unsigned flags) {
  if (p != reinterpret_cast<void *>(0x658178))
    throw std::runtime_error("cast sound ABI");
  event(517, {code, bits(x), bits(y), bits(z), flags});
}
extern "C" unsigned __attribute__((thiscall))
castPath(void *receiver, int sx, int sy, int sz, int dx, int dy, int dz,
         unsigned mode, unsigned source, unsigned target, unsigned spell,
         unsigned flags, void *record) {
  if (receiver != pathObject)
    throw std::runtime_error("cast path receiver");
  CastEffectDescriptor d{};
  std::memcpy(d.data(), record, 252);
  if (mode != d[9] || source != d[6] || target != d[7] || spell != d[8] ||
      flags != 1)
    throw std::runtime_error("cast path ABI");
  bool result = pathWork({sx, sy, sz}, {dx, dy, dz}, d);
  std::memcpy(record, d.data(), 252);
  return result;
}
extern "C" unsigned __attribute__((thiscall))
castSecondary(void *p, unsigned kind, void *record) {
  if (p != reinterpret_cast<void *>(0xc001))
    throw std::runtime_error("secondary receiver");
  CastEffectDescriptor d{};
  std::memcpy(d.data(), record, 252);
  event(511, {0x9001, kind});
  descriptorEvent(518, d);
  return current->number % 3 ? 0x9002 : 0;
}
extern "C" unsigned __attribute__((thiscall))
castAdjust(void *p, unsigned actor, void *query, int *x, int *y, int *z) {
  if (p != reinterpret_cast<void *>(0x6c5490))
    throw std::runtime_error("cast adjust receiver");
  auto *q = static_cast<unsigned char *>(query);
  CastMovementDescriptor d{load(q, 0),  load(q, 4),  load(q, 8),
                           load(q, 12), load(q, 17), bool(q[21])};
  if (q[16])
    throw std::runtime_error("cast descriptor selector");
  Cell point{*x, *y, *z};
  bool r = adjustWork(actor, d, point);
  *x = point.x;
  *y = point.y;
  *z = point.z;
  return r;
}
extern "C" void __attribute__((thiscall))
castRelocate(void *p, int x, int y, int z, unsigned special) {
  relocationWork(slot(p), {x, y, z}, special != 0);
}
extern "C" void __attribute__((thiscall)) castArea(void *p, unsigned amount,
                                                   unsigned mode) {
  areaWork(slot(p), amount, mode);
}
void install(const char *path) {
  map_image(read(path));
  std::memcpy(testedBody.data(), reinterpret_cast<void *>(0x57b710),
              testedBody.size());
  for (auto x : {std::pair<unsigned, void *>{
                     0x49cdb0, reinterpret_cast<void *>(castPool)},
                 {0x4e1360, reinterpret_cast<void *>(castPathInit)},
                 {0x5908e0, reinterpret_cast<void *>(castProfile)},
                 {0x52a670, reinterpret_cast<void *>(castIterator)},
                 {0x507fa0, reinterpret_cast<void *>(castVisible)},
                 {0x5151b0, reinterpret_cast<void *>(castContains)},
                 {0x411e50, reinterpret_cast<void *>(castChargeable)},
                 {0x506220, reinterpret_cast<void *>(castOwnerMessage)},
                 {0x507f00, reinterpret_cast<void *>(castResource)},
                 {0x56f000, reinterpret_cast<void *>(castMessage)},
                 {0x571310, reinterpret_cast<void *>(castCue)},
                 {0x571180, reinterpret_cast<void *>(castSound)},
                 {0x4e1db0, reinterpret_cast<void *>(castPath)},
                 {0x493f20, reinterpret_cast<void *>(castSecondary)},
                 {0x4f5230, reinterpret_cast<void *>(castAdjust)},
                 {0x51e850, reinterpret_cast<void *>(castRelocate)},
                 {0x520100, reinterpret_cast<void *>(castArea)}})
    redirect(x.first, x.second);
}
void compare(Context input, const Context &expected,
             const std::vector<unsigned> &expectedTrace) {
  current = &input;
  original = true;
  trace.clear();
  unsigned count = input.world.creatures.size();
  actors.assign(count * 0xe4b, 0x9d);
  configs.assign(count * 0x5c9, 0xb1);
  for (unsigned i = 0; i < count; ++i) {
    auto *p = actors.data() + i * 0xe4b;
    auto *c = configs.data() + i * 0x5c9;
    const auto &a = input.world.creatures[i];
    packActor(p, a, input.state.resource[i], input.state.direction[i]);
    put(p, 0xac, reinterpret_cast<std::uintptr_t>(c));
    for (auto x : {std::pair<unsigned, unsigned>{0, a.ai.kind},
                   {8, a.width},
                   {12, a.height},
                   {0x44, a.movementKind},
                   {0x198, bits(a.aimXY)},
                   {0x19c, bits(a.aimZ)}})
      put(c, x.first, x.second);
  }
  auto beforeActors = actors;
  const auto beforeConfigs = configs;
  global(0x6def58, reinterpret_cast<std::uintptr_t>(actors.data()));
  global(0x6def5c, count);
  global(0x644520, bits(input.world.localOwner));
  global(0x6def60, input.state.iteratorCursor);
  global(0x6def64, 0xabc001);
  global(0x6c5494, 16);
  global(0x6c5498, 16);
  global(0x6c549c, 4);
  global(0x657d44, input.state.resourceCounter);
  global(0x6c08cc, input.state.specialValue);
  for (unsigned i = 0; i < 28; ++i)
    for (unsigned j = 0; j < 8; ++j) {
      const auto p = input.state.muzzle[i][j];
      for (auto v : {std::pair<unsigned, unsigned>{0, bits(p.x)},
                     {4, bits(p.y)},
                     {8, bits(p.z)}})
        global(0x6a64a0 + i * 0x5c9 + j * 12 + v.first, v.second);
    }
  Bytes spells(104 * 721 + 64, 0xe1);
  packSpells(spells.data(), input.state);
  put(spells.data(), 0x6c08cc - 0x6b1270, input.state.specialValue);
  std::memcpy(reinterpret_cast<void *>(0x6b1270), spells.data(), spells.size());
  profile = profileBytes(input.state.profiles[0]);
  auto profileBefore = profile;
  global(0x65b250 + 0x936,
         profilePresent() ? reinterpret_cast<std::uintptr_t>(profile.data())
                          : 0);
  const auto descriptorBefore = input.command;
  {
    static PrivateFs fs;
    using Cast = void(__attribute__((thiscall)) *)(void *);
    reinterpret_cast<Cast>(0x57b710)(input.command.data());
    fs.verify();
  }
  if (trace != expectedTrace) {
    unsigned i = 0;
    while (i < trace.size() && i < expectedTrace.size() &&
           trace[i] == expectedTrace[i])
      ++i;
    for (unsigned j = 0; j < std::min(80u, unsigned(trace.size())); ++j)
      std::cerr << trace[j] << ",";
    std::cerr << "\n";
    for (unsigned j = 0; j < std::min(80u, unsigned(expectedTrace.size())); ++j)
      std::cerr << expectedTrace[j] << ",";
    std::cerr << "\n";
    std::cerr << "trace " << i
              << " actual=" << (i < trace.size() ? trace[i] : 0) << " expected="
              << (i < expectedTrace.size() ? expectedTrace[i] : 0) << '\n';
    throw std::runtime_error(
        "cast trace mismatch case=" + std::to_string(input.number) +
        " spell=" + std::to_string(input.command[0]));
  }
  for (unsigned i = 0; i < count; ++i)
    packActor(beforeActors.data() + i * 0xe4b, expected.world.creatures[i],
              expected.state.resource[i], expected.state.direction[i]);
  if (actors != beforeActors || configs != beforeConfigs)
    throw std::runtime_error("cast actor/config byte mismatch case=" +
                             std::to_string(input.number));
  packSpells(spells.data(), expected.state);
  put(spells.data(), 0x6c08cc - 0x6b1270, expected.state.specialValue);
  if (std::memcmp(reinterpret_cast<void *>(0x6b1270), spells.data(),
                  spells.size()))
    throw std::runtime_error("cast spell byte mismatch");
  // Preserve unconsumed queue cells, rather than resetting them after count
  // changes.
  for (unsigned i = input.state.profiles[0].notices.size();
       i < expected.state.profiles[0].notices.size(); ++i) {
    put(profileBefore.data(), 0x4d8 + i * 8,
        expected.state.profiles[0].notices[i][0]);
    put(profileBefore.data(), 0x4dc + i * 8,
        expected.state.profiles[0].notices[i][1]);
  }
  for (unsigned i = 0; i < 124; ++i)
    profileBefore[0x26c + i] = expected.state.profiles[0].known[i];
  put(profileBefore.data(), 0x578, expected.state.profiles[0].notices.size());
  if (profile != profileBefore || input.command != descriptorBefore ||
      load(reinterpret_cast<void *>(0x657d44), 0) !=
          expected.state.resourceCounter ||
      load(reinterpret_cast<void *>(0x6def60), 0) !=
          expected.state.iteratorCursor ||
      std::memcmp(testedBody.data(), reinterpret_cast<void *>(0x57b710),
                  testedBody.size()))
    throw std::runtime_error("cast owned state/input/body mismatch case=" +
                             std::to_string(input.number));
  original = false;
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_CAST_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE expected");
  install(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  const unsigned refusals = refusalCases();
  std::uint64_t digest = 1469598103934665603ull;
  auto mix = [&](unsigned n) { digest = (digest ^ n) * 1099511628211ull; };
  std::array<unsigned, 17> coverage{};
  for (unsigned n = 0; n < 24576; ++n) {
    auto input = generate(n);
    auto expected = input;
    current = &expected;
    trace.clear();
    original = false;
    serviceCounts.fill(0);
    admitCast(expected.command, expected.world, expected.state, services());
    const auto expectedTrace = trace;
    for (auto v : trace)
      mix(v);
    for (auto v : expected.state.resource)
      mix(v);
    mix(expected.state.resourceCounter);
    mix(expected.state.iteratorCursor);
    for (auto v : expected.state.profiles[0].known)
      mix(v);
    const unsigned arm =
        input.command[0] < 103 ? castArms[input.command[0]] : 16;
    if (serviceCounts[17])
      ++coverage[arm];
#ifdef MNM_CAST_REFERENCE
    compare(input, expected, expectedTrace);
#endif
  }
  if (std::any_of(coverage.begin(), coverage.end(),
                  [](unsigned n) { return n == 0; }))
    throw std::runtime_error("unexercised cast arm");
  std::cout
      << "{\"success\":true,\"cases\":24576,\"functions\":1,\"outcome_digest\":"
      << digest << ",\"native_refusal_cases\":" << refusals
      << ",\"dispatch_arm_cases\":[";
  for (unsigned i = 0; i < coverage.size(); ++i)
    std::cout << (i ? "," : "") << coverage[i];
  std::cout << "]}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
