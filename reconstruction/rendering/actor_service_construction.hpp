#pragma once
#include "actor_services.hpp"
namespace mnm::reconstruction::actor_services {
// Normal-return widget/canvas ownership and the MSVC vector cleanup body.
// The cold finally entry takes an explicit value-address frame; OS exception
// dispatch and a destructor throwing during unwind remain platform boundaries.
template<class M> class ConstructionServices {
    M& m;
    U r(U p){return m.get32(p);} U w(U p,U v){m.set32(p,v);return v;}
    U call(U id,U self,std::initializer_list<U> args={}){return m.invoke(id,self,args);}
public:
    explicit ConstructionServices(M& memory):m(memory){}
    U run(U entry,U p,std::initializer_list<U> args={}){
        U a[8]{};unsigned n=0;for(U v:args){if(n==8)throw std::invalid_argument("construction argument extent");a[n++]=v;}
        switch(entry){
        case 0x4a4df0:w(p,0x5c5c20);w(p+8,0);w(p+4,U(-1));w(p+12,16);m.set8(p+16,1);return p;
        case 0x464350:
            call(0x4a4df0,p);for(U base:{0x21u,0x53u}){for(U off:{4u,8u,12u,24u,20u,16u,32u,36u,40u})w(p+base+off,0);m.set16(p+base+44,0);}
            w(0x6f694c,0);w(p+0x85,0);w(p+0x89,0);m.set8(p+0x8d,0);w(p+0x8e,0);m.set8(p+0x92,0);w(p+0x93,0);w(p,0x5c5c2c);return p;
        case 0x4a4f70:{
            U index=p;if(index==U(-1))break;U at=0x642034+index*4,widget=r(at),remaining=r(0x642c68)-index-1;
            if(r(0x5e04cc)==r(widget+4))w(0x5e04cc,U(-1));
            w(widget+8,0);
            if(remaining&&index<remaining+index){for(U left=remaining;left;--left,at+=4){U next=r(at+4);w(at,next);w(next+4,r(next+4)-1);}}
            U selected=r(0x5e04cc);if(selected==index)w(0x5e04cc,U(-1));else if(selected>=index&&selected!=U(-1))w(0x5e04cc,selected-1);
            w(0x642c68,r(0x642c68)-1);break;
        }
        case 0x58b3e0:{
            if(!r(p+8))break;
            U attempts=0;
            if(r(0x6e1f1c))m.com(r(r(r(p+8))+0x98),r(p+8),{0});
            for(U off:{4u,8u}){U result;do{++attempts;U buffer=r(p+off);result=m.com(r(r(buffer)+8),buffer,{});}while(result&&attempts<1000);}
            if(r(p+12)){U buffer=r(p+12);m.com(r(r(buffer)+8),buffer,{});w(p+12,0);}
            if(r(0x6f98c0)==p)w(0x6f98c0,0);
            while(r(p+28)<127){U index=r(p+28),next=r(0x6f6954+index*4);w(0x6f6950+index*4,next);if(!next)break;w(next+28,index);w(p+28,r(p+28)+1);}
            w(0x6f6b4c,0);w(0x6f8154,r(0x6f8154)-1);w(0x6f6950+r(p+28)*4,0);for(U off:{4u,8u,24u,20u,16u,40u})w(p+off,0);break;
        }
        case 0x463270:
            w(p,0x5c5c2c);call(0x58b3e0,p+0x53);call(0x58b3e0,p+0x21);
            if(r(p+0x93)){U resource=r(p+0x93);call(r(r(resource)+4),resource,{1});}w(p+0x93,0);call(0x597870,0,{r(p+0x85)});w(p+0x85,0);
            call(0x58b3e0,p+0x53);call(0x58b3e0,p+0x21);w(p,0x5c5c20);call(0x4a4f70,r(p+4));w(p+4,U(-1));break;
        case 0x59bf87:{U at=a[0],remaining=a[2];while(signed32(--remaining)>=0){at-=a[1];call(a[3],at);}break;}
        case 0x59c4ab:if(!r(p-32))call(0x59bf87,0,{r(p+8),r(p+12),r(p-28),r(p+24)});break;
        default:throw std::invalid_argument("unrecovered actor construction service");
        }return 0;
    }
};
}
