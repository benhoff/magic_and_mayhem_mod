#pragma once
// Persistent original PE32 creature core, independently advanced beside the
// native session. No callback may consult the native continuation after setup.
#define main unused_paired_mapper_main
#include "sprite-binary-reference.cpp"
#undef main
#include "../game/persistence/snapshot.hpp"
#include "paired-encounter-projection.hpp"
#include <array>
#include <asm/ldt.h>
#include <cstdlib>
#include <map>
#include <sstream>
#include <sys/syscall.h>
#include <unistd.h>
namespace {
using namespace mnm;
namespace cs = command_session;
namespace model = cs::model;
struct Original {
  encounter::Profile profile;
  Bytes actors, config = Bytes(0x5c9), terrain, types;
  std::array<unsigned, 3> ani{};
  std::vector<Bytes> lists;
  unsigned tick = 0;
  explicit Original(encounter::Profile p) : profile(std::move(p)),actors(profile.initial.world.creatures.size()*0xe4b),lists(profile.initial.world.creatures.size()) {}
};
Original *original = nullptr;
std::array<unsigned, 1024> exceptionThread{};
inline void initializeSearch() {
  // The unmodified MSVC container constructors use FS:0 for their temporary
  // exception registration chain. This private research thread supplies it;
  // Linux C++ TLS remains on GS. No Windows exception handler is executed.
  user_desc segment{};
  segment.entry_number = unsigned(-1);
  segment.base_addr =
      unsigned(reinterpret_cast<std::uintptr_t>(exceptionThread.data()));
  segment.limit = 0xfffff;
  segment.seg_32bit = 1;
  segment.limit_in_pages = 1;
  segment.useable = 1;
  if (syscall(SYS_set_thread_area, &segment))
    throw std::runtime_error("private original FS segment");
  unsigned short selector =
      static_cast<unsigned short>((segment.entry_number << 3) | 3u);
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  using Constructor = void *(__attribute__((thiscall)) *)(void *);
  for (unsigned at : {0x690148u, 0x6c4bd0u})
    reinterpret_cast<Constructor>(0x4bf070)(reinterpret_cast<void *>(at));
}
inline unsigned get(const void *p, unsigned at = 0, unsigned size = 4) {
  unsigned v = 0;
  std::memcpy(&v, static_cast<const char *>(p) + at, size);
  return v;
}
inline void put(void *p, unsigned at, unsigned v, unsigned size = 4) {
  std::memcpy(static_cast<char *>(p) + at, &v, size);
}
inline unsigned address(const void *p) {
  return unsigned(reinterpret_cast<std::uintptr_t>(p));
}
using Step = void(__attribute__((thiscall)) *)(void *);
using Value = void(__attribute__((thiscall)) *)(void *, unsigned);
inline void call(unsigned at, void *p) { reinterpret_cast<Step>(at)(p); }
extern "C" unsigned __attribute__((thiscall)) noArg(void *) { return 0; }
extern "C" unsigned __attribute__((thiscall)) oneArg(void *, unsigned) {
  return 0;
}
extern "C" unsigned __attribute__((thiscall)) twoArgs(void *, unsigned,
                                                      unsigned) {
  return 0;
}
extern "C" unsigned __attribute__((thiscall)) threeArgs(void *, unsigned,
                                                        unsigned, unsigned) {
  return 0;
}
std::map<void *, unsigned> allocations;
unsigned allocatedBytes = 0;
extern "C" void *allocate(unsigned n) {
  if (n > 16 * 1024 * 1024 || allocatedBytes > 32 * 1024 * 1024 - n)
    throw std::runtime_error("original allocation bound");
  auto *p = std::malloc(n ? n : 1);
  if (!p)
    throw std::bad_alloc();
  allocations[p] = n;
  allocatedBytes += n;
  return p;
}
extern "C" void deallocate(void *p) {
  if (!p)
    return;
  auto it = allocations.find(p);
  if (it == allocations.end())
    throw std::runtime_error("unknown original allocation");
  allocatedBytes -= it->second;
  allocations.erase(it);
  std::free(p);
}
extern "C" void __attribute__((thiscall)) emptyEffect(void *, unsigned, int,
                                                      int, int, unsigned zero,
                                                      void *ref,
                                                      unsigned last) {
  if (zero || ref || last)
    throw std::runtime_error("nonempty effect boundary");
}
inline void destroySearch() {
  for (unsigned at : {0x690148u, 0x6c4bd0u})
    call(0x4bef60, reinterpret_cast<void *>(at));
  if (!allocations.empty())
    throw std::runtime_error("original allocation leak");
}
extern "C" unsigned __attribute__((thiscall)) emptyResource(void *,
                                                            unsigned amount) {
  if (amount)
    throw std::runtime_error("nonempty owner resource boundary");
  return 0;
}
extern "C" void __attribute__((thiscall)) retainRemove(void *a) {
  using Position =
      void(__attribute__((thiscall)) *)(void *, unsigned, unsigned, unsigned);
  auto release = reinterpret_cast<Position>(0x5093e0);
  release(a, get(a, 8), get(a, 12), get(a, 16));
  if (get(a, 0x618)) {
    unsigned cursor = get(a, 0x977);
    if (cursor >= get(a, 0x97b) || cursor >= 16)
      throw std::runtime_error("removal route extent");
    release(a, get(a, 0x97f + 28 * cursor), get(a, 0x983 + 28 * cursor),
            get(a, 0x987 + 28 * cursor));
  }
  put(a, 4, 0);
  put(a, 0x618, 0);
  for (unsigned i = 0; i < original->profile.initial.world.creatures.size(); ++i) {
    auto *peer = original->actors.data() + i * 0xe4b;
    if (get(peer, 4) && get(peer, 0x64c) == get(a, 0))
      call(0x50ab30, peer);
  }
}
std::map<unsigned, Bytes> redirects;
void redirect(unsigned at, std::uintptr_t destination) {
  auto *p = reinterpret_cast<unsigned char *>(at);
  if (redirects.count(at))
    throw std::runtime_error("duplicate endpoint");
  redirects[at] = Bytes(p, p + 5);
  p[0] = 0xe9;
  unsigned delta = unsigned(destination) - at - 5;
  std::memcpy(p + 1, &delta, 4);
}
void endpoints() {
  // Deliberately empty presentation / owner services for the admitted fixture.
  for (unsigned at : {0x508040u, 0x54dd60u, 0x522340u, 0x5137d0u, 0x523470u,
                      0x51a8f0u, 0x50f850u, 0x51aa30u})
    redirect(at, reinterpret_cast<std::uintptr_t>(&noArg));
  for (unsigned at : {0x54dad0u, 0x54dce0u, 0x54dba0u, 0x514e40u, 0x51fba0u,
                      0x58f830u, 0x4d9ac0u})
    redirect(at, reinterpret_cast<std::uintptr_t>(&oneArg));
  for (unsigned at : {0x570480u, 0x570bd0u, 0x56c290u})
    redirect(at, reinterpret_cast<std::uintptr_t>(&oneArg));
  redirect(0x571310, reinterpret_cast<std::uintptr_t>(&emptyEffect));
  redirect(0x570e80, reinterpret_cast<std::uintptr_t>(&threeArgs));
  for (unsigned at : {0x570510u})
    redirect(at, reinterpret_cast<std::uintptr_t>(&twoArgs));
  redirect(0x4d9890, reinterpret_cast<std::uintptr_t>(&twoArgs));
  redirect(0x58f890, reinterpret_cast<std::uintptr_t>(&noArg));
  redirect(0x54de10, reinterpret_cast<std::uintptr_t>(&noArg));
  redirect(0x411e50, reinterpret_cast<std::uintptr_t>(&noArg));
  redirect(0x416c30, reinterpret_cast<std::uintptr_t>(&oneArg));
  redirect(0x466870, reinterpret_cast<std::uintptr_t>(&oneArg));
  redirect(0x4656c0, reinterpret_cast<std::uintptr_t>(&twoArgs));
  redirect(0x49cdb0, reinterpret_cast<std::uintptr_t>(&noArg));
  redirect(0x597850, reinterpret_cast<std::uintptr_t>(&allocate));
  redirect(0x597870, reinterpret_cast<std::uintptr_t>(&deallocate));
  redirect(0x507f00, reinterpret_cast<std::uintptr_t>(&emptyResource));
  redirect(0x506d30, reinterpret_cast<std::uintptr_t>(&retainRemove));
}
struct TextGuard {
  unsigned address;
  Bytes bytes;
};
std::vector<TextGuard> textGuards;
inline void sealText(const Bytes &pe) {
  const auto header = u32(pe, 0x3c), opt = header + 24;
  unsigned count = pe.at(header + 6) | (pe.at(header + 7) << 8),
           length = pe.at(header + 20) | (pe.at(header + 21) << 8);
  for (unsigned i = 0; i < count; ++i) {
    auto h = opt + length + i * 40;
    if (!(u32(pe, h + 36) & 0x20000000))
      continue;
    auto at = 0x400000 + u32(pe, h + 12), size = u32(pe, h + 16),
         offset = u32(pe, h + 20);
    Bytes expected(pe.begin() + offset, pe.begin() + offset + size);
    for (const auto &[entry, prior] : redirects)
      if (entry >= at && entry + 5 <= at + size) {
        if (!std::equal(prior.begin(), prior.end(),
                        expected.begin() + entry - at))
          throw std::runtime_error("endpoint expected original bytes");
        std::memcpy(expected.data() + entry - at,
                    reinterpret_cast<void *>(entry), 5);
      }
    if (std::memcmp(expected.data(), reinterpret_cast<void *>(at), size))
      throw std::runtime_error("undeclared text mutation");
    textGuards.push_back({at, std::move(expected)});
    auto first = at & ~4095u;
    auto end = (at + size + 4095u) & ~4095u;
    if (mprotect(reinterpret_cast<void *>(first), end - first,
                 PROT_READ | PROT_EXEC))
      throw std::runtime_error("text RX protection");
  }
  if (textGuards.empty())
    throw std::runtime_error("missing original text");
}
inline void checkText() {
  for (const auto &g : textGuards)
    if (std::memcmp(g.bytes.data(), reinterpret_cast<void *>(g.address),
                    g.bytes.size()))
      throw std::runtime_error("original text changed during execution");
}
void setup(Original &o, const cs::Continuation &initial, unsigned seed) {
  original = &o;
  std::fill(o.actors.begin(),o.actors.end(),0);
  const auto &p = o.profile;
  const auto d = initial.world.dimensions;
  auto &ani = p.native.animation[0];
  o.ani = {0, address(ani.starts.data()), address(ani.records.data())};
  std::memcpy(reinterpret_cast<void *>(0x6dbf00 + 17 * 276), o.ani.data(), 12);
  auto *cfg = o.config.data();
  const auto &type = p.native.types[17];
  put(cfg, 0, 1);
  put(cfg, 8, 2);
  put(cfg, 12, 2);
  put(cfg, 0x10, type.scalar.type_10);
  put(cfg, 0x18, p.maximum);
  put(cfg, 0x3c, type.scalar.type_3c);
  put(cfg, 0x44, type.movementKind);
  put(cfg,0x589,unsigned(type.routeCostFactor));put(cfg,0x5ad,type.routeFlagPenalty);
  std::memcpy(cfg + 0xd8, type.scalar.type_d8.data(), 48 * 4);
  const auto &a = initial.world.creatures[0];
  for (unsigned i = 0; i < 28; ++i) {
    put(cfg, 0x1a0 + 4 * i, unsigned(a.ai.hth[i]));
    put(cfg, 0x210 + 4 * i, unsigned(p.melee.stunThreshold));
    put(cfg, 0x360 + 4 * i, unsigned(a.ai.allyRadius[i]));
    put(cfg, 0x3d0 + 4 * i, unsigned(a.ai.enemyRadius[i]));
    put(cfg, 0x440 + 4 * i, unsigned(a.ai.retaliateThreshold[i]));
  }
  put(cfg, 0x198, a.aimXY);
  put(cfg, 0x19c, a.aimZ);
  for (unsigned i = 0; i < o.profile.initial.world.creatures.size(); ++i) {
    auto *raw = o.actors.data() + i * 0xe4b;
    for (const auto &[offset, f] : encounter::paired::fields(initial, i))
      put(raw, offset, f.value, f.size);
    const auto &route=initial.actions.segments.at(i);
    for(unsigned j=0;j<16;++j){const auto &q=route.route[j];put(raw,0x97f+28*j,unsigned(q.position.x));put(raw,0x983+28*j,unsigned(q.position.y));put(raw,0x987+28*j,unsigned(q.position.z));for(unsigned k=0;k<4;++k)put(raw,0x98b+28*j+4*k,unsigned(q.remainder[k]));}
    put(raw, 0xac, address(cfg));
    put(raw, 0xb0 + 28, 0);
    put(raw, 0x6c, 14);
    put(raw, 0x5f4, initial.world.creatures[i].ai.behavior == 2 ? 0x510010 : 0x50acb0);
    put(raw, 0x600, 0x50f7c0);
    o.lists[i].assign(16, 0);
    put(o.lists[i].data(), 0, address(o.lists[i].data()));
    put(o.lists[i].data(), 4, address(o.lists[i].data()));
    put(raw, 0x8f9, address(o.lists[i].data()));
    for (unsigned j = 0; j < 8; ++j)
      put(raw, 0x24 + 4 * j, initial.world.creatures[i].ai.visible[j]);
    put(raw, 0xdeb + 4 * 0, 0);
  }
  global(0x6def58, address(o.actors.data()));
  global(0x6def5c, unsigned(initial.world.creatures.size()));
  global(0x6df180, unsigned(initial.world.creatures.size()));
  for (auto v : std::vector<std::pair<unsigned, unsigned>>{
           {0x6c5494, unsigned(d.x)},
           {0x6c5498, unsigned(d.y)},
           {0x6c549c, unsigned(d.z)},
           {0x6c54a0, unsigned(d.x * d.y)},
           {0x5e1780, unsigned(d.z)},
           {0x6c007d, 720},
           {0x6e9850, 0},
           {0x6c5c80, 0},
           {0x644520, 0},
           {0x5e1778, 25},
           {0x5e177c, 62},
           {0x5e1910, 5},
           {0x5e1914, 5},
           {0x6bd8ef, 1},
           {0x6df184, 0},
           {0x6e1f30, 0},
           {0x6df008, 0}})
    global(v.first, v.second);
  for (unsigned i = 0; i < 8; ++i) {
    global(0x643a80 + i * 340 + 12, 0);
    for (unsigned j = 0; j < 8; ++j)
      *reinterpret_cast<unsigned char *>(0x643a80 + i * 340 + 24 + j) =
          initial.world.relation[i][j];
  }
  constexpr unsigned response[8] = {0, 0, 25, 50, 75, 50, 25, 0};
  for (unsigned i = 0; i < 8; ++i)
    global(0x5e1758 + 4 * i, response[i]);
  for (unsigned i = 0; i < 28; ++i)
    std::memcpy(reinterpret_cast<void *>(0x6a5f80 + i * 0x5c9), cfg,
                o.config.size());
  o.types.resize(p.native.terrain.size() * 356);
  std::memcpy(o.types.data(), p.native.terrain.data(), o.types.size());
  global(0x65660c, address(o.types.data()));
  for (int y = 0; y < d.y; ++y)
    global(0x6cb942 + y * 4, y * d.x);
  for (int z = 0; z < d.z; ++z)
    global(0x6cb8c2 + z * 4, z * d.x * d.y);
  o.terrain.assign(initial.native.cells.size() * 12, 0);
  for (unsigned i = 0; i < initial.native.cells.size(); ++i) {
    auto *raw = o.terrain.data() + 12 * i;
    const auto &v = initial.native.cells[i];
    put(raw, 0, v.kind, 2);
    put(raw, 2, 0xffff, 2);
    put(raw, 4, v.occupant == 0xffff ? 0xffff : v.occupant - 1, 2);
    put(raw, 6, 0xffff, 2);
    put(raw, 8, v.links, 2);
    put(raw, 10, v.flags, 2);
  }
  global(0x6c54dc, address(o.terrain.data()));
  unsigned slope;std::memcpy(&slope,&p.native.types[17].scalar.slope_global,4);global(0x5e15f0,slope);
  auto random = model::seedRandom(seed);
  global(0x6e1ac8, random.left);
  global(0x6e1acc, random.right);
  std::memcpy(reinterpret_cast<void *>(0x6e1ad0), random.words.data(), 1000);
  // The pool manager's independent scheduling continuation.
  for (unsigned at : {0x22cu, 0x230u, 0x258u, 0x25cu, 0x264u})
    global(0x6def58 + at, 0);
  for(unsigned at=0x6df184;at<=0x6df1ac;at+=4)global(at,0);
  global(0x6e1f3c,0);
  global(0x6def58 + 0x228, unsigned(initial.world.creatures.size()));
}
inline void step(Original &o) {
  if(o.profile.scenario==7){
    if(o.tick==2){using Stun=void(__attribute__((thiscall)) *)(void *,int,unsigned,unsigned);reinterpret_cast<Stun>(0x50d060)(o.actors.data()+0xe4b,8,0,0);}
    if(o.tick==4||o.tick==64){auto slot=o.tick==4?0u:5u;auto p=o.profile.initial.execution[o.tick==4?0:6].roamHome;using Move=void(__attribute__((thiscall)) *)(void *,int,int,int);reinterpret_cast<Move>(0x50fae0)(o.actors.data()+slot*0xe4b,p.x,p.y,p.z);}
    if(o.tick==12){using Health=void(__attribute__((thiscall)) *)(void *,int,int,void *,int);reinterpret_cast<Health>(0x5076f0)(o.actors.data()+0xe4b,-10000,-1,nullptr,1);}
  }
  ++o.tick;
  global(0x6c4830, o.tick);
  for (unsigned i = 0; i < o.profile.initial.world.creatures.size(); ++i) {
    auto *a = o.actors.data() + i * 0xe4b;
    if (get(a, 4)) {
      if (!get(a, 0xb0 + 28)) {
        put(a, 0xb0 + 28, address(o.ani.data()));
        reinterpret_cast<Value>(0x50db60)(a, get(a, 0x5fc));
      }
      call(0x509e50, a);
    }
  }
  call(0x52a780, reinterpret_cast<void *>(0x6def58));
  call(0x52a9f0, reinterpret_cast<void *>(0x6def58));
}
inline encounter::paired::Projection projectOriginal(const Original &o,bool includeCells=true) {
  encounter::paired::Projection out;
  auto add = [&](std::string key, unsigned value) {
    out.push_back({std::move(key), value});
  };
  add("tick", o.tick);
  for (unsigned i = 0; i < o.profile.initial.world.creatures.size(); ++i) {
    auto *raw = o.actors.data() + i * 0xe4b;
    const auto prefix = "actor[" + std::to_string(i) + "].";
    for (const auto &[offset, f] :
         encounter::paired::fields(o.profile.initial, i)) {
      std::ostringstream key;
      key << prefix << "0x" << std::hex << offset << " " << f.name;
      add(key.str(), get(raw, offset, f.size));
    }
    bool bound = get(raw, 0xcc);
    auto pointer = [&](unsigned v) {
      if (!v)
        return 0u;
      auto base = address(o.profile.native.animation[0].records.data());
      if (v < base || (v - base) % 44 ||
          (v - base) / 44 >= o.profile.native.animation[0].records.size())
        throw std::runtime_error("original ANI pointer domain");
      return (v - base) / 44 + 1;
    };
    add(prefix + "ani.bound", bound);
    add(prefix + "ani.sequence", bound ? get(raw, 0xb0) : 0);
    add(prefix + "ani.pc", bound ? pointer(get(raw, 0xc8)) : 0);
    add(prefix + "ani.displayed", bound ? pointer(get(raw, 0xb4)) : 0);
    add(prefix + "ani.active", get(raw, 0xb8));
    add(prefix + "ani.delay", get(raw, 0xbc));
    add(prefix + "ani.elapsed", get(raw, 0xc0));
    add(prefix + "ani.repeats", get(raw, 0xc4));
    add(prefix + "ani.break", get(raw, 0xd0));
    add(prefix + "ani.reverse", get(raw, 0xd4));
    add(prefix + "ani.reverseCursor", get(raw, 0xd8));
    add(prefix + "ani.reverseLength", get(raw, 0xdc));
    const bool moving=get(raw,0x618)!=0;
    const bool inlineSamples=moving && get(raw,0xb93)==address(raw)+0x71e;
    auto sampleOrdinal=[&](unsigned at){
      if(!moving)return 0u;
      const unsigned base=inlineSamples?address(raw)+0x71e:get(raw,0xac)+0xd8;
      const unsigned delta=get(raw,at)-base;
      if(delta%4 || delta/4>=48)throw std::runtime_error("original sample pointer domain");
      return delta/4;
    };
    add(prefix+"samples.inline",inlineSamples);
    add(prefix+"samples.current",sampleOrdinal(0xb8f));
    add(prefix+"samples.initial",sampleOrdinal(0xb93));
    auto count = get(raw, 0x97b);
    if (count > 16)
      throw std::runtime_error("original route count");
    for (unsigned j = 0; j < count; ++j) {
      auto key = prefix + "route[" + std::to_string(j) + "].";
      auto at = 0x97f + 28 * j;
      add(key + "x", get(raw, at));
      add(key + "y", get(raw, at + 4));
      add(key + "z", get(raw, at + 8));
      for (unsigned k = 0; k < 4; ++k)
        add(key + "meta[" + std::to_string(k) + "]", get(raw, at + 12 + 4 * k));
    }
    add(prefix + "health.bar", get(raw, 0x6c));
    for (unsigned j = 0; j < 8; ++j)
      add(prefix + "damage[" + std::to_string(j) + "]",
          get(raw, 0xdeb + 4 * j));
  }
  if(includeCells)for (unsigned i = 0; i < o.terrain.size() / 12; ++i) {
    auto *raw = o.terrain.data() + 12 * i;
    auto key = "cell[" + std::to_string(i) + "].";
    add(key + "kind", get(raw, 0, 2));
    auto token = get(raw, 4, 2);
    add(key + "occupant", token == 0xffff ? token : token + 1);
    add(key + "flags", get(raw, 10, 2));
    add(key + "links", get(raw, 8, 2));
  }
  add("rng.left", get(reinterpret_cast<void *>(0x6e1ac8)));
  add("rng.right", get(reinterpret_cast<void *>(0x6e1acc)));
  for (unsigned i = 0; i < 250; ++i)
    add("rng[" + std::to_string(i) + "]",
        get(reinterpret_cast<void *>(0x6e1ad0 + 4 * i)));
  for (auto [name, at] :
       std::vector<std::pair<const char *, unsigned>>{{"cursor", 0x258},
                                                      {"phase20", 0x25c},
                                                      {"phase90", 0x264},
                                                      {"budget", 0x230},
                                                      {"continuation", 0x22c}})
    add(std::string("scheduler.") + name,
        get(reinterpret_cast<void *>(0x6def58 + at)));
  add("request.active",get(reinterpret_cast<void*>(0x6df184)));
  add("request.displacement",get(reinterpret_cast<void*>(0x6df1a0)));
  for(auto [name,at]:std::vector<std::pair<const char*,unsigned>>{{"requestBudget",0x6df18c},{"mode",0x6df19c},{"destination.x",0x6df190},{"destination.y",0x6df194},{"destination.z",0x6df198},{"origin.x",0x6df1a4},{"origin.y",0x6df1a8},{"origin.z",0x6df1ac}})add(std::string("request.")+name,get(reinterpret_cast<void*>(at)));
  add("request.dirty",get(reinterpret_cast<void*>(0x6c4ddc),0,1));
  for(unsigned i=0;i<o.profile.initial.world.creatures.size();++i)add("request.marker69c["+std::to_string(i)+"]",get(o.actors.data()+i*0xe4b,0x69c));
  return out;
}
inline void compare(const Original &o, const cs::Continuation &native) {
  auto why = encounter::paired::difference(
      encounter::paired::project(native, o.profile), projectOriginal(o));
  if (!why.empty())
    throw std::runtime_error("tick=" + std::to_string(o.tick) + " " + why);
}
} // namespace
