#define main unused_motion_trace_fixture_main
#include "creature-execution-motion-test.cpp"
#undef main
#include "../reconstruction/animation/controller.hpp"
#include "encounter-trace.hpp"
int main(int argc, char **argv) try {
  using namespace mnm;
#ifdef MNM_EXECUTION_REFERENCE
  if (argc != 3)
    throw std::invalid_argument("expected PE and encounter trace");
  install(argv[1]);
  motionBody = Bytes(reinterpret_cast<unsigned char *>(0x5104b0),
                     reinterpret_cast<unsigned char *>(0x510e3a));
#else
  if (argc != 2)
    throw std::invalid_argument("expected encounter trace");
#endif
  encounter::test::Trace input(argv[argc - 1]);
  command_session::Continuation before, after;
  unsigned frames = 0, cases = 0, changedPositions = 0;
  std::uint64_t digest = 1469598103934665603ull;
  while (input.next(before, after)) {
    ++frames;
    const auto &a = before.world.creatures[0];
    const auto &end = after.world.creatures[0];
    if (!a.ai.motion || !end.ai.motion || a.ai.action != 2 ||
        end.ai.action != 2 || a.ai.behavior == 14 || end.ai.behavior == 14 ||
        !(a.ai.position == end.ai.position) ||
        after.actions.motion[0].progress >= 192)
      continue;
    auto initial = generateMotion(0, 0);
    initial.base.world = before.world;
    initial.base.execution = before.execution;
    initial.base.number = 0;
    initial.motion = before.actions.motion[0];
    initial.landing = before.movement[0];
    initial.animationEvents.clear();
    initial.pollCursor = 0;
    initial.base.execution[0].orders.resize(2);
    for (auto &o : initial.base.execution[0].orders) {
      o.kind = 99;
      o.effect.reset();
    }
    const auto &channel = before.native.animation[0].body;
    const auto &ani = input.profile.native.animation.at(channel.asset);
    std::vector<assets::AnimationRecord> records(
        ani.records.begin() + ani.starts.at(channel.sequence),
        ani.records.begin() + ani.starts.at(channel.sequence + 1));
    reconstruction::NoCdAnimationController player(records);
    player.restore(channel.state);
    if (before.actions.callbacks[0].animationBreak)
      player.requestBreak();
    auto expected = initial;
    moving = &expected;
    fixture = &expected.base;
    events.clear();
    auto services = motionServices();
    services.poll = [&](auto &actor, bool overlay) {
      if (overlay)
        throw std::invalid_argument("ordinary motion trace overlay");
      record(200, actor.ai.slot, 0);
      int event = player.tick();
      initial.animationEvents.push_back(event);
      return event;
    };
    services.rewind = [&](auto &actor) {
      record(201, actor.ai.slot);
      player.rewind();
    };
    tickMotionAction(expected.base.world.creatures[0],
                     expected.base.execution[0], expected.motion,
                     expected.landing, expected.base.world, services);
    auto trace = events;
    if (!(expected.base.world.creatures[0].finePosition == end.finePosition) ||
        expected.motion.accumulator != after.actions.motion[0].accumulator ||
        expected.motion.progress != after.actions.motion[0].progress ||
        expected.motion.sampleCursor != after.actions.motion[0].sampleCursor)
      throw std::runtime_error(
          "published fine motion differs from bounded body projection");
#ifdef MNM_EXECUTION_REFERENCE
    compareMotion(initial, expected, trace);
#else
    (void)trace;
#endif
    ++cases;
    changedPositions += !(a.finePosition == end.finePosition);
    digest = (digest ^ unsigned(end.finePosition.x)) * 1099511628211ull;
    digest = (digest ^ unsigned(end.finePosition.y)) * 1099511628211ull;
    digest = (digest ^ unsigned(end.finePosition.z)) * 1099511628211ull;
  }
  if (frames != 1024 || !cases || !changedPositions)
    throw std::runtime_error("motion trace coverage");
  std::cout << "{\"success\":true,\"frames\":" << frames
            << ",\"motion_cases\":" << cases
            << ",\"changed_positions\":" << changedPositions
            << ",\"digest\":" << digest << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
