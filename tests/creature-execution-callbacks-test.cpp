#define main unused_execution_fixture_main
#include "creature-execution-test.cpp"
#undef main
#include "../reconstruction/creature-ai/action_callbacks.hpp"
namespace {
ActionCallbackState callbackState;
constexpr unsigned callbackEntries[] = {0x50f090, 0x50f360, 0x50e7a0,
                                        0x5170e0, 0x50eba0, 0x50f590};
int eventCode(bool overlay) {
  constexpr int codes[] = {0, 1, 2, 3, 4, 6, 100};
  return codes[(fixture->number / 3 + (overlay ? 2 : 0)) % 7];
}
ActionCallbackServices callbacks() {
  ActionCallbackServices s;
  s.poll = [](auto &a, bool overlay) {
    record(150, a.ai.slot, overlay);
    return eventCode(overlay);
  };
  s.immediate = [](auto &a) {
    record(151, a.ai.slot);
    if (fixture->number % 3) {
      a.ai.target = int((a.ai.slot + 1) % fixture->world.creatures.size());
      return true;
    }
    return false;
  };
  s.rangedOpponent = [](auto &a) {
    record(152, a.ai.slot);
    if (fixture->number % 5) {
      a.ai.target = int((a.ai.slot + 2) % fixture->world.creatures.size());
      return true;
    }
    return false;
  };
  s.behavior = [](auto &a, unsigned n) {
    record(153, a.ai.slot, n);
    a.ai.behavior = n;
  };
  s.action = executionServices().action;
  s.runBehavior = [](auto &a) {
    executionServices().behavior(a, a.ai.behavior);
  };
  s.reset = [](auto &a) { record(23, a.ai.slot); };
  s.rewind = [](auto &a) { record(154, a.ai.slot); };
  s.hit = [](auto &a) {
    record(155, a.ai.slot, unsigned(a.ai.target));
    if (fixture->number % 13 == 0)
      fixture->world.creatures.at(unsigned(a.ai.target)).ai.health = 0;
  };
  s.interactionHit = [](auto &a) { record(156, a.ai.slot); };
  s.notifyHit = [](auto &a) { record(157, a.ai.slot); };
  s.effect = [](auto &a, unsigned kind, Cell p) {
    record(158, a.ai.slot, kind, p);
  };
  s.cue = [](auto &a, ActionCue kind, unsigned argument) {
    record(159, a.ai.slot, unsigned(kind), {int(argument), 0, 0});
  };
  s.defendedDamage = [](auto &a, unsigned amount, int owner,
                        const auto &credit) {
    record(160, a.ai.slot, amount, {owner, credit[0], credit[1]});
    if (fixture->number % 11 == 0)
      a.ai.health = 0;
  };
  s.releaseEngagement = [](auto &a) {
    record(161, a.ai.slot);
    int n = std::int32_t(a.ai.incoming - 1u);
    a.ai.incoming = n < 0 ? 0 : unsigned(n);
  };
  s.random = [] {
    record(162, 0);
    return 123u;
  };
  s.diagnostic = [](unsigned n) { record(163, 0, n); };
  s.stun = [](auto &a, unsigned duration, unsigned mode) {
    record(164, a.ai.slot, mode, {int(duration), 0, 0});
  };
  s.drainForm = [](auto &a, unsigned n) { record(165, a.ai.slot, n); };
  s.clearColor = [](auto &a, unsigned n) { record(166, a.ai.slot, n); };
  s.pollColor = [](auto &a, unsigned n) { record(167, a.ai.slot, n); };
  s.releaseEffect = [](unsigned n) { record(168, 0, n); };
  s.updateOrders = [](auto &a) { record(169, a.ai.slot); };
  return s;
}
void nativeCallback(Fixture &f, unsigned op) {
  auto &a = f.world.creatures[0];
  auto &e = f.execution[0];
  auto s = callbacks();
  ActionCallbackEnvironment env{23, f.number % 2 != 0};
  CreatureMaintenance m;
  m.status77c = f.number % 3;
  switch (op) {
  case 0:
    tickIdleAction(a, e, f.world, callbackState, s);
    break;
  case 1:
    tickCorpseAction(a, s);
    break;
  case 2:
  case 3:
    tickMeleeAction(a, e, f.world, m, callbackState, op == 3, env, s);
    break;
  case 4:
    tickInteractionAction(a, env, s);
    break;
  case 5:
    tickStunAction(a, e, callbackState, s);
    break;
  }
}
#ifdef MNM_EXECUTION_REFERENCE
Bytes callbackNodes;
void callbackExtra(void *p, const AutonomousCreature &a,
                   const CreatureExecution &e,
                   const ActionCallbackState &state) {
  write32(p, 0x678, 0);
  write32(p, 0x67c, 0);
  write32(p, 0x855, e.block855);
  write32(p, 0x7cd, a.animationKind);
  write32(p, 0xf0, a.ai.corpseA);
  write32(p, 0xf4, a.ai.corpseB);
  write32(p, 0x6a4, a.targetMovingAttacker);
  write32(p, 0xb0, state.sequence);
  write32(p, 0xd0, state.animationBreak);
  write32(p, 0x77c, fixture->number % 3);
}
extern "C" unsigned __attribute__((thiscall)) callbackPollStub(void *p) {
  auto delta = static_cast<unsigned char *>(p) - actors.data();
  bool overlay = unsigned(delta) % 0xe4b == 0x7d5;
  unsigned id = unsigned(delta) / 0xe4b;
  record(150, id, overlay);
  return unsigned(eventCode(overlay));
}
extern "C" unsigned __attribute__((thiscall)) callbackImmediateStub(void *p) {
  record(151, slot(p));
  if (fixture->number % 3) {
    write32(p, 0x64c, (slot(p) + 1) % fixture->world.creatures.size());
    return 1;
  }
  return 0;
}
extern "C" unsigned __attribute__((thiscall)) callbackRangedStub(void *p) {
  record(152, slot(p));
  if (fixture->number % 5) {
    write32(p, 0x64c, (slot(p) + 2) % fixture->world.creatures.size());
    return 1;
  }
  return 0;
}
extern "C" void __attribute__((thiscall)) callbackBehaviorStub(void *p,
                                                               unsigned n) {
  record(153, slot(p), n);
  write32(p, 0x5ec, n);
}
extern "C" void __attribute__((thiscall)) callbackRewindStub(void *p) {
  record(154,
         unsigned(static_cast<unsigned char *>(p) - actors.data()) / 0xe4b);
}
extern "C" void __attribute__((thiscall)) callbackHitStub(void *p) {
  unsigned target = read32(p, 0x64c);
  record(155, slot(p), target);
  if (fixture->number % 13 == 0)
    write32(actors.data() + target * 0xe4b, 0xe4, 0);
}
extern "C" void __attribute__((thiscall)) callbackInteractionStub(void *p) {
  record(156, slot(p));
}
extern "C" void __attribute__((thiscall)) callbackHitNoticeStub(void *,
                                                                void *p) {
  record(157, slot(p));
}
extern "C" void __attribute__((thiscall))
callbackEffectStub(void *, unsigned kind, int x, int y, int z, unsigned zero,
                   void *ref, unsigned last) {
  if (zero || ref || last)
    throw std::runtime_error("event effect flags");
  record(158, 0, kind, {x, y, z});
}
extern "C" void __attribute__((thiscall)) callbackIdleCueStub(void *, void *p,
                                                              unsigned seq) {
  record(159, slot(p), unsigned(ActionCue::idle), {int(seq), 0, 0});
}
#define CUE_STUB(name, kind)                                                   \
  extern "C" void __attribute__((thiscall)) name(void *, void *p) {            \
    record(159, slot(p), unsigned(ActionCue::kind));                           \
  }
CUE_STUB(callbackInteractionCueStub, interaction)
CUE_STUB(callbackStrikeCueStub, strike)
CUE_STUB(callbackPainCueStub, pain)
CUE_STUB(callbackRemovalCueStub, remove)
#undef CUE_STUB
extern "C" void __attribute__((thiscall))
callbackDefenseStub(void *p, unsigned amount, int owner, const int *credit) {
  record(160, slot(p), amount, {owner, credit[0], credit[1]});
  if (fixture->number % 11 == 0)
    write32(p, 0xe4, 0);
}
extern "C" void __attribute__((thiscall))
callbackEngagementStub(void *p, unsigned flag) {
  if (flag)
    throw std::runtime_error("release flag");
  record(161, slot(p));
  int n = std::int32_t(read32(p, 0x668) - 1u);
  write32(p, 0x668, n < 0 ? 0 : unsigned(n));
}
extern "C" unsigned __attribute__((thiscall)) callbackRandomStub(void *) {
  record(162, 0);
  return 123;
}
extern "C" void __attribute__((thiscall))
callbackDiagnosticStub(void *, const char *, unsigned n) {
  record(163, 0, n);
}
extern "C" void __attribute__((thiscall))
callbackStunStub(void *p, unsigned duration, unsigned mode, unsigned zero) {
  if (zero)
    throw std::runtime_error("stun flags");
  record(164, slot(p), mode, {int(duration), 0, 0});
}
extern "C" void __attribute__((thiscall)) callbackDrainStub(void *p,
                                                            unsigned n) {
  record(165, slot(p), n);
}
extern "C" void __attribute__((thiscall)) callbackClearColorStub(void *p,
                                                                 unsigned n) {
  record(166, unsigned(static_cast<unsigned char *>(p) - actors.data()) / 0xe4b,
         n);
}
extern "C" void __attribute__((thiscall)) callbackPollColorStub(void *p,
                                                                unsigned n) {
  record(167, unsigned(static_cast<unsigned char *>(p) - actors.data()) / 0xe4b,
         n);
}
extern "C" void __attribute__((thiscall)) callbackReleaseStub(void *p) {
  record(168, 0, reinterpret_cast<std::uintptr_t>(p));
}
extern "C" void __attribute__((thiscall)) callbackUpdateStub(void *p) {
  record(169, slot(p));
}
void installCallbacks() {
  for (auto v :
       {std::pair<unsigned, void *>{0x464ec0,
                                    reinterpret_cast<void *>(callbackPollStub)},
        {0x516220, reinterpret_cast<void *>(callbackImmediateStub)},
        {0x5164f0, reinterpret_cast<void *>(callbackRangedStub)},
        {0x50aa30, reinterpret_cast<void *>(callbackBehaviorStub)},
        {0x464d70, reinterpret_cast<void *>(callbackRewindStub)},
        {0x50c0d0, reinterpret_cast<void *>(callbackHitStub)},
        {0x50ec80, reinterpret_cast<void *>(callbackInteractionStub)},
        {0x49d310, reinterpret_cast<void *>(callbackHitNoticeStub)},
        {0x571310, reinterpret_cast<void *>(callbackEffectStub)},
        {0x5705e0, reinterpret_cast<void *>(callbackIdleCueStub)},
        {0x570a90, reinterpret_cast<void *>(callbackInteractionCueStub)},
        {0x570480, reinterpret_cast<void *>(callbackStrikeCueStub)},
        {0x5708e0, reinterpret_cast<void *>(callbackPainCueStub)},
        {0x570bd0, reinterpret_cast<void *>(callbackRemovalCueStub)},
        {0x514860, reinterpret_cast<void *>(callbackDefenseStub)},
        {0x50eb60, reinterpret_cast<void *>(callbackEngagementStub)},
        {0x54e200, reinterpret_cast<void *>(callbackRandomStub)},
        {0x485b10, reinterpret_cast<void *>(callbackDiagnosticStub)},
        {0x50d060, reinterpret_cast<void *>(callbackStunStub)},
        {0x520c70, reinterpret_cast<void *>(callbackDrainStub)},
        {0x54dce0, reinterpret_cast<void *>(callbackClearColorStub)},
        {0x54dcb0, reinterpret_cast<void *>(callbackPollColorStub)},
        {0x4943f0, reinterpret_cast<void *>(callbackReleaseStub)},
        {0x5120c0, reinterpret_cast<void *>(callbackUpdateStub)},
        {0x50ab30, reinterpret_cast<void *>(resetStub)}})
    redirect(v.first, v.second);
}
void compareCallback(const Fixture &initial, const Fixture &expected,
                     const ActionCallbackState &initialState,
                     const ActionCallbackState &expectedState, unsigned op,
                     const std::vector<Event> &trace) {
  auto input = initial;
  fixture = &input;
  events.clear();
  prepare();
  installCallbacks();
  callbackNodes.assign(48 * 3, 0);
  auto *head = callbackNodes.data();
  auto address = [](void *p) { return reinterpret_cast<std::uintptr_t>(p); };
  write32(head, 0, address(head + 48));
  write32(head, 4, address(head + 96));
  for (unsigned i = 0; i < 2; ++i) {
    auto *p = head + (i + 1) * 48;
    const auto &order = input.execution[0].orders[i];
    write32(p, 0, address(i == 1 ? head : p + 48));
    write32(p, 4, address(i == 0 ? head : p - 48));
    write32(p, 8, order.kind);
    write32(p, 0x28, order.effect.value_or(0));
  }
  write32(lists.data(), 4, address(head));
  write32(lists.data(), 8, 2);
  for (unsigned i = 0; i < input.world.creatures.size(); ++i) {
    callbackExtra(actors.data() + i * 0xe4b, input.world.creatures[i],
                  input.execution[i], initialState);
    write32(actors.data() + i * 0xe4b, 0x5f4,
            address(reinterpret_cast<void *>(behaviorStub)));
    write32(actors.data() + i * 0xe4b, 0x5f8, 0);
  }
  global(0x6bfd88, 23);
  global(0x6e1f30, input.number % 2);
  auto before = actors;
  const Bytes *body = nullptr;
  unsigned entry = callbackEntries[op];
  for (const auto &g : originalBodies)
    if (g.first == entry)
      body = &g.second;
  if (!body)
    throw std::runtime_error("callback body guard absent");
  std::memcpy(reinterpret_cast<void *>(entry), body->data(), body->size());
  using F = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<F>(entry)(actors.data());
  if (events != trace) {
    unsigned i = 0;
    while (i < events.size() && i < trace.size() && events[i] == trace[i])
      ++i;
    std::cerr << "callback trace i=" << i << " actual=" << events.size()
              << " expected=" << trace.size() << '\n';
    if (i < events.size())
      std::cerr << "actual " << events[i].kind << "," << events[i].slot << ","
                << events[i].argument << " xyz=" << events[i].position.x << ","
                << events[i].position.y << "," << events[i].position.z << '\n';
    if (i < trace.size())
      std::cerr << "expected " << trace[i].kind << "," << trace[i].slot << ","
                << trace[i].argument << " xyz=" << trace[i].position.x << ","
                << trace[i].position.y << "," << trace[i].position.z << '\n';
    throw std::runtime_error(
        "callback trace n=" + std::to_string(input.number) +
        " op=" + std::to_string(op));
  }
  for (unsigned i = 0; i < expected.world.creatures.size(); ++i) {
    packActor(before.data() + i * 0xe4b, expected.world.creatures[i],
              expected.execution[i]);
    callbackExtra(before.data() + i * 0xe4b, expected.world.creatures[i],
                  expected.execution[i], i == 0 ? expectedState : initialState);
  }
  if (before != actors) {
    unsigned i = 0;
    while (i < actors.size() && before[i] == actors[i])
      ++i;
    throw std::runtime_error(
        "callback write n=" + std::to_string(input.number) +
        " op=" + std::to_string(op) + " offset=" + std::to_string(i % 0xe4b));
  }
  for (unsigned i = 0; i < 2; ++i)
    if (read32(head + (i + 1) * 48, 0x28) !=
        expected.execution[0].orders[i].effect.value_or(0))
      throw std::runtime_error("callback order effect ownership");
  if (std::memcmp(reinterpret_cast<void *>(entry), body->data(), body->size()))
    throw std::runtime_error("callback original text modified");
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_EXECUTION_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE expected");
  install(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  unsigned cases = 0;
  for (unsigned n = 0; n < 4096; ++n)
    for (unsigned op = 0; op < 6; ++op) {
      auto input = generate(n);
      for (auto &a : input.world.creatures) {
        a.animationKind = n % 4;
        a.ai.corpseA = n % 4;
        a.ai.corpseB = n % 5;
        a.ai.readiness = n % 3;
      }
      auto &a = input.world.creatures[0];
      auto &e = input.execution[0];
      a.ai.behavior = n % 3 == 0 ? 1 : 3;
      a.ai.action = n % 3 == 0 ? 10 : n % 3 == 1 ? 20 : n % 25 + 1;
      a.ai.pending = n % 7;
      a.mode = n % 4;
      e.absorptionTimer = n % 5;
      e.routeEffect = n % 2 ? std::optional<unsigned>(45) : std::nullopt;
      e.orders = {{2, 1, {}, -1, 47, {}, false},
                  {n % 2 ? 2u : 3u, 2, {}, -1, 49, {}, false}};
      auto initialState = ActionCallbackState{n % 300, {-1, -1}, false};
      callbackState = initialState;
      auto expected = input;
      fixture = &expected;
      events.clear();
      nativeCallback(expected, op);
      auto trace = events;
      auto state = callbackState;
      for (const auto &b : expected.world.creatures) {
        mix(b.ai.action);
        mix(b.ai.behavior);
        mix(b.ai.health);
        mix(b.ai.engaged);
        mix(b.ai.incoming);
        mix(b.ai.corpseA);
        mix(b.ai.corpseB);
        mix(b.targetMovingAttacker);
        mix(unsigned(b.cooldown));
      }
      mix(state.animationBreak);
      mix(expected.execution[0].absorptionTimer);
      for (const auto &v : trace) {
        mix(v.kind);
        mix(v.slot);
        mix(v.argument);
        mix(unsigned(v.position.x));
        mix(unsigned(v.position.y));
        mix(unsigned(v.position.z));
      }
#ifdef MNM_EXECUTION_REFERENCE
      compareCallback(input, expected, initialState, state, op, trace);
#endif
      ++cases;
    }
  std::cout << "{\"success\":true,\"cases\":" << cases
            << ",\"functions\":6,\"outcome_digest\":" << digest << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
