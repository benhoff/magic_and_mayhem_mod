#include "session.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
using namespace mnm::cure;
int main(int argc, char **argv) try {
  if (argc != 3 || (std::string(argv[1]) != "--checkpoint" && std::string(argv[1]) != "--resume"))
    throw std::invalid_argument("usage: cure-lab --checkpoint NEW-FILE | --resume FILE");
  auto session = [&] {
    if (std::string(argv[1]) == "--resume") {
      std::ifstream file(argv[2],std::ios::binary);
      if (!file) throw std::runtime_error("checkpoint open failed");
      return CureSession::restore({std::istreambuf_iterator<char>(file),{}});
    }
    CureSessionState state; state.cost = 3; state.magnitude = 8;
    state.actors.resize(2); state.effects.resize(2);
    state.actors[0].resource = {0,20 * 256,20,0,0,0};
    state.actors[0].visible[0] = 1;
    auto &target = state.actors[1]; target.resource.kind = 1;
    target.health.health = 9; target.status.status = 7; target.ongoingDamage = 5;
    target.visible[0] = 1; target.position = {2,2,0};
    CureSession result(state); result.cast(0,1); result.advance({});
    const auto checkpoint = result.checkpoint();
    std::ifstream existing(argv[2],std::ios::binary);
    if (existing.good()) throw std::runtime_error("refusing existing checkpoint");
    std::ofstream file(argv[2],std::ios::binary);
    file.write(reinterpret_cast<const char *>(checkpoint.data()),std::streamsize(checkpoint.size()));
    file.close(); if (!file) throw std::runtime_error("checkpoint write failed");
    return CureSession::restore(checkpoint);
  }();
  const auto before = session.state().actors.at(1).health.health;
  std::vector<CureAnimationInput> complete;
  for (unsigned i = 0; i < session.state().effects.size(); ++i) {
    const auto &e = session.state().effects[i];
    if (e.active && e.effect.type == 77) complete.push_back({{i,e.generation},1});
  }
  session.advance(complete);
  std::cout << "{\"success\":true,\"headless\":true,\"health_before\":" << before
            << ",\"health_after\":" << session.state().actors.at(1).health.health
            << ",\"caster_resource_q8\":" << session.state().actors.at(0).resource.value
            << ",\"status_after\":" << session.state().actors.at(1).status.status << "}\n";
  return 0;
} catch (const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
