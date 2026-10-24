#include "../reconstruction/rendering/screen_tick.hpp"
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
using U=std::uint32_t;
using Raw=std::array<unsigned char,96>;
static void need(bool b,const char* m){if(!b)throw std::runtime_error(m);}
static U word(const Raw& b,U at){U v;std::memcpy(&v,b.data()+at,4);return v;}
static void put(Raw& b,U at,U v){std::memcpy(b.data()+at,&v,4);}
struct Data{
 std::array<Raw,3> objects{};std::array<U,16> stack{};
 U depth{},current{},tokenSource=0x11223344,tokenSnapshot=0x55667788,hwnd=0x1234,gate=7,screen{};
 bool operator==(const Data& v)const{return objects==v.objects&&stack==v.stack&&depth==v.depth&&current==v.current&&tokenSource==v.tokenSource&&tokenSnapshot==v.tokenSnapshot&&hwnd==v.hwnd&&gate==v.gate&&screen==v.screen;}
};
static Data data;static bool reference;static U pending[2],mutation;
static std::vector<std::array<U,10>> events;
#ifdef MNM_ORIGINAL_REFERENCE
static U objects[3];
static U token(U pointer){if(!pointer)return 0;for(U i=0;i<3;++i)if(objects[i]==pointer)return i+1;throw std::runtime_error("Unowned screen");}
static U pointer(U id){need(id<=3,"Bad screen identity");return id?objects[id-1]:0;}
static U mem(U at){U v;std::memcpy(&v,(void*)at,4);return v;}
static void write(U at,U v){std::memcpy((void*)at,&v,4);}
static Data observed(){Data d;for(U i=0;i<3;++i){std::memcpy(d.objects[i].data(),(void*)objects[i],96);put(d.objects[i],0,0);for(U at:{0x33u,0x37u,0x43u})put(d.objects[i],at,token(word(d.objects[i],at)));}
 for(U i=0;i<16;++i){d.stack[i]=token(mem(0x6f34a0+4*i));}d.depth=mem(0x6f349c);d.current=token(mem(0x6f34e0));d.tokenSource=mem(0x6e1efc);d.tokenSnapshot=mem(0x6e1ef8);d.hwnd=mem(0x656614);d.gate=mem(0x642c60);d.screen=mem(0x5e04c8);return d;}
#endif
static Data state(){
#ifdef MNM_ORIGINAL_REFERENCE
 if(reference)return observed();
#endif
 return data;
}
static void change(U at,U value,bool byte=false){
#ifdef MNM_ORIGINAL_REFERENCE
 if(reference){if(byte)*(unsigned char*)(objects[0]+at)=value;else write(objects[0]+at,value);return;}
#endif
 if(byte)data.objects[0][at]=value;else put(data.objects[0],at,value);
}
static void emit(U tag,U a=0,U b=0){auto s=state();events.push_back({tag,a,b,s.depth,s.current,s.tokenSnapshot,s.gate,s.objects[0][12],word(s.objects[0],13),word(s.objects[0],0x33)});}
static void slot(U id,U offset){emit(1,id,offset);if(offset==40&&mutation==2)change(13,9);}
static void call(U address){emit(2,address);}
static void special(U v){emit(3,v);if(mutation==1){change(12,1,true);change(13,2);}}
static void fade(){emit(4);if(mutation==3)change(13,5);}
static U peek(U lo,U hi){emit(5,lo,hi);U i=lo==0x100?0:1;need(hi==(i?0x201:0x108),"Peek range");if(pending[i]){--pending[i];return 1;}return 0;}
static void warning(U text){emit(6,text);}
struct State{
 U request(U at)const{return word(data.objects[0],at);}U self()const{return 1;}
 bool fading()const{return data.objects[0][12]!=0;}void fading(bool b){data.objects[0][12]=b;}
 bool active()const{return word(data.objects[0],8)!=0;}bool suppressed()const{return data.objects[0][18]!=0;}
 U depth()const{return data.depth;}void depth(U v){data.depth=v;}
 U current()const{return data.current;}void current(U v){data.current=v;}
 U stack(U i)const{need(i<16,"Stack read");return data.stack[i];}void stack(U i,U v){need(i<16,"Stack write");data.stack[i]=v;}
 U counter()const{return word(data.objects[0],13);}void counter(U v){put(data.objects[0],13,v);}
 U screenClass()const{return data.screen;}void snapshotToken(){data.tokenSnapshot=data.tokenSource;}void gate(U v){data.gate=v;}
};
struct Services{
 void slot(U id,U offset){::slot(id,offset);}void call(U pc){::call(pc);}void special(U v){::special(v);}void fade(){::fade();}
 bool peek(U lo,U hi){return ::peek(lo,hi)!=0;}void warning(U text){::warning(text);}void select(U id){emit(7,id);}
 void present(){emit(8,0x6a49e0,0x6a4fe0);}
};
#ifdef MNM_ORIGINAL_REFERENCE
static void __attribute__((thiscall)) s0(void* p){slot(token(U(p)),0);}static void __attribute__((thiscall)) s4(void* p){slot(token(U(p)),4);}
static void __attribute__((thiscall)) s8(void* p){slot(token(U(p)),8);}static void __attribute__((thiscall)) sc(void* p){slot(token(U(p)),12);}
static void __attribute__((thiscall)) s28(void* p){slot(token(U(p)),40);}static void __attribute__((thiscall)) s2c(void* p){slot(token(U(p)),44);}
static void __attribute__((thiscall)) h0(void*){call(0x4a4370);}static void __attribute__((thiscall)) h1(void*){call(0x4a45b0);}
static void __attribute__((thiscall)) h2(void*){call(0x4a4380);}static void __attribute__((thiscall)) h3(void*){call(0x4a5070);}
static void __attribute__((thiscall)) selectAdapter(void* p){emit(7,token(U(p)));}
static void __attribute__((thiscall)) specialAdapter(void* p){special(U(p));}
static void __attribute__((thiscall)) fadeAdapter(void* p){need(U(p)==0x6a49e0,"Fade receiver");fade();}
static void __attribute__((thiscall)) presentAdapter(void* p,U dest,U x,U y){need(U(p)==0x6a49e0&&dest==0x6a4fe0&&!x&&!y,"Presentation arguments");emit(8,U(p),dest);}
static U __attribute__((stdcall)) peekAdapter(void* buffer,U hwnd,U lo,U hi,U flags){need(buffer&&hwnd==0x1234&&flags==1,"Peek args");std::memset(buffer,0x5a,28);return peek(lo,hi);}
static U __attribute__((stdcall)) warningAdapter(U hwnd,U text,U title,U flags){need(!hwnd&&title==0x5ec020&&flags==0x42030,"Warning arguments");warning(text);return 1;}
static void adapt(U at,void* p){auto* bytes=(unsigned char*)at;bytes[0]=0xe9;U delta=U(p)-at-5;std::memcpy(bytes+1,&delta,4);}
static std::array<U,12> table{};
static void install(const Data& d){static std::array<Raw,3> storage;
 for(U i=0;i<3;++i)objects[i]=U(storage[i].data());
 for(U i=0;i<3;++i){storage[i]=d.objects[i];put(storage[i],0,U(table.data()));for(U at:{0x33u,0x37u,0x43u})put(storage[i],at,pointer(word(storage[i],at)));}
 for(U i=0;i<16;++i){write(0x6f34a0+4*i,pointer(d.stack[i]));}write(0x6f349c,d.depth);write(0x6f34e0,pointer(d.current));write(0x6e1efc,d.tokenSource);write(0x6e1ef8,d.tokenSnapshot);write(0x656614,d.hwnd);write(0x642c60,d.gate);write(0x5e04c8,d.screen);
}
#endif
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
 need(argc==2,"Pinned image required");map_image(read(argv[1]));Bytes body((unsigned char*)0x5595d0,(unsigned char*)0x55981e);
 for(auto entry:{std::pair<U,void*>{0x4a4370,(void*)h0},{0x4a45b0,(void*)h1},{0x4a4380,(void*)h2},{0x4a5070,(void*)h3},{0x5571e0,(void*)selectAdapter},{0x4a39a0,(void*)specialAdapter},{0x58ed80,(void*)fadeAdapter},{0x58bf40,(void*)presentAdapter}})adapt(entry.first,entry.second);
 write(0x5c5264,U((void*)peekAdapter));write(0x5c520c,U((void*)warningAdapter));
 table[0]=U((void*)s0);table[1]=U((void*)s4);table[2]=U((void*)s8);table[3]=U((void*)sc);table[10]=U((void*)s28);table[11]=U((void*)s2c);
