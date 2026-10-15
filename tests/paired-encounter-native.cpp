#include "../game/persistence/snapshot.hpp"
#include "paired-encounter-projection.hpp"
#include <fstream>
#include <iostream>
int main(int argc, char **argv) try {
  if (argc != 5)
    throw std::runtime_error("SNAPSHOT TICKS NEW_SNAPSHOT NEW_TRACE");
  auto saved = mnm::game::readSnapshot(argv[1]);
  auto profile = mnm::encounter::decodeInputs(saved.campaign);
  auto ticks = std::stoul(argv[2]);
  if (ticks > 8192)
    throw std::runtime_error("tick bound");
  if (std::filesystem::exists(argv[4]))
    throw std::runtime_error("existing trace");
  mnm::game::World world(0);
  world.restore(std::move(saved));
  mnm::game::CreatureRuleSession session(std::move(world),
                                         mnm::encounter::runtime(profile));
  std::ofstream trace(argv[4]);
  std::uint64_t digest = 1469598103934665603ull;
  for (unsigned i = 0; i < ticks; ++i) {
    session.step();
    auto c = mnm::command_session::decode(session.world().state().systems);
    digest = mnm::encounter::paired::digest(
        digest, mnm::encounter::paired::project(c, profile));
    trace << c.native.tick << ' ' << digest << '\n';
  }
  if (!trace)
    throw std::runtime_error("trace write");
  mnm::game::writeSnapshot(argv[3], session.world().state());
  std::cout << "{\"success\":true,\"digest\":" << digest << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
