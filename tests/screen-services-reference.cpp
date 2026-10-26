#include "../reconstruction/rendering/screen_services.hpp"
#include "../reconstruction/rendering/screen_tick.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using U=std::uint32_t;
using Event=mnm::reconstruction::OriginalControlEvent;
static void need(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
struct Span {U at;std::vector<unsigned char> bytes;};
struct Memory {
 std::vector<Span> spans;
 bool original=false;
 Memory(){for(auto q:std::vector<std::pair<U,U>>{{0x5e04c8,8},{0x641a70,0x1240},{0x644520,4},{0x650000,0x1000},{0x656614,4},{0x689930,8},{0x6903e8,4},{0x690438,16},{0x6a4940,8},{0x6c482c,4},{0x6cbb68,12},{0x6def58,24},{0x6df17c,8},{0x6e1ef8,24},{0x6e984c,4},{0x6f349c,0x48},{0x6f3a90,4}})spans.push_back({q.first,std::vector<unsigned char>(q.second)});}
 unsigned char* locate(U at,U n){
#ifdef MNM_ORIGINAL_REFERENCE
  if(original)return reinterpret_cast<unsigned char*>(at);
#endif
  for(auto& s:spans)if(at>=s.at && at-s.at+n<=s.bytes.size())return s.bytes.data()+at-s.at;
  throw std::runtime_error("Unadmitted wire address "+std::to_string(at));
 }
 U get(U at){U v;std::memcpy(&v,locate(at,4),4);return v;}
 void set(U at,U v){std::memcpy(locate(at,4),&v,4);}
 U byte(U at){return *locate(at,1);}void byte(U at,U v){*locate(at,1)=v;}
 void install(){for(auto& s:spans)std::memcpy(reinterpret_cast<void*>(s.at),s.bytes.data(),s.bytes.size());}
 std::uint64_t hash(){std::uint64_t h=0;for(auto& s:spans){const auto* b=locate(s.at,s.bytes.size());for(U i=0;i<s.bytes.size();++i)if(s.at+i<0x650e00 || s.at+i>=0x651000)h=h*131+b[i];}return h;}
 bool equals(Memory& other){for(auto& s:spans)if(std::memcmp(locate(s.at,s.bytes.size()),other.locate(s.at,s.bytes.size()),s.bytes.size())){for(U i=0;i<s.bytes.size();++i)if(*locate(s.at+i,1)!=*other.locate(s.at+i,1)){std::cerr<<"byte "<<std::hex<<s.at+i<<" actual "<<U(*locate(s.at+i,1))<<" expected "<<U(*other.locate(s.at+i,1))<<'\n';break;}std::cerr<<"span "<<std::hex<<s.at<<'\n';return false;}return true;}
};
static Memory* memory;
static U mutation,drains[2],waitResult,lookupResult,sequenceLength;
static bool failLoad;
static std::vector<std::array<std::uint64_t,5>> events;
static void note(U tag,U a=0,U b=0,U c=0){events.push_back({tag,a,b,c,memory->hash()});}
struct Services {
 void warning(U text){note(1,text);}
 bool peek(U lo,U hi){note(2,lo,hi);U i=lo==0x100?0:1;need(hi==(i?0x201:0x108),"Peek range");if(drains[i]){--drains[i];return true;}return false;}
 void screen(U object,U offset){
  need(object>=0x650100 && object<=0x650400,"Required screen is not live");
  if(memory->get(object)==0x5c6564){mnm::reconstruction::minimalOriginalScreenSlot(*memory,object,offset);return;}
  note(3,object,offset);
  if(offset==4 && mutation==1)memory->set(0x6f349c,0);
  if(offset==4 && mutation==2)memory->set(0x6e1efc,memory->get(0x6e1efc)+1);
  if(offset==12 && mutation==3)memory->set(0x6f34e0,0x650300);
 }
 U wait(U handle,U timeout){note(4,handle,timeout);if(mutation==4)memory->set(0x642c58,99);return waitResult;}
 void release(U handle){note(5,handle);}
 U load(U module,U id){note(6,module,id);return failLoad?0:id+0x1000;}
 void cursor(U handle){note(7,handle);}
 U hit(U object,U x,U y){note(8,object,x,y);if(mutation==5)memory->set(0x641a88,0);return memory->get(object+0x24);}
 void action(U object,U flags,const Event& e){note(9,object,flags);for(U v:e)note(10,v);if(mutation==6)memory->set(0x642c68,1);if(mutation==7)memory->set(0x641ab8+8,999);}
 U activeUnit(){note(11);return lookupResult?0x650600:0;}
 void clearUnit(U unit){note(12,unit);}
 U lookup(U player){note(13,player);return lookupResult;}
 void commandSelection(U object,U arg){note(14,object,arg);}
 U nextUnit(bool selected){note(15,selected);U i=memory->get(0x6def60);memory->set(0x6def60,i+1);return i<sequenceLength?0x650600+i*0x100:0;}
 void activateUnit(U unit){note(16,unit);}
 void notifyUnit(U id){note(17,id);}
};
struct TickState {
 Memory& m;static constexpr U owner=0x650100;
 U request(U at){return m.get(owner+at);}U self(){return owner;}
 bool fading(){return m.byte(owner+12)!=0;}void fading(bool v){m.byte(owner+12,v);}
 bool active(){return m.get(owner+8)!=0;}bool suppressed(){return m.byte(owner+18)!=0;}
 U depth(){return m.get(0x6f349c);}void depth(U v){m.set(0x6f349c,v);}
 U current(){return m.get(0x6f34e0);}void current(U v){m.set(0x6f34e0,v);}
 U stack(U i){return m.get(0x6f34a0+i*4);}void stack(U i,U v){m.set(0x6f34a0+i*4,v);}
 U counter(){return m.get(owner+13);}void counter(U v){m.set(owner+13,v);}
 U screenClass(){return m.get(0x5e04c8);}void snapshotToken(){m.set(0x6e1ef8,m.get(0x6e1efc));}
 void gate(U v){m.set(0x642c60,v);}
};
struct TickServices:Services {
 void slot(U o,U offset){screen(o,offset);}
 void call(U pc){need(pc==0x4a4370,"Inactive tick escaped transition domain");memory->set(0x642c54,0);}
 void select(U o){mnm::reconstruction::selectOriginalScreen(*memory,*this,o);}
 void special(U kind){mnm::reconstruction::selectOriginalCursor(*memory,*this,kind);}
 void fade(){throw std::runtime_error("Fade outside transition domain");}
 void present(){throw std::runtime_error("Presentation outside transition domain");}
};
#ifdef MNM_ORIGINAL_REFERENCE
static void __attribute__((thiscall)) screen0(void* o){Services{}.screen(U(o),0);}
static void __attribute__((thiscall)) screen4(void* o){Services{}.screen(U(o),4);}
static void __attribute__((thiscall)) screen8(void* o){Services{}.screen(U(o),8);}
static void __attribute__((thiscall)) screen12(void* o){Services{}.screen(U(o),12);}
static U __attribute__((thiscall)) hit(void* o,U x,U y){return Services{}.hit(U(o),x,y);}
static void __attribute__((thiscall)) action(void* o,U flags,Event e){Services{}.action(U(o),flags,e);}
static U __attribute__((stdcall)) waitAdapter(U h,U t){return Services{}.wait(h,t);}
static U __attribute__((stdcall)) releaseAdapter(U h){Services{}.release(h);return 1;}
static U __attribute__((stdcall)) loadAdapter(U m,U id){return Services{}.load(m,id);}
static U __attribute__((stdcall)) cursorAdapter(U h){Services{}.cursor(h);return 0;}
static U __attribute__((stdcall)) warningAdapter(U h,U text,U title,U flags){need(!h&&title==0x5ec020&&flags==0x42030,"Warning ABI");Services{}.warning(text);return 1;}
static U __attribute__((stdcall)) peekAdapter(void* buffer,U hwnd,U lo,U hi,U flags){need(buffer&&hwnd==0x1234&&flags==1,"Peek ABI");return Services{}.peek(lo,hi);}
static U activeAdapter(){return Services{}.activeUnit();}
static void __attribute__((thiscall)) clearAdapter(void* o){Services{}.clearUnit(U(o));}
static U __attribute__((thiscall)) lookupAdapter(void* o,U player){need(U(o)==0x643a80,"Lookup receiver");return Services{}.lookup(player);}
static void __attribute__((thiscall)) commandAdapter(void* o,U arg){Services{}.commandSelection(U(o),arg);}
static U __attribute__((thiscall)) nextSelected(void* o){need(U(o)==0x6def58,"Iterator receiver");return Services{}.nextUnit(true);}
static U __attribute__((thiscall)) nextOther(void* o){need(U(o)==0x6def58,"Iterator receiver");return Services{}.nextUnit(false);}
static void __attribute__((thiscall)) activateAdapter(void* o){Services{}.activateUnit(U(o));}
static void __attribute__((thiscall)) notifyAdapter(void* o,U kind,U index,U id){need(U(o)==0x6f3a70&&kind==7&&index==~U(0),"Notify ABI");Services{}.notifyUnit(id);}
static void write(U at,U v){std::memcpy(reinterpret_cast<void*>(at),&v,4);}
static void adapt(U at,void* fn){auto* b=reinterpret_cast<unsigned char*>(at);b[0]=0xe9;U delta=U(fn)-at-5;std::memcpy(b+1,&delta,4);}
#endif
static Memory initial(){
 Memory m;
 m.set(0x6e1efc,0x11223344);m.set(0x6e1ef8,0x55667788);m.set(0x656614,0x1234);m.set(0x642c54,77);
 m.set(0x650400,0x650f00);m.set(0x5e04c8,16);m.set(0x5e04cc,~U(0));m.set(0x641a8c,8);m.set(0x6cbb68,0x4321);
 m.set(0x6c482c,0x650900);m.set(0x650908,100);m.set(0x65090c,100);
 m.set(0x6def58,0x650600);m.set(0x644520,2);m.set(0x689934,0);m.set(0x6f3a90,1);
 for(U i=0;i<3;++i){U o=0x650100+i*0x100;m.set(o,0x650f00);m.set(o+4,1);m.set(o+8,1);m.set(o+12,i);m.byte(o+16,1);m.byte(o+0x12,0xa5);m.set(o+0x11,0);m.set(o+0x15,0);m.set(o+0x19,100);m.set(o+0x1d,100);m.set(0x642034+i*4,o);m.set(0x650600+i*0x100+0x74,i!=0);m.set(0x650600+i*0x100+0xa8,10+i);}
 for(U i=0;i<23;++i){m.set(0x642684+i*68,i*7);m.set(0x6426b8+i*68,20+i);}
 return m;
}
static std::uint64_t checksum;static U cases[4];
static void compare(Memory in,U operation,U arg=0){
 Services p;Memory expected=in;memory=&expected;events.clear();drains[0]=2;drains[1]=1;
 if(operation==0)mnm::reconstruction::selectOriginalScreen(expected,p,arg);
 if(operation==1)mnm::reconstruction::selectOriginalCursor(expected,p,arg);
 if(operation==2)mnm::reconstruction::dispatchOriginalControls(expected,p);
 if(operation==3){TickState state{expected};TickServices services;mnm::reconstruction::tickOriginalScreen(state,services);}
 const auto expectedEvents=events;
#ifdef MNM_ORIGINAL_REFERENCE
 in.install();Memory actual;actual.original=true;memory=&actual;events.clear();drains[0]=2;drains[1]=1;
 write(0x650f00,U((void*)screen0));write(0x650f04,U((void*)screen4));write(0x650f08,U((void*)screen8));write(0x650f0c,U((void*)screen12));
 // Input-control vtables occupy a separate fixture table.
 write(0x650e00,U((void*)action));write(0x650e04,U((void*)hit));
 // Tables are instrumentation, excluded from the compared storage bytes.
 for(auto& s:expected.spans)if(s.at==0x650000)std::memcpy(s.bytes.data()+0xe00,(void*)0x650e00,0x200);
 using Fn=void(__attribute__((thiscall))*)(void*);
 reinterpret_cast<Fn>(operation==0?0x5571e0:operation==1?0x4a39a0:operation==2?0x4a5070:0x5595d0)((void*)(operation==3?0x650100:arg));
 if(!actual.equals(expected)){std::cerr<<"operation "<<operation<<" case "<<cases[operation]<<" mutation "<<mutation<<'\n';throw std::runtime_error("Service state mismatch");}
 if(events!=expectedEvents){std::cerr<<"operation "<<operation<<" case "<<cases[operation]<<" mutation "<<mutation<<" events "<<events.size()<<'/'<<expectedEvents.size()<<'\n';throw std::runtime_error("Service boundary mismatch");}
#endif
 // Normalize instrumentation table bytes for cross-width checksums.
 for(U i=0;i<0x200;++i)expected.byte(0x650e00+i,0);
 checksum=checksum*131+expected.hash();for(auto e:expectedEvents)for(auto v:e)checksum=checksum*131+v;
 ++cases[operation];
}
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
 need(argc==2,"Pinned PE required");map_image(read(argv[1]));
 std::vector<std::pair<U,U>> ranges={{0x5571e0,0x557368},{0x4a39a0,0x4a3c17},{0x4a5070,0x4a5842},{0x4b9b90,0x4b9bc1},{0x5595d0,0x55981e}};
 std::vector<Bytes> before;for(auto r:ranges)before.emplace_back((unsigned char*)r.first,(unsigned char*)r.second);
 for(auto a:std::vector<std::pair<U,void*>>{{0x5075a0,(void*)activeAdapter},{0x5074f0,(void*)clearAdapter},{0x466870,(void*)lookupAdapter},{0x509bd0,(void*)commandAdapter},{0x52a710,(void*)nextSelected},{0x52a670,(void*)nextOther},{0x5073d0,(void*)activateAdapter},{0x56c060,(void*)notifyAdapter}})adapt(a.first,a.second);
 for(auto a:std::vector<std::pair<U,void*>>{{0x5c5140,(void*)waitAdapter},{0x5c513c,(void*)releaseAdapter},{0x5c5214,(void*)loadAdapter},{0x5c5200,(void*)cursorAdapter},{0x5c520c,(void*)warningAdapter},{0x5c5264,(void*)peekAdapter}})write(a.first,U(a.second));
#else
 (void)argv;need(argc==1,"Native model takes no PE");
#endif
 for(U depth:{0u,1u,2u,3u,15u,0xffffffffu})for(U target:{0x650100u,0x650200u,0x650300u,0x650400u})for(U real:{0u,1u})for(U mut:{0u,1u,2u,3u}){
  auto m=initial();m.set(0x6f349c,depth);m.set(0x6f34e0,0x650300);
  for(U i=0;i<16;++i)m.set(0x6f34a0+i*4,0x650100+(i%3)*0x100);
  if(real)for(U i=0;i<3;++i)m.set(0x650100+i*0x100,0x5c6564);
  mutation=mut;compare(m,0,target);
 }
 for(U push:{0u,0x650400u})for(U select:{0u,0x650200u})for(U pop:{0u,1u})for(U depth:{0u,1u,3u,15u})for(U current:{0u,0x650300u})for(U real:{0u,1u}){
  if(!push && select && depth && !current)continue;
  if(!push && !select && pop && depth && !current)continue;
  auto m=initial();m.set(0x650108,0);m.byte(0x65010c,0);
  m.set(0x650133,push);m.set(0x650137,select);m.set(0x650143,pop);
  m.set(0x6f349c,depth);m.set(0x6f34e0,current);
  for(U i=0;i<16;++i)m.set(0x6f34a0+i*4,0x650100+(i%3)*0x100);
  if(real)for(U i=0;i<4;++i)m.set(0x650100+i*0x100,0x5c6564);
  mutation=0;compare(m,3);
 }
 for(U kind:{0xffffffffu,0u,1u,2u,3u,4u,5u,6u,7u,8u,9u,10u,11u,12u,13u,14u,15u,16u,17u,18u,19u,20u,21u,22u,23u,0x7fffffffu})for(U custom:{0u,1u})for(U handle:{0u,7u})for(U result:{0u,0x80u,0x102u,0xffffffffu})for(U load:{0u,1u})for(U frame:{0u,19u,0xfffffffdu})for(U columns:{3u,0xfffffffdu})for(U mut:{0u,4u}){
  auto m=initial();m.set(0x642c64,custom);m.set(0x642c58,handle);m.set(0x641a8c,columns);
  if(kind<23)m.set(0x642684+kind*68,frame);
  mutation=mut;waitResult=result;failLoad=load;compare(m,1,kind);
 }
 failLoad=false;waitResult=0;
 for(U q:{0u,1u,2u,31u})for(U count:{0u,1u,3u})for(U captured:{0u,1u})for(U previous:{0u,2u,0xffffffffu})for(U world:{0u,1u})for(U mode:{0u,3u})for(U buttons=0;buttons<16;++buttons)for(U mut:{0u,5u,6u,7u}){
  auto m=initial();m.set(0x6f34e0,0x650500);m.set(0x650504,world?2:1);m.set(0x6903e8,mode);m.set(0x642c68,count);m.set(0x642c70,0);m.byte(0x642c6c,captured);m.set(0x5e04cc,previous<count?previous:~U(0));m.set(0x641a74,q);m.set(0x641a88,q==0?0:q==31?1:q+1);m.set(0x641fbc,q);
  for(U i=0;i<3;++i){m.set(0x650100+i*0x100,0x650e00);m.set(0x650100+i*0x100+0x24,i==2);}
  for(U i=0;i<32;++i){Event e={U(i%2?30:120),U(i%3?20:120),1,buttons&1,(buttons>>1)&1,(buttons>>2)&1,(buttons>>3)&1,U(i%2),U(i%3),0};for(U j=0;j<10;++j)m.set(0x641ab8+i*40+j*4,e[j]);}
  mutation=mut;lookupResult=buttons&1?0x650800:0;sequenceLength=buttons%4;compare(m,2);
 }
#ifdef MNM_ORIGINAL_REFERENCE
 for(U i=0;i<ranges.size();++i)need(Bytes((unsigned char*)ranges[i].first,(unsigned char*)ranges[i].second)==before[i],"Recovered service body changed");
#endif
 std::cout<<"{\"success\":true,\"selection_cases\":"<<cases[0]<<",\"cursor_cases\":"<<cases[1]<<",\"control_cases\":"<<cases[2]<<",\"tick_cases\":"<<cases[3]<<",\"checksum\":"<<checksum<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
