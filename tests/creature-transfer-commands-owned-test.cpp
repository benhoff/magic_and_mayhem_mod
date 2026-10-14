#include "../reconstruction/creature-ai/transfer_commands_owned.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
#pragma push_macro("main")
#undef main
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#pragma pop_macro("main")
#endif
using namespace mnm::reconstruction::combat;
namespace {
struct Fixture {
  std::vector<CreatureExecution> executions;
  CommandBehaviorBindings idleBindings;
  std::vector<std::array<unsigned, 4>> descriptorProfiles;
  std::vector<std::vector<CreatureOrder>> initialOrders;
  CreatureTransferState transfer;
  CreatureTransferWorld world;
  std::array<AssociatedOwnerRecord, 8> owners{};
  std::vector<CreatureControlLimit> limits;
  OwnerTargetRepairState repair;
  SourceGraph graph;
  AutonomousWorld relations;
  CreatureOwnerVisibility visibility;
  OwnerVisionSharing sharing{};
  unsigned number = 0;
  int requested = 0;
};
Fixture *current;
bool original = false;
std::vector<unsigned> events;
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
Bytes actor, definition, effects, nodes, displays, container, sentinel, orders,
    relations;
std::map<unsigned, Bytes> retiredNodes;
Bytes mapCells, mapTiles;
void syncQueues();
void importQueues();
unsigned get(const void *p, unsigned off) {
  unsigned v;
  std::memcpy(&v, static_cast<const unsigned char *>(p) + off, 4);
  return v;
}
void put(void *p, unsigned off, unsigned v) {
  std::memcpy(static_cast<unsigned char *>(p) + off, &v, 4);
}
void packReset(void *p, const AutonomousCreature &a,
               const CreatureExecution &e) {
  put(p, 0x8, unsigned(a.ai.position.x));
  put(p, 0xc, unsigned(a.ai.position.y));
  put(p, 0x10, unsigned(a.ai.position.z));
  put(p, 0xbc3, unsigned(a.ai.home.x));
  put(p, 0xbc7, unsigned(a.ai.home.y));
  put(p, 0xbcb, unsigned(a.ai.home.z));
  put(p, 0xa0, unsigned(a.mode));
  put(p, 0x784, unsigned(a.block784));
  put(p, 0xde3, unsigned(a.blockDE3));
  put(p, 0x684, unsigned(a.substate));
  put(p, 0x9c, unsigned(e.absorptionTimer));
  put(p, 0x10c, unsigned(e.mode10c));
  put(p, 0x108, unsigned(a.ai.flying));
  put(p, 0xb73, unsigned(e.markerB73));
  put(p, 0x164, unsigned(a.ai.readiness));
  put(p, 0x963, unsigned(e.block963));
  put(p, 0x932, unsigned(a.ai.dirty932));
  put(p, 0x6a0, unsigned(e.marker6a0));
  put(p, 0xb8b, unsigned(a.ai.marker));
  put(p, 0x6ac, unsigned(e.routeRenewal));
  put(p, 0x698, unsigned(e.commandMode));
  put(p, 0x66c, unsigned(a.cooldown));
  put(p, 0x618, unsigned(a.ai.motion));
  put(p, 0x906, unsigned(e.follow));
  put(p, 0x68, e.marker68);
  put(p, 0x7cd, a.animationKind);
  put(p, 0x688, a.routePhase);
  put(p, 0xb6f, e.routeChanged);
  put(p, 0x570, a.selectedSpell);
  put(p, 0x61c, a.castSpell);
  put(p, 0x638, unsigned(a.castSource));
  put(p, 0x63c, unsigned(a.castTarget));
  put(p, 0x65c, e.targetDistance);
  put(p, 0x62c, unsigned(e.castAim.x));
  put(p, 0x630, unsigned(e.castAim.y));
  put(p, 0x634, unsigned(e.castAim.z));
  put(p, 0x798, a.form);
  put(p, 0x96b, unsigned(a.destination.x));
  put(p, 0x96f, unsigned(a.destination.y));
  put(p, 0x973, unsigned(a.destination.z));
  static_cast<unsigned char *>(p)[0x71c] =
      static_cast<unsigned char>(a.block71c);
  static_cast<unsigned char *>(p)[0x723] =
      static_cast<unsigned char>(a.block723);
  static_cast<unsigned char *>(p)[0x962] =
      static_cast<unsigned char>(e.block962);
  static_cast<unsigned char *>(p)[0x722] =
      static_cast<unsigned char>(e.block722);
  put(p, 0x6e8,
      e.routeEffect ? reinterpret_cast<std::uintptr_t>(effects.data() +
                                                       *e.routeEffect * 4)
                    : 0);
}
std::optional<unsigned> effectToken(unsigned value) {
  if (!value)
    return {};
  const auto start = reinterpret_cast<std::uintptr_t>(effects.data());
  if (value < start || value - start >= effects.size() || (value - start) % 4)
    throw std::runtime_error("effect token backing");
  return unsigned(value - start) / 4;
}
void syncRepair() {
  global(0x6df180, current->repair.count);
  for (unsigned i = 0; i < current->relations.creatures.size(); ++i) {
    const auto &a = current->relations.creatures[i];
    auto *p = actor.data() + i * 0xe4b;
    put(p, 4, a.ai.active);
    put(p, 0xe4, a.ai.health);
    put(p, 0x174, unsigned(a.ai.owner));
    put(p, 0xa8, a.ai.type);
    put(p, 0x5ec, a.ai.behavior);
    put(p, 0x5fc, a.ai.action);
    put(p, 0x5f0, a.ai.pending);
    put(p, 0x664, a.ai.engaged);
    put(p, 0x668, a.ai.incoming);
    put(p, 0x6a8, unsigned(a.alternateTarget));
    put(p, 0x64c, unsigned(a.ai.target));
    put(p, 0x6a4, current->repair.markers6a4[i]);
    packReset(p, a, current->executions[i]);
  }
}
void syncActor() {
  syncRepair();
  syncQueues();
  put(actor.data(), 0x174, unsigned(current->relations.creatures[0].ai.owner));
  put(actor.data(), 0x5ec, current->relations.creatures[0].ai.behavior);
  put(actor.data(), 0x5f0, current->relations.creatures[0].ai.pending);
  put(actor.data(), 0x7cd, current->relations.creatures[0].animationKind);
  put(definition.data(), 0, current->relations.creatures[0].ai.kind);
}
void importActor() {
  auto word = [](unsigned v) {
    return v <= 0x7fffffffu ? int(v) : -1 - int(~v);
  };
  current->relations.creatures[0].ai.owner = word(get(actor.data(), 0x174));
  current->relations.creatures[0].ai.behavior = get(actor.data(), 0x5ec);
  current->relations.creatures[0].ai.pending = get(actor.data(), 0x5f0);
  current->relations.creatures[0].animationKind = get(actor.data(), 0x7cd);
  current->relations.creatures[0].ai.kind = get(definition.data(), 0);
  current->repair.count = get(reinterpret_cast<void *>(0x6df180), 0);
  for (unsigned i = 0; i < current->relations.creatures.size(); ++i) {
    auto &a = current->relations.creatures[i];
    const auto *p = actor.data() + i * 0xe4b;
    a.ai.active = get(p, 4);
    a.ai.health = get(p, 0xe4);
    a.ai.owner = word(get(p, 0x174));
    a.ai.type = get(p, 0xa8);
    a.ai.behavior = get(p, 0x5ec);
    a.ai.action = get(p, 0x5fc);
    a.ai.pending = get(p, 0x5f0);
    a.ai.engaged = get(p, 0x664);
    a.ai.incoming = get(p, 0x668);
    a.alternateTarget = word(get(p, 0x6a8));
    a.ai.target = word(get(p, 0x64c));
    current->repair.markers6a4[i] = get(p, 0x6a4);
    auto &e = current->executions[i];
    a.ai.position.x = word(get(p, 0x8));
    a.ai.position.y = word(get(p, 0xc));
    a.ai.position.z = word(get(p, 0x10));
    a.ai.home.x = word(get(p, 0xbc3));
    a.ai.home.y = word(get(p, 0xbc7));
    a.ai.home.z = word(get(p, 0xbcb));
    a.mode = get(p, 0xa0);
    a.block784 = get(p, 0x784);
    a.blockDE3 = get(p, 0xde3);
    a.substate = get(p, 0x684);
    e.absorptionTimer = get(p, 0x9c);
    e.mode10c = get(p, 0x10c);
    a.ai.flying = get(p, 0x108);
    e.markerB73 = get(p, 0xb73);
    a.ai.readiness = get(p, 0x164);
    e.block963 = get(p, 0x963);
    a.ai.dirty932 = get(p, 0x932);
    e.marker6a0 = get(p, 0x6a0);
    a.ai.marker = get(p, 0xb8b);
    e.routeRenewal = get(p, 0x6ac);
    e.commandMode = get(p, 0x698);
    a.cooldown = word(get(p, 0x66c));
    a.ai.motion = get(p, 0x618) != 0;
    e.follow = word(get(p, 0x906));
    e.marker68 = get(p,0x68); a.animationKind = get(p,0x7cd);
    a.routePhase = get(p, 0x688);
    e.routeChanged = get(p, 0xb6f);
    a.selectedSpell = get(p, 0x570);
    a.castSpell = get(p, 0x61c);
    a.castSource = word(get(p, 0x638)); a.castTarget = word(get(p, 0x63c));
    e.targetDistance = get(p, 0x65c);
    e.castAim = {word(get(p, 0x62c)), word(get(p, 0x630)), word(get(p, 0x634))};
    a.form = get(p, 0x798);
    a.formCapability = current->descriptorProfiles[i][2];
    a.destination = {word(get(p, 0x96b)), word(get(p, 0x96f)),
                     word(get(p, 0x973))};
    a.block71c =
        static_cast<const unsigned char *>(static_cast<const void *>(p))[0x71c];
    a.block723 =
        static_cast<const unsigned char *>(static_cast<const void *>(p))[0x723];
    e.block962 =
        static_cast<const unsigned char *>(static_cast<const void *>(p))[0x962];
    e.block722 =
        static_cast<const unsigned char *>(static_cast<const void *>(p))[0x722];
    e.routeEffect = effectToken(get(p, 0x6e8));
    const auto callback = get(p, 0x5f4);
    current->idleBindings[i] = callback == 0x50acb0 ? CommandBehaviorBinding::idle
        : callback == 0x510010 ? CommandBehaviorBinding::move
        : callback == 0x50afa0 ? CommandBehaviorBinding::target
        : callback == 0x50ccd0 ? CommandBehaviorBinding::coordinate
        : CommandBehaviorBinding::supplied;
  }
  importQueues();
}
#endif
void event(unsigned kind, unsigned argument = 0) {
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
  if (original)
    importActor();
#endif
  events.insert(events.end(),
                {kind, argument,
                 unsigned(current->relations.creatures[0].ai.owner),
                 current->relations.creatures[0].ai.behavior,
                 current->relations.creatures[0].ai.pending});
}
void recountCreatures() {
  throw std::runtime_error("owned recount service escaped");
}
void refreshVisibility() {
  throw std::runtime_error("owned visibility service escaped");
}
void reset(unsigned) {
  throw std::runtime_error("owned reset service escaped");
}
void suppliedIdle(unsigned) {
  throw std::runtime_error("owned idle service escaped");
}
// Lower idle query/action services may mutate fields; original short-circuit
// order, cached type admission and subsequent pointer binding remain guarded.
bool idleQuery(unsigned slot, unsigned kind) {
  event(kind, slot);
  auto &a = current->relations.creatures.at(slot);
  if (kind == 19 && current->number % 41 == 0)
    a.ai.type = 22;
  if (kind == 20 && current->number % 37 == 0)
    a.ai.health = 0;
  if (kind == 21 && current->number % 23 == 0)
    a.ai.pending = 8;
  bool result = kind == 19   ? (current->number + slot) % 3 == 0
                : kind == 20 ? (current->number / 3 + slot) % 3 != 0
                : kind == 21 ? (current->number / 5 + slot) % 4 == 0
                             : (current->number / 7 + slot) % 4 == 0;
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
  if (original)
    syncActor();
#endif
  return result;
}
void idleAction(unsigned slot, unsigned kind) {
  if (kind != 1 && kind != 6 && kind != 15)
    throw std::runtime_error("idle action selector");
  event(23, (slot << 8) | kind);
  auto &a = current->relations.creatures.at(slot);
  a.ai.action = kind;
  ++a.cooldown;
  if (current->number % 29 == 0)
    a.ai.behavior = 3;
  if (current->number % 23 == 0)
    a.ai.pending = 8;
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
  if (original)
    syncActor();
#endif
}
BehaviorExecutionServices idleServices(unsigned slot) {
  BehaviorExecutionServices s;
  s.immediate = [slot](auto &) { return idleQuery(slot, 19); };
  s.interaction = [slot](auto &) { return idleQuery(slot, 20); };
  s.follow = [](auto &) -> bool {
    throw std::runtime_error("owned follow service escaped");
  };
  s.queue = [](auto &) -> bool {
    throw std::runtime_error("owned queue service escaped");
  };
  s.action = [slot](auto &, unsigned kind) { idleAction(slot, kind); };
  return s;
}
void followTrace(unsigned kind, unsigned slot, Cell p) {
  event(kind, slot);
  events.insert(events.end(),
                {kind + 1, slot, unsigned(p.x), unsigned(p.y), unsigned(p.z)});
}
bool followAdjust(unsigned slot, Cell &p) {
  followTrace(24, slot, p);
  const auto target = current->executions.at(slot).follow;
  p.x = (p.x + 1) % current->relations.dimensions.x;
  if (current->number % 17 == 0)
    current->executions[slot].follow = -1;
  if (current->number % 19 == 0)
    current->relations.creatures.at(unsigned(target)).ai.health = 0;
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
  if (original)
    syncActor();
#endif
  return (current->number / 11 + slot) % 3 != 0;
}
FollowServices followServices(unsigned slot) {
  return {[slot](const auto &, Cell &p) { return followAdjust(slot, p); },
          [](auto &, Cell) { throw std::runtime_error("owned move escaped"); }};
}
void commandBody(AutonomousCreature &a, unsigned kind) {
  event(32, (a.ai.slot << 8) | kind);
  ++a.cooldown;
  a.ai.motion = true;
  if (current->number % 29 == 0) a.ai.behavior = 7;
  if (current->number % 23 == 0) a.ai.pending = 8;
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
  if (original) syncActor();
#endif
}
bool commandRenewal(unsigned slot) {
  event(33, slot);
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
  if (original) syncActor();
#endif
  return (current->number / 5 + slot) % 3 != 0;
}
void commandOrient(unsigned slot, Cell p) { followTrace(34, slot, p); }
void commandUpdate(unsigned slot) { event(36, slot); }
ExecutionServices queueServices(unsigned) {
  ExecutionServices s;
  s.performOrder = [](const auto &) {
    throw std::runtime_error("duplicate queue dispatcher escaped");
  };
  s.releaseEffect = [](unsigned) {
    throw std::runtime_error("duplicate queue release escaped");
  };
  return s;
}
void restoreResetColor(unsigned id, unsigned key) {
  event(18, (id << 8) | key);
  auto &a = current->relations.creatures.at(id);
  if (current->number % 31 == 0)
    a.ai.pending = 8;
  if (current->number % 37 == 0)
    a.ai.health = 0;
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
  if (original)
    syncActor();
#endif
}
void releaseEffect(unsigned id) {
  event(3, id);
  if (current->number % 7)
    return;
  for (unsigned slot = 0; slot < current->executions.size(); ++slot) {
    auto &list = current->executions[slot].orders;
    for (unsigned i = 0; i + 1 < list.size(); ++i) {
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
      if (original) {
        if (list[i].effect == id) {
          put(orders.data() + (slot * 16 + list[i+1].serial - 1) * 48, 0x28,
              reinterpret_cast<std::uintptr_t>(effects.data() + (16 + i) * 4));
          return;
        }
      } else
#endif
          if (list[i].effect == id) {
        list[i + 1].effect = 16 + i;
        return;
      }
    }
  }
}
void clearMarkers() {
  throw std::runtime_error("owned markers service escaped");
}
bool wizard() {
  throw std::runtime_error("owned classification service escaped");
}
void unselect() { event(6); }
void color(unsigned key) { event(7, key); }
void repairOwnerTargets() {
  throw std::runtime_error("owned target repair service escaped");
}
void prepareRepair() {
  event(14);
  if (current->number % 7 == 0)
    current->relations.creatures[0].ai.owner = 7;
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
  if (original)
    syncActor();
#endif
}
void repairBehavior(unsigned id) {
  if (current->idleBindings.at(id) != CommandBehaviorBinding::supplied)
    throw std::runtime_error("binding dispatch escaped");
  event(15, id);
  auto &a = current->relations.creatures.at(id);
  a.ai.behavior = current->number % 2 ? 1 : 3;
  a.ai.target = 0;
  a.ai.pending = 3;
  if (current->number % 11 == 0)
    a.ai.health = 0;
  if (current->number % 13 == 0)
    current->repair.count =
        std::min(2u, unsigned(current->relations.creatures.size()));
  if (id + 1 < current->relations.creatures.size() && current->number % 19 == 0)
    current->relations.creatures[id + 1].ai.active = false;
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
  if (original)
    syncActor();
#endif
}
void repairPeer(unsigned id, bool flag) {
  if (flag)
    throw std::runtime_error("repair peer flag");
  event(16, id);
  auto &a = current->relations.creatures.at(id);
  if (a.ai.incoming)
    --a.ai.incoming;
  if (current->number % 17 == 0)
    current->relations.creatures[0].ai.owner =
        (current->relations.creatures[0].ai.owner + 1) % 8;
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
  if (original)
    syncActor();
#endif
}
OwnerTargetRepairServices repairServices() {
  return {[](auto &) { prepareRepair(); },
          [](auto &a) { repairBehavior(a.ai.slot); },
          [](auto &) {
            throw std::runtime_error("duplicate reset service escaped");
          },
          [](auto &a, bool flag) { repairPeer(a.ai.slot, flag); }};
}
ExecutionServices resetServices(unsigned slot) {
  ExecutionServices s;
  s.behavior = [](auto &a, unsigned kind) {
    if (kind != 1)
      throw std::runtime_error("reset behavior selector");
    suppliedIdle(a.ai.slot);
  };
  s.restoreColor = [slot](unsigned key) { restoreResetColor(slot, key); };
  s.releaseEffect = [](unsigned) {
    throw std::runtime_error("duplicate reset release escaped");
  };
  s.peerEngagement = [](auto &, unsigned) {
    throw std::runtime_error("duplicate reset peer escaped");
  };
  s.action = [slot](auto &, unsigned kind) { idleAction(slot, kind); };
  s.movementRenewal = [slot](auto &) { return commandRenewal(slot); };
  s.orient = [slot](auto &, Cell p) { commandOrient(slot, p); };
  s.updateQueue = [slot](auto &) { commandUpdate(slot); };
  s.terrainHeight = [](Cell p) { return (p.x + p.y) % 16; };
  return s;
}
void clearRootActivity(unsigned) {
  throw std::runtime_error("owned root activity service escaped");
}
void controlWork(bool wizardController) {
  event(10, wizardController);
  if (current->number % 17 == 0) {
    current->relations.creatures[0].ai.owner = 6;
    current->relations.creatures[0].animationKind = 2;
    current->relations.creatures[0].ai.kind = 24;
  }
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
  if (original)
    syncActor();
#endif
}
void countDisplay(unsigned slot, unsigned count) {
  event(11, count);
  if (current->number % 13 == 0) {
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
    if (original) {
      put(actor.data() + slot * 0xe4b, 0x8f1, 7);
      put(actor.data() + slot * 0xe4b, 0x8f5, unsigned(-1));
      return;
    }
#endif
    current->limits.at(slot) = {-1, 7};
  } else if (current->number % 17 == 0) {
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
    if (original) {
      put(actor.data() + slot * 0xe4b, 0x8f1, 7);
      put(actor.data() + slot * 0xe4b, 0x8f5, 7);
      return;
    }
#endif
    current->limits.at(slot) = {7, 7};
  }
}
OwnerCountServices countServices() {
  return {[](auto &a, unsigned count) { countDisplay(a.ai.slot, count); },
          [](auto &a) { event(12, a.ai.slot); },
          [](auto &a) { event(13, a.ai.slot); }};
}
CreatureTransferServices services() {
  CreatureTransferServices s;
  s.recountCreatures = [](auto &) { recountCreatures(); };
  s.refreshVisibility = [](auto &) { refreshVisibility(); };
  s.reset = [](auto &a) { reset(a.ai.slot); };
  s.releaseEffect = releaseEffect;
  s.clearMarkers = [](auto &) { clearMarkers(); };
  s.wizard = [](const auto &) { return wizard(); };
  s.unselectWizard = unselect;
  s.color = [](auto &, unsigned key) { color(key); };
  s.repairOwnerTargets = [](auto &) { repairOwnerTargets(); };
  s.clearRootActivity = clearRootActivity;
  s.controlWork = [](auto &, bool wizardController) {
    controlWork(wizardController);
  };
  return s;
}
Fixture generateTransfer(unsigned n) {
  Fixture f;
  f.number = n;
  const auto count = 1 + n % 32;
  f.relations.creatures.resize(count);
  f.limits.resize(count);
  f.executions.resize(count);
  f.descriptorProfiles.resize(count);
  f.relations.dimensions = {16, 16, 4};
  f.idleBindings.resize(count, CommandBehaviorBinding::supplied);
  f.repair.count = count;
  f.repair.markers6a4.resize(count);
  for (unsigned i = 0; i < count; ++i) {
    auto &a = f.relations.creatures[i];
    a.ai.slot = i;
    a.ai.active = (n + i) % 5 != 0;
    a.ai.health = (n + i) % 7 ? 400 : 0;
    a.ai.owner = int((n / 5 + i) % 8);
    a.ai.kind = (n + i) % 3 ? 24 : 1;
    a.ai.type = (n / 3 + i) % 28;
    constexpr unsigned behaviors[] = {1, 3, 8, 9, 10, 11, 12};
    a.ai.behavior = behaviors[(n / 3 + i) % 7];
    a.ai.action = (n + i) % 3 ? 20 : 10;
    a.alternateTarget = (n + i) % 4 ? 0 : -1;
    a.ai.target = (n + i) % 3 ? 0 : -1;
    a.ai.engaged = (n + i) % 2;
    a.ai.pending = (n + i) % 5;
    a.ai.incoming = (n + i) % 7;
    f.repair.markers6a4[i] = (n + i) % 13;
    a.block7c = (n + i) % 11 == 0;
    if (a.ai.engaged && ((a.ai.action == 10 && a.ai.target < 0) ||
                         (a.ai.action == 20 && a.alternateTarget < 0)))
      a.ai.engaged = 0;
    a.ai.position = {int((n + i * 7) % 16), int((n / 3 + i * 11) % 16),
                     int(n % 4)};
    if (i && n % 19 == 0)
      a.ai.position = {-1, 17, int(n % 4)};
    a.ai.home = {2, 3, 1};
    a.ai.motion = (n + i) % 4 == 0;
    a.mode = (n / 3 + i) % 4;
    a.block784 = n % 7;
    a.blockDE3 = n % 11;
    a.block71c = n % 13;
    a.block723 = n % 17;
    a.cooldown = int(n % 23);
    a.ai.readiness = n % 5;
    a.substate = n % 10;
    a.ai.flying = n % 3;
    a.ai.dirty932 = 8;
    a.ai.marker = 6;
    f.descriptorProfiles[i] = {1 + (n + i) % 2, 1 + (n / 5 + i) % 4,
                               (n / 7 + i) % 3, i + 1};
    auto &e = f.executions[i];
    e.follow = (n + i) % 3 ? int((n / 3 + i + 1) % count) : -1;
    e.absorptionTimer = n % 11;
    e.mode10c = (n + i) % 4;
    e.markerB73 = 5;
    e.block962 = 2;
    e.block963 = 9;
    e.block722 = 3;
    e.marker6a0 = 4;
    e.routeRenewal = n % 2;
    e.commandMode = 1;
    if (i) {
      e.routeEffect =
          (n + i) % 4 ? std::optional<unsigned>((n + i) % 32) : std::nullopt;
      e.orders.resize((n + i) % 4);
      for (unsigned j = 0; j < e.orders.size(); ++j) {
        e.orders[j].kind = (n + i + j) % 4;
        e.orders[j].effect =
            (n + i + j) % 3 ? std::optional<unsigned>(j) : std::nullopt;
      }
    }
    f.limits[i] = {int(n % 11) - 2, int(n / 7 % 17) - 2};
    if (n % 13 == 0)
      f.limits[i] = {7, 7};
    else if (n % 17 == 0)
      f.limits[i] = {-1, 7};
  }
  for (unsigned i = 0; i < 8; ++i)
    f.owners[i] = {(n / 3 + i) % 3, (n / 5 + i) % 5 ? i % count : 0xffffffffu};
  const auto localSlot = f.owners[n % 8].actor;
  f.relations.localOwner = n % 3 && localSlot < count
                               ? f.relations.creatures[localSlot].ai.owner
                               : int(n / 3 % 9) - 1;
  f.relations.creatures[0].ai.slot = 0;
  f.relations.creatures[0].ai.owner = int(n % 8);
  f.requested = n % 4 ? int(n / 9 % 8) : f.relations.creatures[0].ai.owner;
  f.relations.creatures[0].ai.behavior = n / 3 % 16 + 1;
  f.relations.creatures[0].ai.pending = n / 5 % 16;
  constexpr unsigned kinds[] = {0, 1, 23, 24, 25, 26, 27};
  f.relations.creatures[0].ai.kind = kinds[n / 7 % 7];
  f.relations.creatures[0].block7c = n % 5 == 0;
  f.relations.creatures[0].animationKind = n / 11 % 4;
  f.executions[0].marker68 = n % 3;
  f.executions[0].routeEffect =
      n % 4 ? std::optional<unsigned>(n % 4) : std::nullopt;
  f.transfer.marker70 = n / 13 % 4;
  f.transfer.marker6e4 = n % 17;
  f.transfer.roots = {n % 4 ? std::optional<unsigned>(n % 4) : std::nullopt,
                      n % 5 ? std::optional<unsigned>(n / 4 % 4)
                            : std::nullopt};
  f.transfer.display = 0;
  f.world.displayOwners = {5};
  f.world.selected = n % 7;
  f.world.selectionPresent = n % 3;
  f.graph.resize(4);
  constexpr unsigned activity[] = {0, 1,          2,          3,
                                   4, 0x7fffffff, 0x80000000, 0xffffffff};
  for (unsigned i = 0; i < 4; ++i)
    f.graph[i].activity = activity[(n / 3 + i) % 8];
  for (unsigned i = 0; i < 8; ++i) {
    f.relations.creatures[0].ai.visible[i] = n * 17 + i;
    f.visibility.secondary[i] = n * 23 + i;
    for (unsigned j = 0; j < 8; ++j) {
      f.relations.relation[i][j] = (n + i + j) % 5;
      f.sharing[i][j] = (n / 3 + i + j) % 3;
    }
  }
  f.executions[0].orders.resize(n % 16);
  for (unsigned i = 0; i < f.executions[0].orders.size(); ++i) {
    f.executions[0].orders[i].kind = (n + i) % 4;
    f.executions[0].orders[i].effect =
        (n + i) % 3 ? std::optional<unsigned>(i) : std::nullopt;
  }
  f.initialOrders.resize(count);
  for (unsigned slot = 0; slot < count; ++slot) {
    auto &a = f.relations.creatures[slot];
    auto &e = f.executions[slot];
    if (a.ai.type == 27) a.ai.type = 1;
    a.selectedSpell = (n + slot) % 104;
    a.castSpell = 103; a.castSource = -1; a.castTarget = -1;
    e.castAim = {2, 3, 4}; e.targetDistance = 7;
    a.routePhase = (n + slot) % 3; e.routeChanged = (n + slot) % 2;
    a.form = (n + slot) % 11 == 0;
    a.formCapability = f.descriptorProfiles[slot][2];
    e.orders.resize((n + slot) % 16);
    e.queuedCount = e.orders.size();
    e.queuedFlag = (n / 3 + slot) % 3 == 0;
    for (unsigned j = 0; j < e.orders.size(); ++j) {
      auto &o = e.orders[j];
      o.serial = j + 1;
      o.kind = (j == 0 || j + 1 == e.orders.size()) ? 2 : 2 + (n / 5 + j) % 3;
      o.destination = {int((n + j) % 16), int((n / 3 + j) % 16), int(n % 4)};
      o.referenceDestination = o.destination;
      o.target = o.kind == 3 ? int((n / 7 + slot + j) % count) : -1;
      o.repeat = (n / 7 + slot + j) % 5 == 0;
      o.effect = (n + slot + j) % 3 ? std::optional<unsigned>(j) : std::nullopt;
    }
    e.orderCursor =
        e.orders.empty()
            ? std::nullopt
            : std::optional<unsigned>(
                  e.orders[(n / 5 + slot) % e.orders.size()].serial);
    f.initialOrders[slot] = e.orders;
  }
  return f;
}
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
unsigned nodeID(const void *p) {
  const auto value = reinterpret_cast<std::uintptr_t>(p),
             base = reinterpret_cast<std::uintptr_t>(orders.data());
  if (value < base || value - base >= orders.size() || (value - base) % 48)
    throw std::runtime_error("queue node backing");
  return unsigned(value - base) / 48;
}
void *orderPointer(unsigned slot, unsigned serial) {
  return orders.data() + (slot * 16 + serial - 1) * 48;
}
void packOrder(void *p, const CreatureOrder &o) {
  put(p, 8, o.kind);
  put(p, 12, unsigned(o.destination.x));
  put(p, 16, unsigned(o.destination.y));
  put(p, 20, unsigned(o.destination.z));
  put(p, 24,
      o.target < 0 ? 0
                   : reinterpret_cast<std::uintptr_t>(
                         actor.data() + unsigned(o.target) * 0xe4b));
  put(p, 28, unsigned(o.referenceDestination.x));
  put(p, 32, unsigned(o.referenceDestination.y));
  put(p, 36, unsigned(o.referenceDestination.z));
  put(p, 40,
      o.effect
          ? reinterpret_cast<std::uintptr_t>(effects.data() + *o.effect * 4)
          : 0);
  static_cast<unsigned char *>(p)[44] = o.repeat;
}
void packQueueActor(void *p, unsigned slot, const CreatureExecution &e) {
  put(p, 0x8fd,
      reinterpret_cast<std::uintptr_t>(e.orderCursor
                                           ? orderPointer(slot, *e.orderCursor)
                                           : sentinel.data() + slot * 48));
  static_cast<unsigned char *>(p)[0x901] =
      static_cast<unsigned char>(e.queuedFlag);
  put(p, 0x902, e.queuedCount);
}
void syncQueues() {
  for (unsigned slot = 0; slot < current->executions.size(); ++slot) {
    auto &e = current->executions[slot];
    packQueueActor(actor.data() + slot * 0xe4b, slot, e);
    for (const auto &o : e.orders)
      packOrder(orderPointer(slot, o.serial), o);
  }
}
int signedQueueWord(unsigned value) {
  return value <= 0x7fffffff ? int(value) : -1 - int(~value);
}
void importQueues() {
  for (unsigned slot = 0; slot < current->executions.size(); ++slot) {
    auto &e = current->executions[slot];
    const auto *p = actor.data() + slot * 0xe4b;
    auto *head = sentinel.data() + slot * 48;
    e.queuedFlag = p[0x901];
    e.queuedCount = get(p, 0x902);
    const auto cursor = get(p, 0x8fd);
    e.orderCursor =
        cursor == reinterpret_cast<std::uintptr_t>(head)
            ? std::nullopt
            : std::optional<unsigned>(
                  nodeID(reinterpret_cast<void *>(cursor)) % 16 + 1);
    std::vector<CreatureOrder> live;
    auto address = get(head, 0);
    std::set<unsigned> seen;
    while (address != reinterpret_cast<std::uintptr_t>(head)) {
      const auto id = nodeID(reinterpret_cast<void *>(address));
      if (id / 16 != slot || !seen.insert(id).second || retiredNodes.count(id))
        throw std::runtime_error("live queue topology");
      auto *node = reinterpret_cast<unsigned char *>(address);
      CreatureOrder o;
      o.serial=id%16+1;
      o.kind = get(node, 8);
      o.destination = {signedQueueWord(get(node, 12)),
                       signedQueueWord(get(node, 16)),
                       signedQueueWord(get(node, 20))};
      auto target = get(node, 24);
      o.target =
          target
              ? int((target - reinterpret_cast<std::uintptr_t>(actor.data())) /
                    0xe4b)
              : -1;
      o.referenceDestination = {signedQueueWord(get(node, 28)),
                                signedQueueWord(get(node, 32)),
                                signedQueueWord(get(node, 36))};
      o.effect = effectToken(get(node, 40));
      o.repeat = node[44] != 0;
      live.push_back(o);
      address = get(node, 0);
    }
    if (live.size() != get(container.data() + slot * 16, 8))
      throw std::runtime_error("live queue count");
    e.orders = std::move(live);
  }
}
extern "C" void queueFreeStub(void *p) {
  const auto id = nodeID(p);
  if (retiredNodes.count(id))
    throw std::runtime_error("queue double free");
  auto *bytes = static_cast<unsigned char *>(p);
  for (unsigned i = 45; i < 48; ++i)
    if (bytes[i] != 0x83)
      throw std::runtime_error("retired queue padding");
  retiredNodes[id] = Bytes(bytes, bytes + 48);
}
extern "C" unsigned __attribute__((thiscall)) commandRenewalStub(void *p) {
  importActor();
  return commandRenewal(unsigned((static_cast<unsigned char *>(p)-actor.data())/0xe4b));
}
extern "C" void __attribute__((thiscall)) commandOrientStub(void *p, int x, int y) {
  importActor();
  commandOrient(unsigned((static_cast<unsigned char *>(p)-actor.data())/0xe4b), {x,y,0});
}
extern "C" void __attribute__((thiscall)) commandUpdateStub(void *p) {
  importActor();
  commandUpdate(unsigned((static_cast<unsigned char *>(p)-actor.data())/0xe4b));
}
extern "C" void __attribute__((thiscall)) commandMoveBodyStub(void *p) {
  importActor(); commandBody(current->relations.creatures.at(unsigned((static_cast<unsigned char *>(p)-actor.data())/0xe4b)),2);
}
extern "C" void __attribute__((thiscall)) commandTargetBodyStub(void *p) {
  importActor(); commandBody(current->relations.creatures.at(unsigned((static_cast<unsigned char *>(p)-actor.data())/0xe4b)),3);
}
extern "C" void __attribute__((thiscall)) commandCoordinateBodyStub(void *p) {
  importActor(); commandBody(current->relations.creatures.at(unsigned((static_cast<unsigned char *>(p)-actor.data())/0xe4b)),4);
}
std::vector<std::pair<unsigned, Bytes>> bodies;
Bytes dispatchTable;
void checkActor(void *p) {
  if (p != actor.data())
    throw std::runtime_error("transfer actor ABI");
}
unsigned actorSlot(void *p) {
  const auto d = static_cast<unsigned char *>(p) - actor.data();
  if (d < 0 || unsigned(d) >= actor.size() || d % 0xe4b)
    throw std::runtime_error("repair actor ABI");
  return unsigned(d) / 0xe4b;
}
extern "C" unsigned __attribute__((thiscall)) idleImmediateStub(void *p) {
  return idleQuery(actorSlot(p), 19);
}
extern "C" unsigned __attribute__((thiscall)) idleInteractionStub(void *p) {
  return idleQuery(actorSlot(p), 20);
}
extern "C" unsigned __attribute__((thiscall))
followAdjustStub(void *receiver, unsigned actorIndex, const void *descriptor,
                 int *x, int *y, int *z) {
  if (reinterpret_cast<std::uintptr_t>(receiver) != 0x6c5490 ||
      actorIndex != 0xffffffffu)
    throw std::runtime_error("follow geometry ABI");
  const auto id = get(descriptor, 12) - 1;
  if (id >= current->descriptorProfiles.size())
    throw std::runtime_error("follow descriptor identity");
  Bytes expected(22, 0);
  for (unsigned i = 0; i < 4; ++i)
    put(expected.data(), 4 * i, current->descriptorProfiles[id][i]);
  put(expected.data(), 17, current->relations.creatures[id].ai.kind);
  if (std::memcmp(descriptor, expected.data(), expected.size()))
    throw std::runtime_error("follow descriptor22 guard");
  Cell p{*x, *y, *z};
  const auto result = followAdjust(id, p);
  *x = p.x;
  *y = p.y;
  *z = p.z;
  return result;
}
extern "C" void __attribute__((thiscall)) idleActionStub(void *p,
                                                         unsigned kind) {
  idleAction(actorSlot(p), kind);
}
extern "C" void __attribute__((thiscall)) resetColorStub(void *p,
                                                         unsigned key) {
  restoreResetColor(actorSlot(static_cast<unsigned char *>(p) - 0xbf3), key);
}
extern "C" void __attribute__((thiscall)) prepareRepairStub(void *p) {
  checkActor(p);
  prepareRepair();
}
extern "C" void __attribute__((thiscall)) repairBehaviorStub(void *p) {
  importActor();
  repairBehavior(actorSlot(p));
}
extern "C" void __attribute__((thiscall)) repairPeerStub(void *p,
                                                         unsigned flag) {
  repairPeer(actorSlot(p), flag);
}
extern "C" void __attribute__((thiscall)) effectStub(void *p) {
  releaseEffect(
      unsigned((static_cast<unsigned char *>(p) - effects.data()) / 4));
}
extern "C" void __attribute__((thiscall)) unselectStub(void *p) {
  if (reinterpret_cast<std::uintptr_t>(p) != 0x6c4e40)
    throw std::runtime_error("transfer unselect ABI");
  unselect();
}
extern "C" void __attribute__((thiscall)) colorStub(void *p, unsigned key) {
  if (p != actor.data() + 0xbf3)
    throw std::runtime_error("transfer color ABI");
  color(key);
}
extern "C" void __attribute__((thiscall)) memberStub(void *p, void *a) {
  checkActor(a);
  auto v = reinterpret_cast<std::uintptr_t>(p);
  if (v != 0x690600 && v != 0x644550)
    throw std::runtime_error("transfer controlWork ABI");
  controlWork(v == 0x644550);
}
unsigned countSubject() {
  const auto owner = get(actor.data(), 0x174);
  return current->owners.at(owner).actor;
}
extern "C" void __attribute__((thiscall)) countDisplayStub(void *p,
                                                           unsigned count) {
  if (reinterpret_cast<std::uintptr_t>(p) != 0x6903f0)
    throw std::runtime_error("count display ABI");
  countDisplay(countSubject(), count);
}
extern "C" void __attribute__((thiscall)) countBelowStub(void *p) {
  if (reinterpret_cast<std::uintptr_t>(p) != 0x6a2dd0)
    throw std::runtime_error("count below ABI");
  event(12, countSubject());
}
extern "C" void __attribute__((thiscall)) countAtStub(void *p) {
  if (reinterpret_cast<std::uintptr_t>(p) != 0x6a2dd0)
    throw std::runtime_error("count at ABI");
  event(13, countSubject());
}
void redirect(unsigned address, void *target) {
  auto *p = reinterpret_cast<unsigned char *>(address);
  p[0] = 0xe9;
  put(p, 1, unsigned(reinterpret_cast<std::uintptr_t>(target) - address - 5));
}
void install(const char *path) {
  map_image(read(path));
  dispatchTable = Bytes(reinterpret_cast<unsigned char *>(0x50a9e4),
                        reinterpret_cast<unsigned char *>(0x50aa24));
  for (auto r : {std::pair<unsigned, unsigned>{0x51ec80, 0x51eded},
                 {0x51a880, 0x51a8ec},
                 {0x512340, 0x512383},
                 {0x411e50, 0x411e6e},
                 {0x407b90, 0x407bb4},
                 {0x522340, 0x52243e},
                 {0x466870, 0x4668b0},
                 {0x51edf0, 0x51ef5a},
                 {0x50ab30, 0x50ac91},
                 {0x50aa30, 0x50ab05},
                 {0x50a930, 0x50a9e2},
                 {0x50acb0, 0x50ad03},
                 {0x513850, 0x51397f},
                 {0x40e250, 0x40e285},
                 {0x524130, 0x524382},
                 {0x524390, 0x5243a3},
                 {0x52cba0, 0x52cba6},
                 {0x40e650, 0x40e672},
                 {0x4132e0, 0x413302},
                 {0x52d0c0, 0x52d105},
                 {0x520ad0, 0x520b24},
                 {0x50fae0,0x50fd4c},
                 {0x50ad10,0x50af02},
                 {0x50ca30,0x50cc0e}})
    bodies.push_back(
        {r.first, Bytes(reinterpret_cast<unsigned char *>(r.first),
                        reinterpret_cast<unsigned char *>(r.second))});
  for (auto v : {std::pair<unsigned, void *>{
                     0x4dea00, reinterpret_cast<void *>(countDisplayStub)},
                 {0x4d7a00, reinterpret_cast<void *>(countBelowStub)},
                 {0x4d78b0, reinterpret_cast<void *>(countAtStub)},
                 {0x516220, reinterpret_cast<void *>(idleImmediateStub)},
                 {0x516800, reinterpret_cast<void *>(idleInteractionStub)},
                 {0x4f5230, reinterpret_cast<void *>(followAdjustStub)},

                 {0x597870, reinterpret_cast<void *>(queueFreeStub)},
                 {0x516fd0, reinterpret_cast<void *>(commandRenewalStub)},
                 {0x5084f0, reinterpret_cast<void *>(commandOrientStub)},
                 {0x5120c0, reinterpret_cast<void *>(commandUpdateStub)},
                 {0x510010, reinterpret_cast<void *>(commandMoveBodyStub)},
                 {0x50afa0, reinterpret_cast<void *>(commandTargetBodyStub)},
                 {0x50ccd0, reinterpret_cast<void *>(commandCoordinateBodyStub)},
                 {0x50db60, reinterpret_cast<void *>(idleActionStub)},
                 {0x54dce0, reinterpret_cast<void *>(resetColorStub)},
                 {0x4943f0, reinterpret_cast<void *>(effectStub)},
                 {0x4d3ea0, reinterpret_cast<void *>(unselectStub)},
                 {0x54dad0, reinterpret_cast<void *>(colorStub)},
                 {0x5178c0, reinterpret_cast<void *>(prepareRepairStub)},
                 {0x50eb60, reinterpret_cast<void *>(repairPeerStub)},
                 {0x416590, reinterpret_cast<void *>(memberStub)}})
    redirect(v.first, v.second);
}
void pack(void *p, const Fixture &f) {
  put(p, 0x174, unsigned(f.relations.creatures[0].ai.owner));
  put(p, 0x5ec, f.relations.creatures[0].ai.behavior);
  put(p, 0x5f0, f.relations.creatures[0].ai.pending);
  put(p, 0x68, f.executions[0].marker68);
  put(p, 0x70, f.transfer.marker70);
  put(p, 0x6e4, f.transfer.marker6e4);
  put(p, 0x6e8,
      f.executions[0].routeEffect
          ? reinterpret_cast<std::uintptr_t>(effects.data() +
                                             *f.executions[0].routeEffect * 4)
          : 0);
  put(p, 0x7c, f.relations.creatures[0].block7c);
  put(p, 0x7cd, f.relations.creatures[0].animationKind);
  put(p, 0x7d1,
      f.transfer.display ? reinterpret_cast<std::uintptr_t>(
                               displays.data() + *f.transfer.display * 0x220)
                         : 0);
  for (unsigned i = 0; i < 2; ++i)
    put(p, 0x678 + i * 4,
        f.transfer.roots[i] ? reinterpret_cast<std::uintptr_t>(
                                  nodes.data() + *f.transfer.roots[i] * 141)
                            : 0);
  for (unsigned i = 0; i < 8; ++i) {
    put(p, 0x24 + i * 4, f.relations.creatures[0].ai.visible[i]);
    put(p, 0x48 + i * 4, f.visibility.secondary[i]);
  }
}
void packCounts(Bytes &data, const Fixture &f) {
  for (unsigned i = 0; i < f.relations.creatures.size(); ++i) {
    const auto &a = f.relations.creatures[i];
    auto *p = data.data() + i * 0xe4b;
    put(p, 0, a.ai.slot);
    put(p, 4, a.ai.active);
    put(p, 0xe4, a.ai.health);
    put(p, 0xa8, a.ai.type);
    put(p, 0x174, unsigned(a.ai.owner));
    put(p, 0xa8, a.ai.type);
    put(p, 0x7c, a.block7c);
    put(p, 0x5ec, a.ai.behavior);
    put(p, 0x5fc, a.ai.action);
    put(p, 0x5f0, a.ai.pending);
    put(p, 0x664, a.ai.engaged);
    put(p, 0x668, a.ai.incoming);
    put(p, 0x6a8, unsigned(a.alternateTarget));
    put(p, 0x64c, unsigned(a.ai.target));
    put(p, 0x6a4, f.repair.markers6a4[i]);
    packReset(p, a, f.executions[i]);
    put(p, 0x8f1, unsigned(f.limits[i].maximum));
    put(p, 0x8f5, unsigned(f.limits[i].current));
  }
}
void packOrders(Bytes &data, const Fixture &f) {
  for (unsigned slot = 0; slot < f.executions.size(); ++slot)
    for (const auto &o : f.executions[slot].orders)
      packOrder(data.data() + (slot * 16 + o.serial - 1) * 48, o);
}
void compare(Fixture input, const Fixture &expected,
             const std::vector<unsigned> &trace,
             const CreatureOrder *directDelivery = nullptr,
             const std::function<void(void *)> &operation = {},
             bool zeroSentinelPayload = false) {
  current = &input;
  original = true;
  events.clear();
  retiredNodes.clear();
  actor.assign(input.relations.creatures.size() * 0xe4b, 0x9d);
  definition.assign(input.relations.creatures.size() * 0x5c9, 0xa7);
  effects.assign(32 * 4, 0xb7);
  packCounts(actor, input);
  for (unsigned i = 0; i < input.relations.creatures.size(); ++i) {
    put(actor.data() + i * 0xe4b, 0xac,
        reinterpret_cast<std::uintptr_t>(definition.data() + i * 0x5c9));
    put(definition.data() + i * 0x5c9, 0, input.relations.creatures[i].ai.kind);
  }
  for (unsigned i = 0; i < input.descriptorProfiles.size(); ++i) {
    const auto &profile = input.descriptorProfiles[i];
    for (unsigned j = 0; j < 4; ++j)
      put(definition.data() + i * 0x5c9,
          std::array<unsigned, 4>{12, 8, 0x3c, 0x44}[j], profile[j]);
  }
  for (unsigned i = 0; i < input.relations.creatures.size(); ++i) {
    const auto binding=input.idleBindings[i];
    put(actor.data()+i*0xe4b,0x5f4,
        binding==CommandBehaviorBinding::idle?0x50acb0:
        binding==CommandBehaviorBinding::move?0x510010:
        binding==CommandBehaviorBinding::target?0x50afa0:
        binding==CommandBehaviorBinding::coordinate?0x50ccd0:
        reinterpret_cast<std::uintptr_t>(repairBehaviorStub));
    put(actor.data() + i * 0xe4b, 0x5f8, 0);
  }
  nodes.assign(4 * 141, 0xc7);
  container.assign(input.executions.size() * 16, 0x81);
  sentinel.assign(input.executions.size() * 48, 0x82);
  orders.assign(input.executions.size() * 16 * 48, 0x83);
  relations.assign(8 * 340, 0x84);
  for (unsigned slot = 0; slot < input.executions.size(); ++slot) {
    auto *c = container.data() + slot * 16;
    auto *tail = sentinel.data() + slot * 48;
    if (zeroSentinelPayload) std::memset(tail+8,0,37);
    const auto &list = input.executions[slot].orders;
    put(actor.data() + slot * 0xe4b, 0x8f9,
        reinterpret_cast<std::uintptr_t>(c));
    put(c, 4, reinterpret_cast<std::uintptr_t>(tail));
    put(c, 8, list.size());
    put(tail, 4, reinterpret_cast<std::uintptr_t>(
        list.empty()?tail:orderPointer(slot,list.back().serial)));
    put(tail, 0, reinterpret_cast<std::uintptr_t>(
        list.empty()?tail:orderPointer(slot,list.front().serial)));
    for(unsigned j=0;j<list.size();++j) {
      auto *node=orderPointer(slot,list[j].serial);
      put(node,4,reinterpret_cast<std::uintptr_t>(
          j?orderPointer(slot,list[j-1].serial):tail));
      put(node,0,reinterpret_cast<std::uintptr_t>(
          j+1<list.size()?orderPointer(slot,list[j+1].serial):tail));
    }
  }
  packOrders(orders, input);
  for (unsigned slot = 0; slot < input.executions.size(); ++slot)
    packQueueActor(actor.data() + slot * 0xe4b, slot, input.executions[slot]);
  for (unsigned i = 0; i < 8; ++i)
    for (unsigned j = 0; j < 8; ++j) {
      relations[i * 340 + 0x18 + j] = input.relations.relation[i][j];
      relations[i * 340 + 0x20 + j] = input.sharing[i][j];
    }
  for (unsigned i = 0; i < 8; ++i) {
    put(relations.data() + i * 340, 0, input.owners[i].enabled);
    put(relations.data() + i * 340, 12, input.owners[i].actor);
  }
  std::memcpy(reinterpret_cast<void *>(0x643a80), relations.data(),
              relations.size());
  global(0x6def58, reinterpret_cast<std::uintptr_t>(actor.data()));
  global(0x6def5c, input.relations.creatures.size());
  global(0x6df180, input.repair.count);
  global(0x644520, unsigned(input.relations.localOwner));
  global(0x6c5494, unsigned(input.relations.dimensions.x));
  global(0x6c5498, unsigned(input.relations.dimensions.y));
  mapCells.assign(32*32*4*12, 0); mapTiles.assign(16*356, 0);
  for (unsigned z=0;z<4;++z) global(0x6cb8c2+4*z,z*32*32);
  for (unsigned y=0;y<32;++y) global(0x6cb942+4*y,y*32);
  for(unsigned z=0;z<4;++z) for(unsigned y=0;y<32;++y) for(unsigned x=0;x<32;++x) {
    const unsigned short tile=(x+y)%16;
    std::memcpy(mapCells.data()+((z*32+y)*32+x)*12,&tile,2);
  }
  for(unsigned i=0;i<16;++i) put(mapTiles.data()+i*356,0x94,i);
  global(0x6c54dc,reinterpret_cast<std::uintptr_t>(mapCells.data()));
  global(0x65660c,reinterpret_cast<std::uintptr_t>(mapTiles.data()));
  auto mapCellsBefore=mapCells, mapTilesBefore=mapTiles;
  displays.assign(0x220, 0xd7);
  pack(actor.data(), input);
  put(actor.data(), 0xac, reinterpret_cast<std::uintptr_t>(definition.data()));
  put(definition.data(), 0, input.relations.creatures[0].ai.kind);
  for (unsigned i = 0; i < 4; ++i)
    put(nodes.data() + i * 141, 0x89, input.graph[i].activity);
  put(displays.data(), 0x212, unsigned(input.world.displayOwners[0]));
  auto before = actor, definitionBefore = definition, effectsBefore = effects,
       nodesExpected = nodes, displaysExpected = displays,
       ordersExpected = orders, containerBefore = container,
       sentinelBefore = sentinel;
  global(0x6e984c, input.world.selected);
  global(0x6903e8, input.world.selectionPresent);
  using F = void(__attribute__((thiscall)) *)(void *, int);
  if (operation)
    operation(actor.data());
  else if (directDelivery) {
    using Deliver = void(__attribute__((thiscall)) *)(void *, unsigned, int, int, int, void *);
    auto &o = *directDelivery;
    auto *target = o.target < 0 ? nullptr : actor.data() + unsigned(o.target) * 0xe4b;
    reinterpret_cast<Deliver>(0x520ad0)(actor.data(), o.kind, o.destination.x,
                                      o.destination.y, o.destination.z, target);
  } else
    reinterpret_cast<F>(0x51ec80)(actor.data(), input.requested);
  packCounts(before, expected);
  pack(before.data(), expected);
  packOrders(ordersExpected, expected);
  std::set<unsigned> expectedRetired;
  for (unsigned slot = 0; slot < expected.executions.size(); ++slot) {
    const auto &e = expected.executions[slot];
    auto *head = sentinelBefore.data() + slot * 48;
    auto *actualHead = sentinel.data() + slot * 48;
    packQueueActor(before.data() + slot * 0xe4b, slot, e);
    put(containerBefore.data() + slot * 16, 8, e.orders.size());
    put(head, 0,
        reinterpret_cast<std::uintptr_t>(
            e.orders.empty() ? actualHead
                             : orderPointer(slot, e.orders.front().serial)));
    put(head, 4,
        reinterpret_cast<std::uintptr_t>(
            e.orders.empty() ? actualHead
                             : orderPointer(slot, e.orders.back().serial)));
    std::set<unsigned> live;
    for (unsigned j = 0; j < e.orders.size(); ++j) {
      const auto serial = e.orders[j].serial;
      live.insert(serial);
      auto *node = ordersExpected.data() + (slot * 16 + serial - 1) * 48;
      put(node, 0,
          reinterpret_cast<std::uintptr_t>(
              j + 1 == e.orders.size()
                  ? actualHead
                  : orderPointer(slot, e.orders[j + 1].serial)));
      put(node, 4,
          reinterpret_cast<std::uintptr_t>(
              j ? orderPointer(slot, e.orders[j - 1].serial) : actualHead));
    }
    for (const auto &o : input.initialOrders[slot])
      if (!live.count(o.serial))
        expectedRetired.insert(slot * 16 + o.serial - 1);
  }
  std::set<unsigned> actualRetired;
  for (const auto &entry : retiredNodes) {
    actualRetired.insert(entry.first);
    std::memcpy(ordersExpected.data() + entry.first * 48, entry.second.data(),
                48);
  }
  if (actualRetired != expectedRetired)
    throw std::runtime_error("queue retirement identity");
  put(definitionBefore.data(), 0, expected.relations.creatures[0].ai.kind);
  for (unsigned i = 0; i < expected.idleBindings.size(); ++i) {
    put(before.data() + i * 0xe4b, 0x5f4,
        expected.idleBindings[i] == CommandBehaviorBinding::idle ? 0x50acb0
        : expected.idleBindings[i] == CommandBehaviorBinding::move ? 0x510010
        : expected.idleBindings[i] == CommandBehaviorBinding::target ? 0x50afa0
        : expected.idleBindings[i] == CommandBehaviorBinding::coordinate ? 0x50ccd0
        : reinterpret_cast<std::uintptr_t>(repairBehaviorStub));
    put(before.data() + i * 0xe4b, 0x5f8, 0);
  }
  for (unsigned i = 0; i < 4; ++i)
    put(nodesExpected.data() + i * 141, 0x89, expected.graph[i].activity);
  put(displaysExpected.data(), 0x212,
      unsigned(expected.world.displayOwners[0]));
  if (mapCells != mapCellsBefore || mapTiles != mapTilesBefore || actor != before || definition != definitionBefore ||
      effects != effectsBefore || nodes != nodesExpected ||
      orders != ordersExpected || container != containerBefore ||
      sentinel != sentinelBefore ||
      std::memcmp(reinterpret_cast<void *>(0x643a80), relations.data(),
                  relations.size()) ||
      displays != displaysExpected || events != trace ||
      get(reinterpret_cast<void *>(0x6e984c), 0) != expected.world.selected ||
      get(reinterpret_cast<void *>(0x6903e8), 0) !=
          expected.world.selectionPresent) {
    std::cerr << "queue case=" << input.number
              << " trace actual=" << events.size()
              << " expected=" << trace.size() << '\n';
    for (auto pair : {std::pair<const Bytes *, const Bytes *>{&actor, &before},
                      {&orders, &ordersExpected},
                      {&container, &containerBefore},
                      {&sentinel, &sentinelBefore}})
      for (unsigned i = 0; i < pair.first->size(); ++i)
        if ((*pair.first)[i] != (*pair.second)[i]) {
          std::cerr << "buffer diff offset=" << std::hex << i
                    << " actual=" << unsigned((*pair.first)[i])
                    << " expected=" << unsigned((*pair.second)[i]) << std::dec
                    << '\n';
          break;
        }
    unsigned i = 0;
    while (i < events.size() && i < trace.size() && events[i] == trace[i])
      ++i;
    for (unsigned j = i / 5 * 5;
         j < std::min(unsigned(events.size()), i / 5 * 5 + 15); ++j)
      std::cerr << "actual[" << j << "]=" << events[j] << ' ';
    std::cerr << '\n';
    for (unsigned j = i / 5 * 5;
         j < std::min(unsigned(trace.size()), i / 5 * 5 + 15); ++j)
      std::cerr << "expected[" << j << "]=" << trace[j] << ' ';
    std::cerr << '\n';
    throw std::runtime_error("transfer actor/service guards case=" +
                             std::to_string(input.number));
  }
  if (get(reinterpret_cast<void *>(0x6def58), 0) !=
          reinterpret_cast<std::uintptr_t>(actor.data()) ||
      get(reinterpret_cast<void *>(0x6def5c), 0) !=
          input.relations.creatures.size() ||
      get(reinterpret_cast<void *>(0x6df180), 0) != expected.repair.count ||
      get(reinterpret_cast<void *>(0x644520), 0) !=
          unsigned(input.relations.localOwner))
    throw std::runtime_error("transfer count globals guard");
  if (std::memcmp(reinterpret_cast<void *>(0x50a9e4), dispatchTable.data(),
                  dispatchTable.size()))
    throw std::runtime_error("behavior resolver table guard");
  if (get(reinterpret_cast<void *>(0x6c5494), 0) !=
          unsigned(input.relations.dimensions.x) ||
      get(reinterpret_cast<void *>(0x6c5498), 0) !=
          unsigned(input.relations.dimensions.y))
    throw std::runtime_error("follow dimensions guard");
  for (const auto &b : bodies)
    if (std::memcmp(reinterpret_cast<void *>(b.first), b.second.data(),
                    b.second.size()))
      throw std::runtime_error("transfer owned body guard");
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE expected");
  install(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  std::uint64_t digest = 1469598103934665603ull;
  auto mix = [&](unsigned n) { digest = (digest ^ n) * 1099511628211ull; };
  unsigned commandBound = 0, commandDivergence = 0;
  auto mixCommandState = [&](const Fixture &f) {
    for (unsigned i=0;i<f.executions.size();++i) {
      const auto &a=f.relations.creatures[i]; const auto &e=f.executions[i];
      for (unsigned v : {a.ai.action,a.routePhase,a.selectedSpell,a.castSpell,
          unsigned(a.castSource),unsigned(a.castTarget),e.routeChanged,e.targetDistance,
          e.marker68,unsigned(e.castAim.x),unsigned(e.castAim.y),unsigned(e.castAim.z),
          e.routeRenewal,e.routeEffect.value_or(0xffffffff),e.queuedFlag,e.queuedCount,
          e.orderCursor.value_or(0xffffffff),unsigned(f.idleBindings[i]),
          a.ai.incoming,a.ai.engaged,unsigned(a.ai.target),unsigned(a.alternateTarget)}) mix(v);
      for(const auto &o:e.orders) {
        for(unsigned v : {o.serial,o.kind,unsigned(o.target),unsigned(o.destination.x),
            unsigned(o.destination.y),unsigned(o.destination.z),unsigned(o.referenceDestination.x),
            unsigned(o.referenceDestination.y),unsigned(o.referenceDestination.z),
            o.effect.value_or(0xffffffff),unsigned(o.repeat)}) mix(v);
      }
    }
  };
  unsigned noops = 0, roots = 0, displayWrites = 0, visibilityChanges = 0,
           markerReleases = 0, countDisplays = 0, below = 0, at = 0,
           repairCallbacks = 0, repairPeers = 0, idleCalls = 0,
           interactionQueries = 0, followQueries = 0, queueQueries = 0,
           idleActions = 0, interactionActions = 0, colorRestores = 0,
           boundCallbacks = 0, deferred = 0, wordDivergence = 0,
           followAdjusts = 0, followMoves = 0, deliveries = 0, coordinates = 0, retired = 0;
  for (unsigned n = 0; n < 16384; ++n) {
    auto input = generateTransfer(n), expected = input;
    current = &expected;
    original = false;
    events.clear();
    transferCreatureOwnerCommandsOwned(
        0, expected.relations, expected.executions, expected.idleBindings,
        expected.transfer, expected.world, expected.sharing,
        expected.visibility, expected.graph, expected.owners, expected.limits,
        countServices(), expected.repair, repairServices(), resetServices,
        idleServices, followServices, queueServices, commandBody, expected.requested,
        services());
    auto trace = events;
    mixCommandState(expected);
    noops += trace.empty();
    for (unsigned i = 0; i < 4; ++i)
      roots += input.graph[i].activity != expected.graph[i].activity;
    visibilityChanges += input.relations.creatures[0].ai.visible !=
                         expected.relations.creatures[0].ai.visible;
    for (unsigned i = 0; i < trace.size(); i += 5) {
      markerReleases += trace[i] == 3;
      countDisplays += trace[i] == 11;
      below += trace[i] == 12;
      at += trace[i] == 13;
      repairCallbacks += trace[i] == 15;
      repairPeers += trace[i] == 16;
      idleCalls += trace[i] == 19;
      interactionQueries += trace[i] == 20;
      followQueries += trace[i] == 24;
      followAdjusts += trace[i] == 24;
      followMoves += trace[i] == 32 && (trace[i+1]&255)==2;
      queueQueries += trace[i] == 32 && (trace[i+1]&255)==3;
      deliveries += trace[i] == 32 && (trace[i+1]&255)==3;
      coordinates += trace[i] == 32 && (trace[i+1]&255)==4;
      idleActions += trace[i] == 23 && (trace[i + 1] & 255) == 1;
      interactionActions += trace[i] == 23 && (trace[i + 1] & 255) == 6;
      colorRestores += trace[i] == 18;
    }
    for (unsigned i = 0; i < expected.executions.size(); ++i) {
      boundCallbacks +=
          expected.idleBindings[i] == CommandBehaviorBinding::idle;
      wordDivergence +=
          expected.idleBindings[i] == CommandBehaviorBinding::idle &&
          expected.relations.creatures[i].ai.behavior != 1;
      deferred += expected.relations.creatures[i].ai.motion &&
                  expected.relations.creatures[i].ai.pending == 1;
      commandBound += unsigned(expected.idleBindings[i])>=2;
      commandDivergence += unsigned(expected.idleBindings[i])>=2 &&
          expected.relations.creatures[i].ai.behavior != unsigned(expected.idleBindings[i]);
      mix(unsigned(expected.idleBindings[i]));
      const auto &e = expected.executions[i];
      retired += input.executions[i].orders.size() - e.orders.size();
      mix(e.queuedFlag);
      mix(e.queuedCount);
      mix(e.orderCursor.value_or(0xffffffff));
      for (unsigned value :
           {e.absorptionTimer, e.mode10c, e.markerB73, e.block962, e.block963,
            e.block722, e.marker6a0, e.routeRenewal, e.commandMode,
            e.routeEffect.value_or(0xffffffff), unsigned(e.follow)})
        mix(value);
      for (const auto &o : e.orders) {
        mix(o.kind);
        mix(o.effect.value_or(0xffffffff));
        mix(o.serial);
        mix(o.repeat);
      }
    }
    mix(expected.repair.count);
    for (auto v : expected.repair.markers6a4)
      mix(v);
    for (const auto &a : expected.relations.creatures) {
      mix(a.ai.active);
      mix(a.ai.health);
      mix(unsigned(a.ai.owner));
      mix(a.ai.behavior);
      mix(a.ai.pending);
      mix(a.ai.engaged);
      mix(a.ai.incoming);
      mix(unsigned(a.ai.target));
      mix(unsigned(a.alternateTarget));
      for (unsigned value : {a.ai.kind,
                             a.ai.type,
                             unsigned(a.ai.motion),
                             a.mode,
                             a.block784,
                             a.blockDE3,
                             a.block71c,
                             a.block723,
                             a.substate,
                             a.ai.flying,
                             a.ai.readiness,
                             a.ai.dirty932,
                             a.ai.marker,
                             unsigned(a.cooldown),
                             unsigned(a.ai.position.x),
                             unsigned(a.ai.position.y),
                             unsigned(a.ai.position.z),
                             unsigned(a.ai.home.x),
                             unsigned(a.ai.home.y),
                             unsigned(a.ai.home.z),
                             unsigned(a.destination.x),
                             unsigned(a.destination.y),
                             unsigned(a.destination.z)})
        mix(value);
    }
    for (const auto &limit : expected.limits) {
      mix(unsigned(limit.current));
      mix(unsigned(limit.maximum));
    }
    for (auto v : expected.relations.creatures[0].ai.visible)
      mix(v);
    for (auto v : expected.visibility.secondary)
      mix(v);
    for (const auto &g : expected.graph)
      mix(g.activity);
    for (const auto &o : expected.executions[0].orders) {
      mix(o.kind);
      mix(o.effect.value_or(0xffffffff));
      mix(o.serial);
      mix(o.repeat);
    }
    displayWrites += input.world.displayOwners != expected.world.displayOwners;
    for (auto v : trace)
      mix(v);
    mix(unsigned(expected.relations.creatures[0].ai.owner));
    mix(expected.world.selected);
    mix(unsigned(expected.world.displayOwners[0]));
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
    compare(input, expected, trace);
#endif
  }
  unsigned directMoves = 0, directTargets = 0, directCoordinates = 0,
           directDeadTargets = 0, directUnknownModes = 0;
  for (unsigned n = 0; n < 4096; ++n) {
    auto input = generateTransfer(n);
    if (n % 11 == 0) input.relations.creatures[0].ai.type = 27;
    input.executions[0].orders.clear(); input.initialOrders[0].clear();
    input.executions[0].queuedCount = input.executions[0].queuedFlag = 0;
    input.executions[0].orderCursor.reset(); input.executions[0].follow = -1;
    CreatureOrder o;
    o.kind = n % 7;
    o.destination = {int(n % 16), int((n / 7) % 16), int(n % 4)};
    o.target = int((n / 7) % input.relations.creatures.size());
    auto &peer = input.relations.creatures.at(unsigned(o.target));
    peer.ai.health = (n / 7) % 2 ? 0 : 400;
    auto expected = input;
    current = &expected;
    original = false;
    events.clear();
    OwnedCreatureCommands commands(expected.relations, expected.executions,
        expected.idleBindings, resetServices, idleServices, followServices,
        queueServices, commandBody, repairServices().behavior,
        services().releaseEffect, repairServices().peerEngagement);
    auto &a = expected.relations.creatures[0];
    commands.deliver(a, o);
    auto trace = events;
    mixCommandState(expected);
    directMoves += o.kind == 2;
    directTargets += o.kind == 3 && peer.ai.health;
    directCoordinates += o.kind == 4;
    directDeadTargets += o.kind == 3 && !peer.ai.health;
    directUnknownModes += o.kind != 2 && o.kind != 3 && o.kind != 4;
    if ((o.kind == 3 && !peer.ai.health) ||
        (o.kind != 2 && o.kind != 3 && o.kind != 4)) {
      if (!trace.empty())
        throw std::runtime_error("dead/unknown delivery escaped");
    }
    for (auto value : trace)
      mix(value);
    for (unsigned i = 0; i < expected.relations.creatures.size(); ++i) {
      const auto &b = expected.relations.creatures[i];
      mix(b.ai.health); mix(b.ai.action); mix(b.ai.behavior); mix(b.ai.motion);
      mix(b.ai.pending); mix(b.cooldown);
      mix(unsigned(b.destination.x)); mix(unsigned(b.destination.y)); mix(unsigned(b.destination.z));
      for (const auto &q : expected.executions[i].orders) mix(q.kind);
    }
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
    compare(input, expected, trace, &o);
#endif
  }
  if (!directMoves || !directTargets || !directCoordinates ||
      !directDeadTargets || !directUnknownModes)
    throw std::runtime_error("empty delivery branch");
  unsigned refused = 0;
  for (unsigned op = 0; op < 2; ++op) {
    auto f = generateTransfer(1);
    f.relations.creatures[0].animationKind = 2;
    f.transfer.display = op ? std::optional<unsigned>(99) : std::nullopt;
    current = &f;
    original = false;
    bool caught = false;
    try {
      transferCreatureOwnerCommandsOwned(
          0, f.relations, f.executions, f.idleBindings, f.transfer, f.world,
          f.sharing, f.visibility, f.graph, f.owners, f.limits, countServices(),
          f.repair, repairServices(), resetServices, idleServices,
          followServices, queueServices, commandBody, f.requested, services());
    } catch (const std::logic_error &) {
      caught = true;
    }
    if (!caught)
      throw std::runtime_error("transfer display refusal absent");
    ++refused;
  }
  {
    auto f = generateTransfer(1);
    auto &a = f.relations.creatures[0];
    a.ai.behavior = 8;
    a.ai.motion = false;
    a.ai.active = true;
    a.ai.health = 400;
    a.ai.action = 20;
    a.alternateTarget = 0;
    a.ai.target = 0;
    a.ai.engaged = 0;
    f.idleBindings[0] = static_cast<CommandBehaviorBinding>(99);
    current = &f;
    original = false;
    bool caught = false;
    try {
      transferCreatureOwnerCommandsOwned(
          0, f.relations, f.executions, f.idleBindings, f.transfer, f.world,
          f.sharing, f.visibility, f.graph, f.owners, f.limits, countServices(),
          f.repair, repairServices(), resetServices, idleServices,
          followServices, queueServices, commandBody, f.requested, services());
    } catch (const std::invalid_argument &) {
      caught = true;
    }
    if (!caught)
      throw std::runtime_error("invalid callback binding refusal absent");
    ++refused;
  }
  {
    auto f = generateTransfer(1);
    f.executions[0].follow = -1;
    f.executions[0].orderCursor = 999;
    f.relations.creatures[0].ai.type = 1;
    f.relations.creatures[0].ai.health = 400;
    f.relations.creatures[0].ai.motion = false;
    current = &f;
    original = false;
    bool caught = false;
    try {
      transferCreatureOwnerCommandsOwned(
          0, f.relations, f.executions, f.idleBindings, f.transfer, f.world,
          f.sharing, f.visibility, f.graph, f.owners, f.limits, countServices(),
          f.repair, repairServices(), resetServices, idleServices,
          followServices, queueServices, commandBody, f.requested, services());
    } catch (const std::invalid_argument &) {
      caught = true;
    }
    if (!caught)
      throw std::runtime_error("queue cursor refusal absent");
    ++refused;
  }
  // These are native admission tests, not original reentry/topology claims.
  for (unsigned policy = 0; policy < 2; ++policy) {
    auto f = generateTransfer(1);
    auto &a = f.relations.creatures[0];
    a.ai.type=1; a.ai.health=400; a.ai.behavior=1; a.ai.motion=false;
    a.mode=a.block784=a.blockDE3=a.block71c=a.block723=a.form=0;
    auto &e=f.executions[0];
    e.orders.resize(2); e.queuedCount=2; e.queuedFlag=1; e.orderCursor=1;
    for(unsigned j=0;j<2;++j) {
      e.orders[j].serial=j+1; e.orders[j].kind=2;
      e.orders[j].effect.reset(); e.orders[j].repeat=false;
    }
    current=&f; original=false; events.clear();
    OwnedCreatureCommands *active=nullptr;
    auto body=[&](auto &actor, unsigned) {
      if(policy==0) active->queue(actor);
      else f.executions[actor.ai.slot].orders.clear();
    };
    OwnedCreatureCommands commands(f.relations,f.executions,f.idleBindings,
        resetServices,idleServices,followServices,queueServices,body,
        repairServices().behavior,services().releaseEffect,repairServices().peerEngagement);
    active=&commands;
    bool caught=false;
    try { commands.queue(a); } catch(const std::invalid_argument &) { caught=true; }
    if(!caught) throw std::runtime_error("unsafe queue callback admitted");
    ++refused;
    // Failure releases the operation-local reentry guard; caller restores state.
    e.orders.clear(); e.queuedCount=0;
    if(commands.queue(a)) throw std::runtime_error("empty queue after refusal");
  }
  if (!noops || !roots || !displayWrites || !visibilityChanges ||
      !markerReleases || !countDisplays || !below || !at || !repairCallbacks ||
      !repairPeers || !idleCalls || !colorRestores || !boundCallbacks ||
      !deferred || !interactionQueries || !followQueries || !queueQueries ||
      !idleActions || !interactionActions || !wordDivergence ||
      !followAdjusts || !followMoves || !deliveries || !coordinates || !commandBound || !commandDivergence)
    throw std::runtime_error("empty transfer branch");
  std::cout << "{\"success\":true,\"cases\":20480,\"functions\":24,\"outcome_"
               "digest\":"
            << digest << ",\"command_bound_callbacks\":" << commandBound
            << ",\"command_binding_word_divergence\":" << commandDivergence
            << ",\"composed_cases\":16384,\"direct_delivery_cases\":4096"
            << ",\"direct_moves\":" << directMoves
            << ",\"direct_living_targets\":" << directTargets
            << ",\"direct_coordinates\":" << directCoordinates
            << ",\"direct_dead_targets\":" << directDeadTargets
            << ",\"direct_unknown_modes\":" << directUnknownModes
            << ",\"native_refusal_cases\":" << refused
            << ",\"same_owner_noops\":" << noops
            << ",\"root_activity_changes\":" << roots
            << ",\"display_writes\":" << displayWrites
            << ",\"visibility_changes\":" << visibilityChanges
            << ",\"effect_releases\":" << markerReleases
            << ",\"count_displays\":" << countDisplays
            << ",\"below_calls\":" << below << ",\"at_calls\":" << at
            << ",\"repair_callbacks\":" << repairCallbacks
            << ",\"repair_peers\":" << repairPeers
            << ",\"idle_calls\":" << idleCalls
            << ",\"target_handlers\":" << deliveries
            << ",\"coordinate_handlers\":" << coordinates
            << ",\"retired_nodes\":" << retired
            << ",\"follow_adjusts\":" << followAdjusts
            << ",\"shared_move_handlers\":" << followMoves
            << ",\"interaction_queries\":" << interactionQueries
            << ",\"follow_queries\":" << followQueries
            << ",\"living_target_queries\":" << queueQueries
            << ",\"idle_actions\":" << idleActions
            << ",\"interaction_actions\":" << interactionActions
            << ",\"color_restores\":" << colorRestores
            << ",\"bound_callbacks\":" << boundCallbacks
            << ",\"binding_word_divergence\":" << wordDivergence
            << ",\"deferred_behaviors\":" << deferred
            << ",\"reset_redirected\":false,\"transition_redirected\":false,"
               "\"resolver_argument\":1,\"resolver_table_guard\":true,\"idle_"
               "redirected\":false,\"queue_redirected\":false,\"delivery_redirected\":false,\"commands_redirected\":false}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
