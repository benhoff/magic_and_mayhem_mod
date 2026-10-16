#include "../apps/cure-world/session.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace mnm::cure;
namespace g = mnm::game;
namespace {
void require(bool yes, const char *why) {
  if (!yes)
    throw std::runtime_error(why);
}
CureSessionState fixture(unsigned n = 1) {
  CureSessionState s;
  s.cost = 3;
  s.magnitude = 8;
  s.dimensions = {8, 8, 3};
  s.actors.resize(2);
  s.effects.resize(2);
  s.actors[0].resource = {0, 20 * 256, 20, 0, 0, 0};
  s.actors[0].visible[0] = 1;
  auto &target = s.actors[1];
  target.resource.kind = 1;
  target.health.health = 1 + n % 20;
  target.status.status = n % 3;
  target.ongoingDamage = n % 5;
  target.visible[0] = 1;
  target.position = {2, 2, 0};
  return s;
}
CureAnimationDefinition definition(unsigned n = 0) {
  CureAnimationDefinition d;
  d.entry = {3, 2, 4, 99};
  d.sequence = {{1, int(n % 4), {}}, {0, 7, {}},  {0, 8, {}},
                {0, 9, {}},          {0, 10, {}}, {6, 0, {}}};
  return d;
}
bool sameEvents(const std::vector<CureSessionEvent> &a,
                const std::vector<CureSessionEvent> &b) {
  if (a.size() != b.size())
    return false;
  for (unsigned i = 0; i < a.size(); ++i)
    if (a[i].kind != b[i].kind || a[i].actor != b[i].actor ||
        a[i].value != b[i].value || a[i].position.x != b[i].position.x ||
        a[i].position.y != b[i].position.y ||
        a[i].position.z != b[i].position.z)
      return false;
  return true;
}
template <class F> void refuses(CureWorldSession &s, F f) {
  const auto before = s.checkpoint();
  bool failed = false;
  try {
    f();
  } catch (const std::exception &) {
    failed = true;
  }
  require(failed && before == s.checkpoint(), "atomic World/Cure refusal");
}
void targeted() {
  CureWorldSession session(fixture(), definition());
  auto caster = session.actor(0), target = session.actor(1);
  session.cast(caster, target);
  auto effect = *session.effect(0);
  require(session.world().find(effect)->family == g::Family::missileEffect,
          "native effect family");
  session.advance();
  require(session.state().effects[0].effect.type == 77 &&
              session.presentation().records[0].animation->player.sprite() == 7,
          "real animation binding");
  session.advance();
  session.relocate(target, {5, 5, 1});
  session.advance();
  const auto *entity = session.world().find(effect);
  require(entity && entity->x == 5 && entity->y == 5 && entity->z == 1,
          "native effect follows target");
  auto resumed = CureWorldSession::restore(session.checkpoint());
  while (session.effect(0)) {
    require(sameEvents(session.advance(), resumed.advance()), "resumed events");
    require(session.checkpoint() == resumed.checkpoint(),
            "resumed World/ANI state");
  }
  require(!session.world().find(effect) && session.world().find(target) &&
              session.state().actors[1].health.health == 10 &&
              session.state().actors[0].resource.value == 17 * 256,
          "native release/heal/resource");
  session.cast(caster, target);
  const auto reused = *session.effect(0);
  require(reused.slot == effect.slot && reused.generation != effect.generation,
          "native effect generation reuse");
  session.enqueue({g::Operation::release, effect, {}, {}, {}});
  session.advance();
  require(session.world().find(reused),
          "stale queued release cannot alias new effect");
  session.changeHealth(target, -100);
  session.advance();
  require(!session.effect(0) &&
              session.state().actors[0].resource.value == 14 * 256,
          "dead target releases without refund");
  refuses(session, [&] {
    session.cast(caster, target);
  }); // cleaned actor has no active semantic command admission.
  CureWorldSession gone(fixture(), definition(2));
  auto goneTarget = gone.actor(1);
  gone.cast(gone.actor(0), goneTarget);
  gone.advance();
  gone.enqueue({g::Operation::release, goneTarget, {}, {}, {}});
  gone.advance();
  require(!gone.effect(0) && !gone.world().find(goneTarget),
          "queued target release");
  auto world = gone.world();
  g::Entity replacement;
  replacement.type = 17;
  auto newTarget = world.spawn(replacement);
  require(newTarget.slot == goneTarget.slot &&
              newTarget.generation != goneTarget.generation,
          "actor generation reuse");
  CureWorldSession replaced(std::move(world));
  refuses(replaced, [&] { replaced.cast(replaced.actor(0), goneTarget); });
  refuses(replaced, [&] { replaced.cast(replaced.actor(0), newTarget); });
  CureWorldSession early(fixture(), definition());
  early.cast(early.actor(0), early.actor(1));
  early.enqueue({g::Operation::release, early.actor(1), {}, {}, {}});
  early.advance();
  early.advance();
  require(!early.effect(0), "target removal during initial11 phase");
  CureWorldSession cancelled(fixture(), definition());
  cancelled.cast(cancelled.actor(0), cancelled.actor(1));
  auto cancelledEffect = *cancelled.effect(0);
  cancelled.enqueue({g::Operation::cleanup, cancelledEffect, {}, {}, {}});
  cancelled.advance();
  require(!cancelled.effect(0) && !cancelled.world().find(cancelledEffect),
          "external effect cleanup releases native identity");
  CureWorldSession full(fixture(), definition(), 2);
  refuses(full, [&] { full.cast(full.actor(0), full.actor(1)); });
  CureWorldSession atomic(fixture(), definition());
  const auto before = atomic.checkpoint();
  require(atomic.advance({false, false, false}).empty() &&
              atomic.checkpoint() == before,
          "unadmitted tick freezes all cursors");
  refuses(atomic, [&] { atomic.relocate(atomic.actor(1), {99, 0, 0}); });
  auto corrupted = g::decodeSnapshot(atomic.checkpoint());
  corrupted.systems[0] ^= 1;
  g::World corrupt;
  corrupt.restore(corrupted);
  bool rejected = false;
  try {
    CureWorldSession invalid(std::move(corrupt));
  } catch (const std::exception &) {
    rejected = true;
  }
  require(rejected, "malformed Cure envelope");
  corrupted = atomic.world().state();
  corrupted.slots[atomic.actor(1).slot].entity->owner = 7;
  g::World mismatch;
  mismatch.restore(corrupted);
  rejected = false;
  try {
    CureWorldSession invalid(std::move(mismatch));
  } catch (const std::exception &) {
    rejected = true;
  }
  require(rejected, "mismatched World actor binding");
  atomic.cast(atomic.actor(0), atomic.actor(1));
  atomic.advance();
  const auto worldTick = atomic.world().state().tick;
  const auto phase20 = atomic.world().state().phase20;
  const auto cureTick = atomic.state().tick;
  atomic.advance({true, true, false});
  require(atomic.world().state().tick == worldTick &&
              atomic.world().state().phase20 == phase20 &&
              atomic.state().tick == cureTick + 1,
          "alternate World mode retains the effects phase and ANI clock");
  corrupted = atomic.world().state();
  corrupted.slots[atomic.effect(0)->slot].entity->state[0] ^= 1;
  g::World wrongTag;
  wrongTag.restore(corrupted);
  rejected = false;
  try {
    CureWorldSession invalid(std::move(wrongTag));
  } catch (const std::exception &) {
    rejected = true;
  }
  require(rejected, "mismatched native effect tag");
  corrupted = atomic.world().state();
  for (unsigned i = 0; i < 4; ++i)
    corrupted.systems[8 + i] = 0xff;
  g::World oversized;
  oversized.restore(corrupted);
  rejected = false;
  try {
    CureWorldSession invalid(std::move(oversized));
  } catch (const std::exception &) {
    rejected = true;
  }
  require(rejected, "valid outer snapshot with oversized embedded extent");
  // Each count must match the embedded core before handles/players are
  // allocated. In particular, a forged record count must not multiply the ANI
  // budget.
  const auto original = atomic.world().state();
  const auto &bytes = original.systems;
  auto word = [&](std::size_t offset) {
    require(offset + 4 <= bytes.size(), "test envelope field extent");
    unsigned value = 0;
    for (unsigned i = 0; i < 4; ++i)
      value |= unsigned(bytes[offset + i]) << (8 * i);
    return value;
  };
  const std::size_t actorCount = 12 + word(8);
  const std::size_t effectCount = actorCount + 4 + 8 * word(actorCount);
  std::size_t cursor = effectCount + 4;
  for (unsigned i = 0; i < word(effectCount); ++i) {
    const auto present = word(cursor);
    cursor += 4 + (present ? 8 : 0);
  }
  cursor += 16;                    // recipe metadata
  cursor += 4 + 44 * word(cursor); // normalized ANI records
  cursor += 16;                    // spatial dimensions and high-water mark
  const std::size_t presentationCount = cursor + 4 + 20 * word(cursor);
  for (auto offset : {actorCount, effectCount, presentationCount}) {
    for (auto count : {word(offset) - 1, word(offset) + 1}) {
      corrupted = original;
      for (unsigned i = 0; i < 4; ++i)
        corrupted.systems[offset + i] = std::uint8_t(count >> (8 * i));
      g::World forged;
      forged.restore(corrupted);
      rejected = false;
      try {
        CureWorldSession invalid(std::move(forged));
      } catch (const std::invalid_argument &error) {
        rejected = std::string(error.what()) == "Cure World vector count";
      }
      require(rejected, "component count rejected before materialization");
    }
  }
}
} // namespace
int main() try {
  targeted();
  std::uint64_t hash = 1469598103934665603ull;
  unsigned ticks = 0;
  for (unsigned n = 0; n < 512; ++n) {
    auto state = fixture(n);
    state.actors[1].resource.blocked = n % 7 == 0;
    CureWorldSession a(state, definition(n));
    a.cast(a.actor(0), a.actor(1));
    auto b = CureWorldSession::restore(a.checkpoint());
    for (unsigned step = 0; step < 32 && a.effect(0); ++step) {
      if (step == 2 && n % 5 == 0) {
        a.changeHealth(a.actor(1), -100);
        b.changeHealth(b.actor(1), -100);
      }
      require(sameEvents(a.advance(), b.advance()),
              "World step continuation events");
      require(a.checkpoint() == b.checkpoint(),
              "World step continuation bytes");
      b = CureWorldSession::restore(b.checkpoint());
      ++ticks;
    }
    require(!a.effect(0), "ANI completes bounded fixture");
    const auto result = a.state();
    const auto expected =
        n % 5 == 0 ? 0
        : state.actors[1].resource.blocked
            ? state.actors[1].health.health
            : std::min(24u, state.actors[1].health.health + 8);
    require(result.actors[1].health.health == expected &&
                result.actors[0].resource.value == 17 * 256,
            "World Cure result");
    for (auto byte : a.checkpoint())
      hash = (hash ^ byte) * 1099511628211ull;
  }
  std::cout << "{\"success\":true,\"cases\":512,\"animation_ticks\":" << ticks
            << ",\"world_checkpoint_continuation\":true,\"generation_"
               "refusals\":true,\"outcome_digest\":\""
            << std::hex << hash << "\"}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
