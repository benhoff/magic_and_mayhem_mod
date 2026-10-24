#include "../reconstruction/rendering/alternate_canvas_dispatch.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction;
using U=std::uint32_t;
using Planes=std::array<std::vector<std::uint16_t>,3>;
static Planes* planes;
static AlternateDispatch input;
static DispatchState current;
static bool missing,originalMode;
static U unlockResult;
static std::vector<std::array<U,11>> events;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static U index(DispatchSpan span){for(U i=0;i<3;++i)if(span.words==&(*planes)[i])return i+1;throw std::runtime_error("Unknown backing");}
static std::array<U,10> stateWords(const DispatchState& s){return {index(s.binding.span),U(s.binding.span.at),s.binding.stride,s.binding.height,s.clip.left,s.clip.top,s.clip.right,s.clip.bottom,0,0};}
#ifdef MNM_ORIGINAL_REFERENCE
static U pointer(DispatchSpan span){return U(reinterpret_cast<std::uintptr_t>(span.words->data()+span.at));}
static DispatchSpan locate(U p){for(auto& v:*planes){auto start=reinterpret_cast<std::uintptr_t>(v.data());if(p>=start&&p-start<=v.size()*2&&!(p&1))return {&v,(p-start)/2};}throw std::runtime_error("Unowned original pointer");}
static DispatchState originalState(){return {{locate(*reinterpret_cast<U*>(0x658174)),*reinterpret_cast<U*>(0x6a2dc8),*reinterpret_cast<U*>(0x6def50)},
 {*reinterpret_cast<U*>(0x6e0008),*reinterpret_cast<U*>(0x6cbb6c),*reinterpret_cast<U*>(0x6a49b8),*reinterpret_cast<U*>(0x656618)}};}
#endif
static DispatchState state(){
#ifdef MNM_ORIGINAL_REFERENCE
 if(originalMode)return originalState();
#endif
 return current;
}
static void event(U tag,U extra=0){const auto s=stateWords(state());std::array<U,11> row{};row[0]=tag;std::copy(s.begin(),s.end(),row.begin()+1);row[9]=extra;events.push_back(row);}
static void draw(U tag,U salt){event(tag,salt);auto s=state();auto& v=*s.binding.span.words;
 for(U y=0;y<3;++y)for(U x=0;x<5;++x){const auto at=s.binding.span.at+y*s.binding.stride+x;require(at<v.size(),"Authored writer extent");v[at]=std::uint16_t(0xa100+salt*61+x+y*7);}}
#ifdef MNM_ORIGINAL_REFERENCE
static U __attribute__((thiscall)) lockAdapter(void* self){require(U(self)==0x6a49e0,"Lock receiver");event(1);return missing?0:pointer(input.ordinary.span);}
static void __attribute__((thiscall)) worldAdapter(void*){draw(2,0);}
static void __attribute__((thiscall)) overlayAdapter(void* self,void* pixels,void* canvas){U actor=U(self);require(actor>=0x11110000&&(actor-0x11110000)%4==0&&U(pixels)==pointer(input.ordinary.span)&&U(canvas)==0x6a49e0,"Overlay arguments");draw(3,(actor-0x11110000)/4+1);}
static U __attribute__((stdcall)) unlockAdapter(void*,void* pixels){require(U(pixels)==pointer(input.ordinary.span),"Unlock pixels");event(4,unlockResult);return unlockResult;}
static void replace(U at,const unsigned char* bytes,unsigned size,void* target){require(size>=5&&!std::memcmp((void*)at,bytes,size),"Adapter bytes");auto* p=(unsigned char*)at;p[0]=0xe9;U delta=U(target)-at-5;std::memcpy(p+1,&delta,4);}
extern "C" __attribute__((naked,noinline,noipa)) void originalWindow(void*,void*){
 asm volatile("push %ebp;push %ebx;push %esi;push %edi;mov 20(%esp),%esi;mov 24(%esp),%ebp;xor %ebx,%ebx;mov $0x4fd438,%eax;call *%eax;pop %edi;pop %esi;pop %ebx;pop %ebp;ret");}
#endif
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
 require(argc==2,"Pinned image required");map_image(read(argv[1]));
 const auto body=[](U at,U n){auto* p=(unsigned char*)at;return Bytes(p,p+n);};
 const auto window=body(0x4fd438,0x177),bind=body(0x57dc20,0x18),clipBody=body(0x57dc60,0x28),copy=body(0x594508,84);
 for(auto at:{0x58b660u,0x5002a0u,0x58f9c0u}){const auto bytes=body(at,6);replace(at,bytes.data(),6,at==0x58b660?(void*)lockAdapter:at==0x5002a0?(void*)worldAdapter:(void*)overlayAdapter);}
 require(!std::memcmp((void*)0x4fd5af,"\x8b\x96\x29\x02\x00\x00",6),"Continuation signature");*(unsigned char*)0x4fd5af=0xc3;
#else
 (void)argv;require(argc==1,"Native execution consumes no image");
