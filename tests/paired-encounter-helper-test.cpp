#include "../reconstruction/creature-ai/coarse_route.hpp"
#include "../reconstruction/creature-ai/idle_gesture.hpp"
#include "../reconstruction/creature-ai/motion_response.hpp"
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
unsigned number = 0;
std::vector<m::Cell> orientations;
void check(bool ok, const char *why) {
  if (!ok)
    throw std::runtime_error(std::string(why) +
                             " case=" + std::to_string(number));
}
bool sight(m::Cell p) {
  return (unsigned(p.x * 3 + p.y * 5 + p.z * 7) + number) % 5 != 0;
}
bool reach(unsigned slot) { return (number + slot) % 3 != 0; }
bool terrainMove(m::Cell from, m::Cell to, unsigned &category) {
  category = (number + unsigned(to.z)) % 5;
  return (number + unsigned(from.x + to.x + 3 * to.y + 7 * to.z)) % 19 != 0;
}
bool hazard(m::Cell p) {
  return (number + unsigned(p.x + 3 * p.y + 7 * p.z)) % 13 == 0;
}
#ifdef MNM_PAIRED_REFERENCE
extern "C" unsigned
    __attribute__((thiscall)) sightEndpoint(void *, int x, int y, int z) {
  return sight({x, y, z});
}
extern "C" void __attribute__((thiscall)) orientEndpoint(void *, int x, int y) {
  orientations.push_back({x, y, 0});
}
extern "C" unsigned __attribute__((thiscall)) reachEndpoint(void *, void *b,
                                                            int, int, int) {
  return reach(get(b, 0));
}
extern "C" unsigned __attribute__((thiscall))
moveEndpoint(void *, int x, int y, int z, int tx, int ty, int tz, void *,
             unsigned *category, int override) {
  check(override == -1, "coarse movement override");
  return terrainMove({x, y, z}, {tx, ty, tz}, *category);
}
extern "C" unsigned __attribute__((thiscall)) hazardEndpoint(void *, int x,
                                                             int y, int z) {
  return hazard({x, y, z});
}
void compareActors(const Original &o, const command_session::Continuation &c) {
  for (unsigned i = 0; i < 2; ++i)
    for (const auto &[at, f] : encounter::paired::fields(c, i))
      check(get(o.actors.data() + i * 0xe4b, at, f.size) == f.value, f.name);
}
#endif
} // namespace
int main(int argc, char **argv) try {
  using namespace mnm;
#ifdef MNM_PAIRED_REFERENCE
  check(argc == 3, "expected PE and installation");
  auto pe = read(argv[1]);
  map_image(pe);
  endpoints();
  for (auto e : std::vector<std::pair<unsigned, std::uintptr_t>>{
           {0x5151b0, reinterpret_cast<std::uintptr_t>(&sightEndpoint)},
           {0x5084f0, reinterpret_cast<std::uintptr_t>(&orientEndpoint)},
           {0x50bd60, reinterpret_cast<std::uintptr_t>(&reachEndpoint)},
           {0x4f3990, reinterpret_cast<std::uintptr_t>(&moveEndpoint)},
           {0x522f50, reinterpret_cast<std::uintptr_t>(&hazardEndpoint)}})
    redirect(e.first, e.second);
  sealText(pe);
#else
  check(argc == 2, "expected installation");
#endif
  auto profile = encounter::decodeInputs(
      encounter::loadInputs(argv[argc - 1], {0, 0, 12, 12}));
  unsigned gestures = 0, responses = 0, cached = 0, coarse = 0;
  std::uint64_t digest = 1469598103934665603ull;
  auto mix = [&](unsigned n) { digest = (digest ^ n) * 1099511628211ull; };
  for (number = 0; number < 16384; ++number) {
    auto c = profile.initial;
    auto &a = c.world.creatures[0];
    auto &b = c.world.creatures[1];
    a.ai.type = number % 28;
    a.mode = number % 17 == 0;
    a.form = number % 19 == 0;
    a.formCapability = number % 3 == 0;
    a.blockDE3 = number % 23 == 0;
    a.block71c = number % 29 == 0;
    a.block723 = number % 31 == 0;
    a.block784 = number % 37 == 0;
    a.specialVision = number % 41 == 0;
    a.ai.engaged = number % 3 == 0;
    a.ai.incoming = number % 5 == 0;
    a.ai.readiness = number % 7 == 0;
    a.targetMovingAttacker = number % 3 ? 2 : 0;
    a.alternateTarget = 1;
    a.ai.position = {int(number % 12), int(number / 12 % 12),
                     int(number / 144 % 23)};
    b.ai.position = {int(number / 7 % 12), int(number / 17 % 12),
                     int(number / 197 % 23)};
    b.ai.active = number % 11 != 0;
    b.ai.health = number % 13 ? 360 : 0;
    b.ai.motion = number % 2;
    b.ai.action = number % 7 ? 1 : 14;
    b.finePosition = {b.ai.position.x * 32, b.ai.position.y * 32,
                      b.ai.position.z * 16};
    b.aimXY = 16;
    b.aimZ = 12;
    b.ai.history = 4;
    c.world.relation[0][1] = number % 9 == 0;
    b.ai.visible[0] = number % 8 != 0;
    std::array<int, 28> scores;
    for (unsigned i = 0; i < 28; ++i)
      scores[i] = int((number + i) % 20) - 2;
    m::AiGeometry geometry;
    geometry.lineOfSight = [](const auto &, auto p) { return sight(p); };
    geometry.reach = [](const auto &, const auto &target, auto) {
      return reach(target.ai.slot);
    };
    auto random = m::seedRandom(number + 1);
    auto expected = c;
    orientations.clear();
    bool result = m::idleGesture(
        expected.world.creatures[0], expected.world, geometry, scores,
        [&] { return m::nextRandom(random); },
        [](auto &, auto p) { orientations.push_back(p); });
    auto expectedOrient = orientations;
    gestures += result;
#ifdef MNM_PAIRED_REFERENCE
    Original o(profile);
    setup(o, c, number + 1);
    put(o.config.data(), 0x3c, a.formCapability);
    put(o.config.data(), 0x198, b.aimXY);
    put(o.config.data(), 0x19c, b.aimZ);
    for (unsigned i = 0; i < 28; ++i)
      put(o.config.data(), 0x2f0 + i * 4, unsigned(scores[i]));
    orientations.clear();
    using Query = unsigned(__attribute__((thiscall)) *)(void *);
    check(bool(reinterpret_cast<Query>(0x516800)(o.actors.data())) == result,
          "gesture result");
    check(orientations == expectedOrient, "gesture orientation");
    compareActors(o, expected);
    check(get(reinterpret_cast<void *>(0x6e1ac8)) == random.left &&
              get(reinterpret_cast<void *>(0x6e1acc)) == random.right &&
              !std::memcmp(reinterpret_cast<void *>(0x6e1ad0),
                           random.words.data(), 1000),
          "gesture RNG");
#endif
    mix(result);
    mix(random.left);
    mix(expected.world.creatures[1].ai.history);
    expected = c;
    result = m::motionOpponentResponse(expected.world.creatures[0],
                                       expected.world, geometry);
    responses += result;
#ifdef MNM_PAIRED_REFERENCE
    setup(o, c, number + 1);
    check(bool(reinterpret_cast<Query>(0x517580)(o.actors.data())) == result,
          "motion response result");
    compareActors(o, expected);
#endif
    mix(result);
    mix(expected.world.creatures[0].targetMovingAttacker);
    mix(expected.world.creatures[1].targetMovingAttacker);
    expected = c;
    result = m::cachedMotionOpponent(expected.world.creatures[0],
                                     expected.world, geometry);
    cached += result;
#ifdef MNM_PAIRED_REFERENCE
    setup(o, c, number + 1);
    check(bool(reinterpret_cast<Query>(0x516ee0)(o.actors.data())) == result,
          "cached response result");
    compareActors(o, expected);
#endif
    mix(result);
    mix(expected.world.creatures[0].targetMovingAttacker);
    a.hazardSensitive = number % 3 == 0;
    a.protection8c9 = number % 7 == 0;
    auto target = b.ai.position;
    m::ActorRouteSnapshot route;
    result = m::coarseCreatureRoute(a, number % 8, c.world.dimensions, target,
                                    route, terrainMove, hazard);
    coarse += result;
#ifdef MNM_PAIRED_REFERENCE
    setup(o, c, number + 1);
    put(o.actors.data(), 0x608, number % 8);
    global(0x6a652d + a.ai.type * 0x5c9, a.hazardSensitive);
    Bytes raw(131 * 4);
    using Coarse =
        unsigned(__attribute__((thiscall)) *)(void *, int, int, int, void *);
    check(bool(reinterpret_cast<Coarse>(0x513370)(o.actors.data(), target.x,
                                                  target.y, target.z,
                                                  raw.data())) == result,
          "coarse result");
    check(get(raw.data(), 12) == route.cursor &&
              get(raw.data(), 16) == route.count,
          "coarse route extent");
    for (unsigned i = 0; i < route.count; ++i) {
      const auto &p = route.points[i];
      check(int(get(raw.data(), 20 + 28 * i)) == p.position.x &&
                int(get(raw.data(), 24 + 28 * i)) == p.position.y &&
                int(get(raw.data(), 28 + 28 * i)) == p.position.z,
            "coarse route position");
      for (unsigned j = 0; j < 4; ++j)
        check(int(get(raw.data(), 32 + 28 * i + 4 * j)) == p.remainder[j],
              "coarse route metadata");
    }
#endif
    mix(result);
    mix(route.count);
    for (unsigned i = 0; i < route.count; ++i) {
      const auto &p = route.points[i];
      mix(unsigned(p.position.x));
      mix(unsigned(p.position.y));
      mix(unsigned(p.position.z));
      for (int v : p.remainder)
        mix(unsigned(v));
    }
  }
  check(gestures && responses && cached && coarse, "helper branch coverage");

#ifdef MNM_PAIRED_REFERENCE
  checkText();
#endif
  std::cout << "{\"success\":true,\"cases\":65536,\"gestures\":" << gestures
            << ",\"responses\":" << responses << ",\"cached\":" << cached
            << ",\"coarse\":" << coarse << ",\"digest\":" << digest << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
