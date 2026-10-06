// Unchanged original recovery wrappers with explicit scripted COM fault inputs.
// No real DirectDraw loss, driver Restore, reload pixels or native renderer here.
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
#include "../reconstruction/rendering/surface_retry.hpp"
using namespace mnm::reconstruction;
using U=std::uint32_t;using I=std::int32_t;
struct Rect {I l,t,r,b;};struct Object {std::array<U,16> words{};};
struct Interface {void** table;U id;};
struct Event {U kind,id,result,flags,a,b;};
static std::vector<Event> events;static std::vector<U> draws;static unsigned drawAt;
static U restores[3],keysResult;static Interface surfaces[3];
static void event(U kind,U id,U result=0,U flags=0,U a=0,U b=0){
 if(events.size()>=128)throw std::runtime_error("Original call budget exceeded");
 events.push_back({kind,id,result,flags,a,b});
}
static U draw(void* target,U op,U flags,U a,U b){
 if(drawAt>=draws.size())throw std::runtime_error("Original exhausted finite result schedule");
 U result=draws[drawAt++];event(op,static_cast<Interface*>(target)->id,result,flags,a,b);return result;
}
static I restore(void* object){U id=static_cast<Interface*>(object)->id;event(3,id,restores[id]);return restores[id];}
static I key(void* object,U flags,U* range){U id=static_cast<Interface*>(object)->id;event(4,id,keysResult,flags,range[0],range[1]);return keysResult;}
struct Adapter:SurfaceRetryAdapter {
 U draw(bool isFast,bool fill,U flags,std::uint16_t color) override {return ::draw(&surfaces[2],isFast?2:1,flags,fill?0:1,fill?color:0);}
 U restore(RecoverySurface id) override {return ::restore(&surfaces[U(id)]);}
 U setSourceKey(RecoverySurface id,std::uint16_t value) override {U range[2]={value,value};return key(&surfaces[U(id)],8,range);}
 void reload(RecoverySurface id) override {event(5,U(id));}
 void report(U h) override {if(h && h!=0x887601c2u){event(6,0);event(7,0);}}
};
int main(int argc,char** argv)try{
 if(argc!=2 && argc!=3)throw std::runtime_error("Expected input and optional draw budget");
 unsigned budget=argc==3?std::stoul(argv[2]):16;
 surfaces[1].id=1;surfaces[2].id=2;
 std::ifstream input(argv[1]);if(!input)throw std::runtime_error("Cannot open cases");
 U id,entry,fastMode,noWait,keyEnabled,reloadBits,rs,rd,kr,count,color;bool first=true;
 std::cout<<"[";
 while(input>>id>>entry>>fastMode>>noWait>>keyEnabled>>reloadBits>>rs>>rd>>kr>>color>>count){
  if(count>16 || reloadBits>3)throw std::runtime_error("Case limits");
  draws.resize(count);for(auto& value:draws)if(!(input>>value))throw std::runtime_error("Incomplete schedule");
  events.clear();drawAt=0;restores[1]=rs;restores[2]=rd;keysResult=kr;
  SurfaceRetryInput config;config.wrapper=entry==0x58bac0?SurfaceWrapper::PartialFill:entry==0x58bc10?SurfaceWrapper::FullFill:entry==0x58c4a0?SurfaceWrapper::OpaqueCopy:SurfaceWrapper::KeyedCopy;
  config.fast=fastMode;config.noWait=noWait;config.keyEnabled=keyEnabled;config.sourceReload=reloadBits&1;config.destinationReload=reloadBits&2;config.sourceKey=0x5678;config.destinationKey=0xe123;config.color=color;
  Adapter adapter;const auto result=runSurfaceRetry(config,adapter,budget);
  if(!first)std::cout<<',';
  first=false;std::cout<<"{\"id\":"<<id<<",\"draws_consumed\":"<<drawAt<<",\"returned\":"<<(result.returned?"true":"false")<<",\"budget_exhausted\":"<<(result.budgetExhausted?"true":"false")<<",\"events\":[";
  bool firstEvent=true;for(auto e:events){if(!firstEvent)std::cout<<',';firstEvent=false;std::cout<<'['<<e.kind<<','<<e.id<<','<<e.result<<','<<e.flags<<','<<e.a<<','<<e.b<<']';}
  std::cout<<"]}";
 }
 if(!input.eof())throw std::runtime_error("Malformed input");
 std::cout<<"]\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
