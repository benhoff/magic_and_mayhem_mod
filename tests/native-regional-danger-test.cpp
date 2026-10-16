#include "../apps/command-session/native_regional_danger.hpp"
#include "../apps/native-encounter/encounter.hpp"
#include <iostream>
namespace {
using namespace mnm;
namespace cs = command_session;
void need(bool ok, const char *why) {
  if (!ok)
    throw std::runtime_error(why);
}
} // namespace
int main(int argc, char **argv) try {
  need(argc == 2, "INSTALL");
  auto p = encounter::decodeInputs(
      encounter::loadInputs(argv[1], {0, 0, 32, 32}, 0));
  encounter::enableRegionalNavigation(p);
  for (auto &a : p.initial.world.creatures) {
    a.region = int(a.ai.slot % p.initial.native.regions.size());
    p.initial.execution[a.ai.slot].combatValue[5] =
        10000 + 1000 * int(a.ai.slot);
  }
  encounter::enableRegionalDanger(p);
  auto c = p.initial;
  unsigned checks = 0, restores = 0, refusals = 0;
  const auto check = [&](bool ok, const char *why) {
    need(ok, why);
    ++checks;
  };
  const auto reject = [&](const auto &change) {
    auto bad = c;
    change(bad);
    bool failed = false;
    try {
      (void)cs::encode(bad);
    } catch (const std::invalid_argument &) {
      failed = true;
    }
    check(failed, "malformed danger accepted");
    ++refusals;
  };
  check(c.native.regionalDanger.has_value(), "native danger missing");
  check(c.native.regionalDanger->inputs.memories[0].size() == 1,
        "initial visible threat missing");
  const auto remembered = c.native.regionalDanger->inputs.memories[0][0];
  c.world.creatures[1].ai.visible[0] = 0;
  c.world.creatures[1].ai.position.x =
      (c.world.creatures[1].ai.position.x + 7) % 32;
  for (unsigned tick = 0; tick < 96; ++tick) {
    ++c.native.tick;
    cs::updateNativeRegionalDanger(c, p.native);
    check(c.native.regionalDanger->inputs.memories[0][0].observedTick ==
              remembered.observedTick,
          "hidden threat timestamp refreshed");
    check(c.native.regionalDanger->inputs.memories[0][0].actor.position ==
              remembered.actor.position,
          "hidden threat position leaked");
    auto bytes = cs::encode(c);
    check(bytes.at(4) == 13, "danger wire version");
    c = cs::decode(bytes);
    check(cs::encode(c) == bytes, "danger checkpoint changed");
    ++restores;
  }
  c.world.creatures[1].ai.visible[0] = 1;
  c.world.creatures[1].ai.health = 0;
  ++c.native.tick;
  cs::updateNativeRegionalDanger(c, p.native);
  check(c.native.regionalDanger->inputs.memories[0][0].actor.health == 0,
        "visible death not observed");
  c.world.relation[0][1] = 1;
  ++c.native.tick;
  cs::updateNativeRegionalDanger(c, p.native);
  check(c.native.regionalDanger->inputs.memories[0].empty(),
        "visible alliance retains hostile snapshot");
  auto legacy = c;
  legacy.native.regionalDanger.reset();
  auto old = cs::encode(legacy, 8);
  auto restored = cs::decode(old);
  check(!restored.native.regionalDanger && cs::encode(restored, 8) == old,
        "v8 absent danger compatibility");
  bool refused = false;
  try {
    (void)cs::encode(c, 8);
  } catch (const std::invalid_argument &) {
    refused = true;
  }
  check(refused, "lossy v8 write accepted");
  ++refusals;
  reject([](auto &v) { v.native.regionalDanger->inputs.commander[0] = 125; });
  reject([](auto &v) { v.native.regionalDanger->state.actorCursor = 126; });
  reject([](auto &v) { v.native.regionalDanger->state.commander = 125; });
  reject(
      [](auto &v) { v.native.regionalDanger->state.pendingDanger.pop_back(); });
  reject([](auto &v) { v.native.regionalDanger->inputs.dimensions.x = 31; });
  reject(
      [](auto &v) { v.native.regionalDanger->inputs.actors[0].region = 4096; });
  reject([](auto &v) {
    v.native.regionalDanger->inputs.effects.push_back({0x41, 8, 3, {1, 1, 1}});
  });
  reject([](auto &v) {
    auto &in = v.native.regionalDanger->inputs;
    auto a = in.actors[0];
    in.memories[0] = {{3, 0, a}, {3, 0, a}};
  });
  const auto before = cs::encode(c);
  auto inputs = c.native.regionalDanger->inputs;
  inputs.commander[0] = 125;
  refused = false;
  try {
    cs::prepareNativeRegionalDanger(c, p.native, inputs);
  } catch (const std::invalid_argument &) {
    refused = true;
  }
  check(refused && cs::encode(c) == before,
        "configuration failure published partial state");
  ++refusals;
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"restores\":" << restores << ",\"refusals\":" << refusals
            << "}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
