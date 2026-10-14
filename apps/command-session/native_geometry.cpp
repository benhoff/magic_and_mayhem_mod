#include "native_geometry.hpp"
#include "../../reconstruction/pathfinding/route_footprint_height.hpp"
#include "../../reconstruction/pathfinding/route_world.hpp"
#include "../../reconstruction/pathfinding/route_boundary.hpp"
#include "../../reconstruction/pathfinding/route_cell_support.hpp"
#include "../../reconstruction/pathfinding/route_clearance.hpp"
#include <cstring>
#include <stdexcept>
namespace mnm::command_session {
namespace {
reconstruction::Coordinates coordinates(model::Cell p){return {p.x,p.y,p.z};}
std::size_t index(const Continuation &s,model::Cell p){auto d=s.world.dimensions;if(p.x<0||p.x>=d.x||p.y<0||p.y>=d.y||p.z<0||p.z>=d.z)throw std::invalid_argument("native geometry coordinates");return (std::size_t(p.z)*d.y+p.y)*d.x+p.x;}
template<class T>void put(void *p,unsigned at,T value){std::memcpy(static_cast<unsigned char*>(p)+at,&value,sizeof(value));}
}
NativeCreatureGeometry::NativeCreatureGeometry(Continuation &s,const NativeCreatureProfile &p):state_(s),profile_(p),cells_(s.native.cells.size()){
  const auto d=s.world.dimensions;rows_.resize(d.y);layers_.resize(d.z);
  for(int y=0;y<d.y;++y)rows_[y]=y*d.x;
  for(int z=0;z<d.z;++z)layers_[z]=z*d.x*d.y;
}
void NativeCreatureGeometry::refresh(){
  if(cells_.size()!=state_.native.cells.size())throw std::invalid_argument("native geometry backing changed");
  for(unsigned i=0;i<cells_.size();++i){const auto &v=state_.native.cells[i];auto &c=cells_[i];put(&c,0,v.kind);c.occupant_04=v.occupant;put(&c,8,v.links);c.flags_0a=v.flags&255;c.unknown_0b=std::byte(v.flags>>8);}
}
reconstruction::CellValidityMapView NativeCreatureGeometry::view(){refresh();auto d=state_.world.dimensions;return {{ {d.x,d.y,d.z},cells_.data(),cells_.size(),rows_.data(),rows_.size(),layers_.data(),layers_.size(),d.x*d.y,d.z},profile_.terrain.data(),profile_.terrain.size()};}
reconstruction::CreatureAcceptanceState NativeCreatureGeometry::actor(unsigned slot) const{
  const auto &a=state_.world.creatures.at(slot);const auto &t=profile_.types.at(a.ai.type);
  if(a.width!=unsigned(t.width)||a.height!=unsigned(t.height))throw std::invalid_argument("native actor definition shape differs");
  return {int(slot+1),int(a.ai.type),int(a.flag104),int(a.ai.behavior),std::uint8_t(a.block723),t.width,t.height,t.movementKind};
}
reconstruction::MovementHelpers NativeCreatureGeometry::movement(){
  auto v=view();auto d=state_.world.dimensions;reconstruction::MovementHelpers h;h.dimensions={d.x,d.y,d.z};h.boundary=[this]{return profile_.boundary;};h.layer_count=[d]{return d.z;};
  reconstruction::RecordQueryHelpers q;q.dimensions=h.dimensions;q.boundary=h.boundary;
  h=reconstruction::with_clearance_tests(h,v);h=reconstruction::with_record_query(h,reconstruction::with_cell_support(q,v));
  reconstruction::ValidityHelpers valid;valid.dimensions=h.dimensions;h=reconstruction::with_validity_test(h,reconstruction::with_cell_validity_test(valid,v));return reconstruction::with_boundary_test(h);
}
reconstruction::CreatureAcceptanceHelpers NativeCreatureGeometry::acceptance(){
  auto v=view();auto h=movement();reconstruction::CreatureAcceptanceHelpers a;a.dimensions=h.dimensions;
  a.object_state=[this](unsigned token){if(!token)throw std::invalid_argument("native actor token");return actor(token-1);};
  a=reconstruction::with_movement_test(a,h);reconstruction::CellHelpers c;c.dimensions=a.dimensions;return reconstruction::with_cell_test(a,reconstruction::with_occupancy_test(c,v.map));
}
bool NativeCreatureGeometry::cell(const model::AutonomousCreature &a,model::Cell p){(void)index(state_,p);return reconstruction::test_creature_cell(a.ai.slot+1,coordinates(p),acceptance());}
bool NativeCreatureGeometry::legal(const model::AutonomousCreature &a,model::Cell p){auto h=movement();const auto parameters=reconstruction::creature_movement_parameters(actor(a.ai.slot));return cell(a,p)&&h.check(reconstruction::MovementCheck::check_46b0,coordinates(p),parameters);}
bool NativeCreatureGeometry::ground(const model::AutonomousCreature &a,model::Cell p){(void)index(state_,p);return reconstruction::test_cell_support(coordinates(p),reconstruction::creature_movement_parameters(actor(a.ai.slot)),view())!=0;}
bool NativeCreatureGeometry::move(const model::AutonomousCreature &a,model::Cell from,model::Cell to,unsigned &category){int out=int(category);const auto ok=reconstruction::accept_creature_move(a.ai.slot+1,coordinates(from),coordinates(to),false,out,acceptance());category=unsigned(out);return ok;}
int NativeCreatureGeometry::height(model::Cell p,unsigned width){

  if(width!=2){const auto kind=state_.native.cells.at(index(state_,p)).kind;return kind?profile_.terrain.at(kind).classification_94:0;}
  const auto d=state_.world.dimensions;std::array<std::uint16_t,4> kinds{};unsigned cursor=0;
  for(int y=0;y<2;++y)for(int x=0;x<2;++x){auto q=p;q.x=(q.x+x)%d.x;q.y=(q.y+y)%d.y;kinds[cursor++]=state_.native.cells.at(index(state_,q)).kind;}
  return reconstruction::large_footprint_height(kinds,[this](auto kind){return profile_.terrain.at(kind).classification_94;});
}
bool NativeCreatureGeometry::line(const model::AutonomousCreature &a,model::Cell from,model::Cell to){
  auto h=movement();auto p=reconstruction::creature_movement_parameters(actor(a.ai.slot));
  return reconstruction::test_creature_line(coordinates(from),coordinates(to),p,{h.dimensions,[h](auto x,auto y,const auto &parameters,int &category,int original){return reconstruction::test_movement(x,y,parameters,category,original,h);}});
}
bool NativeCreatureGeometry::reach(const model::AutonomousCreature &a,const model::AutonomousCreature &b,model::Cell source){
  auto projection=[&](const auto &v){model::ReachActor r{v.ai.position,v.finePosition.z,int(v.width),int(v.height),{},0};if(v.ai.motion){const auto &route=state_.actions.segments.at(v.ai.slot);if(route.cursor>=route.count)throw std::invalid_argument("native reach endpoint missing");r.endpoint=route.route.at(route.cursor).position;r.endpointTerrainHeight=height(*r.endpoint,v.width);}return r;};
  auto attacker=projection(a);attacker.position=source;auto target=projection(b);
  return model::meleeReach(attacker,target,state_.world.dimensions,[&](auto from,auto to){return line(a,from,to);}).admitted;
}
void NativeCreatureGeometry::reserve(const model::AutonomousCreature &a,model::Cell p,bool add){auto d=state_.world.dimensions;for(unsigned y=0;y<a.width;++y)for(unsigned x=0;x<a.width;++x)for(unsigned z=0;z<a.height&&p.z+int(z)<d.z;++z){auto q=p;q.x=(p.x+int(x))%d.x;q.y=(p.y+int(y))%d.y;q.z+=z;auto &c=state_.native.cells.at(index(state_,q));if(add){if((c.flags&3)&&c.occupant!=0xffff&&c.occupant!=a.ai.slot+1)throw std::invalid_argument("native occupancy reservation conflicts");c.flags|=3;c.occupant=a.ai.slot+1;}else if(c.occupant==a.ai.slot+1){c.flags&=std::uint16_t(~3u);c.occupant=0xffff;}}}
void NativeCreatureGeometry::snap(model::AutonomousCreature &a,model::Cell p){model::snapCreaturePosition(a,p,unsigned(profile_.types.at(a.ai.type).width),{[this](auto q){return state_.native.cells.at(index(state_,q)).kind;},[this](unsigned kind){return profile_.terrain.at(kind).classification_94;},[this](auto q){return height(q,2);}});}
std::shared_ptr<reconstruction::RouteWorldSnapshot> NativeCreatureGeometry::snapshot(const model::AutonomousCreature &a,model::Cell target){
  refresh();auto s=std::make_shared<reconstruction::RouteWorldSnapshot>();auto d=state_.world.dimensions;const auto &t=profile_.types.at(a.ai.type);s->dimensions={d.x,d.y,d.z};s->rows=rows_;s->layers=layers_;s->plane_stride=d.x*d.y;s->boundary=profile_.boundary;s->object_token=a.ai.slot+1;s->cell_base=0x20000000;s->default_scalar=profile_.defaultScalar;s->slope_global=t.scalar.slope_global;s->target=coordinates(target);s->cells=cells_;s->terrain=profile_.terrain;
  put(&s->object,0,int(a.ai.slot+1));put(&s->object,8,a.ai.position.x);put(&s->object,12,a.ai.position.y);put(&s->object,16,a.ai.position.z);put(&s->object,0xa8,a.ai.type);put(&s->object,0x104,a.flag104);put(&s->object,0x5ec,a.ai.behavior);put(&s->object,0x723,std::uint8_t(a.block723));put(&s->object,0x608,state_.actions.motion.at(a.ai.slot).direction);put(&s->object,0x778,state_.maintenance.at(a.ai.slot).status778);put(&s->object,0x77c,state_.maintenance.at(a.ai.slot).status77c);
  put(s->scalar_type.data(),8,t.width);put(s->scalar_type.data(),12,t.height);put(s->scalar_type.data(),0x44,t.movementKind);put(s->scalar_type.data(),0x3c,t.scalar.type_3c);put(s->scalar_type.data(),0x10,t.scalar.type_10);std::memcpy(s->scalar_type.data()+0xd8,t.scalar.type_d8.data(),48*4);
  put(s->generator_type.data(),0x589,t.routeCostFactor);put(s->generator_type.data(),0x5ad,unsigned(t.routeFlagPenalty));return s;
}
} // namespace mnm::command_session
