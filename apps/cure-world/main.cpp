#include "installed.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::cure;
namespace game = mnm::game;
int main(int argc, char **argv) try {
  auto session = [&] {
    if (argc == 3 && std::string(argv[1]) == "--resume") {
      game::World world;
      world.restore(game::readSnapshot(argv[2]));
      return CureWorldSession(std::move(world));
    }
    if (argc != 5 || std::string(argv[1]) != "--assets" ||
        std::string(argv[3]) != "--checkpoint")
      throw std::invalid_argument("usage: cure-world --assets ROOT "
                                  "--checkpoint NEW-FILE | --resume FILE");
    auto result = mnm::assets::AssetStore::create(argv[2]);
    if (auto *error = std::get_if<mnm::assets::Error>(&result))
      throw std::runtime_error(error->detail);
    auto definition =
        loadInstalledCureAnimation(std::get<mnm::assets::AssetStore>(result));
    CureSessionState state;
    state.cost = 3;
    state.magnitude = 8;
    state.actors.resize(2);
    state.effects.resize(2);
    state.dimensions = {8, 8, 3};
    state.actors[0].resource = {0, 20 * 256, 20, 0, 0, 0};
    state.actors[0].visible[0] = 1;
    auto &target = state.actors[1];
    target.resource.kind = 1;
    target.health.health = 9;
    target.status.status = 7;
    target.ongoingDamage = 5;
    target.visible[0] = 1;
    target.position = {2, 2, 0};
    CureWorldSession world(state, definition);
    world.cast(world.actor(0), world.actor(1));
    world.advance();
    const auto saved = mnm::game::writeSnapshot(argv[4], world.world().state());
    if (!saved.durable)
      throw std::runtime_error(saved.detail);
    return CureWorldSession::restore(world.checkpoint());
  }();
  const auto before = session.state().actors.at(1).health.health;
  unsigned ticks = 0;
  while (session.effect(0) && ticks < 4096) {
    session.advance();
    ++ticks;
  }
  if (session.effect(0))
    throw std::runtime_error(
        "Cure ANI did not produce completion within4096 ticks");
  const auto binding = session.presentation().records.at(0).animation;
  if (!binding)
    throw std::invalid_argument("resume requires a bound Cure effect");
  const auto state = session.state();
  std::cout << "{\"success\":true,\"recipe\":74,\"animation_ticks\":" << ticks
            << ",\"selected_sequence\":" << binding->entry.sequence
            << ",\"selected_asset\":" << binding->entry.assetIndex
            << ",\"printer_property\":" << binding->entry.property
            << ",\"opaque_data1\":" << binding->entry.opaque
            << ",\"health_before\":" << before
            << ",\"health_after\":" << state.actors.at(1).health.health
            << ",\"caster_resource_q8\":" << state.actors.at(0).resource.value
            << ",\"world_tick\":" << session.world().state().tick
            << ",\"effect_released\":true,\"status_after\":"
            << state.actors.at(1).status.status << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