#endif
 U cases=0;std::uint64_t words=0,checksum=0;
 for(bool video:{false,true})for(U xp:{0u,4u,96u})for(U yp:{0u,3u})for(bool alt:{false,true})for(bool absent:{false,true})for(U overlays:{0u,1u,3u})for(U relation=0;relation<4;++relation)for(U failure:{0u,1u}){
  Planes initial{std::vector<std::uint16_t>(1000000),std::vector<std::uint16_t>(8192),std::vector<std::uint16_t>(8192)};
  for(U p=0;p<3;++p)for(std::size_t i=0;i<initial[p].size();++i)initial[p][i]=std::uint16_t(i*7919+p*431+cases*17);
  auto configure=[&](Planes& p){planes=&p;input={};input.highResolution=video;input.xPadding=xp;input.yPadding=yp;input.overlays=overlays;
   input.alternate=alt?DispatchSpan{&p[0],64}:DispatchSpan{};input.ordinary={{&p[1],relation==3?67u:64u},32,9};
   input.copySource=relation<2?DispatchSpan{&p[0],relation?128u:64u}:DispatchSpan{&p[relation==2?2:1],64};input.clip={3,1,24,8};
   current={{{&p[2],7},16,4},{1,2,20,7}};missing=absent;unlockResult=failure?0x88760096u:0;events.clear();};
  auto actual=initial;configure(actual);originalMode=false;
  const bool locked=dispatchAlternateCanvas(input,current,[]{event(1);return !missing;},[](auto&){draw(2,0);},[](auto&,U i){draw(3,i+1);},[]{event(4,unlockResult);});
  require(locked!=absent,"Lock result");const auto nativeState=stateWords(current);const auto nativeEvents=events;
#ifdef MNM_ORIGINAL_REFERENCE
  auto expected=initial;configure(expected);originalMode=true;
  const U resolution=video;std::memcpy((void*)0x6de6d5,&resolution,4);global(0x6e1f68,alt?pointer(input.alternate):0);global(0x6e1f6c,pointer(input.copySource));global(0x6e1f70,xp);global(0x6e1f74,yp);
  const U clips[]{input.clip.left,input.clip.top,input.clip.right,input.clip.bottom};global(0x6c482c,U(clips));
  global(0x658174,pointer(current.binding.span));global(0x6a2dc8,current.binding.stride);global(0x6def50,current.binding.height);
  global(0x6e0008,current.clip.left);global(0x6cbb6c,current.clip.top);global(0x6a49b8,current.clip.right);global(0x656618,current.clip.bottom);
  global(0x6a49f4,input.ordinary.height);global(0x6a49f8,input.ordinary.stride*2);
  U table[33]={};table[32]=U((void*)unlockAdapter);U face=U(table);global(0x6a49e4,U(&face));
  std::array<unsigned char,0x240> object{};std::array<U,6> queue{};std::memcpy(object.data()+0x1e5,&overlays,4);
  for(U i=0;i<overlays;++i){U actor=0x11110000+i*4;std::memcpy(object.data()+0xf5+i*4,&actor,4);}
  originalWindow(object.data(),queue.data());
  require(actual==expected,"Complete original/native backing pixels");require(nativeState==stateWords(originalState()),"Final binding/clip");
  require(nativeEvents==events,"Original/native ordered calls and observed state");
#else
  // Native-only controls still check exact event order and owned pixel guards.
  std::vector<U> tags;for(const auto& e:events)tags.push_back(e[0]);std::vector<U> wanted;
  if(alt){wanted.push_back(2);}
  wanted.push_back(1);if(!absent){if(!alt)wanted.push_back(2);for(U i=0;i<overlays;++i)wanted.push_back(3);wanted.push_back(4);}require(tags==wanted,"Native event ordering");
  require(actual[0].front()==initial[0].front()&&actual[1].back()==initial[1].back()&&actual[2].front()==initial[2].front(),"Native exterior guards");
#endif
  for(const auto& v:actual){words+=v.size();for(auto value:v)checksum=(checksum*131+value)&0xffffffffffffffffULL;}
  for(auto v:nativeState)checksum=checksum*131+v;
  for(const auto& e:nativeEvents)for(auto v:e)checksum=checksum*131+v;
  ++cases;
 }
 U refusals=0;Planes data{std::vector<std::uint16_t>(1000000),std::vector<std::uint16_t>(8192),std::vector<std::uint16_t>(8192)};planes=&data;
 for(U bad=0;bad<8;++bad){AlternateDispatch request;request.alternate={&data[0],64};request.copySource={&data[2],64};request.ordinary={{&data[1],64},32,9};request.clip={0,0,32,9};
  if(bad==0)request.xPadding=2049;
  if(bad==1)request.ordinary.stride=31;
  if(bad==2)request.copySource={};
  if(bad==3)request.copySource.at=8190;
  if(bad==4)request.alternate.at=999999;
  if(bad==5)request.overlays=257;
  if(bad==6)request.clip.right=0xffffffffu,request.xPadding=1;
  if(bad==7)request.ordinary.span={};
  DispatchState s={{{&data[1],4},8,2},{1,2,3,4}};const auto before=stateWords(s);const auto pixels=data;U calls=0;
  try{dispatchAlternateCanvas(request,s,[&]{++calls;return true;},[&](auto&){++calls;},[&](auto&,U){++calls;},[&]{++calls;});}catch(const std::invalid_argument&){++refusals;}
  require(stateWords(s)==before&&data==pixels&&!calls,"Atomic preflight refusal");
 }
#ifdef MNM_ORIGINAL_REFERENCE
 require(body(0x4fd438,0x177)==window&&body(0x57dc20,0x18)==bind&&body(0x57dc60,0x28)==clipBody&&body(0x594508,84)==copy,"Original contracted bodies changed");
#endif
 require(cases==1152&&refusals==8,"Composition denominator");
 std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"owned_words_checked\":"<<words<<",\"atomic_refusals\":"<<refusals<<",\"checksum\":"<<checksum<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
