#include "../reconstruction/creature-ai/cached_attack.hpp"
#ifdef MNM_PAIRED_REFERENCE
#include "paired-encounter-original.hpp"
#else
#include "paired-encounter-state.hpp"
#include <iostream>
#include <stdexcept>
#endif
namespace {
using namespace mnm;
namespace m = command_session::model;
unsigned actions = 0, faces = 0;
void check(bool ok, const char *why) {
  if (!ok)
    throw std::runtime_error(why);
}
#ifdef MNM_PAIRED_REFERENCE
bool admitReach = false;
extern "C" unsigned __attribute__((thiscall)) cachedReach(void *, void *, int,
                                                          int, int) {
  return admitReach;
}
extern "C" void __attribute__((thiscall)) cachedAction(void *a, unsigned n) {
  ++actions;
  put(a, 0x5fc, n);
}
extern "C" void __attribute__((thiscall)) cachedFace(void *, void *) {
  ++faces;
}
#endif
} // namespace
int main(int argc, char **argv) try {
  using namespace mnm;
#ifdef MNM_PAIRED_REFERENCE
  check(argc == 3 || argc == 4, "PE and installation");
  bool facingOnly = argc == 4;
  auto pe = read(argv[1]);
  map_image(pe);
  endpoints();
  if (!facingOnly) {
    redirect(0x50bd60, reinterpret_cast<std::uintptr_t>(&cachedReach));
    redirect(0x50db60, reinterpret_cast<std::uintptr_t>(&cachedAction));
    redirect(0x508810, reinterpret_cast<std::uintptr_t>(&cachedFace));
  }
  sealText(pe);
  const char *install = argv[2];
#else
  check(argc == 2 || argc == 3, "installation");
  bool facingOnly = argc == 3;
  const char *install = argv[1];
#endif
  auto profile =
      encounter::decodeInputs(encounter::loadInputs(install, {0, 0, 12, 12}));
  std::uint64_t digest = 1469598103934665603ull;
  auto mix = [&](unsigned v) {
    for (unsigned b = 0; b < 4; ++b)
      digest = (digest ^ ((v >> (8 * b)) & 255)) * 1099511628211ull;
  };
  if (facingOnly) {
    unsigned admitted = 0;
    for (unsigned n = 0; n < 32768; ++n) {
      auto c = profile.initial;
      auto &a = c.world.creatures[0];
      auto &b = c.world.creatures[1];
      a.width = 1 + n % 2;
      b.width = 1 + n / 2 % 2;
      a.ai.position = {int(n / 4 % 12), int(n / 48 % 12), 3};
      b.ai.position = {int(n / 576 % 12), int(n / 6912 % 12), 3};
      c.actions.motion[0].direction = n % 8;
      c.execution[0].animationFacing = n / 8 % 8;
      c.execution[0].block722 = n / 64 % 2;
      const unsigned camera = n / 128 % 4;
      auto direction = m::opponentFacing(a, b, c.world.dimensions);
      auto expected = c;
      if (direction) {
        expected.actions.motion[0].direction = *direction;
        expected.execution[0].animationFacing =
            (*direction - 2 * camera - (c.execution[0].block722 ? 4 : 0)) & 7;
        ++admitted;
      }
#ifdef MNM_PAIRED_REFERENCE
      Original original(profile);
      setup(original, c, n + 1);
      put(original.config.data(), 8, a.width);
      auto peerConfig = original.config;
      put(peerConfig.data(), 8, b.width);
      put(original.actors.data() + 0xe4b, 0xac, address(peerConfig.data()));
      global(0x6e9850, camera);
      using Face = unsigned(__attribute__((thiscall)) *)(void *, void *);
      check(bool(reinterpret_cast<Face>(0x508810)(
                original.actors.data(), original.actors.data() + 0xe4b)) ==
                direction.has_value(),
            "footprint facing result");
      for (unsigned i = 0; i < 2; ++i)
        for (const auto &[at, f] : encounter::paired::fields(expected, i))
          check(get(original.actors.data() + i * 0xe4b, at, f.size) == f.value,
                "footprint facing actor fields");
#endif
      mix(direction.has_value());
      mix(expected.actions.motion[0].direction);
      mix(expected.execution[0].animationFacing);
    }
#ifdef MNM_PAIRED_REFERENCE
    checkText();
#endif
    check(admitted > 0, "facing admission coverage");
    std::cout << "{\"success\":true,\"cases\":32768,\"admitted\":" << admitted
              << ",\"digest\":" << digest << "}\n";
    return 0;
  }
  unsigned attacks = 0, pauses = 0, cleared = 0, responses = 0;
  for (unsigned n = 0; n < 32768; ++n) {
    auto c = profile.initial;
    auto &a = c.world.creatures[0];
    auto &b = c.world.creatures[1];
    a.width = 1 + n % 2;
    a.height = 1 + n / 2 % 4;
    a.ai.position = {int(n % 12), int(n / 12 % 12), int(n / 144 % 23)};
    a.destination = {int(n % 31) - 10, int(n / 31 % 31) - 10,
                     int(n / 961 % 23)};
    a.targetMovingAttacker = n % 7 ? 2 : 0;
    a.alternateTarget = 1;
    a.ai.incoming = n / 3 % 2;
    a.ai.enemyRadius[17] = int(n / 5 % 3) - 1;
    b.ai.active = n % 11 != 0;
    b.ai.health = n % 13 ? 360 : 0;
    b.ai.action = n % 17 ? 15 : 14;
    b.ai.motion = n / 7 % 2;
    c.world.relation[0][1] = n % 19 == 0;
    for (unsigned i = 0; i < c.native.cells.size(); ++i) {
      auto &v = c.native.cells[i];
      v.flags = std::uint16_t((i * 17 + n) % 8);
      auto token = (i * 13 + n / 3) % 7;
      v.occupant = token < 2 ? std::uint16_t(token + 1) : 0xffff;
    }
    auto cell = [&](m::Cell q) {
      const auto &v = c.native.cells[(q.z * 12 + q.y) * 12 + q.x];
      return m::ObstacleCell{v.flags,
                             v.occupant == 0xffff ? 0u : unsigned(v.occupant)};
    };
    auto expected = c;
    actions = faces = 0;
    auto obstacle =
        m::footprintObstacle(a, a.destination, c.world.dimensions, cell);
    const bool reach = n % 5 != 0;
    m::CachedAttackServices services;
    services.reach = [&](const auto &, const auto &, auto) { return reach; };
    services.obstacle = [&](const auto &actor, auto q) {
      return m::footprintObstacle(actor, q, c.world.dimensions, cell);
    };
    services.action = [&](auto &actor, unsigned kind) {
      ++actions;
      actor.ai.action = kind;
    };
    services.face = [](auto &, const auto &) { ++faces; };
    bool result = m::attackCachedOpponent(expected.world.creatures[0],
                                          expected.maintenance[0],
                                          expected.world, services);
    const auto actionCalls = actions, faceCalls = faces;
    const auto &out = expected.world.creatures[0];
    attacks += result && out.ai.action == 20;
    pauses += result && out.ai.action == 15;
    cleared += !out.targetMovingAttacker;
    responses += expected.maintenance[0].recent6b0 == 30;
#ifdef MNM_PAIRED_REFERENCE
    Original original(profile);
    setup(original, c, n + 1);
    put(original.config.data(), 8, a.width);
    put(original.config.data(), 12, a.height);
    admitReach = reach;
    actions = faces = 0;
    using Query = unsigned(__attribute__((thiscall)) *)(void *);
    using Obstacle =
        unsigned(__attribute__((thiscall)) *)(void *, int, int, int);
    check(reinterpret_cast<Obstacle>(0x51b130)(
              original.actors.data(), (a.destination.x % 12 + 12) % 12,
              (a.destination.y % 12 + 12) % 12, a.destination.z) == obstacle,
          "footprint obstacle differs");
    check(bool(reinterpret_cast<Query>(0x516d60)(original.actors.data())) ==
              result,
          "cached attack result differs");
    check(actions == actionCalls && faces == faceCalls,
          "cached attack ordered consequences differ");
    for (unsigned i = 0; i < 2; ++i)
      for (const auto &[at, f] : encounter::paired::fields(expected, i))
        check(get(original.actors.data() + i * 0xe4b, at, f.size) == f.value,
              "cached attack actor projection differs");
#endif
    mix(obstacle);
    mix(result);
    mix(actionCalls);
    mix(faceCalls);
    for (const auto &[at, f] : encounter::paired::fields(expected, 0)) {
      mix(at);
      mix(f.value);
    }
  }
#ifdef MNM_PAIRED_REFERENCE
  checkText();
#endif
  check(attacks && pauses && cleared && responses,
        "cached attack coverage absent");
  std::cout << "{\"success\":true,\"cases\":65536,\"attacks\":" << attacks
            << ",\"pauses\":" << pauses << ",\"cleared\":" << cleared
            << ",\"responses\":" << responses << ",\"digest\":" << digest
            << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
