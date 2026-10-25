#include "../reconstruction/rendering/screen_cursor.hpp"
#include <array>
#include <vector>
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using U=uint32_t;
using State=mnm::reconstruction::CursorBackgroundState;
static void need(bool b,const char* m){if(!b)throw std::runtime_error(m);}
static State data;static bool reference;static U mutation,waitResult;
static const std::array<U,12> addresses={0x642c64,0x5e04c8,0x6a49e4,0x641a78,0x641a7c,0x641a80,0x641a84,0x642c54,0x642c58,0x641aa0,0x641aa4,0x6e1ee4};
using Row=std::array<U,24>;static std::vector<Row> events;
static_assert(sizeof(State)==48,"Explicit test word extent");
static U getWord(const State& s,U i){U v;std::memcpy(&v,reinterpret_cast<const unsigned char*>(&s)+4*i,4);return v;}
[[maybe_unused]] static void setWord(State& s,U i,U v){std::memcpy(reinterpret_cast<unsigned char*>(&s)+4*i,&v,4);}
static State observed(){
#ifdef MNM_ORIGINAL_REFERENCE
 if(reference){State s{};for(U i=0;i<12;++i){U v;std::memcpy(&v,(void*)addresses[i],4);setWord(s,i,v);}return s;}
#endif
 return data;
}
static void writeState(const State& s){
#ifdef MNM_ORIGINAL_REFERENCE
 if(reference){for(U i=0;i<12;++i){const U v=getWord(s,i);std::memcpy((void*)addresses[i],&v,4);}return;}
#endif
 data=s;
}
static void emit(U tag,std::initializer_list<U> args){Row row{};row[0]=tag;U n=1;for(auto a:args){need(n<12,"Event arguments");row[n++]=a;}const auto s=observed();for(U i=0;i<12;++i)row[12+i]=getWord(s,i);events.push_back(row);}
static U wait(U mutex,U milliseconds){emit(1,{mutex,milliseconds});auto s=observed();
 if(mutation==1)s.valid=0;
 if(mutation==2){s.valid=1;s.rectangle[0]=7;s.rectangle[2]=2;s.offsetX=0xffffffffu;s.offsetY=4;s.deferred^=1;s.mutex=77;}
 writeState(s);return waitResult;}
static U release(U mutex){emit(2,{mutex});return 0;}
static void copy(U source,U target,const U* r,U x,U y){emit(3,{source,target,r[0],r[1],r[2],r[3],x,y});if(mutation==3){auto s=observed();s.mutex=91;s.valid=0;writeState(s);}}
static void queue(U source,U kind,U target,const U* r,U x,U y,U priority){emit(4,{source,kind,target,r[0],r[1],r[2],r[3],x,y,priority});if(mutation==3){auto s=observed();s.mutex=91;s.valid=0;writeState(s);}}
struct Services {
 U wait(U h,U t){return ::wait(h,t);}U release(U h){return ::release(h);}
 void copy(U s,U d,const U* r,U x,U y){::copy(s,d,r,x,y);}
 void queue(U s,U k,U d,const U* r,U x,U y,U p){::queue(s,k,d,r,x,y,p);}
};
#ifdef MNM_ORIGINAL_REFERENCE
static U __attribute__((stdcall)) waitAdapter(U h,U t){return wait(h,t);}
static U __attribute__((stdcall)) releaseAdapter(U h){return release(h);}
static void __attribute__((thiscall)) copyAdapter(void* self,U dst,const U* r,U x,U y){copy(U(self),dst,r,x,y);}
static void __attribute__((thiscall)) queueAdapter(void* self,U kind,U dst,const U* r,U x,U y,U p){queue(U(self),kind,dst,r,x,y,p);}
static void adapt(U at,void* p){auto* b=(unsigned char*)at;b[0]=0xe9;U delta=U(p)-at-5;std::memcpy(b+1,&delta,4);}
#endif
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
 need(argc==2,"Pinned image required");map_image(read(argv[1]));const Bytes body((unsigned char*)0x4a4380,(unsigned char*)0x4a44ae),invalid((unsigned char*)0x4a4370,(unsigned char*)0x4a437b);
 adapt(0x58c4a0,(void*)copyAdapter);adapt(0x58d5b0,(void*)queueAdapter);global(0x5c5140,U((void*)waitAdapter));global(0x5c513c,U((void*)releaseAdapter));
#else
 (void)argv;need(argc==1,"Native model takes no image");
#endif
 const std::array<std::array<U,4>,12> rects={{{0,0,5,3},{0,0,0,3},{0,0,5,0},{5,4,1,1},{0xffffffffu,0,2,2},{0,0xffffffffu,2,2},{0,0,0xffffffffu,2},{0,0,2,0xffffffffu},{0,0,0x7fffffffu,0x7fffffffu},{0x7ffffffeu,1,0,3},{0,0,0,0},{4,3,4,3}}};
 const std::array<std::array<U,2>,6> offsets={{{0,0},{2,3},{0xffffffffu,4},{4,0xffffffffu},{0x80000000u,0x80000000u},{0x7fffffffu,0x7fffffffu}}};
 U cases=0,waits=0,direct=0,deferred=0;uint64_t checksum=0;
 for(U enabled:{0u,1u})for(U screen:{3u,22u})for(U surface:{0u,17u})for(U valid:{0u,1u,0xffffffffu})for(U mode:{0u,1u})for(auto rect:rects)for(auto offset:offsets)for(U result:{0u,128u,258u,0xffffffffu})for(U mut:{0u,1u,2u,3u}){
  State initial{enabled,screen,surface,{rect[0],rect[1],rect[2],rect[3]},valid,37,offset[0],offset[1],mode};mutation=mut;waitResult=result;data=initial;reference=false;events.clear();Services services;mnm::reconstruction::restoreOriginalCursorBackground(data,services);const auto expected=data;const auto expectedEvents=events;
#ifdef MNM_ORIGINAL_REFERENCE
  reference=true;writeState(initial);events.clear();((void(*)())0x4a4380)();const auto actual=observed();need(!std::memcmp(&actual,&expected,sizeof(State)),"Restore state differs");need(events==expectedEvents,"Restore ordered boundary differs");
#endif
  for(U i=0;i<12;++i)checksum=checksum*131+getWord(expected,i);
  for(auto row:expectedEvents){waits+=row[0]==1;direct+=row[0]==3;deferred+=row[0]==4;for(auto v:row)checksum=checksum*131+v;}
  reference=false;data=initial;mnm::reconstruction::invalidateOriginalCursorBackground(data);auto cleared=initial;cleared.valid=0;need(!std::memcmp(&data,&cleared,sizeof(State)),"Invalidation changed unrelated state");
#ifdef MNM_ORIGINAL_REFERENCE
  reference=true;writeState(initial);((void(*)())0x4a4370)();auto actualClear=observed();need(!std::memcmp(&actualClear,&cleared,sizeof(State)),"Invalidation differs");
#endif
  ++cases;
 }
#ifdef MNM_ORIGINAL_REFERENCE
 need(Bytes((unsigned char*)0x4a4380,(unsigned char*)0x4a44ae)==body&&Bytes((unsigned char*)0x4a4370,(unsigned char*)0x4a437b)==invalid,"Original helper bodies changed");
#else
 (void)addresses;
#endif
 need(waits&&direct&&deferred,"Missing helper branch");std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"waits\":"<<waits<<",\"direct\":"<<direct<<",\"deferred\":"<<deferred<<",\"checksum\":"<<checksum<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
