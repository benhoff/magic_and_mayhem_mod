#include "../../assets/creature_movement.hpp"
#include "../../reconstruction/motion/creature_profile.hpp"
#include "../command-session/native_regional_danger.hpp"
#include "../command-session/native_regional_graph.hpp"
#include "../command-session/native_owner_visibility.hpp"
#include <charconv>
#include <sstream>
#include "encounter.hpp"
#include <stdexcept>
namespace mnm::encounter {
namespace {
assets::Config encounterConfig(const Profile &p) {
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
  return std::get<assets::Config>(std::move(config));
}
int decimal(const std::string &s){int n=0;const auto r=std::from_chars(s.data(),s.data()+s.size(),n);if(r.ec!=std::errc{}||r.ptr!=s.data()+s.size())throw std::invalid_argument("visibility CFG integer");return n;}
}
std::array<cs::model::RegionalCreatureShape, 28>
navigationShapes(const Profile &p) {
  const auto config=encounterConfig(p);
  std::array<cs::model::RegionalCreatureShape, 28> out;
  for (unsigned i = 0; i < 28; ++i) {
    auto v = reconstruction::normalizeCreatureMovementConfig(
        assets::creatureMovementConfig(config, i));
    out[i] = {v.height, v.width, v.swimming};
  }
  return out;
}
void enableRegionalNavigation(Profile &p, unsigned capacity) {
  cs::prepareNativeRegionalGraph(p.initial, p.native, navigationShapes(p),
                                 capacity);
  cs::prepareNativeContinuation(p.initial, p.native);
}
void enableRegionalDanger(Profile &p) {
  cs::model::RegionalDangerInputs inputs;
  for (const auto &a : p.initial.world.creatures)
    if (inputs.commander.at(unsigned(a.ai.owner)) < 0) {
      inputs.commander.at(unsigned(a.ai.owner)) = int(a.ai.slot);
      inputs.enabled.at(unsigned(a.ai.owner)) =
          p.initial.world.ownerEligible.at(unsigned(a.ai.owner));
    }
  for (unsigned i = 0; i < 8; ++i)
    inputs.shared[i][i] = 1;
  cs::prepareNativeRegionalDanger(p.initial, p.native, std::move(inputs));
}
void enableOwnerKnowledge(Profile &p) {
  enableRegionalDanger(p);
  cs::prepareNativeOwnerKnowledge(p.initial);
}
void enableOwnerVisibility(Profile &p) {
  if(!p.initial.native.regionalDanger)enableOwnerKnowledge(p);
  else if(!p.initial.native.ownerKnowledge)cs::prepareNativeOwnerKnowledge(p.initial);
  const auto config=encounterConfig(p);
  std::vector<cs::model::SightDefinition> definitions;
  for(auto &a:p.initial.world.creatures){
    const auto section="CREATURE_"+std::to_string(a.ai.type);
    auto field=[&](const char *key)->const std::string&{const auto *s=config.find(section,key);if(!s)throw std::invalid_argument("visibility CFG field");return *s;};
    cs::model::SightDefinition d;
    d.range=decimal(field("ViewRange"));d.aimXY=a.width==1?16:32;d.aimZ=int(a.height)*8;
    std::istringstream stream(field("LRCOffsets"));std::string v;unsigned count=0;
    while(std::getline(stream,v,',')){
      if(count>=24)throw std::invalid_argument("visibility CFG eye count");
      const auto first=v.find_first_not_of(" \t"),last=v.find_last_not_of(" \t");
      if(first==std::string::npos)throw std::invalid_argument("visibility CFG eye value");
      const int n=decimal(v.substr(first,last-first+1));auto &offset=d.offsets[count/3];
      if(count%3==0)offset.x=n;else if(count%3==1)offset.y=n;else offset.z=n;
      ++count;
    }
    if(count!=24)throw std::invalid_argument("visibility CFG eye count");
    a.aimXY=d.aimXY;a.aimZ=d.aimZ;definitions.push_back(d);
  }
  cs::prepareNativeOwnerVisibility(p.initial,std::move(definitions));
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
