#include "../reconstruction/pathfinding/route_line.hpp"
#include <iostream>
#include <stdexcept>
#ifdef MNM_ROUTE_LINE_REFERENCE
#define main unused_line_map_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction;
namespace {
struct Call {Coordinates from,to;int before;bool operator==(const Call &o)const{return from.x==o.from.x&&from.y==o.from.y&&from.z==o.from.z&&to.x==o.to.x&&to.y==o.to.y&&to.z==o.to.z&&before==o.before;}};
std::vector<Call> trace;unsigned number=0;
bool movement(Coordinates a,Coordinates b,int &category) {
  category=5;trace.push_back({a,b,category});
  if(number%7==0)return false;
  if(number%11==0)return a.z!=b.z;
  return (number+unsigned(a.x*3+b.x*5+b.y*7+b.z*11))%5!=0;
}
#ifdef MNM_ROUTE_LINE_REFERENCE
extern "C" unsigned __attribute__((thiscall)) moveStub(void *,int x,int y,int z,int u,int v,int w,void *,int *category,int original) {
  if(original!=-1)throw std::runtime_error("line movement source override");
  return movement({x,y,z},{u,v,w},*category);
}
void put(unsigned a,unsigned v){std::memcpy(reinterpret_cast<void*>(a),&v,4);}
#endif
} // namespace
int main(int argc,char **argv)try {
#ifdef MNM_ROUTE_LINE_REFERENCE
  if(argc!=2)throw std::runtime_error("expected pinned PE");
  map_image(read(argv[1]));
  Bytes body(reinterpret_cast<unsigned char*>(0x4f4ba0),reinterpret_cast<unsigned char*>(0x4f4f82));
  auto *entry=reinterpret_cast<unsigned char*>(0x4f3990);entry[0]=0xe9;
  auto jump=std::uint32_t(reinterpret_cast<std::uintptr_t>(&moveStub)-0x4f3995u);std::memcpy(entry+1,&jump,4);
  using Line=unsigned(__attribute__((thiscall)) *)(void*,int,int,int,int,int,int,void*);
#else
  (void)argc;(void)argv;
#endif
  std::uint64_t digest=1469598103934665603ull;unsigned calls=0,admitted=0;
  auto mix=[&](unsigned v){digest=(digest^v)*1099511628211ull;};
  for(number=0;number<8192;++number) {
    MapDimensions dims{2+int(number%15),2+int(number/15%11),1+int(number/255%4)};
    Coordinates from{int(number*13%unsigned(dims.x)),int(number*9%unsigned(dims.y)),int(number%unsigned(dims.z))};
    Coordinates to{int((number*7+1)%unsigned(dims.x)),int((number*3+1)%unsigned(dims.y)),int((number+1)%unsigned(dims.z))};
    CreatureMovementParameters p;p.type_0c=1;p.type_08=1;p.object_104=number%2;
    CreatureLineServices services{dims,[](auto a,auto b,const auto &,int &category,int override){if(override!=-1)throw std::runtime_error("native source override");return movement(a,b,category);}};
    trace.clear();auto result=test_creature_line(from,to,p,services);auto expected=trace;
#ifdef MNM_ROUTE_LINE_REFERENCE
    trace.clear();put(0x6c5494,dims.x);put(0x6c5498,dims.y);put(0x6c549c,dims.z);
    const auto before=p;
    auto actual=reinterpret_cast<Line>(0x4f4ba0)(reinterpret_cast<void*>(0x6c5490),from.x,from.y,from.z,to.x,to.y,to.z,&p);
    if(bool(actual)!=result || trace!=expected || std::memcmp(&p,&before,sizeof(p)))throw std::runtime_error("original greedy line mismatch n="+std::to_string(number));
    if(std::memcmp(reinterpret_cast<void*>(0x4f4ba0),body.data(),body.size()))throw std::runtime_error("original line body changed");
#endif
    admitted+=result;calls+=expected.size();mix(result);
    for(const auto &v:expected){mix(v.from.x);mix(v.from.y);mix(v.from.z);mix(v.to.x);mix(v.to.y);mix(v.to.z);mix(v.before);}
  }
  std::cout<<"{\"success\":true,\"cases\":8192,\"functions\":1,\"admitted\":"<<admitted<<",\"movement_calls\":"<<calls<<",\"outcome_digest\":"<<digest<<"}\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
