#define main unused_control_trace_fixture_main
#include "creature-control-test.cpp"
#undef main
#include "../apps/command-session/native_geometry.hpp"
#include "../reconstruction/combat/autonomy_tables.hpp"
#include "encounter-trace.hpp"
namespace {
namespace cs = mnm::command_session;
#ifdef MNM_CONTROL_REFERENCE
cs::NativeCreatureGeometry *geometry = nullptr;
const AutonomousCreature *subject = nullptr;
int endpointHeight = 0;
extern "C" unsigned __attribute__((thiscall))
traceLine(void *, int x, int y, int z, int tx, int ty, int tz, void *) {
  return geometry->line(*subject, {x, y, z}, {tx, ty, tz});
}
extern "C" int __attribute__((thiscall)) traceHeight(void *, void *) {
  return endpointHeight;
}
#endif
} // namespace
int main(int argc, char **argv) try {
  using namespace mnm;
#ifdef MNM_CONTROL_REFERENCE
  check(argc == 3, "expected PE and encounter trace");
  initialize(argv[1]);
  redirect(0x4f4ba0, reinterpret_cast<std::uintptr_t>(&traceLine));
  redirect(0x4f3260, reinterpret_cast<std::uintptr_t>(&traceHeight));
#else
  check(argc == 2, "expected encounter trace");
#endif
  encounter::test::Trace input(argv[argc - 1]);
  cs::Continuation before, after;
  unsigned frames = 0, reachSamples = 0, directionSamples = 0;
  std::uint64_t digest = 1469598103934665603ull;
  while (input.next(before, after)) {
    ++frames;
    cs::NativeCreatureGeometry g(before, input.profile.native);
    AiWorld world;
    world.dimensions = before.world.dimensions;
    world.relation = before.world.relation;
    world.human = before.world.human;
    for (const auto &a : before.world.creatures)
      world.creatures.push_back(a.ai);
    AiGeometry adm;
    adm.routeOffsets = originalRouteOffsets;
    adm.reach = [&](const auto &a, const auto &b, Cell point) {
      return g.reach(a, b, point);
    };
    adm.legal = [&](const auto &a, Cell x, Cell y, unsigned) {
      unsigned category = 5;
      return g.move(a, x, y, category);
    };
    unsigned mask = 0;
    for (unsigned i = 0; i < 2; ++i)
      if (targetAdmitted(before.world.creatures[0], before.world.creatures[i],
                         world.dimensions, adm))
        mask |= 1u << i;
    selection(world, Selector::score, mask);
    selection(world, Selector::home, mask);
    const auto &a = before.world.creatures[0];
    const auto &b = before.world.creatures[1];
    const auto facing = before.actions.motion[1].direction;
    const auto directionValue = relativeDirection(a.ai.position, b.ai.position,
                                                  facing, world.dimensions);
    auto project = [&](const auto &actor) {
      ReachActor v{actor.ai.position,
                   actor.finePosition.z,
                   int(actor.width),
                   int(actor.height),
                   {},
                   0};
      if (actor.ai.motion) {
        const auto &route = before.actions.segments.at(actor.ai.slot);
        check(route.cursor < route.count, "trace route endpoint");
        v.endpoint = route.route[route.cursor].position;
        v.endpointTerrainHeight = g.height(*v.endpoint, actor.width);
      }
      return v;
    };
    auto source = project(a), target = project(b);
    auto result = meleeReach(source, target, world.dimensions,
                             [&](auto x, auto y) { return g.line(a, x, y); });
    check(result.admitted == g.reach(a, b, a.ai.position),
          "canonical trace reach projection differs");
#ifdef MNM_CONTROL_REFERENCE
    Bytes rawSource(0xe4b), rawTarget(0xe4b), sourceCfg(0x5c9),
        targetCfg(0x5c9);
    reachObject(rawSource, sourceCfg, source);
    reachObject(rawTarget, targetCfg, target);
    put(rawTarget, 0x608, facing);
    auto sourceBefore = rawSource, targetBefore = rawTarget;
    global(0x6c5494, world.dimensions.x);
    global(0x6c5498, world.dimensions.y);
    global(0x6c549c, world.dimensions.z);
    Bytes cells(std::size_t(world.dimensions.x) * world.dimensions.y *
                world.dimensions.z * 12);
    global(0x6c54dc,
           std::uint32_t(reinterpret_cast<std::uintptr_t>(cells.data())));
    for (int y = 0; y < world.dimensions.y; ++y)
      global(0x6cb942 + 4 * y, y * world.dimensions.x);
    for (int z = 0; z < world.dimensions.z; ++z)
      global(0x6cb8c2 + 4 * z, z * world.dimensions.x * world.dimensions.y);
    geometry = &g;
    subject = &a;
    endpointHeight = target.endpointTerrainHeight;
    using Direction = int(__attribute__((thiscall)) *)(void *, void *);
    check(reinterpret_cast<Direction>(0x50c930)(
              rawSource.data(), rawTarget.data()) == directionValue,
          "encounter original direction differs");
    using Reach =
        int(__attribute__((thiscall)) *)(void *, void *, int, int, int);
    check(bool(reinterpret_cast<Reach>(0x50bd60)(
              rawSource.data(), rawTarget.data(), source.position.x,
              source.position.y, source.position.z)) == result.admitted,
          "encounter original reach differs");
    check(rawSource == sourceBefore && rawTarget == targetBefore,
          "original trace reach/direction input guard");
    guards();
#endif
    ++reachSamples;
    ++directionSamples;
    digest = (digest ^ unsigned(directionValue)) * 1099511628211ull;
    digest = (digest ^ unsigned(result.admitted)) * 1099511628211ull;
  }
  check(frames == 1024 && selections == 2048 && reachSamples == 1024,
        "control trace coverage");
  std::cout << "{\"success\":true,\"frames\":" << frames
            << ",\"selector_cases\":" << selections
            << ",\"reach_cases\":" << reachSamples
            << ",\"direction_cases\":" << directionSamples
            << ",\"digest\":" << digest << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
