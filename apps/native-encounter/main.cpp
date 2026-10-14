#include "../../game/persistence/snapshot.hpp"
#include "encounter.hpp"
#include <charconv>
#include <iostream>
#include <stdexcept>
namespace {
unsigned number(const char *s) {
  unsigned v = 0;
  const auto end = s + std::char_traits<char>::length(s);
  auto r = std::from_chars(s, end, v);
  if (r.ec != std::errc{} || r.ptr != end)
    throw std::invalid_argument("invalid encounter argument");
  return v;
}
void advance(mnm::game::CreatureRuleSession &s, unsigned ticks) {
  if (ticks > 8192)
    throw std::invalid_argument("encounter tick bound");
  for (unsigned i = 0; i < ticks; ++i)
    s.step();
}
void result(const mnm::game::CreatureRuleSession &s) {
  auto c = mnm::command_session::decode(s.world().state().systems);
  auto stats = mnm::encounter::statistics(c);
  std::cout << "{\"success\":true,\"ticks\":" << c.native.tick
            << ",\"hits\":" << stats.hits << ",\"damage\":" << stats.damage
            << ",\"stuns\":" << stats.stuns
            << ",\"knockbacks\":" << stats.knockbacks
            << ",\"deaths\":" << stats.deaths << ",\"actors\":[";
  bool first = true;
  for (const auto &a : c.world.creatures) {
    if (!first)
      std::cout << ',';
    first = false;
    std::cout << "{\"health\":" << a.ai.health
              << ",\"active\":" << (a.ai.active ? "true" : "false")
              << ",\"behavior\":" << a.ai.behavior
              << ",\"action\":" << a.ai.action
              << ",\"motion\":" << (a.ai.motion ? "true" : "false")
              << ",\"position\":[" << a.ai.position.x << ',' << a.ai.position.y
              << ',' << a.ai.position.z << "]}";
  }
  std::cout << "]}\n";
}
} // namespace
int main(int argc, char **argv) try {
  using namespace mnm;
  if (argc == 10 && std::string(argv[1]) == "run") {
    auto p = encounter::decodeInputs(
        encounter::loadInputs(argv[2], {number(argv[3]), number(argv[4]),
                                        number(argv[5]), number(argv[6])}));
    auto s = encounter::create(p, number(argv[7]));
    advance(s, number(argv[8]));
    game::writeSnapshot(argv[9], s.world().state());
    result(s);
    return 0;
  }
  if (argc == 5 && std::string(argv[1]) == "resume") {
    auto saved = game::readSnapshot(argv[2]);
    auto p = encounter::decodeInputs(saved.campaign);
    game::World world(0);
    world.restore(std::move(saved));
    game::CreatureRuleSession s(std::move(world), encounter::runtime(p));
    advance(s, number(argv[3]));
    game::writeSnapshot(argv[4], s.world().state());
    result(s);
    return 0;
  }
  std::cerr << "Usage: native-encounter run INSTALL X Y WIDTH HEIGHT SEED "
               "TICKS NEW_SNAPSHOT\n       native-encounter resume SNAPSHOT "
               "TICKS NEW_SNAPSHOT\n";
  return 2;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
