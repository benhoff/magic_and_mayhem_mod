#include "../apps/command-session/native_geometry.hpp"
#include "../reconstruction/creature-ai/cached_attack.hpp"
#include "../reconstruction/creature-ai/friendly_displacement.hpp"
#ifdef MNM_PAIRED_REFERENCE
#include "paired-encounter-original.hpp"
extern "C" void __attribute__((thiscall)) displacementAction(void *a,
                                                             unsigned kind) {
  put(a, 0x5fc, kind);
}
extern "C" void __attribute__((thiscall)) displacementOrder(void *a, int x,
                                                            int y, int z) {
  put(a, 0x96b, unsigned(x));
  put(a, 0x96f, unsigned(y));
  put(a, 0x973, unsigned(z));
  put(a, 0x5ec, 13);
}
#else
#include "paired-encounter-state.hpp"
#include <iostream>
#endif
int main(int argc, char **argv) try {
  using namespace mnm;
  namespace m = command_session::model;
#ifdef MNM_PAIRED_REFERENCE
  if (argc != 3)
    throw std::runtime_error("PE INSTALL");
  auto pe = read(argv[1]);
  map_image(pe);
  endpoints();
  redirect(0x50db60, reinterpret_cast<std::uintptr_t>(&displacementAction));
  redirect(0x51c050, reinterpret_cast<std::uintptr_t>(&displacementOrder));
  sealText(pe);
  initializeSearch();
  const auto install = argv[2];
#else
  if (argc != 2)
    throw std::runtime_error("INSTALL");
  const auto install = argv[1];
#endif
  auto profile = encounter::decodeInputs(
      encounter::loadInputs(install, {0, 0, 12, 12}, 4));
  auto c = profile.initial;
  command_session::NativeCreatureGeometry geometry(c, profile.native);
#ifdef MNM_PAIRED_REFERENCE
  Original o(profile);
  setup(o, c, 17);
#endif
  m::FriendlyDisplacementServices s;
  s.move = [&](const auto &a, auto from, auto to, bool ignore) {
    int category = 5;
    return reconstruction::accept_creature_move(
        a.ai.slot + 1, {from.x, from.y, from.z}, {to.x, to.y, to.z}, ignore,
        category, geometry.acceptance());
  };
  s.flags = [&](auto p) {
    auto d = c.world.dimensions;
    return unsigned(c.native.cells.at((p.z * d.y + p.y) * d.x + p.x).flags);
  };
  s.obstacle = [&](const auto &a, auto p) {
    return m::footprintObstacle(a, p, c.world.dimensions, [&](auto q) {
      auto d = c.world.dimensions;
      auto v = c.native.cells.at((q.z * d.y + q.y) * d.x + q.x);
      return m::ObstacleCell{v.flags,
                             v.occupant == 0xffff ? 0u : unsigned(v.occupant)};
    });
  };
  unsigned cases = 0;
  std::uint64_t digest = 1469598103934665603ull;
  auto record = [&](unsigned expected, unsigned actual, const char *what,
                    unsigned n) {
    if (expected != actual)
      throw std::runtime_error(std::string(what) +
                               " case=" + std::to_string(n) +
                               " native=" + std::to_string(expected) +
                               " original=" + std::to_string(actual));
    digest = (digest ^ expected) * 1099511628211ull;
    ++cases;
  };
  for (unsigned n = 0; n < 16384; ++n) {
    auto &a = c.world.creatures[0];
    auto &b = c.world.creatures[1];
    auto &e = c.execution[1];
    a.ai.owner = int(n % 8);
    b.ai.owner = int((n / 8) % 8);
    b.ai.type = n % 11 == 0 ? 22 : 17;
    b.ai.health = n % 13 ? 360 : 0;
    b.ai.behavior = (n / 64) % 17;
    b.ai.action = n % 3 ? 15 : 14;
    e.marker6a0 = n % 5 == 0 ? 2 : 0;
    b.routePhase = (n / 7) % 3;
    b.mode = n % 19 == 0 ? 1 : 0;
    b.form = n % 23 == 0 ? 1 : 0;
    b.ai.motion = n % 17 == 0;
    auto permission = m::canDisplaceCreature(a, b, e, c.world);
    auto moving = m::displacementMoving(b, e);
    auto next = m::Cell{int((n * 7) % 12), int((n / 12) % 12), int(2 + n % 4)};
    auto candidate = m::Cell{int((n / 7) % 12), int((n / 144) % 12), 3};
    auto overlap = m::displacementSweptOverlap(a, next, candidate, 1 + n % 2,
                                               c.world.dimensions);
#ifdef MNM_PAIRED_REFERENCE
    for (unsigned actor : {0u, 1u}) {
      auto raw = o.actors.data() + actor * 0xe4b;
      for (const auto &[offset, f] : encounter::paired::fields(c, actor))
        put(raw, offset, f.value, f.size);
    }
    put(o.config.data(), 8, 2);
    put(o.config.data(), 12, 2);
    auto raw = o.actors.data();
    using Query = unsigned(__attribute__((thiscall)) *)(void *, unsigned);
    using Overlap =
        unsigned(__attribute__((thiscall)) *)(void *, int, int, unsigned);
    put(raw, 0x97f, next.x);
    put(raw, 0x983, next.y);
    put(raw, 0x987, next.z);
    put(raw, 0x977, 0);
    record(permission, reinterpret_cast<Query>(0x51b710)(raw, 1), "permission",
           n);
    record(moving, reinterpret_cast<Query>(0x512390)(raw + 0xe4b, unsigned(-1)),
           "moving", n);
    record(overlap,
           reinterpret_cast<Overlap>(0x51bcb0)(raw, candidate.x, candidate.y,
                                               1 + n % 2),
           "swept", n);
#else
    record(permission, permission, "permission", n);
    record(moving, moving, "moving", n);
    record(overlap, overlap, "swept", n);
#endif
    // Static occupied map; legal next cells include seams, diagonals, blocked
    // geometry and different elevations. Width1 adds intermediate probes.
    a.width = 1 + n % 2;
    a.height = 1 + n % 2;
    profile.native.types[17].width = int(a.width);
    profile.native.types[17].height = int(a.height);
    auto blocker = m::displacementBlocker(a, next, s);
#ifdef MNM_PAIRED_REFERENCE
    put(o.config.data(), 8, a.width);
    put(o.config.data(), 12, a.height);
    using Blocker = unsigned(__attribute__((thiscall)) *)(void *);
    record(blocker, reinterpret_cast<Blocker>(0x51ae60)(raw), "blocker", n);
#else
    record(blocker, blocker, "blocker", n);
#endif
    for (unsigned peer = 0; peer < c.execution.size(); ++peer) {
      c.execution[peer].marker6a0 =
          n % 3 == 0 ? (peer + n) % c.execution.size() + 1 : 0;
    }
#ifdef MNM_PAIRED_REFERENCE
    for (unsigned peer = 0; peer < c.execution.size(); ++peer)
      put(o.actors.data() + peer * 0xe4b, 0x6a0, c.execution[peer].marker6a0);
    const auto waited = reinterpret_cast<Blocker>(0x511c40)(raw);
#endif
    const auto wait = m::displacementWait(
        a, c.execution[0], next, c.world, c.execution,
        [&](auto q) {
          auto d = c.world.dimensions;
          auto v = c.native.cells.at((q.z * d.y + q.y) * d.x + q.x);
          return m::ObstacleCell{
              v.flags, v.occupant == 0xffff ? 0u : unsigned(v.occupant)};
        },
        s);
#ifdef MNM_PAIRED_REFERENCE
    record(wait, waited, "wait", n);
    record(c.execution[0].marker6a0, get(raw, 0x6a0), "wait chain", n);
#else
    record(wait, wait, "wait", n);
    record(c.execution[0].marker6a0, c.execution[0].marker6a0, "wait chain", n);
#endif
    a.width = a.height = 2;
    profile.native.types[17].width = profile.native.types[17].height = 2;
  }
  for (unsigned n = 0; n < 4096; ++n) {
    c = profile.initial;
    auto &a = c.world.creatures[0];
    auto &b = c.world.creatures[1];
    a.ai.behavior = n % 7 == 0 ? 13 : 2;
    c.execution[0].displacementActive = n % 3 ? 6000000 : 0;
    b.ai.behavior = n % 3 == 0 ? 13 : 1;
    b.ai.motion = n % 5 == 0;
    b.routePhase = n % 7 == 0 ? 1 : 0;
    c.execution[1].displacementActive = n % 2 ? 7000000 : 0;
    a.destination = {int(n % 12), int((n / 12) % 12), 3};
    c.actions.segments[0].count = n % 4 + 1;
    for (unsigned i = 0; i < c.actions.segments[0].count; ++i)
      c.actions.segments[0].route[i].position = {int((n + i) % 12),
                                                 int((n / 12 + i) % 12), 3};
    if (n % 11 == 0)
      for (auto &cell : c.native.cells)
        cell.flags |= 0x4000;
    s.order = [](auto &b, auto p) {
      b.destination = p;
      b.ai.behavior = 13;
    };
    s.action = [](auto &a, unsigned kind) { a.ai.action = kind; };
    s.hazard = [&](const auto &actor, auto p) {
      auto d = c.world.dimensions;
      for (unsigned y = 0; y < actor.width; ++y)
        for (unsigned x = 0; x < actor.width; ++x)
          for (int z = p.z; z >= p.z - 1; --z)
            if (z >= 0 && z < d.z) {
              m::Cell q{(p.x + int(x)) % d.x, (p.y + int(y)) % d.y, z};
              if (s.flags(q) & 0x10)
                return true;
            }
      return false;
    };
#ifdef MNM_PAIRED_REFERENCE
    setup(o, c, 17);
    using Issue = unsigned(__attribute__((thiscall)) *)(void *, unsigned);
    const auto actual = reinterpret_cast<Issue>(0x51b7f0)(o.actors.data(), 1);
#endif
    const auto expected = m::issueFriendlyDisplacement(
        a, b, c.world, c.execution, c.actions.segments, c.actions.motion,
        c.native.displacementOwners, s);
#ifdef MNM_PAIRED_REFERENCE
    record(expected, actual, "issue", n);
    for (unsigned i : {0u, 1u})
      for (const auto &[at, f] : encounter::paired::fields(c, i))
        if (get(o.actors.data() + i * 0xe4b, at, f.size) != f.value)
          throw std::runtime_error(
              std::string("issue ") + f.name + " case=" + std::to_string(n) +
              " native=" + std::to_string(f.value) + " original=" +
              std::to_string(get(o.actors.data() + i * 0xe4b, at, f.size)));
#else
    record(expected, expected, "issue", n);
#endif
    for (unsigned i : {0u, 1u})
      for (const auto &[at, f] : encounter::paired::fields(c, i)) {
        (void)at;
        digest = (digest ^ f.value) * 1099511628211ull;
      }
  }
#ifdef MNM_PAIRED_REFERENCE
  checkText();
  destroySearch();
#endif
  std::cout << "{\"success\":true,\"cases\":" << cases
            << ",\"digest\":" << digest << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
