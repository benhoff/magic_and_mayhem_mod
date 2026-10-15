#include "../../assets/creature_movement.hpp"
#include "../../reconstruction/motion/creature_profile.hpp"
#include "../command-session/native_regional_graph.hpp"
#include "encounter.hpp"
#include <stdexcept>
namespace mnm::encounter {
std::array<cs::model::RegionalCreatureShape, 28>
navigationShapes(const Profile &p) {
  const auto &b = p.bytes;
  std::size_t at = b.at(7) == '2' ? 28 : 24;
  if (at + 4 > b.size())
    throw std::invalid_argument("regional CFG input extent");
  unsigned n = 0;
  for (unsigned i = 0; i < 4; ++i)
    n |= unsigned(b[at++]) << (8 * i);
  if (n > b.size() - at)
    throw std::invalid_argument("regional CFG blob extent");
  std::vector<std::uint8_t> input(b.begin() + at, b.begin() + at + n);
  auto decoded = assets::decodePackedContainer(input);
  if (!std::holds_alternative<assets::PackedContainer>(decoded))
    throw std::invalid_argument("regional packed CFG");
  auto config =
      assets::decodeConfig(std::get<assets::PackedContainer>(decoded).decoded);
  if (!std::holds_alternative<assets::Config>(config))
    throw std::invalid_argument("regional CFG");
  std::array<cs::model::RegionalCreatureShape, 28> out;
  for (unsigned i = 0; i < 28; ++i) {
    auto v = reconstruction::normalizeCreatureMovementConfig(
        assets::creatureMovementConfig(std::get<assets::Config>(config), i));
    out[i] = {v.height, v.width, v.swimming};
  }
  return out;
}
void enableRegionalNavigation(Profile &p, unsigned capacity) {
  cs::prepareNativeRegionalGraph(p.initial, p.native, navigationShapes(p),
                                 capacity);
  cs::prepareNativeContinuation(p.initial, p.native);
}
void editTerrain(game::CreatureRuleSession &session, const Profile &p,
                 const std::vector<cs::NativeTerrainEdit> &edits) {
  auto state = session.world().state();
  auto c = cs::decode(state.systems);
  cs::editNativeTerrain(c, p.native, edits);
  state.systems = cs::encode(c);
  game::World world(0);
  world.restore(std::move(state));
  auto staged = game::CreatureRuleSession(std::move(world), runtime(p));
  session = std::move(staged);
}
} // namespace mnm::encounter
