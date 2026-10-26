#pragma once
#include <array>
#include <cstdint>
#include <stdexcept>
namespace mnm::reconstruction::cursor {
using U=std::uint32_t;using Rectangle=std::array<U,4>;
inline std::int64_t signedWord(U v){return v<0x80000000u?v:std::int64_t(v)-0x100000000LL;}
inline bool less(U a,U b){return signedWord(a)<signedWord(b);}
inline U maximum(U a,U b){return less(a,b)?b:a;}
inline U minimum(U a,U b){return less(a,b)?a:b;}
inline U positive(U v){return maximum(v,0);}
// NoCD value-address memory. OS/copy services may mutate memory synchronously.
// Resource addresses are borrowed; no ownership or synchronization is invented.
template<class Memory,class Services> class Clients {
 Memory& m;Services& p;
 U r(U a){return m.get32(a);}void w(U a,U v){m.set32(a,v);}
 Rectangle rect(U a){return {r(a),r(a+4),r(a+8),r(a+12)};}
 void rect(U a,const Rectangle& v){for(U i=0;i<4;++i)w(a+4*i,v[i]);}
 U width(){return r(0x6de6d5)?800:640;}U height(){return r(0x6de6d5)?600:480;}
 U descriptor(){return 0x642678+r(0x5e04c8)*68;}
 void animate(){
  if(less(0,r(0x641fc0)))return;
  U type=r(0x5e04c8),d=0x642678+type*68,count=r(d+8);
  if(count==1||type==22)return;
  U frame=r(0x642c5c)+1;if(!less(frame,count))frame=0;w(0x642c5c,frame);
  U divisor=r(0x641a8c),image=r(d+12+frame*4);
  if(!divisor||(image==0x80000000u&&divisor==0xffffffffu))throw std::domain_error("Original cursor IDIV fault");
  auto q=signedWord(image)/signedWord(divisor),rem=signedWord(image)%signedWord(divisor);
  U x=U(rem)*40,y=U(q)*40;w(0x641fc0,r(d+64));rect(0x641a90,{x,y,x+40,y+40});
 }
 Rectangle crop(U x,U y,U bx,U by){
  auto v=rect(0x641a90);U dx=x-bx+40,dy=y-by+40;
  if(less(0,dx))v[2]=maximum(v[0],v[2]-dx);
  if(less(0,dy))v[3]=maximum(v[1],v[3]-dy);
  if(less(x,0))v[0]=minimum(v[2],v[0]-x);
  if(less(y,0))v[1]=minimum(v[3],v[1]-y);
  return v;
 }
public:
 Clients(Memory& memory,Services& services):m(memory),p(services){}
 // 004a45b0. Save-under protected; sprite copy/publication after release.
 void foreground(){
  if(!r(0x642c64)||!r(0x6c5038)||r(0x5e04c8)==22)return;
  U bx=width(),by=height();auto point=p.cursorPosition();p.screenToClient(r(0x656614),point);
  U d=descriptor(),x=point[0]-r(d),y=point[1]-r(d+4);
  if(p.wait(r(0x642c58),10))return;
  rect(0x641a78,p.intersect({x,y,x+40,y+40},{0,0,bx,by}));auto saved=rect(0x641a78);
  if(saved[0]!=saved[2]&&saved[1]!=saved[3]){
   if(r(0x6e1ee4))p.queue(0x6a49e0,2,0x641fc8,saved,0,0,1);
   else p.copy(0x58c4a0,0x6a49e0,0x641fc8,saved,0,0);
   w(0x641aa0,x);w(0x641aa4,y);w(0x642c54,1);
  }
  animate();p.release(r(0x642c58));auto v=crop(x,y,bx,by);
  if(v[0]==v[2]||v[1]==v[3])return;
  if(r(0x6e1ee4))p.queue(0x642000,0,0x6a49e0,v,positive(x),positive(y),1);
  else p.copy(0x58c8a0,0x642000,0x6a49e0,v,positive(x),positive(y));
  rect(0x641aa8,{positive(x),positive(y),positive(x)-v[0]+v[2],positive(y)-v[1]+v[3]});
 }
 // 004a4910. Timer uses immediate copies even in deferred mode.
 void timer(){
  if(!less(r(0x5e04c8),22)||less(r(0x5e04c8),0))return;
  if(p.wait(r(0x6e1ec0),10))return;
  if(r(0x6c5038)){
   if(less(0,r(0x641fc0)))w(0x641fc0,r(0x641fc0)-1);
   if(r(0x642c60)){
    auto client=p.clientRectangle(r(0x656614));U bx=width()+client[0],by=height()+client[1];
    auto screen=p.cursorPosition(),point=screen;p.screenToClient(r(0x656614),point);
    U d=descriptor(),x=point[0]-r(d),y=point[1]-r(d+4);
    screen[0]=positive(screen[0]-r(d));screen[1]=positive(screen[1]-r(d+4));
    if(x!=r(0x641aa0)||y!=r(0x641aa4)||(r(d+8)!=1&&!less(0,r(0x641fc0)))){
     p.wait(r(0x642c58),0xffffffffu); // Original ignores raw inner result.
     if(r(0x642c54)){
      auto saved=rect(0x641a78);Rectangle v={0,0,saved[2]-saved[0],saved[3]-saved[1]};
      if(v[2]&&v[3])p.copy(0x58c360,0x641fc8,0x6a4fe0,v,positive(r(0x641aa0)),positive(r(0x641aa4)));
     }
     Rectangle saved={minimum(positive(x),width()),minimum(positive(y),height()),positive(minimum(x+40,width())),positive(minimum(y+40,height()))};rect(0x641a78,saved);
     if(saved[0]!=saved[2]&&saved[1]!=saved[3]){
      auto v=saved;
      if(!r(0x6f68d8))v={maximum(client[0],screen[0]),maximum(client[1],screen[1]),minimum(screen[0]+40,bx),minimum(screen[1]+40,by)};
      p.copy(0x58c360,0x6a4fe0,0x641fc8,v,0,0);w(0x642c54,1);w(0x641aa0,x);w(0x641aa4,y);
     }
     p.release(r(0x642c58));auto v=crop(x,y,width(),height());
     if(v[0]!=v[2]&&v[1]!=v[3]){p.copy(0x58ca90,0x642000,0x6a4fe0,v,positive(x),positive(y));animate();}
    }
   }
  }
  p.release(r(0x6e1ec0));
 }
 // 004a37d0. Does not drain/cancel/retain pending work. Disposal is a service.
 void dispose(){if(r(0x642c58)){p.close(r(0x642c58));w(0x642c58,0);}p.dispose(0x642000);p.dispose(0x641fc8);}
};
}
