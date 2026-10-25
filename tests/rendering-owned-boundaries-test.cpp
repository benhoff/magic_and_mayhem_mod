#include "terrain_traversal.hpp"
#include "terrain_generated.hpp"
#include "terrain_creature_lighting.hpp"
#include "graphics_initializer.hpp"
#include "../apps/command-session/native_facing.hpp"
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
using namespace mnm::reconstruction;
static unsigned checks=0;
static void need(bool value){++checks;if(!value)throw std::runtime_error("Owned rendering boundary failed");}
template<class F>static void rejects(F fn){bool caught=false;try{fn();}catch(const std::exception&){caught=true;}need(caught);}
static std::string state(const RegionPlacementState& s){
 std::ostringstream out;out<<s.grid.columns<<','<<s.grid.rows<<','<<s.bank.internalThreshold<<';';
 for(const auto& b:s.bank.blocks){const auto& d=b.selection;out<<d.section<<','<<d.specific<<','<<d.special<<','<<d.maximum<<','<<d.placed<<','<<b.sourceX<<','<<b.sourceY<<','<<b.columns<<','<<b.rows<<','<<b.layers<<';';for(auto e:d.edges)out<<e<<',';}
 for(unsigned i=0;i<s.grid.cells.size();++i){out<<s.grid.cells[i].occupied<<','<<s.storedSourceRows[i]<<';';for(auto e:s.grid.cells[i].edges)out<<e<<',';if(s.assignments[i])out<<s.assignments[i]->descriptor<<','<<s.assignments[i]->rotation;else out<<'-';for(auto c:s.candidates[i])out<<c.descriptor<<','<<unsigned(c.rotations)<<';';}
 if(s.carry){out<<s.carry->section<<','<<s.carry->descriptor<<','<<s.carry->rotation;for(auto e:s.carry->edges)out<<','<<e;}
 return out.str();
}
static RegionDescriptorBank bank(){RegionDescriptorBank b;RegionBlockDescriptor d;d.selection.maximum=20;d.selection.edges={{0,0,0,0}};b.blocks.push_back(d);return b;}
static void traversal(){
 TerrainCamera c;c.view=2;c.column=c.row=c.diagonal=0;c.span=3;c.f41=0;c.f15=64;c.cutLevel=2;
 std::vector<std::array<std::uint16_t,2>> flags(4*5*3);flags[20][0]=0x80;
 const auto frozen=flags;auto actual=traverseOwnedTerrain(4,5,3,c,{},flags);
 auto expected=traverseTerrain(4,5,3,c,{},[&](auto x,auto y,auto z){return frozen[(z*5+y)*4+x];});
 need(actual.size()==expected.size() && flags==frozen);
 for(unsigned i=0;i<actual.size();++i){const auto &a=actual[i],&b=expected[i];need(a.cell==b.cell&&a.column==b.column&&a.row==b.row&&a.layer==b.layer&&a.anchorX==b.anchorX&&a.anchorY==b.anchorY&&a.priority==b.priority);}
 for(const auto& v:actual)need(v.cell!=20); // Both logical aliases use one physical flag.
 rejects([&]{traverseOwnedTerrain(4,5,3,c,{},std::vector<std::array<std::uint16_t,2>>(59));});
 rejects([&]{traverseOwnedTerrain(0,5,3,c,{},flags);});
 flags.clear();need(!actual.empty() && actual.front().cell<60);
}
static void constraints(){
 auto b=bank();b.blocks.resize(50);validateRegionDescriptorBank(b);const auto count=b.blocks.size();
 b.blocks.push_back({});rejects([&]{validateRegionDescriptorBank(b);});need(b.blocks.size()==count+1);
 b.blocks.pop_back();b.blocks.back().sourceY=1;rejects([&]{validateRegionDescriptorBank(b);});
 b=bank();b.blocks[0].selection.placed=std::numeric_limits<std::int32_t>::max();validateRegionDescriptorBank(b); // Above max is not normalized.
 mnm::assets::MapAsset map;map.width=map.height=4;map.layers=1;map.metadata[0]=map.metadata[1]=2;
 RegionConnectorState counters;const auto before=counters.next;map.metadata[1]=3;
 rejects([&]{expandRegionDescriptors(map,0,false,1,counters);});need(counters.next==before);
 map.metadata[1]=2;counters.next[3]=std::numeric_limits<std::int32_t>::max();const auto overflow=counters.next;
 rejects([&]{expandRegionDescriptors(map,0,false,1,counters);});need(counters.next==overflow);
}
static void solver(){
 auto s=makeRegionPlacementState(2,2,bank());for(unsigned y=0;y<2;++y)for(unsigned x=0;x<2;++x)placeRegionSection(s,0,0,x,y);
 const auto frozen=state(s);rejects([&]{removeRegionBlocks(s,{{0,0},{2,0}});});need(state(s)==frozen);
 rejects([&]{removeRegionBlocks(s,{{1,1},{1,1}});});need(state(s)==frozen);
 removeRegionBlocks(s,{});need(state(s)==frozen);
 const auto carry=s.carry;removeRegionBlocks(s,{{0,0},{1,1}});
 need(!s.assignments[0]&&!s.assignments[3]&&s.assignments[1]&&s.assignments[2]);
 need(s.bank.blocks[0].selection.placed==2&&s.storedSourceRows[0]==0xffffffffu&&s.storedSourceRows[3]==0xffffffffu);
 need(s.carry->descriptor==carry->descriptor&&s.carry->edges==carry->edges);
}
static void specific(){
 auto b=bank();b.blocks.resize(2);b.blocks[0].selection.specific=true;
 auto s=makeRegionPlacementState(2,2,b);const auto frozen=state(s);
 std::vector<RegionSpecificRequest> requests{{1,1,3},{-1,-1,-1}};validateRegionSpecificRequests(s,requests);need(state(s)==frozen);
 for(auto invalid:std::vector<RegionSpecificRequest>{{2,0,0},{0,-2,0},{0,0,4},{0,0,-2}}){auto bad=requests;bad[1]=invalid;rejects([&]{validateRegionSpecificRequests(s,bad);});need(state(s)==frozen);}
 rejects([&]{validateRegionSpecificRequests(s,{});});
 auto result=placeRegionSpecifics(s,requests,123);requests[0]={0,0,0};s.grid.cells[0].edges[0]=23;
 need(result.state.assignments[3]&&result.state.assignments[3]->rotation==3&&result.requests[0].column==1&&result.state.grid.cells[0].edges[0]!=23);
}
static void generation(){
 auto b=bank();b.blocks[0].selection.specific=true;b.blocks[0].selection.maximum=1;
 auto initial=makeRegionPlacementState(2,2,b);initial.storedSourceRows[1]=71;
 auto current=initial;std::vector<RegionSpecificRequest> requests{{1,0,0}};const auto seed=0xfffffff0u;auto nextSeed=seed;
 for(unsigned i=0;i<10;++i){auto next=stepRegionGeneration(current,requests,nextSeed);need(!next.complete&&next.attempts.size()==1&&next.attempts[0].seed==seed+500u*i&&next.nextSeed==seed+500u*(i+1));current=std::move(next.state);requests=std::move(next.requests);nextSeed=next.nextSeed;}
 const auto all=generateRegionPlacement(initial,{{1,0,0}},seed);
 need(!all.complete&&all.attempts.size()==10&&state(current)==state(all.state)&&nextSeed==all.nextSeed&&current.storedSourceRows[1]==71);
 need(initial.bank.blocks[0].selection.placed==0&&!initial.assignments[2]);
 const auto success=stepRegionGeneration(makeRegionPlacementState(2,2,bank()),{{}},0xffffffffu);
 need(success.complete&&success.nextSeed==499&&success.attempts.size()==1);
}
static void generated(){
 mnm::assets::MapAsset map;map.width=map.height=2;map.layers=1;map.metadata[0]=map.metadata[1]=1;map.cells.resize(4);
 for(unsigned i=0;i<4;++i){map.cells[i].definition=i;map.cells[i].references.fill(7);map.cells[i].flags10=8;}
 mnm::assets::RegionRecipe recipe;recipe.columns=recipe.rows=2;recipe.specific={{4,3,0,0}};recipe.random={{4,3}};
 mnm::assets::TerrainCatalog terrain;terrain.records.resize(4);for(unsigned i=0;i<4;++i)for(unsigned r=1;r<4;++r)terrain.records[i][0x94-4*r]=i;
 auto result=generateTerrainRegion(recipe,{&map,&map},terrain,123);
 need(result.assembly&&result.generated.plan&&result.assembly->map.cells.size()==16&&result.generated.plan->blocks[0].source==0);
 for(unsigned i=1;i<4;++i)need(result.generated.plan->blocks[i].source==1);
 for(const auto& cell:result.assembly->map.cells)need(cell.definition<4&&cell.references[0]==0xffff&&!(cell.flags10&8));
 need(result.assembly->projectedObjects==8&&result.assembly->projectedReferences==24);
 map.cells.clear();need(result.assembly->map.cells.size()==16);
 rejects([&]{generateTerrainRegion(recipe,{&map,&map},terrain,123);});
 recipe.specific.clear();recipe.random[0].occurrences=1;
 auto failed=generateTerrainRegion(recipe,{&map},terrain,123);
 need(!failed.assembly&&!failed.generated.plan&&!failed.generated.generation.complete&&failed.generated.generation.attempts.size()==10);
}
static void lighting(){
 TerrainLightField batch(32,32,5,{-50,7}),sequential=batch;
 const std::vector<TerrainLightSource> sources{{0,0,0,17},{31,31,4,12},{16,16,1,2},{8,9,3,1}};
 batch.stampBatch(sources);for(const auto& s:sources)sequential.stamp(s);need(batch.buffers()==sequential.buffers()&&batch.at(0,0,0)==-127);
 batch.publish();const auto before=batch.buffers();auto invalid=sources;invalid.push_back({32,0,0,17});rejects([&]{batch.stampBatch(invalid);});need(batch.buffers()==before);
 invalid=sources;invalid.push_back({0,0,0,18});rejects([&]{batch.stampBatch(invalid);});need(batch.buffers()==before);
 batch.stampBatch({});need(batch.buffers()==before);
}
static bool equal(TerrainCreatureLightState a,TerrainCreatureLightState b){return a.phase==b.phase&&a.remaining==b.remaining&&a.quota==b.quota&&a.cursor==b.cursor&&a.published==b.published;}
static void creature(){
 TerrainLightField original(32,32,3,{-50,7});TerrainCreatureLightCycle first;
 std::vector<TerrainLightCreature> inputs(17,{true,true,0,3,4,1});TerrainLightRelations relations;
 for(unsigned i=0;i<3;++i)first.step(original,inputs,20,0,relations);
 TerrainCreatureLightCycle restored;restored.restore(first.state());auto clone=original;
 for(unsigned i=0;i<13;++i){first.step(original,inputs,20,0,relations);restored.step(clone,inputs,20,0,relations);need(equal(first.state(),restored.state())&&original.buffers()==clone.buffers());}
 auto before=restored.state();auto bad=before;bad.phase=8;rejects([&]{restored.restore(bad);});need(equal(before,restored.state()));bad=before;bad.cursor=65537;rejects([&]{restored.restore(bad);});need(equal(before,restored.state()));
 restored.restore({3,0xffffffffu,0xffffffffu,65536,true});need(restored.state().remaining==0xffffffffu&&restored.state().quota==0xffffffffu);
}
static unsigned oracleDirection(std::int32_t x,std::int32_t y){
 const double ax=x<0?-double(x):double(x),ay=y<0?-double(y):double(y);
 if(ax>1.5*ay)return x<0?6:2;
 if(ay>1.5*ax)return y<0?0:4;
 if(x<0)return y<0?7:5;
 return y<0?1:3;
}
static void facing(){
 using mnm::command_session::nativeFacingFromDelta;
 for(int y=-64;y<=64;++y)for(int x=-64;x<=64;++x)for(unsigned camera=0;camera<4;++camera)for(bool reverse:{false,true}){
  auto actual=nativeFacingFromDelta(x,y,camera,reverse);if(!x&&!y){need(!actual);continue;}
  const auto direction=oracleDirection(x,y);need(actual&&actual->direction==direction);
  const int raw=int(direction)-int(2*camera)-(reverse?4:0);need(actual->animation==unsigned((raw%8+8)%8));
 }
 for(auto x:{std::numeric_limits<std::int32_t>::min(),-1,0,1,std::numeric_limits<std::int32_t>::max()})for(auto y:{std::numeric_limits<std::int32_t>::min(),-1,0,1,std::numeric_limits<std::int32_t>::max()})if(x||y){auto result=nativeFacingFromDelta(x,y,0xffffffffu,true);need(result&&result->direction==oracleDirection(x,y));need(result->animation==((result->direction+6u)&7u));}
}
static void initializer(){
 using Step=GraphicsInitializerStep;
 for(bool video:{false,true})for(bool window:{false,true})for(auto config:{0u,1u,0x80000000u}){
  const auto plan=planGraphicsInitialization({video,window,config,7,5});need(plan.width==(video?800u:640u)&&plan.height==(video?600u:480u)&&plan.depth==16);
  need(plan.paddingX==(config?96u:7u)&&plan.paddingY==(config?0u:5u)&&plan.offscreen[3]==plan.width+plan.paddingX&&plan.offscreen[2]==plan.height+plan.paddingY);
  need(plan.steps[0]==(window?Step::setupWindow:Step::setupMode)&&plan.steps[4]==Step::queryPixelFormat);
  for(unsigned i=0;i<27;++i){if(i!=0&&i!=1&&i!=26)need(plan.primary[i]==0);if(i!=0&&i!=1&&i!=2&&i!=3&&i!=26)need(plan.offscreen[i]==0);}
 }
 rejects([]{planGraphicsInitialization({false,false,0,0xffffffffu,0});});
 rejects([]{planGraphicsInitialization({true,false,0,0,0xffffffffu});});
 const auto overridden=planGraphicsInitialization({true,true,1,0xffffffffu,0xffffffffu});need(overridden.paddingX==96&&overridden.paddingY==0);
 const auto edge=planGraphicsInitialization({false,false,0,0xffffffffu-640,0xffffffffu-480});need(edge.offscreen[2]==0xffffffffu&&edge.offscreen[3]==0xffffffffu);
}
int main(int argc,char** argv)try{
 if(argc!=2)throw std::runtime_error("Expected boundary name");
 const std::string name=argv[1];
 if(name=="traversal")traversal();else if(name=="constraints")constraints();else if(name=="solver")solver();else if(name=="specific")specific();else if(name=="generation")generation();else if(name=="generated")generated();else if(name=="lighting")lighting();else if(name=="creature")creature();else if(name=="facing")facing();else if(name=="initializer")initializer();else throw std::runtime_error("Unknown boundary");
 std::cout<<"{\"success\":true,\"boundary\":\""<<name<<"\",\"checks\":"<<checks<<"}\n";
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
