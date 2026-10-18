#define main unused_melee_trace_fixture_main
#include "ordinary-melee-test.cpp"
#undef main
#include "../apps/command-session/native_geometry.hpp"
#include "../reconstruction/combat/autonomy_tables.hpp"
#include "encounter-trace.hpp"
namespace {
namespace cs = mnm::command_session;
State projectedHealth(const cs::Continuation &c, unsigned slot) {
  State s;
  const auto &a = c.world.creatures.at(slot);
  const auto &e = c.execution.at(slot);
  const auto &m = c.maintenance.at(slot);
  unsigned at = 8 + slot * 31 * 4;
  auto word = [&] {
    unsigned n = 0;
    for (unsigned i = 0; i < 4; ++i)
      n |= unsigned(c.backend.at(at++)) << (8 * i);
    return n;
  };
  s.health = word();
  s.bar = word();
  s.dirty = word();
  s.cooldown = word();
  s.target = word();
  s.gate9c = word();
  s.expiry = word();
  s.timer = word();
  s.cacheA = word();
  s.cacheB = word();
  s.marker = word();
  s.flag104 = word();
  s.rank = word();
  for (auto &v : s.incoming)
    v = word();
  for (auto &v : s.ages)
    v = word();
  for (auto &v : s.credit)
    v = word();
  s.health = a.ai.health;
  s.target = unsigned(a.ai.target);
  s.cooldown = m.timer660;
  s.gate9c = e.absorptionTimer;
  s.expiry = m.status778;
  s.timer = m.status780;
  s.cacheA = m.animation889;
  s.cacheB = m.animation8bd;
  s.flag104 = a.flag104;
  s.rank = e.bonusMode124;
  return s;
}
} // namespace
int main(int argc, char **argv) try {
  using namespace mnm;
#ifdef MNM_HEALTH_REFERENCE
  check(argc == 3, "expected PE and native encounter trace");
  initializeMelee(argv[1]);
#else
  check(argc == 2, "expected native encounter trace");
#endif
  encounter::test::Trace input(argv[argc - 1]);
  cs::Continuation before, after;
  unsigned frames = 0, polls = 0, hits = 0, ready = 0, publishedTiming = 0;
  std::uint64_t digest = 1469598103934665603ull;
  while (input.next(before, after)) {
    ++frames;
    const auto &a = before.world.creatures[0];
    const auto &b = before.world.creatures[1];
    const auto &e = before.execution[0];
    if (!before.native.randomSeeded || !a.ai.active || a.ai.action != 10 ||
        a.ai.target != 1 || e.block855 ||
        !before.native.animation[0].body.bound)
      continue;
    const auto &channel = before.native.animation[0].body;
    check(!channel.state.reverse, "ordinary trace attack reverse");
    const auto &ani = input.profile.native.animation.at(channel.asset);
    std::vector<assets::AnimationRecord> records(
        ani.records.begin() + ani.starts.at(channel.sequence),
        ani.records.begin() + ani.starts.at(channel.sequence + 1));
    NoCdAnimationController player(records);
    player.restore(channel.state);
    if (before.actions.callbacks[0].animationBreak)
      player.requestBreak();
    const auto event = player.tick();
    ++polls;
    auto p = input.profile.melee;
    p.power = int(e.valueScale8ed);
    p.height = a.ai.position.z - b.ai.position.z;
    p.heightBonus = 25;
    p.charge = e.previousMove != 0;
    p.chargeScalar = int(e.previousMoveVariant);
    p.chargeBonus = 62;
    p.bloodLust = a.ai.bloodLust;
    p.targetResponse =
        before.execution[1].routeRenewal || before.maintenance[1].recent6b0;
    constexpr int response[8] = {0, 0, 25, 50, 75, 50, 25, 0};
    p.responseModifier = response[relativeDirection(
        a.ai.position, b.ai.position, before.actions.motion[1].direction,
        before.world.dimensions)];
    auto random = before.native.random;
    State health = projectedHealth(before, 1);
    Inputs hp;
    hp.baseMaximum = 360;
    hp.slot = 1;
    hp.attributionOwner = 0;
    hp.contributions = true;
    hp.contributionIds = {0xffffffffu, 0xffffffffu};
    hp.creditAvailable = {false, false};
    Result changed{health, {}, 0};
    MeleeResult melee;
    auto staged = before;
    staged.world.creatures[0].ai.engaged = 0;
    auto &target = staged.world.creatures[1];
    if (target.ai.incoming)
      --target.ai.incoming;
    MeleeDispatch dispatch;
    dispatch.event = [&](const auto &v) {
      if (v.kind == MeleeEffect::damage) {
        changed = apply(health, hp, Entry::defendedDamage, v.value);
        target.ai.health = changed.state.health;
        target.ai.target = int(changed.state.target);
        target.flag104 = changed.state.flag104;
      }
    };
    dispatch.tryStun = [&](int) {
      KnockbackInput k;
      k.actor.health = target.ai.health;
      k.actor.mode = target.mode;
      k.actor.behavior = target.ai.behavior;
      k.actor.pending = target.ai.pending;
      k.actor.form = target.form;
      k.actor.configFlag = target.configFlags;
      k.actor.animationKind = target.animationKind;
      k.actor.position = target.ai.position;
      k.motion = target.ai.motion;
      k.facing = staged.actions.motion[1].direction;
      k.dimensions = staged.world.dimensions;
      k.response = unsigned(relativeDirection(a.ai.position, target.ai.position,
                                              k.facing, k.dimensions));
      std::copy_n(originalRouteOffsets.begin(), 8, k.offsets.begin());
      cs::NativeCreatureGeometry g(staged, input.profile.native);
      auto result = admitKnockback(
          k, nextRandom(random), [&](auto q) { return g.legal(target, q); },
          [&](auto q) {
            unsigned category = 5;
            return g.move(target, target.ai.position, q, category);
          });
      p.stunHandled = result.admitted;
      return result.admitted;
    };
    if (event == 2) {
      melee = resolveMelee(p, [&] { return nextRandom(random); }, dispatch);
      check(after.world.creatures[1].ai.health == changed.state.health,
            "published target health differs from original-bound melee/health "
            "trace");
      ++hits;
    }
    MeleeActionState actionState;
    actionState.engaged = a.ai.engaged;
    actionState.incoming = b.ai.incoming;
    actionState.recentAttacker = b.targetMovingAttacker;
    actionState.cooldown = unsigned(a.cooldown);
    auto classified =
        meleeEvent(actionState, event, true, changed.state.health != 0, false,
                   before.maintenance[0].status77c != 0, 0,
                   [&] { return nextRandom(random); });
    ready += event == 1;
    if (event == 1 && after.world.creatures[0].ai.action == 15)
      check(after.world.creatures[0].cooldown == int(classified.state.cooldown),
            "published recovery timing differs");
#ifdef MNM_HEALTH_REFERENCE
    Fixture original(p, health.health);
    original.health = health;
    healthFields(original.targetBefore, health);
    put(original.sourceBefore, 0x5fc, 10u);
    put(original.sourceBefore, 0x664, a.ai.engaged);
    put(original.sourceBefore, 0x66c, unsigned(a.cooldown));
    put(original.targetBefore, 0x668, b.ai.incoming);
    put(original.targetBefore, 0x6a4, b.targetMovingAttacker);
    std::copy(original.sourceBefore.begin(), original.sourceBefore.end(),
              original.pool.begin());
    std::copy(original.targetBefore.begin(), original.targetBefore.end(),
              original.pool.begin() + 0xe4b);
    std::uint32_t offsets[] = {0, unsigned(records.size())};
    std::uint32_t asset[] = {
        0, std::uint32_t(reinterpret_cast<std::uintptr_t>(offsets)),
        std::uint32_t(reinterpret_cast<std::uintptr_t>(records.data()))};
    auto *raw = original.pool.data() + 0xb0;
    const auto &state = channel.state.forward;
    put(original.pool, 0xb0 + 4,
        state.displayedRecord ? std::uint32_t(reinterpret_cast<std::uintptr_t>(
                                    records.data() + *state.displayedRecord))
                              : 0u);
    put(original.pool, 0xb0 + 8, unsigned(state.active));
    put(original.pool, 0xb0 + 12, state.delay);
    put(original.pool, 0xb0 + 16, state.elapsed);
    put(original.pool, 0xb0 + 20, state.repeats);
    put(original.pool, 0xb0 + 24,
        std::uint32_t(
            reinterpret_cast<std::uintptr_t>(records.data() + state.pc)));
    put(original.pool, 0xb0 + 28,
        std::uint32_t(reinterpret_cast<std::uintptr_t>(asset)));
    put(original.pool, 0xb0 + 32,
        unsigned(state.breakFlag ||
                 before.actions.callbacks[0].animationBreak));
    original.sourceBefore = {original.pool.begin(),
                             original.pool.begin() + 0xe4b};
    putRandom(before.native.random);
    streamStun = true;
    trace.clear();
    meleeTrace.clear();
    using Fn = void(__attribute__((thiscall)) *)(void *);
    reinterpret_cast<Fn>(0x50e7a0)(original.pool.data());
    const auto &expected = player.state().forward;
    std::uint32_t words[12];
    std::memcpy(words, raw, 48);
    auto index = [&](unsigned n) {
      return (std::uintptr_t(n) - std::uintptr_t(records.data())) / 44;
    };
    check(index(words[6]) == expected.pc && bool(words[2]) == expected.active &&
              words[3] == expected.delay && words[4] == expected.elapsed &&
              words[5] == expected.repeats && words[8] == expected.breakFlag &&
              (!words[1] ? !expected.displayedRecord
                         : expected.displayedRecord &&
                               index(words[1]) == *expected.displayedRecord),
          "original action ANI timing differs");
    std::copy(original.pool.begin() + 0xb0, original.pool.begin() + 0xe0,
              original.sourceBefore.begin() + 0xb0);
    original.verify(melee, changed.state, true, classified.state);
    check(trace == changed.events,
          "encounter original health endpoint trace differs");
    checkRandom(random);
#endif
    const auto &published = after.native.animation[0].body;
    auto timing = [](const AnimationControllerState &v) {
      const auto &f = v.forward;
      return std::array<unsigned, 10>{
          unsigned(f.pc),     unsigned(f.displayedRecord.value_or(0xffffffffu)),
          unsigned(f.active), f.delay,
          f.elapsed,          f.repeats,
          f.breakFlag,        unsigned(v.reverse),
          v.reverseCursor,    v.reverseLength};
    };
    if (published.sequence == channel.sequence &&
        timing(published.state) == timing(player.state()))
      ++publishedTiming;
    digest = (digest ^ unsigned(event)) * 1099511628211ull;
    digest = (digest ^ changed.state.health) * 1099511628211ull;
    digest = (digest ^ random.left) * 1099511628211ull;
  }
  check(frames == 1024 && hits > 0 && ready > 0 && polls > hits &&
            publishedTiming > 0,
        "encounter combat trace coverage");
  std::cout << "{\"success\":true,\"frames\":" << frames
            << ",\"action_polls\":" << polls << ",\"hit_events\":" << hits
            << ",\"ready_events\":" << ready
            << ",\"published_timing_samples\":" << publishedTiming
            << ",\"digest\":" << digest << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
