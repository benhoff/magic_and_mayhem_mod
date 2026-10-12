#include "persistence/snapshot.hpp"
#include "profile.hpp"
#include <iostream>
namespace {
void trace(const mnm::game::CreatureEncounterSession &s,
           const std::vector<mnm::game::CreatureEvent> &events) {
  auto v = s.state();
  std::cout << "{\"tick\":" << s.world().state().tick << ",\"actors\":[";
  for (unsigned i = 0; i < v.actors.size(); ++i) {
    if (i)
      std::cout << ',';
    const auto &a = v.actors[i];
    const auto &e = *s.world().state().slots[i].entity;
    std::cout << '[' << a.health << ',' << e.x << ',' << e.y << ',' << a.target
              << ',' << unsigned(a.phase) << ',' << a.stunTimer << ']';
  }
  std::cout << "],\"events\":[";
  bool comma = false;
  for (const auto &e : events) {
    if (comma)
      std::cout << ',';
    comma = true;
    std::cout << "[" << e.actor << ",\"" << e.kind << "\"," << e.target << ','
              << e.amount << ']';
  }
  std::cout << "]}\n";
}
} // namespace
int main(int argc, char **argv) try {
  if (argc != 6)
    throw std::invalid_argument("Usage: mnm-creature-cli new INSTALL TICKS "
                                "SEED OUTPUT | resume SAVE TICKS 0 OUTPUT");
  auto ticks = std::stoul(argv[3]), seed = std::stoul(argv[4]);
  if (ticks > 8192 || seed > 0xffffffffUL)
    throw std::invalid_argument("creature ticks/seed bound");
  auto s = std::string(argv[1]) == "new"
               ? mnm::creature_lab::create(
                     mnm::creature_lab::loadProfile(argv[2]), unsigned(seed))
           : std::string(argv[1]) == "resume"
               ? mnm::creature_lab::load(argv[2])
               : throw std::invalid_argument("unknown creature command");
  trace(s, {});
  for (unsigned i = 0; i < ticks && !s.finished(); ++i)
    trace(s, s.step());
  auto result = mnm::game::writeSnapshot(argv[5], s.world().state(), false);
  if (!result.durable)
    throw std::runtime_error(result.detail);
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
