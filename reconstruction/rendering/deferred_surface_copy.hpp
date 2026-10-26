#pragma once
#include "cursor_clients.hpp"
namespace mnm::reconstruction::cursor {
// Original 128x44-byte queue. Owns rectangle values; borrows wrapper addresses.
// Valid serial value-address state only; this is not an owning native queue.
template<class Memory,class Services> class DeferredCopies {
 Memory& m;Services& p;
 U r(U a){return m.get32(a);}void w(U a,U v){m.set32(a,v);}
public:
 DeferredCopies(Memory& memory,Services& services):m(memory),p(services){}
 // 0058d5b0. Full preserves every word. Null rect retains previous slot words.
 void enqueue(U source,U kind,U dest,const Rectangle* rect,U x,U y,U priority){
  U at=0x6f6b50,slot=0;while(slot<128&&r(at)){++slot;at+=44;}if(slot==128)return;
  w(at+8,source);w(at+12,dest);w(at+4,priority);w(at+16,kind);
  if(rect)for(U i=0;i<4;++i)w(at+20+i*4,(*rect)[i]);
  w(at+36,x);w(at+40,y);w(at,1);w(0x6f98d4,r(0x6f98d4)+1);
 }
 // 0058d660. Three priority passes; busy records retained, copy result ignored.
 void drain(){
  U count=r(0x6f98d4);
  for(U priority=0;priority<3;++priority){
   if(!count)return;U remaining=count,at=0x6f6b50;
   while(less(0,remaining)){
    if(at>=0x6f6b50+128*44)throw std::out_of_range("Corrupt original deferred count");
    if(r(at)){
     --remaining;
     if(r(at+4)==priority){
      U result=p.status(r(r(at+12)+8),1);
      if(!result||result==0x8876023fu){
       w(0x6f98d8,1);U kind=r(at+16);
       if(kind<=2){
        Rectangle v={r(at+20),r(at+24),r(at+28),r(at+32)};
        p.copy(kind==0?0x58c8a0:kind==1?0x58c140:0x58c4a0,r(at+8),r(at+12),v,r(at+36),r(at+40));
        w(at,0);w(0x6f98d4,r(0x6f98d4)-1);
       }
       w(0x6f98d8,0);
      }
     }
    }
    at+=44;
   }
   count=r(0x6f98d4);
  }
 }
};
}
