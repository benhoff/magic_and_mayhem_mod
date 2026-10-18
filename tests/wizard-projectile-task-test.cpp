#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#define main unused_wizard_target_fixture
#include "wizard-target-task-test.cpp"
#undef main
#pragma GCC diagnostic pop
#include "../reconstruction/creature-ai/wizard_projectile_task.hpp"
namespace {
#ifdef MNM_BRAIN_REFERENCE
void __attribute__((thiscall)) taskPost(void *actor, unsigned command, int x,
                                        int y, int z, unsigned argument) {
  call(5, read(actor, 0), command, argument);
  calls.insert(calls.end(), {unsigned(x), unsigned(y), unsigned(z)});
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "original PE");
  map_image(load(argv[1]));
  redirect(0x51e530, reinterpret_cast<void *>(&taskWait));
  redirect(0x51d570, reinterpret_cast<void *>(&taskMana));
  redirect(0x40ad00, reinterpret_cast<void *>(&taskPrepare));
  redirect(0x57c6e0, reinterpret_cast<void *>(&taskCast));
  redirect(0x520b30, reinterpret_cast<void *>(&taskPost));
  global(0x6c5494, 32);
  global(0x6c5498, 32);
  global(0x6c549c, 8);
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "wizard projectile RX");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned emitted = 0, posted = 0;
  for (unsigned n = 0; n < 16384; ++n) {
    input = {};
    input.spell = 73;
    input.wait = std::array<int, 4>{0, 1, INT_MAX, -1}[(n / 16) % 4];
    input.prepare = int((n / 8) % 2);
    input.kill = n & 256;
    input.refill = n & 512;
    m::AutonomousWorld world;
    world.dimensions = {32, 32, 8};
    world.creatures.resize(2);
    for (unsigned i = 0; i < 2; ++i) {
      world.creatures[i].ai.slot = i;
      world.creatures[i].ai.health = 50;
      world.creatures[i].ai.maximumHealth = 100;
    }
    world.creatures[0].ai.position = {int(n % 89) - 32, int((n / 5) % 89) - 32,
                                      int(n % 7)};
    world.creatures[0].ai.action = n & 128 ? 2 : 15;
    world.creatures[1].ai.health = n & 1 ? 50 : 0;
    const auto before = world;
    (void)before;
    m::WizardProjectileTask task{
        {73, 0, 1, bool(n & 2)},
        world.creatures[0].ai.position,
        {n % 5,
         n % 7,
         {int(n % 193) - 96, int((n / 3) % 193) - 96, int(n % 7)}}};
    if (n & 64)
      ++task.stand.x;
    const auto initial = task;
    (void)initial;
    std::vector<unsigned> mana{n & 4 ? 51200u : 256u, 0}, cost(104, 100);
    calls.clear();
    m::WizardTargetTaskServices services;
    services.wait = [&](unsigned a, unsigned spell, unsigned count) {
      call(1, a, spell, count);
      if (input.kill)
        world.creatures[1].ai.health = 0;
      return input.wait;
    };
    services.useMana = [&](unsigned a) {
      call(2, a);
      if (input.refill)
        mana[a] += 25600;
    };
    services.prepare = [&](unsigned a, unsigned spell, unsigned target) {
      call(3, a, spell, target);
      const auto q = world.creatures[a].ai.position;
      calls.insert(calls.end(),
                   {unsigned((q.x % 32 + 32) % 32),
                    unsigned((q.y % 32 + 32) % 32), unsigned(q.z)});
      return input.prepare;
    };
    services.cast = [&](unsigned a, unsigned spell, unsigned target) {
      call(4, a, spell, target);
      calls.insert(calls.end(), {1, UINT32_MAX, 0});
      ++emitted;
    };
    const auto continuing = m::executeWizardProjectileTask(
        task, world, mana, cost, services,
        [&](unsigned actor, const auto &after) {
          call(5, actor, after.command, after.argument);
          calls.insert(calls.end(), {unsigned(after.destination.x),
                                     unsigned(after.destination.y),
                                     unsigned(after.destination.z)});
          ++posted;
        });
    const auto expected = calls;
#ifdef MNM_BRAIN_REFERENCE
    Bytes raw(0x300), actor(0xe4b), target(0xe4b);
    sourceActor = actor.data();
    targetActor = target.data();
    put(actor.data(), 0, 0);
    put(target.data(), 0, 1);
    put(target.data(), 0xe4, before.creatures[1].ai.health);
    put(actor.data(), 0xe8, n & 4 ? 51200u : 256u);
    put(actor.data(), 0x5fc, before.creatures[0].ai.action);
    const auto pos = before.creatures[0].ai.position;
    put(actor.data(), 8, unsigned(pos.x));
    put(actor.data(), 12, unsigned(pos.y));
    put(actor.data(), 16, unsigned(pos.z));
    put(raw.data(), 0x14a, address(actor.data()));
    put(raw.data(), 0x176, address(target.data()));
    put(raw.data(), 0x17a, unsigned(initial.stand.x));
    put(raw.data(), 0x17e, unsigned(initial.stand.y));
    put(raw.data(), 0x182, unsigned(initial.stand.z));
    put(raw.data(), 0x18f, initial.after.command);
    put(raw.data(), 0x193, unsigned(initial.after.destination.x));
    put(raw.data(), 0x197, unsigned(initial.after.destination.y));
    put(raw.data(), 0x19b, unsigned(initial.after.destination.z));
    put(raw.data(), 0x19f, initial.after.argument);
    raw[0x1a3] = initial.cast.issued;
    global(0x6b1270 + 73 * 721, 100);
    calls.clear();
    using Body =
        unsigned(__attribute__((thiscall)) *)(void *, void *, unsigned, void *);
    const auto result =
        reinterpret_cast<Body>(0x44ae70)(raw.data(), nullptr, 31, nullptr);
    if (calls != expected)
      throw std::runtime_error("wizard projectile call mismatch case=" +
                               std::to_string(n));
    record(continuing, result & 255u, "wizard projectile continuing");
    record(task.cast.issued, raw[0x1a3], "wizard projectile issued");
    record(mana[0], read(actor.data(), 0xe8), "wizard projectile mana");
    record(world.creatures[1].ai.health, read(target.data(), 0xe4),
           "wizard projectile target");
#else
    record(continuing, continuing, "wizard projectile continuing");
    record(task.cast.issued, task.cast.issued, "wizard projectile issued");
    record(mana[0], mana[0], "wizard projectile mana");
    record(world.creatures[1].ai.health, world.creatures[1].ai.health,
           "wizard projectile target");
#endif
    for (auto v : expected)
      record(v, v, "wizard projectile call");
  }
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(reinterpret_cast<void *>(0x401000), text.data(),
                    text.size()),
       "wizard projectile text changed");
#endif
  need(emitted && posted, "wizard projectile fixture coverage");
  std::cout << "{\"success\":true,\"cases\":16384,\"checks\":" << checks
            << ",\"casts\":" << emitted << ",\"post_commands\":" << posted
            << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
