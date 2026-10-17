#include "../apps/command-session/native_owner_visibility.hpp"
#include "../apps/command-session/native_regional_danger.hpp"
#include "../apps/native-encounter/encounter.hpp"
#include <iostream>
namespace cs = mnm::command_session;
int main(int argc, char **argv) try {
  if (argc != 2)
    throw std::runtime_error("INSTALL");
  auto p = mnm::encounter::decodeInputs(
      mnm::encounter::loadInputs(argv[1], {0, 0, 32, 32}, 0));
  mnm::encounter::enableRegionalNavigation(p);
  mnm::encounter::enableOwnerVisibility(p);
  auto c = p.initial, restored = c;
  unsigned checks = 0, restores = 0, refusals = 0, seen = 0, unseen = 0;
  auto check = [&](bool b, const char *why) {
    if (!b)
      throw std::runtime_error(why);
    ++checks;
  };
  check(c.native.ownerVisibility->definitions[0].range == 15 &&
            c.native.ownerVisibility->definitions[0].aimXY == 32 &&
            c.native.ownerVisibility->definitions[0].aimZ == 16,
        "installed sight profile");
  for (unsigned tick = 1; tick <= 2304; ++tick) {
    for (auto *v : {&c, &restored}) {
      v->native.tick = tick;
      v->world.creatures[1].specialVision =
          tick >= 64 && tick < 1408 ? 10000 : 0;
      v->world.relation[0][1] = tick >= 128 && tick < 256;
      v->native.ownerVisibility->sharing[0][1] = tick >= 128 && tick < 256;
      cs::updateNativeOwnerVisibility(*v, p.native);
      cs::updateNativeRegionalDanger(*v, p.native);
    }
    const auto bytes = cs::encode(c);
    check(bytes.at(4) == 15, "visibility wire version");
    check(bytes == cs::encode(restored), "visibility continuation differs");
    if (tick % 8 == 0) {
      restored = cs::decode(bytes);
      ++restores;
    }
    seen += c.world.creatures[1].ai.visible[0] != 0;
    unseen += c.world.creatures[1].ai.visible[0] == 0;
  }
  auto reject = [&](auto change) {
    auto bad = c;
    change(bad);
    bool failed = false;
    try {
      (void)cs::encode(bad);
    } catch (const std::invalid_argument &) {
      failed = true;
    }
    check(failed, "malformed visibility accepted");
    ++refusals;
  };
  reject([](auto &v) { v.native.ownerVisibility->scan.phase = 12; });
  reject([](auto &v) { v.native.ownerVisibility->scan.capacity = 0; });
  reject([](auto &v) { v.native.ownerVisibility->definitions.pop_back(); });
  reject([](auto &v) { v.native.ownerVisibility->observers.pop_back(); });
  reject([](auto &v) { v.native.ownerVisibility->observers[0][0] = 125; });
  reject([](auto &v) { v.native.ownerVisibility->definitions[0].range = -1; });
  reject([](auto &v) {
    v.native.ownerVisibility->definitions[0].offsets[0].z = 100000;
  });
  reject([](auto &v) { v.native.ownerKnowledge.reset(); });
  auto legacy = c;
  legacy.native.ownerVisibility.reset();
  for (unsigned version : {10u}) {
    auto bytes = cs::encode(legacy, version);
    auto value = cs::decode(bytes);
    check(!value.native.ownerVisibility && cs::encode(value, version) == bytes,
          "v10 compatibility");
  }
  bool failed = false;
  try {
    (void)cs::encode(c, 10);
  } catch (const std::invalid_argument &) {
    failed = true;
  }
  check(failed, "lossy legacy visibility write");
  ++refusals;
  const auto before = cs::encode(c);
  failed = false;
  try {
    cs::prepareNativeOwnerVisibility(c, {});
  } catch (const std::invalid_argument &) {
    failed = true;
  }
  check(failed && before == cs::encode(c),
        "failed visibility activation published state");
  ++refusals;
  check(seen && unseen, "missing visibility lifecycle");
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"restores\":" << restores << ",\"refusals\":" << refusals
            << ",\"seen\":" << seen << ",\"unseen\":" << unseen << "}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
