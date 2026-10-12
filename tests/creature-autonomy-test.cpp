#include "autonomy.hpp"
#include "autonomy_tables.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_AUTONOMY_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction::combat;
namespace {
[[maybe_unused]] void require(bool ok, const char *why) {
  if (!ok)
    throw std::runtime_error(why);
}
struct Probe {
  int kind;
  Cell from, to;
  bool operator==(const Probe &p) const {
    return kind == p.kind && from == p.from && to == p.to;
  }
};
struct Fixture {
  AutonomousWorld world;
  std::vector<AiObject> objects;
  std::vector<AiThreat> threats;
  std::vector<AiRegion> regions;
  unsigned pattern = 0, legalMode = 0, reachMode = 0, useCount = 0;
  bool moveSuccess = false;
  unsigned behaviorMode = 0;
};
Fixture *fixture = nullptr;
std::vector<Probe> probes;
std::vector<AutonomousEvent> events;
unsigned rng = 1, draws = 0;
std::uint32_t randomValue() {
  ++draws;
  rng ^= rng << 13;
  rng ^= rng >> 17;
  rng ^= rng << 5;
  return rng;
}
unsigned key(Cell p) { return unsigned(p.x + 13 * p.y + 179 * p.z); }
bool hazard(Cell p) {
  probes.push_back({0, {}, p});
  return fixture->pattern && (key(p) * 17 + fixture->pattern) % 7 < 3;
}
bool legal(Cell from, Cell to, unsigned flag) {
  probes.push_back({1 + 10 * int(flag), from, to});
  return fixture->legalMode == 0 ||
         (fixture->legalMode == 1 && key(to) % 5 != 1);
}
bool reach(Cell from, Cell to) {
  probes.push_back({2, from, to});
  return fixture->reachMode &&
         creatureDistance(from, to, fixture->world.dimensions) <=
             int(fixture->reachMode);
}
bool sight(Cell to) {
  probes.push_back({3, {}, to});
  return (key(to) + fixture->pattern) % 3 != 1;
}
AiTerrainCell cell(Cell p) {
  return {std::uint16_t(fixture->pattern && key(p) % 11 == 0 ? 0 : 0xffff),
          std::uint16_t(fixture->pattern && (key(p) + fixture->pattern) % 9 < 2
                            ? 0x30
                            : 0),
          fixture->pattern && (key(p) + fixture->pattern) % 13 == 0};
}
AutonomousServices services() {
  AutonomousServices s;
  s.behavior = [](AutonomousCreature &a, unsigned n) {
    events.push_back({AutonomousEffect::behavior, {}, int(n)});
    if (fixture->behaviorMode) {
      if (a.ai.motion)
        a.ai.pending = n;
      else {
        a.ai.behavior = n;
        if (n != 8 && n != 14)
          a.ai.pending = 0;
      }
    }
  };
  s.action = [](AutonomousCreature &a, unsigned n) {
    events.push_back({AutonomousEffect::action, {}, int(n)});
    if (fixture->behaviorMode)
      a.ai.action = n;
  };
  s.reset = [](AutonomousCreature &) {
    events.push_back({AutonomousEffect::reset, {}, 0});
  };
  s.prepareInteraction = [](AutonomousCreature &) {
    events.push_back({AutonomousEffect::prepareInteraction, {}, 0});
  };
  s.move = [](AutonomousCreature &, Cell p) {
    events.push_back({AutonomousEffect::move, p, 0});
  };
  s.orient = [](Cell p) { events.push_back({AutonomousEffect::orient, p, 0}); };
  s.useInventory = [](AutonomousCreature &a, InventoryEntry &item) {
    auto i = unsigned(&item - a.inventory.data());
    events.push_back({AutonomousEffect::useInventory, {}, int(i)});
    item.count -= std::min(item.count, fixture->useCount);
  };
  s.refreshInventory = [](unsigned i) {
    events.push_back({AutonomousEffect::refreshInventory, {}, int(i)});
  };
  s.beginMotion = [](AutonomousCreature &) {
    events.push_back({AutonomousEffect::beginMotion, {}, 0});
  };
  s.routeMove = [](AutonomousCreature &a, Cell p) {
    events.push_back({AutonomousEffect::move, p, int(a.routeMode)});
    return fixture->moveSuccess;
  };
  s.rangedAttack = [](AutonomousCreature &) {
    events.push_back({AutonomousEffect::rangedAttack, {}, 0});
  };
  s.faceTarget = [](AutonomousCreature &, const AutonomousCreature &b) {
    events.push_back({AutonomousEffect::faceTarget, {}, int(b.ai.slot)});
  };
  s.changeHealth = [](AutonomousCreature &, int amount) {
    events.push_back({AutonomousEffect::health, {}, amount});
  };
  s.threats = fixture->threats;
  s.regions = fixture->regions;
  s.random = randomValue;
  return s;
}
AiGeometry geometry() {
  AiGeometry g;
  g.hazardOffsets = originalHazardOffsets;
  g.escapeOffsets = originalEscapeOffsets;
  g.retreatOffsets = originalRetreatOffsets;
  g.routeOffsets = originalRouteOffsets;
  g.cell = cell;
  g.hazardous = [](const auto &, Cell p) { return hazard(p); };
  g.legal = [](const auto &, Cell p, Cell q, unsigned flag) {
    return legal(p, q, flag);
  };
  g.objectReachable = [](const auto &, Cell p, Cell q) {
    return legal(p, q, 2);
  };
  g.reach = [](const auto &, const auto &b, Cell p) {
    return reach(p, b.ai.position);
  };
  g.lineOfSight = [](const auto &, Cell p) { return sight(p); };
  g.threatReaches = [](const auto &a, const auto &threat) {
    Cell from{a.finePosition.x + a.aimXY, a.finePosition.y + a.aimXY,
              (a.ai.position.z + int(a.height)) * 16 - 16};
    Cell to{threat.position.x * 32 + 16, threat.position.y * 32 + 16,
            (threat.position.z + 1) * 16};
    probes.push_back({4, from, to});
    return (key(to) + fixture->pattern) % 3 != 0;
  };
  return g;
}
Fixture generate(unsigned n) {
  Fixture f;
  f.world.dimensions = {16, 16, 4};
  f.world.creatures.resize(4);
  for (unsigned i = 0; i < 4; ++i) {
    auto &a = f.world.creatures[i];
    a.ai.slot = i;
    a.ai.type = (n + 7 * i) % 28;
    a.ai.kind = (n / 7 + i) % 28;
    a.ai.owner = i % 2;
    a.ai.position = {int((n + 3 * i) % 16), int((n / 16 + 5 * i) % 16),
                     int((n / 256 + i) % 4)};
    a.ai.health = (n * 17 + i * 43) % 600;
    if ((n + i) % 23 == 0)
      a.ai.health = 0;
    a.ai.maximumHealth = 400;
    a.ai.active = (n + i) % 9 != 0;
    a.ai.visible[0] = (n + i) % 5 != 0;
    a.ai.visible[1] = (n + i) % 7 != 0;
    a.ai.behavior = (n / 2 + i) % 17;
    a.substate = (n / 5 + i) % 10;
    a.ai.pending = n % 53 == 0 ? 8 : 0;
    a.ai.history = n % 4;
    a.ai.age = int(n % 150) - 20;
    a.ai.recent = n % 5;
    a.ai.flag815 = n % 2;
    a.ai.gateFC = n % 17 == 0;
    a.ai.flag704 = n % 3;
    a.ai.readiness = 7;
    a.ai.bravery = int(n % 50) - 25;
    a.ai.engaged = n % 7 == 0;
    a.ai.incoming = (n + i) % 3;
    a.ai.corpseA = (n + i) % 2;
    a.ai.corpseB = (n + i) % 3;
    a.ai.corpseClaim = (n + i) % 4;
    a.ai.home = {int((n / 17 + 3 * i) % 16), int((n / 19 + 5 * i) % 16),
                 int((n / 256 + i) % 4)};
    a.ai.countdown = a.substate;
    a.ai.action = std::array<unsigned, 6>{15, 10, 20, 8, 16, 17}[(n + i) % 6];
    a.alternateTarget = (n / 7 + i) % 4;
    a.ai.motion = (n + i) % 3 == 0;
    a.ai.target = (n / 3 + i) % 4;
    a.ai.marker = 7;
    a.ai.flying = (n + i) % 5 == 0;
    a.ai.ranged = (n + i) % 4 == 0;
    a.ai.physical = (n + i) % 2;
    a.width = 1 + (n / 3 + i) % 2;
    a.height = 1 + (n / 11 + i) % 3;
    a.animationKind = (n / 13 + i) % 4;
    a.inventoryCount = n % 44;
    a.configFlags = (n % 3 ? 4 : 0);
    a.block7c = n % 17 == 0;
    a.hazardSensitive = n % 2;
    a.protection8c9 = n % 3 == 0;
    a.rangedDistance = a.ai.ranged ? 1 + n % 7 : 0;
    a.mode = n % 19 == 0;
    a.form = n % 23 == 0;
    a.formCapability = n % 2;
    a.block784 = n % 31 == 0;
    a.blockDE3 = n % 37 == 0;
    a.block71c = n % 41 == 0;
    a.block723 = n % 43 == 0;
    a.specialVision = (n + i) % 3 == 0;
    a.destination = {int((n / 7 + 2 * i) % 16), int((n / 9 + 4 * i) % 16),
                     int(n / 13 % 4)};
    a.chosenDestination = {-4, -5, -6};
    a.range = n % 40;
    a.routePhase = n % 3;
    a.selectedSpell = 93 + n % 8;
    a.castSpell = 7;
    a.castSource = 8;
    a.castTarget = 9;
    a.cooldown = n % 13;
    a.finePosition = {a.ai.position.x * 32, a.ai.position.y * 32,
                      a.ai.position.z * 16};
    a.aimXY = 16;
    a.aimZ = 12;
    for (unsigned t = 0; t < 28; ++t) {
      a.ai.allyRadius[t] = int((n + t) % 11) - 5;
      a.ai.hth[t] = int((n + t) % 41) - 20;
      if (a.ai.hth[t] == -1)
        a.ai.hth[t] = -2;
      a.ai.enemyRadius[t] = int((n / 3 + t) % 17) - 8;
      a.ai.retaliateThreshold[t] = int((n / 7 + t) % 13) - 6;
    }
    for (unsigned j = 0; j < 42; ++j)
      a.inventory[j] = {unsigned((n + 11 * j) % 90), unsigned((n + j) % 100),
                        unsigned((n + j) % 3), unsigned((n + j) % 4 != 0)};
  }
  f.world.relation[0][0] = f.world.relation[1][1] = 1;
  f.world.relation[0][1] = n % 17 == 0;
  f.world.relation[1][0] = n % 19 == 0;
  f.world.human[0] = n % 11 == 0;
  f.world.ownerEligible[0] = n % 13 != 0;
  for (unsigned i = 0; i < 8; ++i)
    f.objects.push_back(
        {{int((n + 2 * i) % 16), int((n / 16 + 3 * i) % 16),
          int((n / 256 + i) % 4)},
         std::array<unsigned, 8>{0, 3, 6, 19, 61, 5, 18, 62}[(n + i) % 8]});
  f.regions.resize(4);
  for (unsigned i = 0; i < 4; ++i) {
    f.regions[i].position = {int((n + 5 * i) % 16), int((n / 16 + 7 * i) % 16),
                             int((n / 256 + i) % 4)};
    for (unsigned j = 0; j < 18; ++j) {
      auto &edge = f.regions[i].edges[j];
      edge.active = (n + j) % 4;
      edge.destination = (j + i) % 4;
      for (unsigned k = 8; k < 25; ++k)
        edge.bounds[k] = std::int8_t(int((n + j + k) % 7) - 2);
    }
    auto &a = f.world.creatures[i];
    a.region = n % 7 == 0 ? -1 : int(i);
    a.movementKind = 1 + n % 3;
    a.flag104 = n % 2;
  }
  for (unsigned i = 0; i < 3; ++i)
    f.threats.push_back({{int((n + 3 * i) % 16), int((n / 16 + 5 * i) % 16),
                          int((n / 256 + i) % 4)},
                         (n + i) % 3 == 0 ? 70U : 69U,
                         int(i % 2)});
  f.behaviorMode = n % 2;
  f.pattern = n % 8;
  f.legalMode = n % 3;
  f.reachMode = n % 5;
  f.moveSuccess = n % 2;
  f.useCount = n % 3;
  return f;
}
#ifdef MNM_AUTONOMY_REFERENCE
std::vector<std::pair<unsigned, Bytes>> guards;
Bytes actors, configs, terrainCells, tileTypes, objectBytes, threatNodes,
    threatBytes, regionBytes;

template <class T> void put(Bytes &b, unsigned offset, T value) {
  std::memcpy(b.data() + offset, &value, sizeof(value));
}
template <class T = unsigned> T get(const void *p, unsigned offset) {
  T v;
  std::memcpy(&v, static_cast<const unsigned char *>(p) + offset, sizeof(v));
  return v;
}
void pointer(Bytes &b, unsigned offset, void *p) {
  put(b, offset, std::uint32_t(reinterpret_cast<std::uintptr_t>(p)));
}
Cell position(const void *p, unsigned offset = 8) {
  return {get<int>(p, offset), get<int>(p, offset + 4),
          get<int>(p, offset + 8)};
}
void pack(unsigned i) {
  const auto &a = fixture->world.creatures[i];
  Bytes b(0xe4b), c(0x5c9);
  put(b, 0, a.ai.slot);
  put(b, 4, unsigned(a.ai.active));
  put(b, 8, a.ai.position.x);
  put(b, 12, a.ai.position.y);
  put(b, 16, a.ai.position.z);
  put(b, 20, a.finePosition.x);
  put(b, 24, a.finePosition.y);
  put(b, 28, a.finePosition.z);
  put(b, 0xa8, a.ai.type);
  put(b, 0xe4, a.ai.health);
  put(b, 0x174, a.ai.owner);
  put(b, 0x178, a.inventoryCount);
  put(b, 0x7c, a.block7c);
  put(b, 0x5ec, a.ai.behavior);
  put(b, 0x5f0, a.ai.pending);
  put(b, 0x5fc, a.ai.action);
  put(b, 0x618, unsigned(a.ai.motion));
  put(b, 0x64c, a.ai.target);
  put(b, 0x684, a.substate);
  put(b, 0x6a8, a.alternateTarget);
  put(b, 0x688, a.routePhase);
  put(b, 0x570, a.selectedSpell);
  put(b, 0x61c, a.castSpell);
  put(b, 0x638, a.castSource);
  put(b, 0x63c, a.castTarget);
  put(b, 0x66c, a.cooldown);
  put(b, 0x6a4, a.targetMovingAttacker);
  put(b, 0x694, a.routeMode);
  put(b, 0x7cd, a.animationKind);
  put(b, 0x108, a.ai.flying);
  put(b, 0xf8, a.ai.physical);
  put(b, 0x8c9, a.protection8c9);
  put(b, 0xa0, a.mode);
  put(b, 0x798, a.form);
  put(b, 0x784, a.block784);
  put(b, 0xde3, a.blockDE3);
  put(b, 0x71c, std::uint8_t(a.block71c));
  put(b, 0x723, std::uint8_t(a.block723));
  put(b, 0x8c, a.specialVision);
  put(b, 0xfc, a.ai.gateFC);
  put(b, 0x88, a.ai.bravery);
  put(b, 0x664, a.ai.engaged);
  put(b, 0x668, a.ai.incoming);
  put(b, 0x164, a.ai.readiness);
  put(b, 0x680, a.ai.history);
  put(b, 0x80d, a.ai.recent);
  put(b, 0x811, a.ai.age);
  put(b, 0x815, a.ai.flag815);
  put(b, 0x704, a.ai.flag704);
  put(b, 0x932, a.ai.dirty932);
  put(b, 0xf0, a.ai.corpseA);
  put(b, 0xf4, a.ai.corpseB);
  put(b, 0x690, a.ai.corpseClaim);
  put(b, 0xbc3, a.ai.home.x);
  put(b, 0xbc7, a.ai.home.y);
  put(b, 0xbcb, a.ai.home.z);
  put(b, 0x68c, a.range);
  put(c, 0x44, a.movementKind);
  put(b, 0x104, a.flag104);
  put(b, 0xde7, a.region);
  put(b, 0xb8b, a.ai.marker);
  put(b, 0x96b, a.destination.x);
  put(b, 0x96f, a.destination.y);
  put(b, 0x973, a.destination.z);
  put(b, 0x650, a.chosenDestination.x);
  put(b, 0x654, a.chosenDestination.y);
  put(b, 0x658, a.chosenDestination.z);
  put(c, 0, a.ai.kind);
  put(c, 8, a.width);
  put(c, 12, a.height);
  put(c, 0x34, a.rangedDistance);
  put(c, 0x3c, a.formCapability);
  put(c, 0x580, a.configFlags);
  put(c, 0x198, a.aimXY);
  put(c, 0x19c, a.aimZ);
  for (unsigned j = 0; j < 28; ++j) {
    put(c, 0x1a0 + 4 * j, a.ai.hth[j]);
    put(c, 0x360 + 4 * j, a.ai.allyRadius[j]);
    put(c, 0x3d0 + 4 * j, a.ai.enemyRadius[j]);
    put(c, 0x440 + 4 * j, a.ai.retaliateThreshold[j]);
  }
  for (unsigned j = 0; j < 8; ++j)
    put(b, 0x24 + 4 * j, a.ai.visible[j]);
  for (unsigned j = 0; j < 42; ++j) {
    const auto &it = a.inventory[j];
    auto off = 0x17c + j * 24;
    put(b, off, it.kind);
    put(b, off + 4, it.magnitude);
    put(b, off + 16, it.count);
    put(b, off + 20, it.available);
  }
  std::memcpy(configs.data() + i * 0x5c9, c.data(), c.size());
  pointer(b, 0xac, configs.data() + i * 0x5c9);
  std::memcpy(actors.data() + i * 0xe4b, b.data(), b.size());
  global(0x6a652d + a.ai.type * 0x5c9, a.hazardSensitive);
}
void setup(Fixture &f) {
  fixture = &f;
  actors.assign(f.world.creatures.size() * 0xe4b, 0);
  configs.assign(f.world.creatures.size() * 0x5c9, 0);
  for (unsigned i = 0; i < f.world.creatures.size(); ++i)
    pack(i);
  global(0x6def58,
         std::uint32_t(reinterpret_cast<std::uintptr_t>(actors.data())));
  global(0x6def5c, unsigned(f.world.creatures.size()));
  global(0x6df180, unsigned(f.world.creatures.size()));
  auto d = f.world.dimensions;
  global(0x6c5494, d.x);
  global(0x6c5498, d.y);
  global(0x6c549c, d.z);
  terrainCells.assign(d.x * d.y * d.z * 12, 0);
  tileTypes.assign(2 * 356, 0);
  put(tileTypes, 356 + 0xa0, 1U);
  global(0x65660c,
         std::uint32_t(reinterpret_cast<std::uintptr_t>(tileTypes.data())));
  for (int z = 0; z < d.z; ++z) {
    global(0x6cb8c2 + 4 * z, z * d.x * d.y);
    for (int y = 0; y < d.y; ++y) {
      global(0x6cb942 + 4 * y, y * d.x);
      for (int x = 0; x < d.x; ++x) {
        auto c = cell({x, y, z});
        auto off = (x + d.x * (y + d.y * z)) * 12;
        put(terrainCells, off, std::uint16_t(c.tilePropertyA0));
        put(terrainCells, off + 4, c.occupant);
        put(terrainCells, off + 10, c.flags);
      }
    }
  }
  threatNodes.assign((f.threats.size() + 1) * 12, 0);
  threatBytes.assign(f.threats.size() * 0x238, 0);
  for (unsigned i = 0; i <= f.threats.size(); ++i) {
    pointer(threatNodes, i * 12,
            threatNodes.data() + ((i + 1) % (f.threats.size() + 1)) * 12);
    if (i) {
      auto &t = f.threats[i - 1];
      auto off = (i - 1) * 0x238;
      put(threatBytes, off + 8, t.position.x);
      put(threatBytes, off + 12, t.position.y);
      put(threatBytes, off + 16, t.position.z);
      put(threatBytes, off + 0x4c, t.spell);
      put(threatBytes, off + 0x212, t.owner);
      pointer(threatNodes, i * 12 + 8, threatBytes.data() + off);
    }
  }
  global(0x6898e4,
         std::uint32_t(reinterpret_cast<std::uintptr_t>(threatNodes.data())));
  global(0x6bd8ef, 1U);
  regionBytes.assign(f.regions.size() * 0x294, 0);
  for (unsigned i = 0; i < f.regions.size(); ++i) {
    auto &r = f.regions[i];
    auto off = i * 0x294;
    put(regionBytes, off + 4, r.position.x);
    put(regionBytes, off + 8, r.position.y);
    put(regionBytes, off + 12, r.position.z);
    for (unsigned j = 0; j < 18; ++j) {
      auto eoff = off + 16 + j * 28;
      auto &edge = r.edges[j];
      std::memcpy(regionBytes.data() + eoff, edge.bounds.data(), 28);
      put(regionBytes, eoff, edge.active);
      put(regionBytes, eoff + 4, edge.destination);
    }
  }
  global(0x6eaf08,
         std::uint32_t(reinterpret_cast<std::uintptr_t>(regionBytes.data())));
  global(0x6eaf0c, unsigned(f.regions.size()));
  global(0x6c54dc,
         std::uint32_t(reinterpret_cast<std::uintptr_t>(terrainCells.data())));
  for (unsigned i = 0; i < 8; ++i) {
    global(0x643a80 + i * 340 + 4, unsigned(f.world.human[i]));
    global(0x643a80 + i * 340 + 12, f.world.ownerEligible[i] ? 0 : -1);
    for (unsigned j = 0; j < 8; ++j)
      global(0x643a80 + i * 340 + 24 + j, f.world.relation[i][j]);
  }
  global(0x644520, f.world.localOwner);
  global(0x6e1f30, 0U);
  objectBytes.assign(f.objects.size() * 64, 0);
  global(0x6b1258, 0U);
  for (unsigned i = 0; i < f.objects.size(); ++i) {
    auto &o = f.objects[i];
    auto off = i * 64;
    put(objectBytes, off + 8, o.position.x);
    put(objectBytes, off + 12, o.position.y);
    put(objectBytes, off + 16, o.position.z);
    put(objectBytes, off + 0x2a, o.kind);
  }
  probes.clear();
  events.clear();
  rng = 1;
  draws = 0;
}
extern "C" unsigned __attribute__((thiscall)) originalHazard(void *, int x,
                                                             int y, int z) {
  return hazard({x, y, z});
}
extern "C" unsigned __attribute__((thiscall))
originalLegal(void *, int x, int y, int z, int u, int v, int w, void *,
              unsigned zero) {
  require(zero <= 1, "legality flag");
  return legal({x, y, z}, {u, v, w}, zero);
}
extern "C" unsigned __attribute__((thiscall))
originalObjectLegal(void *, int x, int y, int z, int u, int v, int w) {
  return legal({x, y, z}, {u, v, w}, 2);
}
extern "C" unsigned __attribute__((thiscall))
originalReach(void *, void *target, int x, int y, int z) {
  return reach({x, y, z}, position(target));
}
extern "C" unsigned __attribute__((thiscall)) originalSight(void *, int x,
                                                            int y, int z) {
  return sight({x, y, z});
}
extern "C" unsigned __attribute__((thiscall))
originalThreatReach(void *, int x, int y, int z, int u, int v, int w,
                    int radius) {
  require(radius == 32, "threat radius");
  Cell from{x, y, z}, to{u, v, w};
  probes.push_back({4, from, to});
  return (key(to) + fixture->pattern) % 3 != 0;
}
extern "C" void __attribute__((thiscall)) originalOrderedMove(void *, int x,
                                                              int y, int z) {
  events.push_back({AutonomousEffect::move, {x, y, z}, 0});
}
extern "C" void __attribute__((thiscall)) originalOrient(void *, int x, int y) {
  events.push_back({AutonomousEffect::orient, {x, y, 0}, 0});
}
extern "C" void __attribute__((thiscall)) originalBehavior(void *p,
                                                           unsigned n) {
  events.push_back({AutonomousEffect::behavior, {}, int(n)});
  if (fixture->behaviorMode) {
    if (get(p, 0x618))
      std::memcpy(static_cast<unsigned char *>(p) + 0x5f0, &n, 4);
    else {
      std::memcpy(static_cast<unsigned char *>(p) + 0x5ec, &n, 4);
      if (n != 8 && n != 14) {
        unsigned zero = 0;
        std::memcpy(static_cast<unsigned char *>(p) + 0x5f0, &zero, 4);
      }
    }
  }
}
extern "C" void __attribute__((thiscall)) originalAction(void *p, unsigned n) {
  events.push_back({AutonomousEffect::action, {}, int(n)});
  if (fixture->behaviorMode)
    std::memcpy(static_cast<unsigned char *>(p) + 0x5fc, &n, 4);
}
extern "C" void __attribute__((thiscall)) originalFace(void *, void *target) {
  events.push_back({AutonomousEffect::faceTarget, {}, int(get(target, 0))});
}
extern "C" void __attribute__((thiscall)) originalRanged(void *) {
  events.push_back({AutonomousEffect::rangedAttack, {}, 0});
}
extern "C" void __attribute__((thiscall))
originalHealth(void *, int amount, int source, unsigned zero, unsigned one) {
  require(source == -1 && !zero && one == 1, "corpse signed health arguments");
  events.push_back({AutonomousEffect::health, {}, amount});
}
extern "C" void __attribute__((thiscall)) originalPrepare(void *) {
  events.push_back({AutonomousEffect::prepareInteraction, {}, 0});
}
extern "C" void __attribute__((thiscall)) originalReset(void *) {
  events.push_back({AutonomousEffect::reset, {}, 0});
}
extern "C" unsigned __attribute__((thiscall))
originalMove(void *, int x, int y, int z, unsigned mode) {
  require(!mode, "movement mode");
  events.push_back({AutonomousEffect::move, {x, y, z}, 0});
  return fixture->moveSuccess;
}
extern "C" void __attribute__((thiscall)) originalMotion(void *) {
  events.push_back({AutonomousEffect::beginMotion, {}, 0});
}
extern "C" void *__attribute__((thiscall)) originalNextObject(void *) {
  auto &cursor = *reinterpret_cast<unsigned *>(0x6b1258);
  if (cursor >= fixture->objects.size())
    return nullptr;
  return objectBytes.data() + 64 * cursor++;
}
extern "C" unsigned __attribute__((thiscall)) originalMaximum(void *p) {
  return fixture->world.creatures[get(p, 0)].ai.maximumHealth;
}
extern "C" unsigned __attribute__((thiscall)) originalRandom(void *) {
  return randomValue();
}
extern "C" void __attribute__((fastcall)) originalUse(void *actor,
                                                      void *entry) {
  auto i = unsigned((static_cast<unsigned char *>(entry) -
                     static_cast<unsigned char *>(actor) - 0x17c) /
                    24);
  events.push_back({AutonomousEffect::useInventory, {}, int(i)});
  auto count = get(entry, 16);
  count -= std::min(count, fixture->useCount);
  std::memcpy(static_cast<unsigned char *>(entry) + 16, &count, 4);
}
extern "C" void __attribute__((thiscall)) originalRefresh(void *,
                                                          unsigned index) {
  events.push_back({AutonomousEffect::refreshInventory, {}, int(index)});
}
void redirect(unsigned at, std::uintptr_t fn) {
  auto *p = reinterpret_cast<unsigned char *>(at);
  p[0] = 0xe9;
  auto delta = std::uint32_t(fn) - at - 5;
  std::memcpy(p + 1, &delta, 4);
}
void initialize(const char *file) {
  map_image(read(file));
  for (auto range : std::vector<std::pair<unsigned, unsigned>>{
           {0x5198b0, 0x519ad8}, {0x51d200, 0x51d391}, {0x524910, 0x524cd2},
           {0x51d7b0, 0x51db09}, {0x51db30, 0x51dee3}, {0x51d660, 0x51d7ab},
           {0x516220, 0x5164e5}, {0x51d3a0, 0x51d563}, {0x51e090, 0x51e190},
           {0x5186c0, 0x518743}, {0x518420, 0x5186b3}, {0x519330, 0x51983e},
           {0x51c3a0, 0x51c506}, {0x518750, 0x5189df}, {0x5189e0, 0x518c49},
           {0x518c50, 0x518f77}, {0x518f80, 0x5190fb}, {0x519100, 0x5192c9},
           {0x51c510, 0x51c5d4}, {0x51c600, 0x51c7f8}, {0x51c800, 0x51c879},
           {0x51c880, 0x51ccb8}, {0x51ccc0, 0x51ce4b}, {0x51ce50, 0x51cf81},
           {0x51cf90, 0x51d188}, {0x51def0, 0x51dfc0}, {0x51dfc0, 0x51e090}}) {
    auto *p = reinterpret_cast<unsigned char *>(range.first);
    guards.push_back({range.first, Bytes(p, p + range.second - range.first)});
  }
#define REDIRECT(at, name) redirect(at, reinterpret_cast<std::uintptr_t>(&name))
  REDIRECT(0x50db60, originalAction);
  REDIRECT(0x508810, originalFace);
  REDIRECT(0x50b2c0, originalRanged);
  REDIRECT(0x5076f0, originalHealth);
  REDIRECT(0x4e1400, originalThreatReach);
  REDIRECT(0x50fae0, originalOrderedMove);
  REDIRECT(0x50aa30, originalBehavior);
  REDIRECT(0x517820, originalPrepare);
  REDIRECT(0x522f50, originalHazard);
  REDIRECT(0x514360, originalLegal);
  REDIRECT(0x517db0, originalObjectLegal);
  REDIRECT(0x50bd60, originalReach);
  REDIRECT(0x5151b0, originalSight);
  REDIRECT(0x5084f0, originalOrient);
  REDIRECT(0x50ab30, originalReset);
  REDIRECT(0x512800, originalMove);
  REDIRECT(0x510e80, originalMotion);
  REDIRECT(0x544d60, originalNextObject);
  REDIRECT(0x5075f0, originalMaximum);
  REDIRECT(0x54e200, originalRandom);
  REDIRECT(0x468b10, originalUse);
  REDIRECT(0x4d7180, originalRefresh);
#undef REDIRECT
  require(std::memcmp(reinterpret_cast<void *>(0x5e5450),
                      originalHazardOffsets.data(), 324) == 0,
          "hazard table");
  require(std::memcmp(reinterpret_cast<void *>(0x5e5708),
                      originalEscapeOffsets.data(), 324) == 0,
          "escape table");
  require(std::memcmp(reinterpret_cast<void *>(0x5e55c4),
                      originalRetreatOffsets.data(), 324) == 0,
          "retreat table");
  require(std::memcmp(reinterpret_cast<void *>(0x5e178c),
                      originalRouteOffsets.data(), 312) == 0,
          "route table");
}
void checkGuards() {
  for (auto &g : guards)
    require(std::memcmp(reinterpret_cast<void *>(g.first), g.second.data(),
                        g.second.size()) == 0,
            "tested original body changed");
}
#endif
std::uint64_t digest = 1469598103934665603ULL;
void mix(std::uint32_t word) {
  digest ^= word;
  digest *= 1099511628211ULL;
}
void mixCell(Cell p) {
  mix(std::uint32_t(p.x));
  mix(std::uint32_t(p.y));
  mix(std::uint32_t(p.z));
}
void mixActor(const AutonomousCreature &a) {
  for (auto v : {a.ai.slot,
                 a.ai.type,
                 a.ai.kind,
                 a.ai.health,
                 a.ai.maximumHealth,
                 a.ai.behavior,
                 a.ai.pending,
                 a.ai.action,
                 a.ai.history,
                 a.ai.recent,
                 a.ai.engaged,
                 a.ai.incoming,
                 a.ai.flying,
                 a.ai.ranged,
                 a.ai.physical,
                 a.ai.corpseA,
                 a.ai.corpseB,
                 a.ai.corpseClaim,
                 a.ai.gateFC,
                 a.ai.flag704,
                 a.ai.countdown,
                 a.ai.flag815,
                 a.ai.flag80d,
                 a.ai.dirty932,
                 a.ai.readiness,
                 a.ai.marker,
                 a.width,
                 a.height,
                 a.animationKind,
                 a.inventoryCount,
                 a.block7c,
                 a.configFlags,
                 a.hazardSensitive,
                 a.protection8c9,
                 a.mode,
                 a.form,
                 a.formCapability,
                 a.block784,
                 a.blockDE3,
                 a.block71c,
                 a.block723,
                 a.specialVision,
                 a.routePhase,
                 a.substate,
                 a.movementKind,
                 a.flag104,
                 a.selectedSpell,
                 a.targetMovingAttacker,
                 a.routeMode,
                 a.castSpell})
    mix(v);
  for (auto v : {a.ai.owner, a.ai.age, a.ai.bravery, a.ai.target,
                 a.alternateTarget, a.rangedDistance, a.region, a.range,
                 a.castSource, a.castTarget, a.cooldown})
    mix(std::uint32_t(v));
  mix(a.ai.active);
  mix(a.ai.motion);
  mix(a.ai.bloodLust);
  mixCell(a.ai.position);
  mixCell(a.ai.home);
  mixCell(a.destination);
  mixCell(a.chosenDestination);
  mixCell(a.finePosition);
  for (auto &item : a.inventory) {
    mix(item.kind);
    mix(item.magnitude);
    mix(item.count);
    mix(item.available);
  }
}
unsigned cases = 0;
void compare(Fixture f, unsigned operation) {
  fixture = &f;
  probes.clear();
  events.clear();
  rng = 1;
  draws = 0;
  if (operation >= 13)
    for (auto &b : f.world.creatures)
      b.chosenDestination = f.objects[(cases / 13) % 8].position;
  if (operation >= 11)
    for (auto &b : f.world.creatures)
      for (auto &h : b.ai.hth)
        h = std::abs(h) + 1;
  auto beforeWorld = f.world;
  auto g = geometry();
  auto s = services();
  s.geometry = g;
  s.objects = f.objects;
  auto &a = f.world.creatures[0];
  int value = 0;
  switch (operation) {
  case 0:
    value = localHazard(a, a.ai.position, f.world.dimensions, g);
    break;
  case 1:
    value = targetAdmitted(a, f.world.creatures[1], f.world.dimensions, g);
    break;
  case 2:
    events = escapeLocal(a, f.world.dimensions, g,
                         [](Cell) { return fixture->moveSuccess; });
    break;
  case 3:
    value = chooseInteraction(a, f.world, f.objects, g);
    break;
  case 4:
    value = chooseDestination(a, f.world, f.objects, g);
    break;
  case 5:
    events = healFromInventory(a, f.world, [](InventoryEntry &item) {
      item.count -= std::min(item.count, fixture->useCount);
    });
    break;
  case 6:
    value = immediateOpponent(a, f.world, g, randomValue);
    break;
  case 7:
    events = orientToThreat(a, f.world, g);
    break;
  case 8:
    value = assistAdmitted(a, f.world.creatures[1], f.world.dimensions);
    break;
  case 9:
    attackResponse(a, f.world);
    break;
  case 10:
    setAutonomousSubstate(a, f.world, 1 + f.pattern, s);
    break;
  case 11:
    value = priorityResponse(a, f.world, s);
    break;
  case 12:
    autonomousDecision(a, f.world, s);
    break;
  case 13:
    value = retreatLocal(a, f.world, s);
    break;
  case 14:
    autonomousBehavior(a, f.world, s);
    break;
  default:
    throw std::runtime_error("operation");
  }
  mix(operation);
  mix(std::uint32_t(value));
  mix(draws);
  mix(rng);
  for (const auto &b : f.world.creatures)
    mixActor(b);
  for (auto p : probes) {
    mix(std::uint32_t(p.kind));
    mixCell(p.from);
    mixCell(p.to);
  }
  for (auto event : events) {
    mix(unsigned(event.kind));
    mixCell(event.position);
    mix(std::uint32_t(event.argument));
  }
#ifdef MNM_AUTONOMY_REFERENCE
  auto expectedProbes = probes;
  auto expectedEvents = events;
  auto expectedDraws = draws;
  auto expectedWorld = f.world;
  expectedWorld.creatures[0] = a;
  f.world = expectedWorld;
  setup(f);
  auto expected = actors;
  f.world = beforeWorld;
  setup(f);
  using Void = void(__attribute__((thiscall)) *)(void *);
  using Unary = unsigned(__attribute__((thiscall)) *)(void *);
  using Binary = unsigned(__attribute__((thiscall)) *)(void *, void *);
  using Ternary = unsigned(__attribute__((thiscall)) *)(void *, void *, void *);
  using Coord = int(__attribute__((thiscall)) *)(void *, int, int, int);
  int actual = 0;
  switch (operation) {
  case 0:
    actual = reinterpret_cast<Coord>(0x5198b0)(
        actors.data(), a.ai.position.x, a.ai.position.y, a.ai.position.z);
    break;
  case 1:
    actual = reinterpret_cast<Binary>(0x524910)(actors.data(),
                                                actors.data() + 0xe4b);
    break;
  case 2:
    reinterpret_cast<Void>(0x51d200)(actors.data());
    break;
  case 3:
    actual = reinterpret_cast<Unary>(0x51d7b0)(actors.data());
    break;
  case 4:
    actual = reinterpret_cast<Unary>(0x51db30)(actors.data());
    break;
  case 5:
    reinterpret_cast<Void>(0x51d660)(actors.data());
    break;
  case 6:
    actual = reinterpret_cast<Unary>(0x516220)(actors.data());
    break;
  case 7:
    reinterpret_cast<Void>(0x51d3a0)(actors.data());
    break;
  case 8:
    actual = reinterpret_cast<Ternary>(0x51e090)(actors.data(), actors.data(),
                                                 actors.data() + 0xe4b);
    break;
  case 9:
    reinterpret_cast<Void>(0x5186c0)(actors.data());
    break;
  case 10:
    reinterpret_cast<void(__attribute__((thiscall)) *)(void *, unsigned)>(
        0x51c3a0)(actors.data(), 1 + f.pattern);
    break;
  case 11:
    actual = reinterpret_cast<Unary>(0x519330)(actors.data());
    break;
  case 12:
    reinterpret_cast<Void>(0x518420)(actors.data());
    break;
  case 13:
    actual = reinterpret_cast<Unary>(0x51c880)(actors.data());
    break;
  case 14:
    reinterpret_cast<Void>(0x51c510)(actors.data());
    break;
  }
  if (value != actual || expectedProbes != probes || expectedEvents != events ||
      actors != expected || draws != expectedDraws) {
    std::cerr << "operation " << operation << " case " << cases
              << " native/original " << value << "/" << actual << " probes "
              << expectedProbes.size() << "/" << probes.size() << " events "
              << expectedEvents.size() << "/" << events.size() << " draws "
              << expectedDraws << "/" << draws << "\n";
    for (unsigned i = 0; i < actors.size(); ++i)
      if (actors[i] != expected[i]) {
        std::cerr << "byte " << std::hex << i << " " << unsigned(expected[i])
                  << "/" << unsigned(actors[i]) << std::dec << "\n";
        break;
      }
    for (unsigned i = 0; i < std::min(expectedProbes.size(), probes.size());
         ++i)
      if (!(expectedProbes[i] == probes[i])) {
        auto n = expectedProbes[i], o = probes[i];
        std::cerr << "probe " << i << " kind " << n.kind << "/" << o.kind
                  << " from " << n.from.x << "," << n.from.y << "," << n.from.z
                  << "/" << o.from.x << "," << o.from.y << "," << o.from.z
                  << " to " << n.to.x << "," << n.to.y << "," << n.to.z << "/"
                  << o.to.x << "," << o.to.y << "," << o.to.z << "\n";
        break;
      }
    for (auto collection : {expectedProbes, probes}) {
      std::cerr << "TRACE\n";
      for (auto p : collection)
        std::cerr << p.kind << " " << p.from.x << "," << p.from.y << ","
                  << p.from.z << " -> " << p.to.x << "," << p.to.y << ","
                  << p.to.z << "\n";
    }
    throw std::runtime_error("original autonomous decision differs");
  }
  checkGuards();
#endif
  (void)value;
  ++cases;
}
} // namespace
int main(int argc, char **argv) {
  try {
#ifdef MNM_AUTONOMY_REFERENCE
    require(argc == 2, "pinned image required");
    initialize(argv[1]);
#else
    (void)argc;
    (void)argv;
#endif
    for (unsigned op = 0; op < 15; ++op)
      for (unsigned n = 0; n < 2048; ++n)
        compare(generate(n), op);
    std::cout << "{\"success\":true,\"cases\":" << cases
              << ",\"functions\":27,\"outcome_digest\":" << digest << "}\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
}
