#define main unused_actions_fixture_main
#include "creature-execution-actions-test.cpp"
#undef main
#include "../reconstruction/creature-ai/command_actions_owned.hpp"
namespace {
unsigned savedBehavior = 2;
CommandActionProviders ownedActionProviders() {
  CommandActionProviders p;
  p.selection = [](unsigned) { return actionServices(); };
  p.animation = [](unsigned) { return eventServices(); };
  p.remaining = [](auto &a, unsigned kind, auto &) { record(50, a.ai.slot, kind); };
  return p;
}
CommandActionEnvironment ownedActionEnvironment() {
  return {{fixture->number % 4, 64, 4 + fixture->number % 4, 32}, 100, 9, 7};
}
void executeOwned(Fixture &f, unsigned op, unsigned kind) {
  CommandBehaviorBindings bindings(f.world.creatures.size(),
                                  static_cast<CommandBehaviorBinding>(savedBehavior));
  OwnedCreatureCommands commands(f.world, f.execution, bindings,
      [](unsigned) { return executionServices(); },
      [](unsigned) { return BehaviorExecutionServices{}; },
      [](unsigned) { return FollowServices{}; },
      [](unsigned) { return ExecutionServices{}; },
      [](auto &a, unsigned binding) { executionServices().behavior(a, binding); },
      [](auto &) { throw std::runtime_error("unexpected supplied behavior"); },
      [](unsigned) {}, [](auto &, bool) {});
  commands.installActions([&f](auto &a, unsigned n, auto &context) {
    OwnedCommandActions actions(f.world, f.execution, context,
                               ownedActionEnvironment(), ownedActionProviders());
    actions.transition(a, n);
  }, [&f](auto &a, auto &context) {
    OwnedCommandActions actions(f.world, f.execution, context,
                               ownedActionEnvironment(), ownedActionProviders());
    actions.tick(a);
  });
  commands.action(f.world.creatures[0], kind);
  if (op) {
    // A word edit must not retarget either installed callback. Restore-form
    // also edits the behavior word during an original ranged completion.
    f.world.creatures[0].ai.action = 22;
    f.world.creatures[0].ai.behavior = 4;
    commands.tickAction(f.world.creatures[0]);
  }
}
#ifdef MNM_EXECUTION_REFERENCE
extern "C" void __attribute__((thiscall)) boundBehaviorStub(void *p) {
  record(6, slot(p), savedBehavior);
  write32(p, 0x66c, read32(p, 0x66c) + 1);
}
extern "C" void __attribute__((thiscall)) remainingActionStub(void *p) {
  record(50, slot(p), 1);
}
void compareOwnedActions(const Fixture &input, const Fixture &expected,
                         unsigned op, unsigned kind, const std::vector<Event> &trace) {
  auto source = input; fixture = &source; events.clear(); randomState = 1 + source.number;
  prepareExtra();
  for (unsigned i = 0; i < source.world.creatures.size(); ++i)
    write32(actors.data() + i * 0xe4b, 0x5f4, behaviorAddresses[savedBehavior - 1]);
  auto before = actors, configuration = configs, list = lists;
  auto overlays = overlayConfig, definitions = overlayDefinitions;
  using Select = void(__attribute__((thiscall)) *)(void *, unsigned);
  using Tick = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<Select>(0x50db60)(actors.data(), kind);
  if (op) {
    write32(actors.data(), 0x5fc, 22); write32(actors.data(), 0x5ec, 4);
    auto callback = read32(actors.data(), 0x600);
    if (read32(actors.data(), 0x604)) throw std::runtime_error("unexpected receiver adjustment");
    if (callback) reinterpret_cast<Tick>(callback)(actors.data());
  }
  if (events != trace) throw std::runtime_error("owned action trace n=" + std::to_string(input.number) + " op=" + std::to_string(op));
  for (unsigned i = 0; i < expected.world.creatures.size(); ++i) {
    auto *p = before.data() + i * 0xe4b;
    packActor(p, expected.world.creatures[i], expected.execution[i]);
    packExtra(p, expected.world.creatures[i], expected.execution[i]);
    auto handler = expected.execution[i].actionCallback;
    write32(p, 0x600, handler ? actionAddresses[*handler - 1] : 0); write32(p, 0x604, 0);
  }
  if (actors != before) {
    unsigned at = 0; while (actors[at] == before[at]) ++at;
    throw std::runtime_error("owned action writes n=" + std::to_string(input.number) + " op=" + std::to_string(op) + " offset=" + std::to_string(at % 0xe4b));
  }
  if (configs != configuration || lists != list || overlayConfig != overlays || overlayDefinitions != definitions)
    throw std::runtime_error("owned action configuration/list guard");
  for (unsigned entry : {0x50da50u, 0x50db60u, 0x50e670u, 0x50e6f0u, 0x50f7c0u}) checkBody(entry);
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_EXECUTION_REFERENCE
  if (argc != 2) throw std::runtime_error("expected pinned PE");
  install(argv[1]); installExtra(); restoreBody(0x50db60);
  for (auto address : behaviorAddresses) if (address) redirect(address, reinterpret_cast<void *>(boundBehaviorStub));
  redirect(0x50f090, reinterpret_cast<void *>(remainingActionStub));
  using Resolve = std::uint64_t(__attribute__((thiscall)) *)(void *, unsigned);
  for (unsigned n = 0; n < 40; ++n) {
    auto h = actionHandler(n);
    if (reinterpret_cast<Resolve>(0x50da50)(nullptr, n) != (h ? actionAddresses[*h - 1] : 0u))
      throw std::runtime_error("owned action resolver mapping");
  }
#else
  (void)argc; (void)argv;
#endif
  unsigned cases = 0, casts = 0, resumed = 0, nulls = 0, doubleCalls = 0;
  for (unsigned n = 0; n < 4096; ++n) for (unsigned op = 0; op < 7; ++op) {
    auto input = generate(n); savedBehavior = 2 + n % 3;
    auto &a = input.world.creatures[0]; auto &e = input.execution[0];
    a.ai.behavior = savedBehavior; a.ai.motion = op == 0 ? n % 7 == 0 : op == 4;
    a.animationKind = n % 4; a.ai.flying = n % 3 == 0; a.formCapability = n % 2;
    a.ai.corpseB = n % 6; a.form = n % 3 == 0; a.castSpell = n % 104;
    a.ai.readiness = n % 3; a.cooldown = int(n % 5) - 1;
    e.actionCallback = 15; e.animationFacing = n % 8; e.animationOffset = n % 300;
    e.moveVariant = n % 7; e.previousMove = n % 2; e.previousMoveVariant = n % 6;
    e.actionRouteCounters = {3,4,5}; e.block855 = n % 4 == 0;
    e.rangedDeadline = 95 + n % 10;
    unsigned kind = op == 0 ? n % 40 : op == 1 || op == 4 ? 8 : op == 2 ? 9 : op == 3 ? 15 : op == 5 ? 4 : 1;
    auto expected = input; fixture = &expected; events.clear(); randomState = 1 + n;
    executeOwned(expected, op, kind); auto trace = events;
    unsigned calls = 0;
    for (auto event : trace) {
      casts += event.kind == 39; calls += event.kind == 6;
      mix(event.kind); mix(event.slot); mix(event.argument); mix(unsigned(event.position.x));
    }
    resumed += calls; doubleCalls += calls == 2; nulls += !expected.execution[0].actionCallback;
    auto &v = expected.world.creatures[0]; auto &x = expected.execution[0];
    for (auto word : {v.ai.action, v.ai.behavior, v.ai.readiness, unsigned(v.cooldown), x.animationOffset, x.rangedDeadline, x.actionCallback.value_or(0), randomState}) mix(word);
#ifdef MNM_EXECUTION_REFERENCE
    compareOwnedActions(input, expected, op, kind, trace);
#endif
    ++cases;
  }
  std::cout << "{\"success\":true,\"cases\":" << cases << ",\"functions\":5,\"outcome_digest\":" << digest
            << ",\"casts\":" << casts << ",\"behavior_resumes\":" << resumed << ",\"double_behavior_calls\":" << doubleCalls << ",\"null_callbacks\":" << nulls << "}\n";
} catch (const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
