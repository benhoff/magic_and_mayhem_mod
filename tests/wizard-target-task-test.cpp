#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma push_macro("main")
#undef main
#define main unused_wizard_target_brain_fixture
#include "owner-brain-inference-test.cpp"
#undef main
#pragma pop_macro("main")
#pragma GCC diagnostic pop
#include "../reconstruction/creature-ai/wizard_target_task.hpp"
#include <climits>
namespace {
struct Case {
  unsigned spell = 41, status = 0;
  int wait = 0, prepare = 0;
  bool kill = false, refill = false;
};
Case input;
std::vector<unsigned> calls;
void call(unsigned kind, unsigned source, unsigned spell = 0,
          unsigned target = 0) {
  calls.insert(calls.end(), {kind, source, spell, target});
}
bool indirect(unsigned spell) {
  return spell == 78 || spell == 83 || spell == 84;
}
#ifdef MNM_BRAIN_REFERENCE
void *sourceActor, *targetActor;
unsigned read(const void *p, unsigned at) {
  unsigned n;
  std::memcpy(&n, static_cast<const char *>(p) + at, 4);
  return n;
}
unsigned __attribute__((thiscall)) taskMaximum(void *actor) {
  return read(actor, 0x904);
}
unsigned __attribute__((thiscall)) taskBlocked(void *actor) {
  call(0, read(actor, 0));
  return input.status != 0;
}
int __attribute__((thiscall)) taskWait(void *actor, unsigned spell,
                                       unsigned count) {
  call(1, read(actor, 0), spell, count);
  if (input.kill)
    put(targetActor, 0xe4, 0);
  return input.wait;
}
void __attribute__((thiscall)) taskMana(void *actor) {
  call(2, read(actor, 0));
  if (input.refill)
    put(actor, 0xe8, read(actor, 0xe8) + 25600);
}
int __attribute__((fastcall)) taskPrepare(void *actor, unsigned spell, int x,
                                          int y, int z, void *target) {
  call(3, read(actor, 0), spell, read(target, 0));
  calls.insert(calls.end(), {unsigned(x), unsigned(y), unsigned(z)});
  return input.prepare;
}
void __attribute__((thiscall)) taskCast(void *descriptor) {
  call(4, read(descriptor, 28), read(descriptor, 0), read(descriptor, 32));
  calls.insert(calls.end(), {read(descriptor, 36), read(descriptor, 40),
                             read(descriptor, 44)});
}
#endif
} // namespace
int main(int argc, char **argv) try {
  const std::array<unsigned, 8> spells{41, 42, 74, 78, 79, 83, 84, 85};
  const std::array<unsigned, 8> entries{0x4466f0, 0x45d2b0, 0x44f9d0, 0x450780,
                                        0x44c560, 0x4500c0, 0x450ee0, 0x45bb60};
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "original PE");
  map_image(load(argv[1]));
  redirect(0x5075f0, reinterpret_cast<void *>(&taskMaximum));
  redirect(0x51fef0, reinterpret_cast<void *>(&taskBlocked));
  redirect(0x51e530, reinterpret_cast<void *>(&taskWait));
  redirect(0x51d570, reinterpret_cast<void *>(&taskMana));
  redirect(0x40ad00, reinterpret_cast<void *>(&taskPrepare));
  redirect(0x57c6e0, reinterpret_cast<void *>(&taskCast));
  global(0x6c5498, 32);
  global(0x6c5494, 32);
  global(0x6c549c, 8);
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "wizard target RX");
#else
  (void)argc;
  (void)argv;
  (void)entries;
