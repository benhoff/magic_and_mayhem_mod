#include "../../assets/creature_movement.hpp"
#include "../../reconstruction/motion/creature_profile.hpp"
#include "../command-session/native_regional_danger.hpp"
#include "../command-session/native_regional_graph.hpp"
#include "../command-session/native_owner_visibility.hpp"
#include "../command-session/native_regional.hpp"
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
void placeOwnerPlannerEncounter(Profile &p) {
  auto staged=p.initial;
  if(p.scenario==9){
    if(staged.native.regions.empty() || staged.native.tick)throw std::invalid_argument("skirmish setup requires fresh regions");
    for(auto &cell:staged.native.cells)cell.occupant=0xffff;
    cs::NativeCreatureGeometry geometry(staged,p.native);
    std::vector<cs::model::Cell> points;
    for(const auto &region:staged.native.regions){
      if(!region.parameter)continue;
      points.clear();const auto center=region.position;
      for(int x=-6;x<=6 && points.size()<6;x+=3)for(int y=-6;y<=6 && points.size()<6;y+=3){
        cs::model::Cell q{center.x+x,center.y+y,center.z};
        if(q.x<0 || q.y<0 || q.x>=staged.world.dimensions.x || q.y>=staged.world.dimensions.y || !geometry.legal(staged.world.creatures[0],q) || !geometry.ground(staged.world.creatures[0],q))continue;
        points.push_back(q);
      }
      if(points.size()==6)break;
    }
    if(points.size()!=6)throw std::invalid_argument("crop has no six legal skirmish stands");
    for(unsigned i=0;i<6;++i){auto &a=staged.world.creatures[i];a.ai.position=points[i];a.ai.home=a.destination=a.ai.position;staged.execution[i].roamHome=a.ai.position;
      const auto query=cs::nativeRegionalQuery(geometry,a,{i,a.ai.owner,0,0});
      a.region=cs::model::lookupNavigationRegion(staged.native.regions,a.ai.position,query,staged.world.dimensions,p.native.boundary,0,0,cs::nativeRegionalServices(staged,p.native,geometry,a));
      if(a.region<0)throw std::invalid_argument("skirmish stand has no assigned region");}
    cs::prepareNativeContinuation(staged,p.native);p.initial=std::move(staged);return;
  }
  if((staged.world.creatures.size()!=2&&staged.world.creatures.size()!=3)||staged.native.regions.empty()||staged.native.tick)
    throw std::invalid_argument("owner planner encounter setup domain");
  for(auto &cell:staged.native.cells)cell.occupant=0xffff;
  cs::NativeCreatureGeometry geometry(staged,p.native);
  auto &a=staged.world.creatures[0];
  const auto query=cs::nativeRegionalQuery(geometry,a,{0,0,0,0});
  auto region=[&](cs::model::Cell point){return cs::model::lookupNavigationRegion(staged.native.regions,point,query,staged.world.dimensions,p.native.boundary,0,0,cs::nativeRegionalServices(staged,p.native,geometry,a));};
  const auto d=staged.world.dimensions;
  auto inside=[&](cs::model::Cell q){return q.x>=0&&q.y>=0&&q.z>=0&&q.x<d.x&&q.y<d.y&&q.z<d.z;};
  bool found=false;
  for(unsigned id=0;id<staged.native.regions.size()&&!found;++id){
    if(!staged.native.regions[id].parameter)continue;
    const auto center=staged.native.regions[id].position;
    for(int dx=-2;dx<=2&&!found;++dx)for(int dy=-2;dy<=2&&!found;++dy){
      cs::model::Cell first{center.x+dx,center.y+dy,center.z};
      if(!inside(first)||!geometry.legal(a,first)||!geometry.ground(a,first)||region(first)!=int(id))continue;
      for(auto offset:std::array<cs::model::Cell,4>{{{4,0,0},{-4,0,0},{0,4,0},{0,-4,0}}}){
        cs::model::Cell second{first.x+offset.x,first.y+offset.y,first.z};
        if(!inside(second)||!geometry.legal(a,second)||!geometry.ground(a,second)||region(second)!=int(id))continue;
        if(staged.world.creatures.size()==3) {
          bool thirdFound=false;
          for(auto offset3:std::array<cs::model::Cell,8>{{{0,4,0},{0,-4,0},{4,4,0},{-4,4,0},{4,-4,0},{-4,-4,0},{8,0,0},{0,8,0}}}) {
            cs::model::Cell third{first.x+offset3.x,first.y+offset3.y,first.z};
            if((std::abs(third.x-second.x)<2 && std::abs(third.y-second.y)<2) ||
               !inside(third)||!geometry.legal(a,third)||!geometry.ground(a,third)||region(third)!=int(id))continue;
            staged.world.creatures[2].ai.position=third;thirdFound=true;break;
          }
          if(!thirdFound)continue;
        }
        staged.world.creatures[0].ai.position=first;staged.world.creatures[1].ai.position=second;
        for(auto &actor:staged.world.creatures){actor.region=int(id);actor.ai.home=actor.ai.position;actor.destination=actor.ai.position;staged.execution[actor.ai.slot].roamHome=actor.ai.position;}
        found=true;break;
      }
    }
  }
  if(!found)throw std::invalid_argument("crop has no admitted same-region planner encounter");
  cs::prepareNativeContinuation(staged,p.native);p.initial=std::move(staged);
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