#else
 (void)argv;need(argc==1,"Native model takes no image");
#endif
 U count=0;std::uint64_t checksum=0;
 for(U push:{0u,2u})for(U selection:{0u,3u})for(U pop:{0u,2u})for(U fading:{0u,1u})for(U counter:{0u,1u,4u,5u,0x7fffffffu,0xffffffffu})for(U active:{0u,1u})for(U suppressed:{0u,1u})for(U depth:{0u,1u,14u,15u})for(U current:{0u,1u})for(U back:{0u,3u})for(U screen:{0u,22u})for(U drains:{0u,2u})for(U mut:{0u,1u,2u,3u}){
  // Pop dereferences current unconditionally; authored input admits only valid
  // required screens. Negative-depth push would index outside the owned stack.
  if(pop&&!fading&&!push&&!selection&&depth>0&&!current)continue;
  Data initial;for(auto& o:initial.objects){o.fill(0xa5);put(o,0,0);put(o,0x33,0);put(o,0x37,0);put(o,0x43,0);}
  put(initial.objects[0],8,active);initial.objects[0][12]=fading;put(initial.objects[0],13,counter);initial.objects[0][18]=suppressed;
  put(initial.objects[0],0x33,push);put(initial.objects[0],0x37,selection);put(initial.objects[0],0x43,pop);
  initial.stack.fill(back);initial.depth=depth;initial.current=current;initial.screen=screen;mutation=mut;
  data=initial;reference=false;pending[0]=drains;pending[1]=drains/2;events.clear();State s;Services p;mnm::reconstruction::tickOriginalScreen(s,p);auto expected=data;auto expectedEvents=events;
#ifdef MNM_ORIGINAL_REFERENCE
  install(initial);reference=true;pending[0]=drains;pending[1]=drains/2;events.clear();using Fn=void(__attribute__((thiscall))*)(void*);reinterpret_cast<Fn>(0x5595d0)((void*)objects[0]);
  need(observed()==expected,"Complete caller state mismatch");need(events==expectedEvents,"Complete caller ordered boundary mismatch");
#endif
  for(const auto& event:expectedEvents)for(U v:event)checksum=checksum*131+v;
  for(const auto& object:expected.objects)for(auto v:object)checksum=checksum*131+v;
  ++count;
 }
#ifdef MNM_ORIGINAL_REFERENCE
 need(Bytes((unsigned char*)0x5595d0,(unsigned char*)0x55981e)==body,"Original whole caller changed");
#endif
 std::cout<<"{\"success\":true,\"cases\":"<<count<<",\"checksum\":"<<checksum<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
