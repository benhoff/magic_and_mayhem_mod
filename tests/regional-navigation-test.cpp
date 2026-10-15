#include "../apps/command-session/native_regional.hpp"
#ifdef MNM_PAIRED_REFERENCE
#include "regional-navigation-original.hpp"
#else
#include "paired-encounter-projection.hpp"
#include <iostream>
#endif
#include <algorithm>
namespace {
using namespace mnm;
namespace m=command_session::model;
namespace cs=command_session;
void require(bool ok,const std::string &s){if(!ok)throw std::runtime_error(s);}
}
int main(int argc,char **argv) try {
#ifdef MNM_PAIRED_REFERENCE
  require(argc==3,"PE INSTALL");auto pe=read(argv[1]);map_image(pe);endpoints();sealText(pe);initializeSearch();
  using Constructor=void *(__attribute__((thiscall)) *)(void *,unsigned);
  reinterpret_cast<Constructor>(0x53a660)(regionManager,512);
  const auto install=argv[2];
#else
  require(argc==2,"INSTALL");const auto install=argv[1];
#endif
  auto p=encounter::decodeInputs(encounter::loadInputs(install,{0,0,32,32},0));auto c=p.initial;
  cs::NativeCreatureGeometry geometry(c,p.native);
#ifdef MNM_PAIRED_REFERENCE
  Original o(p);setup(o,c,17);
#endif
  unsigned checks=0,found=0,misses=0,partial=0,completed=0,candidateChanges=0;
  std::uint64_t digest=1469598103934665603ull;
  auto record=[&](unsigned expected,unsigned actual,const std::string &what,unsigned n){if(expected!=actual)throw std::runtime_error(what+" case="+std::to_string(n)+" native="+std::to_string(expected)+" original="+std::to_string(actual));digest=(digest^expected)*1099511628211ull;++checks;};
  for(unsigned n=0;n<2048;++n){
    auto &a=c.world.creatures[0];a.ai.owner=n%8;a.ai.bravery=1+n%100;a.ai.physical=n%2;a.flag104=n/2%2;
    const unsigned count=4+n%125;c.native.regions.assign(count,{});c.native.regionalRiskPowers={17,31};auto &g=c.native.regions;
    for(unsigned i=0;i<count;++i){auto &v=g[i];v.position={int(i*32/count),int((i*7)%32),int(2+i%6)};v.parameter=i%2;v.traffic=i%7;v.dangerBase=i%3;
      for(unsigned owner=0;owner<8;++owner)for(unsigned j=0;j<7;++j){v.danger[owner][j]=(i+owner*3+j)%11;v.deductedDanger[owner][j]=(i+j)%3;}
      for(unsigned j=0;j<18;++j){auto &e=v.connections[j];e.cost=(j<4?int(1+(i*13+j*7)%90):0);e.destination=int((i+(j%2?count-(j+1)/2:1+j/2))%count);for(unsigned k=0;k<20;++k)e.clearance[k]=std::int8_t(((i+j+k+n)%9)-2);}
    }
    m::RegionalQuery query;query.movement={int(1+n/4%5),int(1+n%2),int(n/8%2),int(1+n/16%3),0,17,0};query.dangerPenalty=n/32%2;query.trafficPenalty=n/64%2;query.creature=n%3!=0;
    auto lower=cs::nativeRegionalServices(c,p.native,geometry,a);
#ifdef MNM_PAIRED_REFERENCE
    publishRegions(g);auto *raw=o.actors.data();put(raw,0x174,a.ai.owner);put(raw,0x88,a.ai.bravery);put(raw,0xf8,a.ai.physical);put(raw,0x104,a.flag104);put(o.config.data(),8,query.movement.type_08);put(o.config.data(),12,query.movement.type_0c);put(o.config.data(),0x44,query.movement.type_44);global(0x6c12bf,17);global(0x6c1590,31);
    // The actor-derived member must equal the explicit member in mode0.
    query.movement.object_104=int(a.flag104);
    using Distance=int(__attribute__((thiscall)) *)(void *,int,int);
    using Index=int(__attribute__((thiscall)) *)(void *,int);
    using Risk=int(__attribute__((thiscall)) *)(void *,void *,unsigned);
    auto queryWire=wire(o,query);
#else
    query.movement.object_104=int(a.flag104);
#endif
    const int from=int(n%count),to=int((n*7+3)%count),x=int(n%42)-5;
    const auto distance=m::regionEdgeDistance(g,from,to),index=m::regionLowerBound(g,x),danger=m::regionDanger(a,g[to],c.native.regionalRiskPowers,n%2);
#ifdef MNM_PAIRED_REFERENCE
    record(distance,reinterpret_cast<Distance>(0x53ea50)(regionManager,from,to),"distance",n);record(index,reinterpret_cast<Index>(0x53e520)(regionManager,x),"index",n);record(danger,reinterpret_cast<Risk>(0x523070)(raw,reinterpret_cast<void *>(get(regionManager)+660*to),n%2),"danger",n);
#else
    record(distance,distance,"distance",n);record(index,index,"index",n);record(danger,danger,"danger",n);
#endif
    std::array<int,50> path;path.fill(0x12345678);m::RegionalSearchState state;int budget=2+int(n%80);
    const auto reached=m::searchNavigationRegions(g,from,to,query,c.world.dimensions,path,budget,true,state,lower);completed+=reached;partial+=!state.dirty;
#ifdef MNM_PAIRED_REFERENCE
    using Search=unsigned(__attribute__((thiscall)) *)(void *,int,int,WireQuery,int *,unsigned,unsigned);
    auto expected=path;expected.fill(0x12345678);record(reached,reinterpret_cast<Search>(0x53e5e0)(regionManager,from,to,queryWire,expected.data(),2+n%80,1),"search",n);
    for(unsigned j=0;j<50;++j)record(path[j],expected[j],"path["+std::to_string(j)+"]",n);
    record(state.dirty,get(regionManager,0x154,1),"dirty",n);
    for(unsigned j=0;j<count;++j)record(g[j].traffic,get(reinterpret_cast<void *>(get(regionManager)+660*j),528),"traffic",n);
#else
    record(reached,reached,"search",n);for(int v:path)record(v,v,"path",n);record(state.dirty,state.dirty,"dirty",n);for(auto &v:g)record(v.traffic,v.traffic,"traffic",n);
#endif
  }
  // Real installed geometry for region lookup, failed lines and the bounded
  // tile fallback, with wrapped XY and directional preference.
  c=p.initial;c.native.regions.clear();auto &a=c.world.creatures[0];
  for(int x=1;x<31;x+=3)for(int y=1;y<31;y+=3)for(int z=1;z<8;++z){m::Cell point{x,y,z};if(cs::nativeRegionalLegal(geometry,a,point,0)){m::NavigationRegion v;v.position=point;c.native.regions.push_back(v);break;}}
  require(c.native.regions.size()>20,"installed navigation fixture missing regions");
#ifdef MNM_PAIRED_REFERENCE
  setup(o,c,17);publishRegions(c.native.regions);using Lookup=int(__attribute__((thiscall)) *)(void *,int,int,int,WireQuery,int,int);auto *raw=o.actors.data();
#endif
  auto q=cs::nativeRegionalQuery(geometry,a,{0,a.ai.owner,0,0});auto lower=cs::nativeRegionalServices(c,p.native,geometry,a);
  for(unsigned n=0;n<1024;++n){m::Cell point{int(n*7%32),int(n/7%32),int(2+n%7)};const int dx=int(n%3)-1,dy=int(n/3%3)-1;
    const auto id=m::lookupNavigationRegion(c.native.regions,point,q,c.world.dimensions,0,dx,dy,lower);found+=id!=-1;misses+=id==-1;
#ifdef MNM_PAIRED_REFERENCE
    record(id,reinterpret_cast<Lookup>(0x53d7d0)(regionManager,point.x,point.y,point.z,wire(o,q),dx,dy),"lookup",n);
#else
    record(id,id,"lookup",n);
#endif
    auto candidate=point;auto desired=m::Cell{int(n*11%32),int(n/11%32),int(1+n%15)};const bool flight=n%2;const unsigned clearance=1+n%6,parameter=n/2%2;const int kind=1+n/4%3,boundary=int(n%6);
    auto params=q.movement;params.type_44=kind;
    auto h=geometry.movement();m::regionalCandidate(a,candidate,parameter,desired,flight,clearance,kind,boundary,c.world.dimensions.z,[&](auto v){return h.check(reconstruction::MovementCheck::check_46b0,{v.x,v.y,v.z},params);});candidateChanges+=!(candidate==point);
#ifdef MNM_PAIRED_REFERENCE
    using Candidate=void(__attribute__((thiscall)) *)(void *,int *,int *,int *,unsigned,int,int,int);auto original=point;put(o.config.data(),0x3c,flight);put(o.config.data(),0x40,clearance);put(o.config.data(),0x44,kind);global(0x6c5c80,boundary);reinterpret_cast<Candidate>(0x513cf0)(raw,&original.x,&original.y,&original.z,parameter,desired.x,desired.y,desired.z);
    record(candidate.x,original.x,"candidate.x",n);record(candidate.y,original.y,"candidate.y",n);record(candidate.z,original.z,"candidate.z",n);
    put(o.config.data(),0x3c,p.native.types[17].scalar.type_3c);put(o.config.data(),0x44,p.native.types[17].movementKind);global(0x6c5c80,0);
#else
    record(candidate.x,candidate.x,"candidate.x",n);record(candidate.y,candidate.y,"candidate.y",n);record(candidate.z,candidate.z,"candidate.z",n);
#endif
    auto adjusted=point;cs::nativeRegionalAdjust(c,geometry,a,adjusted,0);
#ifdef MNM_PAIRED_REFERENCE
    using Adjust=unsigned(__attribute__((thiscall)) *)(void *,const reconstruction::CreatureMovementParameters *,int *,int *,int *);auto originalAdjusted=point;reinterpret_cast<Adjust>(0x4f4f90)(reinterpret_cast<void *>(0x6c5490),&q.movement,&originalAdjusted.x,&originalAdjusted.y,&originalAdjusted.z);
    record(adjusted.x,originalAdjusted.x,"adjust.x",n);record(adjusted.y,originalAdjusted.y,"adjust.y",n);record(adjusted.z,originalAdjusted.z,"adjust.z",n);
#else
    record(adjusted.x,adjusted.x,"adjust.x",n);record(adjusted.y,adjusted.y,"adjust.y",n);record(adjusted.z,adjusted.z,"adjust.z",n);
#endif
  }
  require(found&&misses&&partial&&completed&&candidateChanges,"regional helper branch coverage");
  for(unsigned n=0;n<32;++n){
    c.native.regions.assign(64,{});for(unsigned i=0;i<64;++i){auto &v=c.native.regions[i];v.position={int(i/2),int(2+i%2),3};if(i+1<64){auto &edge=v.connections[0];edge.destination=int(i+1);edge.cost=100;edge.clearance[0]=edge.clearance[1]=edge.clearance[2]=8;}}
    auto query=q;query.trafficPenalty=n%2;std::array<int,50> path;path.fill(0x12345678);m::RegionalSearchState state;int budget=int(n%4);auto services=cs::nativeRegionalServices(c,p.native,geometry,a);
#ifdef MNM_PAIRED_REFERENCE
    publishRegions(c.native.regions);auto queryWire=wire(o,query);using Search=unsigned(__attribute__((thiscall)) *)(void *,int,int,WireQuery,int *,unsigned,unsigned);auto expected=path;
#endif
    auto reached=m::searchNavigationRegions(c.native.regions,0,63,query,c.world.dimensions,path,budget,true,state,services);
#ifdef MNM_PAIRED_REFERENCE
    record(reached,reinterpret_cast<Search>(0x53e5e0)(regionManager,0,63,queryWire,expected.data(),n%4,1),"limited-search",n);for(unsigned j=0;j<50;++j)record(path[j],expected[j],"limited-path",n);
#else
    record(reached,reached,"limited-search",n);for(auto v:path)record(v,v,"limited-path",n);
#endif
    budget=100;reached=m::searchNavigationRegions(c.native.regions,0,63,query,c.world.dimensions,path,budget,false,state,services);
#ifdef MNM_PAIRED_REFERENCE
    record(reached,reinterpret_cast<Search>(0x53e5e0)(regionManager,0,63,queryWire,expected.data(),100,0),"resumed-search",n);for(unsigned j=0;j<50;++j)record(path[j],expected[j],"truncated-path",n);for(unsigned j=0;j<64;++j)record(c.native.regions[j].traffic,get(reinterpret_cast<void *>(get(regionManager)+660*j),528),"truncated-traffic",n);
#else
    record(reached,reached,"resumed-search",n);for(auto v:path)record(v,v,"truncated-path",n);for(auto &v:c.native.regions)record(v.traffic,v.traffic,"truncated-traffic",n);
#endif
    require(reached&&path.back()==49,"regional50-point truncation missing");
  }
  // Native persistence policy: unfinished regional graph search survives exact
  // wire roundtrip, while pre-v7 output and malformed graph references refuse.
  auto empty=p.initial;empty.native.route.useRegions=true;require(cs::encode(cs::decode(cs::encode(empty)))==cs::encode(empty),"enabled empty regional graph roundtrip");empty.native.route.useRegions=false;auto legacy=cs::encode(empty,6);require(cs::encode(cs::decode(legacy))==cs::encode(empty),"v6 migration changed native state");
  c=p.initial;c.native.regions.assign(32,{});c.native.route.useRegions=true;
  for(unsigned i=0;i<32;++i){auto &v=c.native.regions[i];v.position={int(i),2,3};if(i+1<32){auto &e=v.connections[0];e.destination=int(i+1);e.cost=100;e.clearance[0]=e.clearance[1]=e.clearance[2]=8;}}
  auto policyQuery=cs::nativeRegionalQuery(geometry,c.world.creatures[0],{0,0,0,0});auto policyServices=cs::nativeRegionalServices(c,p.native,geometry,c.world.creatures[0]);
  std::array<int,50> policyPath{};int budget=2;m::searchNavigationRegions(c.native.regions,0,31,policyQuery,c.world.dimensions,policyPath,budget,true,c.native.regionalSearch,policyServices);
  require(!c.native.regionalSearch.dirty,"native unfinished regional search absent");auto bytes=cs::encode(c);auto restored=cs::decode(bytes);require(bytes==cs::encode(restored),"regional wire roundtrip");
  const auto refuses=[&](const auto &fn){bool rejected=false;try{fn();}catch(const std::exception &){rejected=true;}require(rejected,"invalid regional continuation accepted");};
  refuses([&]{cs::encode(c,6);});auto bad=c;bad.native.regions[0].connections[0].destination=32;refuses([&]{cs::encode(bad);});bad=c;bad.native.regionalSearch.best=32;refuses([&]{cs::encode(bad);});bad=c;bad.native.regions[2].position.x=-1;refuses([&]{cs::encode(bad);});
  auto restoredPath=policyPath;budget=700;int restoredBudget=700;const bool finish=m::searchNavigationRegions(c.native.regions,0,31,policyQuery,c.world.dimensions,policyPath,budget,false,c.native.regionalSearch,policyServices);
  cs::NativeCreatureGeometry restoredGeometry(restored,p.native);auto restoredServices=cs::nativeRegionalServices(restored,p.native,restoredGeometry,restored.world.creatures[0]);const bool restoreFinish=m::searchNavigationRegions(restored.native.regions,0,31,policyQuery,restored.world.dimensions,restoredPath,restoredBudget,false,restored.native.regionalSearch,restoredServices);
  require(finish&&restoreFinish&&policyPath==restoredPath&&budget==restoredBudget&&cs::encode(c)==cs::encode(restored),"regional search resumed differently");
#ifdef MNM_PAIRED_REFERENCE
  checkText();call(0x53a840,regionManager);destroySearch();
#endif
  std::cout<<"{\"success\":true,\"checks\":"<<checks<<",\"digest\":"<<digest<<",\"lookup_found\":"<<found<<",\"lookup_missed\":"<<misses<<",\"partial_searches\":"<<partial<<",\"completed_searches\":"<<completed<<",\"height_candidates\":"<<candidateChanges<<"}\n";
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
