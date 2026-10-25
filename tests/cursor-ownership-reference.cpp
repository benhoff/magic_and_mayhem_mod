// Original client/queue bodies unchanged. Scripted OS/COM endpoints; original
// copy wrappers are shared external services, not independent pixel evidence.
#define main cursor_unused_surface_main
#include "surface-retry-reference.cpp"
#undef main
#include "../reconstruction/rendering/deferred_surface_copy.hpp"
#include <algorithm>
namespace C=mnm::reconstruction::cursor;using R=C::Rectangle;
// Value-address DWORDs include the original unaligned resolution global.
// Copy bytes rather than binding host references to those addresses.
struct Word {
 U at;
 operator U()const{U v;std::memcpy(&v,reinterpret_cast<void*>(at),4);return v;}
 Word& operator=(U v){std::memcpy(reinterpret_cast<void*>(at),&v,4);return *this;}
 Word& operator^=(U v){return *this=U(*this)^v;}
};
static Word mem(U at){return {at};}
static std::vector<std::vector<U>> trace;
static U px,py,originX,originY,outer,inner,mutation,statusResult,apiResult;
static U statusMutation;
static void check(bool b,const char* why){if(!b)throw std::runtime_error(why);}
static void note(std::initializer_list<U> args){
 check(trace.size()<1024,"Endpoint budget");trace.emplace_back(args);
 for(U at:{0x642c54u,0x641aa0u,0x641aa4u,0x641fc0u,0x642c5cu,0x642c58u,0x6f98d4u,0x6f98d8u})trace.back().push_back(mem(at));
}
static void mutate(){
 if(mutation==1)mem(0x642c58)=0x2222;
 if(mutation==2)mem(0x642c54)=0;
 if(mutation==3){mem(0x641a90)=80;mem(0x641a98)=120;}
 if(mutation==4)mem(0x6e1ee4)^=1;
 if(mutation==5){mem(0x641a78)=5;mem(0x641a80)=9;mem(0x641aa0)=U(-2);}
}
static U __attribute__((stdcall)) ownedWait(U h,U t){note({1,h,t});mutate();return t==10?outer:inner;}
static U __attribute__((stdcall)) ownedRelease(U h){note({2,h});mutate();return apiResult;}
static U __attribute__((stdcall)) ownedClose(U h){note({3,h});return apiResult;}
static U __attribute__((stdcall)) ownedCursor(U* point){note({4});point[0]=px;point[1]=py;return apiResult;}
static U __attribute__((stdcall)) ownedTransform(U window,U* point){note({5,window,point[0],point[1]});point[0]-=originX;point[1]-=originY;return apiResult;}
static U __attribute__((stdcall)) ownedClient(U window,U* rect){note({6,window});rect[0]=originX;rect[1]=originY;rect[2]=originX+640;rect[3]=originY+480;return apiResult;}
static U __attribute__((stdcall)) ownedIntersect(U* dest,const U* a,const U* b){
 note({7,a[0],a[1],a[2],a[3],b[0],b[1],b[2],b[3]});
 R v={C::maximum(a[0],b[0]),C::maximum(a[1],b[1]),C::minimum(a[2],b[2]),C::minimum(a[3],b[3])};
 if(!C::less(v[0],v[2])||!C::less(v[1],v[3]))v={0,0,0,0};
 std::copy(v.begin(),v.end(),dest);return v==R{}?0:1;
}
static U __attribute__((stdcall)) ownedStatus(Interface* self,U flags){note({8,self->id,flags});if(statusMutation)mem(0x642008)=reinterpret_cast<U>(self);return statusResult;}
static U __attribute__((stdcall)) ownedRef(Interface* self){note({9,self->id});return 0;}
static I __attribute__((stdcall)) ownedBlt(Interface* self,Rect* dest,Interface* source,Rect* src,U flags,void* fx){
 check(dest&&source&&src&&!fx,"Owned blit args");
 note({10,self->id,source->id,U(dest->l),U(dest->t),U(dest->r),U(dest->b),U(src->l),U(src->t),U(src->r),U(src->b),flags});return 0;
}
static I __attribute__((stdcall)) ownedFast(Interface* self,U x,U y,Interface* source,Rect* src,U flags){check(source&&src,"Owned fast args");note({11,self->id,source->id,x,y,U(src->l),U(src->t),U(src->r),U(src->b),flags});return 0;}
struct Memory{U get32(U at){return mem(at);}void set32(U at,U v){mem(at)=v;}};
struct Services{
 bool model=false;
 U wait(U h,U t){return ownedWait(h,t);}void release(U h){ownedRelease(h);}void close(U h){ownedClose(h);}
 std::array<U,2> cursorPosition(){std::array<U,2> v{};ownedCursor(v.data());return v;}
 void screenToClient(U window,std::array<U,2>& v){ownedTransform(window,v.data());}
 R clientRectangle(U window){R v{};ownedClient(window,v.data());return v;}
 R intersect(const R& a,const R& b){R v{};ownedIntersect(v.data(),a.data(),b.data());return v;}
 void copy(U method,U source,U dest,const R& v,U x,U y){
  if(method==0x58c140){using F=void(__attribute__((thiscall))*)(U,U,U,U);reinterpret_cast<F>(method)(source,dest,x,y);}
  else{using F=void(__attribute__((thiscall))*)(U,U,const U*,U,U);reinterpret_cast<F>(method)(source,dest,v.data(),x,y);}
 }
 void queue(U source,U kind,U dest,const R& v,U x,U y,U priority){
  if(model){Memory m;C::DeferredCopies q(m,*this);q.enqueue(source,kind,dest,&v,x,y,priority);}
  else{using F=void(__attribute__((thiscall))*)(U,U,U,const U*,U,U,U);reinterpret_cast<F>(0x58d5b0)(source,kind,dest,v.data(),x,y,priority);}
 }
 U status(U iface,U flags){return ownedStatus(reinterpret_cast<Interface*>(iface),flags);}
 void dispose(U wrapper){using F=void(__attribute__((thiscall))*)(U);reinterpret_cast<F>(0x58b3e0)(wrapper);}
};
static std::array<void*,40> vtable;static std::array<Interface,5> objects;
static std::vector<std::pair<U,U>> state(){
 std::vector<std::pair<U,U>> v;
 for(auto span:{std::pair<U,U>{0x641a78,0x641ab8},{0x641fc0,0x642038},{0x642678,0x642c68},{0x6a49e0,0x6a4a18},{0x6a4fe0,0x6a5018},{0x6f6b50,0x6f8150},{0x6f98c0,0x6f98dc}})
  for(U at=span.first;at<span.second;at+=4)v.emplace_back(at,mem(at));
 for(U at:{0x6e1ec0u,0x6e1ee4u,0x6c5038u,0x5e04c8u,0x6f68d8u,0x6de6d5u})v.emplace_back(at,mem(at));
 return v;
}
static void restore(const std::vector<std::pair<U,U>>& v){for(auto [at,value]:v)mem(at)=value;}
static void reset(){
 trace.clear();for(auto [at,v]:state()){(void)v;mem(at)=0;}
 mem(0x642c58)=0x1111;mem(0x6e1ec0)=0x3333;mem(0x656614)=0x4444;
 mem(0x642c64)=1;mem(0x642c60)=1;mem(0x6c5038)=1;
 mem(0x641a8c)=4;mem(0x641a98)=40;mem(0x641a9c)=40;
 mem(0x641aa0)=17;mem(0x641aa4)=19;mem(0x641a80)=40;mem(0x641a84)=40;
 for(U type=0;type<22;++type){U d=0x642678+type*68;mem(d)=3;mem(d+4)=5;mem(d+8)=3;for(U f=0;f<13;++f)mem(d+12+f*4)=f;mem(d+64)=2;}
 U index=0;for(U at:{0x641fc8u,0x642000u,0x6a49e0u,0x6a4fe0u}){
  for(U off=0;off<56;off+=4)mem(at+off)=0;
  mem(at+4)=reinterpret_cast<U>(&objects[index]);mem(at+8)=reinterpret_cast<U>(&objects[index++]);mem(at+28)=127;mem(at+20)=800;mem(at+24)=600;
 }
 mem(0x6e1f1c)=0;mem(0x6f8154)=4;
 outer=inner=statusResult=mutation=statusMutation=0;apiResult=1;originX=originY=0;px=100;py=120;
}
static void drain(bool model,Memory& m,Services& p){if(model){C::DeferredCopies q(m,p);q.drain();}else reinterpret_cast<void(*)()>(0x58d660)();}
int main(int argc,char** argv)try{
 check(argc==2,"Expected pinned PE");mapImage(argv[1]);
 vtable.fill(nullptr);vtable[2]=reinterpret_cast<void*>(&ownedRef);vtable[5]=reinterpret_cast<void*>(&ownedBlt);vtable[7]=reinterpret_cast<void*>(&ownedFast);vtable[13]=reinterpret_cast<void*>(&ownedStatus);
 for(U i=0;i<objects.size();++i)objects[i]={vtable.data(),i+1};
 for(auto [at,fn]:std::initializer_list<std::pair<U,void*>>{{0x5c5140,reinterpret_cast<void*>(&ownedWait)},{0x5c513c,reinterpret_cast<void*>(&ownedRelease)},{0x5c50a0,reinterpret_cast<void*>(&ownedClose)},{0x5c5210,reinterpret_cast<void*>(&ownedCursor)},{0x5c5298,reinterpret_cast<void*>(&ownedTransform)},{0x5c51fc,reinterpret_cast<void*>(&ownedClient)},{0x5c5294,reinterpret_cast<void*>(&ownedIntersect)}})mem(at)=reinterpret_cast<U>(fn);
 std::vector<std::pair<U,std::vector<unsigned char>>> code;
 for(auto [at,n]:std::initializer_list<std::pair<U,U>>{{0x4a45b0,849},{0x4a4910,1234},{0x4a37d0,46},{0x58d5b0,138},{0x58d660,227}}){auto* p=reinterpret_cast<unsigned char*>(at);code.emplace_back(at,std::vector<unsigned char>(p,p+n));}
 Memory m;Services p;U clients=0,queues=0,lifetime=0,retries=0;
 for(U timer=0;timer<2;++timer)for(U scenario=0;scenario<24;++scenario)for(U position=0;position<9;++position)for(U large=0;large<2;++large)for(U deferred=0;deferred<2;++deferred)for(U change=0;change<6;++change){
  reset();mutation=change;mem(0x6de6d5)=large;mem(0x6e1ee4)=deferred;
  static const U positions[9]={U(-50),U(-10),0,3,100,639,799,850,0x7ffffff0u};px=positions[position];py=position==8?U(-50):positions[(position+2)%8];
  if(scenario==1)mem(0x642c64)=0;
  if(scenario==2)mem(0x6c5038)=0;
  if(scenario==3)mem(0x5e04c8)=22;
  if(scenario>=4&&scenario<=6)outer=scenario==4?0x80:scenario==5?0x102:~0u;
  if(scenario==7)mem(0x642c60)=0;
  if(scenario==8)mem(0x642c54)=1;
  if(scenario>=9&&scenario<=11)inner=scenario==9?0x80:scenario==10?0x102:~0u;
  if(scenario==12)mem(0x641fc0)=2;
  if(scenario==13)mem(0x641fc0)=1;
  if(scenario==14)mem(0x641fc0)=~0u;
  if(scenario==15){mem(0x642680)=1;mem(0x641aa0)=px-3;mem(0x641aa4)=py-5;}
  if(scenario==16){originX=13;originY=17;}
  if(scenario==17)mem(0x6f68d8)=1;
  if(scenario==18)apiResult=0;
  if(scenario==19){mem(0x642c5c)=2;mem(0x6426b8)=~0u;}
  if(scenario==20){mem(0x642680)=1;mem(0x641aa0)=px-3;mem(0x641aa4)=py-5;mem(0x641fc0)=5;}
  if(scenario==21){mem(0x642c54)=1;mem(0x641a80)=0;}
  if(scenario==22){mem(0x5e04c8)=21;mem(0x642678+21*68+8)=1;}
  if(scenario==23){mem(0x641a8c)=U(-4);mem(0x642688)=U(-5);}
  auto before=state();trace.clear();p.model=false;reinterpret_cast<void(*)()>(timer?0x4a4910:0x4a45b0)();drain(false,m,p);auto expected=state();auto calls=trace;
  restore(before);trace.clear();p.model=true;C::Clients client(m,p);if(timer)client.timer();else client.foreground();drain(true,m,p);
  if(expected!=state()||calls!=trace){
   std::cerr<<"client mismatch "<<timer<<' '<<scenario<<' '<<position<<' '<<large<<' '<<deferred<<' '<<change<<'\n';
   auto actual=state();for(size_t i=0;i<expected.size();++i)if(expected[i]!=actual[i])std::cerr<<std::hex<<expected[i].first<<": "<<expected[i].second<<" != "<<actual[i].second<<std::dec<<'\n';
   for(auto& row:calls){std::cerr<<"original ";for(auto v:row)std::cerr<<std::hex<<v<<' ';std::cerr<<'\n';}
   for(auto& row:trace){std::cerr<<"model ";for(auto v:row)std::cerr<<std::hex<<v<<' ';std::cerr<<'\n';}
   throw std::runtime_error("Client control/state/endpoint mismatch");
  }++clients;
 }
 for(U count:{0u,1u,63u,127u,128u})for(U kind:{0u,1u,2u,3u})for(U priority:{0u,1u,2u,3u})for(U status:{0u,0x8876023fu,0x8876021cu,~0u})for(U mode=0;mode<4;++mode){
  reset();statusResult=status;R rect={1,2,6,7};
  for(U i=0;i<count;++i){mem(0x6f6b50+i*44)=1;mem(0x6f6b54+i*44)=3;mem(0x6f6b5c+i*44)=0x6a49e0;}
  mem(0x6f98d4)=count;for(U i=0;i<4;++i)mem(0x6f6b50+std::min(count,127u)*44+20+i*4)=rect[i];
  auto before=state();std::vector<std::pair<U,U>> expected;std::vector<std::vector<U>> calls;
  for(U model=0;model<2;++model){restore(before);trace.clear();p.model=model;C::DeferredCopies q(m,p);
   if(model)q.enqueue(0x642000,kind,0x6a49e0,mode==1?nullptr:&rect,3,4,priority);
   else{using F=void(__attribute__((thiscall))*)(U,U,U,const U*,U,U,U);reinterpret_cast<F>(0x58d5b0)(0x642000,kind,0x6a49e0,mode==1?nullptr:rect.data(),3,4,priority);}
   if(mode==2)mem(0x642008)=reinterpret_cast<U>(&objects[4]);
   if(mode==3)mem(0x6a49e8)=reinterpret_cast<U>(&objects[4]);
   drain(model,m,p);
   if(!model){expected=state();calls=trace;}else check(expected==state()&&calls==trace,"Queue state/endpoint mismatch");
  }++queues;
 }
 // Sparse holes, rectangle source destruction, ordered priorities and busy retry.
 for(U change=0;change<3;++change)for(U fastMode=0;fastMode<2;++fastMode){
  reset();mem(0x6f68d8)=fastMode;
  auto before=state();std::vector<std::pair<U,U>> expected;std::vector<std::vector<U>> calls;
  for(U model=0;model<2;++model){
   restore(before);trace.clear();p.model=model;C::DeferredCopies q(m,p);
   for(U slot:{1u,50u,127u}){
    U at=0x6f6b50+slot*44;mem(at)=1;mem(at+4)=slot==1?2:slot==50?0:1;
    mem(at+8)=0x642000;mem(at+12)=0x6a49e0;mem(at+16)=2;
    mem(at+20)=1;mem(at+24)=2;mem(at+28)=6;mem(at+32)=7;mem(at+36)=3;mem(at+40)=4;
   }
   mem(0x6f98d4)=3;R rectangle={2,3,7,8};
   p.queue(0x642000,0,0x6a49e0,rectangle,4,5,1);rectangle.fill(~0u);
   check(mem(0x6f6b64)==2&&mem(0x6f6b68)==3,"Queue failed to own rectangle values");
   if(change==1)mem(0x642008)=reinterpret_cast<U>(&objects[4]);
   statusMutation=change==2;statusResult=0x8876021c;drain(model,m,p);
   check(mem(0x6f98d4)==4,"Busy drain retired borrowed records");
   statusResult=0;drain(model,m,p);check(!mem(0x6f98d4),"Retry failed to drain");
   if(!model){expected=state();calls=trace;}else check(expected==state()&&calls==trace,"Sparse/retry/rebind mismatch");
  }++retries;
 }
 for(U pending:{0u,1u,128u})for(U handle:{0u,0x1111u})for(U result:{0u,1u}){
  reset();apiResult=result;mem(0x642c58)=handle;mem(0x6f98d4)=pending;
  for(U i=0;i<pending;++i){mem(0x6f6b50+i*44)=1;mem(0x6f6b58+i*44)=0x642000;mem(0x6f6b5c+i*44)=0x641fc8;}
  auto before=state();trace.clear();reinterpret_cast<void(*)()>(0x4a37d0)();auto expected=state();auto calls=trace;
  restore(before);mem(0x6f8154)=4;trace.clear();C::Clients client(m,p);client.dispose();
  check(expected==state()&&calls==trace,"Dispose caller mismatch");check(!mem(0x642008)&&!mem(0x641fd0)&&mem(0x6f98d4)==pending,"Disposal does not pin/drain deferred work");++lifetime;
 }
 for(auto& [at,bytes]:code)check(std::equal(bytes.begin(),bytes.end(),reinterpret_cast<unsigned char*>(at)),"Original instructions changed");
 std::cout<<"{\"success\":true,\"client_cases\":"<<clients<<",\"queue_cases\":"<<queues<<",\"dispose_cases\":"<<lifetime<<",\"retry_cases\":"<<retries<<",\"original_instructions_unchanged\":true}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