#endif
  unsigned casts = 0, uses = 0, completed = 0;
  for (unsigned family = 0; family < spells.size(); ++family)
    for (unsigned n = 0; n < 8192; ++n) {
      input = {};
      input.spell = spells[family];
      input.status =
          std::array<unsigned, 4>{0, 1, 2, UINT32_MAX}[(n / 128) % 4];
      input.wait = std::array<int, 4>{0, 1, INT_MAX, -1}[(n / 32) % 4];
      input.prepare = int((n / 16) % 2);
      input.kill = n & 512;
      input.refill = n & 1024;
      m::AutonomousWorld world;
      world.dimensions = {32, 32, 8};
      world.creatures.resize(2);
      for (unsigned i = 0; i < 2; ++i) {
        world.creatures[i].ai.slot = i;
        world.creatures[i].ai.health = 50;
        world.creatures[i].ai.maximumHealth = 100;
      }
      auto &target = world.creatures[1];
      target.ai.health = std::array<unsigned, 4>{0, 50, 100, 101}[n % 4];
      world.creatures[0].ai.position = {int(n % 97) - 32,
                                        int((n / 5) % 97) - 32, int(n % 7)};
      const auto initialWorld = world;
      (void)initialWorld;
      std::vector<unsigned> mana{(n & 8) ? 51200u : 256u, 0}, cost(104, 100);
      m::WizardTargetTask task{input.spell, 0, 1, bool(n & 4)};
      const auto initial = task;
      (void)initial;
      calls.clear();
      m::WizardTargetTaskServices s;
      s.blocked = [&](unsigned actor) {
        if (indirect(input.spell))
          call(0, actor);
        return input.spell == 74
                   ? input.status && input.status <= unsigned(INT_MAX)
                   : input.status != 0;
      };
      s.wait = [&](unsigned actor, unsigned spell, unsigned count) {
        call(1, actor, spell, count);
        if (input.kill)
          target.ai.health = 0;
        return input.wait;
      };
      s.useMana = [&](unsigned actor) {
        call(2, actor);
        if (input.refill)
          mana[actor] += 25600;
      };
      s.prepare = [&](unsigned actor, unsigned spell, unsigned victim) {
        call(3, actor, spell, victim);
        const auto p = world.creatures[actor].ai.position;
        calls.insert(calls.end(),
                     {unsigned((p.x % 32 + 32) % 32),
                      unsigned((p.y % 32 + 32) % 32), unsigned(p.z)});
        return input.prepare;
      };
      s.cast = [&](unsigned actor, unsigned spell, unsigned victim) {
        call(4, actor, spell, victim);
        calls.insert(calls.end(), {1, UINT32_MAX, 0});
        ++casts;
      };
      const bool continuing =
          m::executeWizardTargetTask(task, world, mana, cost, s);
      completed += !continuing;
      for (unsigned i = 0; i < calls.size();) {
        uses += calls[i] == 2;
        i += calls[i] >= 3 ? 7 : 4;
      }
      const auto expectedCalls = calls;
#ifdef MNM_BRAIN_REFERENCE
      Bytes raw(0x300), actor(0xe4b), victim(0xe4b);
      sourceActor = actor.data();
      targetActor = victim.data();
      put(raw.data(), 0x14a, address(actor.data()));
      put(raw.data(), 0x15e, address(victim.data()));
      raw[0x162] = initial.issued;
      put(actor.data(), 0, 0);
      put(victim.data(), 0, 1);
      put(victim.data(), 0xe4, initialWorld.creatures[1].ai.health);
      put(victim.data(), 0x904, 100);
      put(victim.data(), 0x90, input.status);
      put(victim.data(), 0x78c, input.status);
      put(victim.data(), 0x8c, input.status);
      put(victim.data(), 0x94, input.status);
      put(actor.data(), 0xe8, (n & 8) ? 51200u : 256u);
      for (unsigned i = 0; i < 3; ++i)
        put(actor.data(), 8 + 4 * i,
            unsigned(i == 0   ? initialWorld.creatures[0].ai.position.x
                     : i == 1 ? initialWorld.creatures[0].ai.position.y
                              : initialWorld.creatures[0].ai.position.z));
      global(0x6b1270 + input.spell * 721, 100);
      calls.clear();
      using Body = unsigned(__attribute__((thiscall)) *)(void *, void *,
                                                         unsigned, void *);
      const auto result = reinterpret_cast<Body>(entries[family])(
          raw.data(), nullptr, 73, nullptr);
      if (calls != expectedCalls)
        throw std::runtime_error(
            "wizard target calls family=" + std::to_string(family) +
            " case=" + std::to_string(n));
      record(continuing, result & 255u, "wizard task continuing");
      record(task.issued, raw[0x162], "wizard task issued");
      record(target.ai.health, read(victim.data(), 0xe4),
             "wizard target health");
      record(mana[0], read(actor.data(), 0xe8), "wizard source mana");
#else
      record(continuing, continuing, "wizard task continuing");
      record(task.issued, task.issued, "wizard task issued");
      record(target.ai.health, target.ai.health, "wizard target health");
      record(mana[0], mana[0], "wizard source mana");
#endif
      for (auto v : expectedCalls)
        record(v, v, "wizard task call");
    }
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(reinterpret_cast<void *>(0x401000), text.data(),
                    text.size()),
       "wizard target text changed");
#endif
  std::cout << "{\"success\":true,\"cases\":65536,\"checks\":" << checks
            << ",\"casts\":" << casts << ",\"mana_uses\":" << uses
            << ",\"completed\":" << completed << ",\"digest\":\"" << digest
            << "\"}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
