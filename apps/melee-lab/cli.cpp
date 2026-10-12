#include "persistence/snapshot.hpp"
#include "profile.hpp"
#include <iostream>
#include <stdexcept>
namespace {
void trace(const mnm::game::MeleeExchangeSession &session,
           const std::vector<mnm::game::ExchangeEvent> &events) {
  const auto s = session.state();
  std::cout << "{\"tick\":" << session.world().state().tick << ",\"health\":["
            << s.actors[0].health << ',' << s.actors[1].health
            << "],\"phase\":[" << unsigned(s.actors[0].phase) << ','
            << unsigned(s.actors[1].phase) << "],\"cooldown\":["
            << s.actors[0].cooldown << ',' << s.actors[1].cooldown
            << "],\"events\":[";
  bool comma = false;
  for (auto e : events) {
    if (comma)
      std::cout << ',';
    comma = true;
    std::cout << '[' << e.actor.slot << ',' << e.animation << ',' << e.requested
              << ',' << e.applied << ',' << e.draws << ',' << (e.died ? 1 : 0)
              << ']';
  }
  std::cout << "]}\n";
}
} // namespace
int main(int argc, char **argv) try {
  if (argc != 6)
    throw std::invalid_argument("Usage: mnm-melee-cli new INSTALL TICKS SEED "
                                "OUTPUT | resume SAVE TICKS 0 OUTPUT");
  const auto ticks = std::stoul(argv[3]), seed = std::stoul(argv[4]);
  if (ticks > 4096 || seed > 0xffffffffUL)
    throw std::invalid_argument("bounded ticks/seed required");
  auto session = std::string(argv[1]) == "new"
                     ? mnm::melee_lab::create(
                           mnm::melee_lab::loadProfile(argv[2]), unsigned(seed))
                 : std::string(argv[1]) == "resume"
                     ? mnm::melee_lab::load(argv[2])
                     : throw std::invalid_argument("unknown melee command");
  trace(session, {});
  for (unsigned i = 0; i < ticks && !session.finished(); ++i) {
    auto events = session.step();
    trace(session, events);
  }
  auto r = mnm::game::writeSnapshot(argv[5], session.world().state(), false);
  if (!r.durable)
    throw std::runtime_error(r.detail);
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
