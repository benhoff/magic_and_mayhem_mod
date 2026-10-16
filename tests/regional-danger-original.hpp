#pragma once
#include "../reconstruction/creature-ai/regional_danger.hpp"
#include "regional-navigation-original.hpp"
namespace {
using namespace mnm;
namespace m = command_session::model;
struct DangerOriginal {
  Bytes context = Bytes(0x12100), pool = Bytes(125 * 0xe4b),
        configs = Bytes(125 * 0x5c9);
  Bytes header = Bytes(20), nil = Bytes(20), empty = Bytes(16),
        memoryNil = Bytes(0xe68);
  std::array<Bytes, 8> memoryHeads;
  std::array<Bytes, 4> effectHeads;
  std::vector<Bytes> effectNodes, effectObjects;
  Bytes hazardAnimation = Bytes(32);
  void effectInputs(const m::RegionalDangerInputs &in) {
    effectNodes.resize(in.effects.size() + in.hazards.size());
    effectObjects.resize(effectNodes.size());
    std::array<std::vector<unsigned>, 4> groups;
    for (unsigned i = 0; i < effectNodes.size(); ++i) {
      effectNodes[i].assign(12, 0);
      effectObjects[i].assign(0x216, 0);
      auto *raw = effectObjects[i].data();
      unsigned group = 0;
      m::Cell position;
      if (i < in.effects.size()) {
        const auto &v = in.effects[i];
        position = v.position;
        put(raw, 0x4c, v.kind);
        put(raw, 0x212, v.owner);
        global(0x6b13c9 + v.kind * 721, v.radius);
      } else {
        unsigned j = i - unsigned(in.effects.size());
        group = j % 3 + 1;
        position = in.hazards[j].position;
      }
      put(raw, 8, position.x);
      put(raw, 12, position.y);
      put(raw, 16, position.z);
      put(effectNodes[i].data(), 8, address(raw));
      groups[group].push_back(i);
    }
    for (unsigned group = 0; group < 4; ++group) {
      auto &head = effectHeads[group];
      head.assign(12, 0);
      auto &ids = groups[group];
      auto h = address(head.data());
      put(head.data(), 0,
          ids.empty() ? h : address(effectNodes[ids.front()].data()));
      put(head.data(), 4,
          ids.empty() ? h : address(effectNodes[ids.back()].data()));
      for (unsigned i = 0; i < ids.size(); ++i) {
        auto *n = effectNodes[ids[i]].data();
        put(n, 0,
            i + 1 < ids.size() ? address(effectNodes[ids[i + 1]].data()) : h);
        put(n, 4, i ? address(effectNodes[ids[i - 1]].data()) : h);
      }
      global(std::array<unsigned, 4>{0x6898e4, 0x689908, 0x6898fc,
                                     0x6898f0}[group],
             h);
    }
    global(0x6be5af, in.hazards.empty() ? 3 : in.hazards[0].radius);
    global(0x6db79a, address(hazardAnimation.data()));
    global(0x6b73a3, 0);
    global(0x6c0bb9, 1);
    put(hazardAnimation.data(), 12,
        in.hazards.size() > 1 ? in.hazards[1].radius : 4);
    put(hazardAnimation.data(), 28,
        in.hazards.size() > 2 ? in.hazards[2].radius : 5);
    global(0x6b7387, in.hazards.size() > 1 ? in.hazards[1].amount * 10 : 70);
    global(0x6c0b9d, in.hazards.size() > 2 ? in.hazards[2].amount * 10 : 130);
  }
  std::array<std::vector<Bytes>, 8> memories;
  DangerOriginal() {
    // Use the real tree helpers. Only storage is supplied by the fixture.
    put(nil.data(), 16, 1);
    global(0x5faf20, address(nil.data()));
    put(header.data(), 0, address(header.data()));
    put(header.data(), 4, address(nil.data()));
    put(header.data(), 8, address(header.data()));
    put(context.data(), 0x1208c, address(header.data()));
    put(context.data(), 0x12073, 1, 1);
    put(context.data(), 0x12098, 1, 1);
    put(empty.data(), 0, address(empty.data()));
    put(empty.data(), 4, address(empty.data()));
    for (unsigned at : {0x6898e4u, 0x689908u, 0x6898fcu, 0x6898f0u})
      global(at, address(empty.data()));
    put(memoryNil.data(), 0xe64, 1);
    global(0x5faf4c, address(memoryNil.data()));
    global(0x6def58, address(pool.data()));
    global(0x6def5c, 125);
    global(0x6df180, 125);
  }
  void packActor(void *raw, const m::RegionalDangerActor &a) {
    auto *cfg = configs.data() + unsigned(a.slot) * 0x5c9;
    put(cfg, 0, a.typeClass);
    put(cfg, 8, a.width);
    put(cfg, 12, a.height);
    put(cfg, 0x34, a.ranged ? unsigned(a.attackRange) : 0);
    global(0x6a5fb4 + a.type * 0x5c9, a.ranged ? unsigned(a.attackRange) : 0);
    put(cfg, 0x44, a.movementKind);
    put(cfg, 0x589, a.aging);
    put(raw, 0, a.slot);
    put(raw, 4, a.active);
    put(raw, 8, a.position.x);
    put(raw, 12, a.position.y);
    put(raw, 16, a.position.z);
    put(raw, 0xac, address(cfg));
    put(raw, 0xa8, a.type);
    put(raw, 0x7c, a.block7c);
    put(raw, 0xe4, a.health);
    put(raw, 0xe8, a.resource);
    put(raw, 0xec, a.maximumResource);
    put(raw, 0x174, a.owner);
    put(raw, 0x104, a.flag104);
    put(raw, 0x108, a.flying);
    put(raw, 0x114, a.status114);
    put(raw, 0x723, a.block723, 1);
    put(raw, 0x5ec, a.behavior14 ? 14 : 1);
    put(raw, 0x7cd, a.animationKind);
    put(raw, 0xde7, a.region);
    put(raw, 0x91e, a.threat);
    for (unsigned i = 0; i < 8; ++i)
      put(raw, 0xe2b + 4 * i, a.estimatedResource[i]);
    if (a.inventoryResource) {
      put(raw, 0x17c, 19);
      put(raw, 0x180, a.inventoryResource / 256);
      put(raw, 0x190, 1);
    }
  }
  void inputs(const m::RegionalDangerInputs &in, bool publishActors = true) {
    put(context.data(), 0x14, in.tick);
    global(0x6c5494, in.dimensions.x);
    global(0x6c5498, in.dimensions.y);
    global(0x6c549c, in.dimensions.z);
    global(0x6bcad6, in.firePower);
    global(0x6bcada, in.fireRange);
    global(0x6bef73, in.animationRange);
    global(0x6c5c84, in.observationPoints.size());
    for (unsigned i = 0; i < in.observationPoints.size(); ++i) {
      const auto v = in.observationPoints[i];
      global(0x6c5490 + 0x84 + 20 * i, v.x);
      global(0x6c5490 + 0x88 + 20 * i, v.y);
      global(0x6c5490 + 0x8c + 20 * i, v.z);
    }
    for (unsigned i = 0; i < 8; ++i) {
      void *o = reinterpret_cast<void *>(0x643a80 + i * 340);
      put(o, 0, in.enabled[i]);
      put(o, 4, 0);
      put(o, 12, in.commander[i]);
      for (unsigned j = 0; j < 8; ++j) {
        put(o, 24 + j, in.allied[i][j], 1);
        put(o, 40 + j, in.shared[i][j], 1);
      }
    }
    if (publishActors)
      for (auto &a : in.actors)
        packActor(pool.data() + unsigned(a.slot) * 0xe4b, a);
  }
  void memoryInputs(const m::RegionalDangerInputs &in) {
    for (unsigned owner = 0; owner < 8; ++owner) {
      auto &head = memoryHeads[owner];
      head.assign(0xe68, 0);
      auto &nodes = memories[owner];
      nodes.resize(in.memories[owner].size());
      for (unsigned i = 0; i < nodes.size(); ++i)
        nodes[i].assign(0xe68, 0);
      const auto h = address(head.data()), nilAt = address(memoryNil.data());
      put(head.data(), 0, nodes.empty() ? h : address(nodes.front().data()));
      put(head.data(), 4,
          nodes.empty() ? nilAt : address(nodes.front().data()));
      put(head.data(), 8, nodes.empty() ? h : address(nodes.back().data()));
      for (unsigned i = 0; i < nodes.size(); ++i) {
        auto *n = nodes[i].data();
        auto &v = in.memories[owner][i];
        put(n, 0, nilAt);
        put(n, 4, i ? address(nodes[i - 1].data()) : h);
        put(n, 8, i + 1 < nodes.size() ? address(nodes[i + 1].data()) : nilAt);
        put(n, 12, v.key);
        put(n, 16, v.observedTick);
        put(n, 0xe64, 1);
        packActor(n + 24, v.actor);
      }
      put(context.data(), 0x11c06 + owner * 48, h);
      put(context.data(), 0x11c0e + owner * 48, nodes.size());
    }
  }
};
} // namespace
