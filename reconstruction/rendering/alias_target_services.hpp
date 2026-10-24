#pragma once
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <stdexcept>

namespace mnm::reconstruction::actor_services {
// Pinned NoCD40209ca7 actor callback51e890 and order-cursor524130. Lower
// world/action/presentation bodies remain explicit services. M supplies checked
// PE32 values, invocation-scoped temporary storage and ordered invoke().
// Checked memory refusal is native policy: the original has no stale8fd guard.
template<class M> class AliasTargetServices {
    using U=std::uint32_t;
    M& m;
    U r(U p){return m.get32(p);}
    void w(U p,U v){m.set32(p,v);}
    U call(U entry,U self,std::initializer_list<U> args={}){return m.invoke(entry,self,args);}
    static std::int64_t signedValue(U v){return v<0x80000000u?v:std::int64_t(v)-0x100000000ll;}
    U wrapped(U value,U extent){
        if(!extent||extent>0x7fffffffu)throw std::invalid_argument("Alias consumer map extent");
        auto n=signedValue(value)%extent;
        return U(n<0?n+extent:n);
    }
    U x(U value){return wrapped(value,r(0x6c5494));}
    U y(U value){return wrapped(value,r(0x6c5498));}
    void erase(U p,U list,U node,bool advance){
        if(advance)w(p+0x8fd,r(node));
        w(r(node+4),r(node));w(r(node)+4,r(node+4));
        call(0x597870,0,{node});
        // Both counters change after free, including wrapping underflow.
        w(list+8,r(list+8)-1);w(p+0x902,r(p+0x902)-1);
    }
    U actorEvent(U p,U event){
        U scratch=m.temporary(64);
        w(scratch,x(r(p+0x70c)));w(scratch+4,y(r(p+0x710)));w(scratch+8,r(p+0x714));
        if(event==5||event==14){
            if(r(p+0x7c)&&r(p+0x174)==r(0x644520))
                call(0x54dad0,p+0xbf3,{r(p+0xa8)==13?12u:7u});
            return 0;
        }
        if(event==8){w(p+0x8cd,0);call(0x51f5e0,p,{1});call(0x506d30,p);return 0;}
        if(event!=9)return 0;
        U saved=r(p+0x104);
        if(r(p+0x718))w(p+0x104,1);
        U query=call(0x44d970,p,{scratch+12,scratch,scratch+4,scratch+8});
        if(call(0x4f5230,0x6c5490,{r(p),query})){
            U nx=r(scratch),ny=r(scratch+4),nz=r(scratch+8);
            call(0x5093e0,p,{r(p+8),r(p+12),r(p+16)});
            if(r(p+0x618)){
                U path=p+r(p+0x977)*28+0x97f;
                call(0x5093e0,p,{r(path),r(path+4),r(path+8)});
            }
            call(0x5070e0,p,{nx,ny,nz,r(p+0xa8)});
            call(0x509230,p,{nx,ny,nz});
            call(0x51a8f0,p);call(0x50f850,p);call(0x51aa30,p);
            // Inactive actors return before restoring104 or sending event10.
            if(!r(p+4))return 0;
            w(p+0x618,0);call(0x51a880,p);call(0x5178c0,p);
            U state=r(p+0xa0);
            if(state){
                if(state==2||state==1)call(0x54dce0,p+0xbf3,{state==2?1u:4u});
                w(p+0xa0,0);w(p+0x9c,0);
            }
            w(p+0x108,0);
            if(r(p+0x798)){call(0x520e70,p);saved=0;}
            call(0x50ab30,p);call(0x4f2dd0,0x6c5490);
            w(p+0xde3,0);w(p+0x963,0);m.set8(p+0x962,0);m.set8(p+0x723,0);
            w(p+0x96b,r(p+8));w(p+0x96f,r(p+12));w(p+0x973,r(p+16));
        }
        w(p+0x104,saved);
        if(r(p+0x718)){
            U definition=r(p+0xac),descriptor=scratch+16;
            w(descriptor,r(definition+12));w(descriptor+4,r(definition+8));
            w(descriptor+8,saved);w(descriptor+12,r(definition+0x44));
            m.set8(descriptor+16,0);w(descriptor+17,r(p+0xa8));
            m.set8(descriptor+21,m.get8(p+0x723)||r(p+0x5ec)==14);
            if(!call(0x4f4900,0x6c5490,{x(r(p+8)),y(r(p+12)),r(p+16),descriptor})){
                if(r(p+0xa8)==27)call(0x50ab30,p);
                else{
                    w(p+0xb8b,0);w(p+0xa4,r(p+16));w(p+0xde3,0);
                    m.set8(p+0x962,0);w(p+0x963,0);w(p+0x164,0);
                    if(r(p+0x664)){
                        U kind=r(p+0x5fc);
                        if(kind==10){
                            U slot=r(p+0x64c),peer=slot<r(0x6def5c)?r(0x6def58)+slot*0xe4b:0;
                            w(p+0x664,0);
                            U count=r(peer+0x668)-1;
                            w(peer+0x668,signedValue(count)<0?0:count);
                        }else if(kind==20){
                            U peer=call(0x4970b0,0x6def58,{r(p+0x6a8)});
                            w(p+0x664,0);call(0x50eb60,peer,{0});
                        }
                    }
                    call(0x50aa30,p,{16});
                }
            }
        }
        call(0x54dad0,p+0xbf3,{10});return 0;
    }
    U orderTick(U p){
        U list=r(p+0x8f9),state=r(p+0x5ec);
        if(!r(list+8)||state==15||state==6||state==8||r(p+0xa8)==22||
           r(p+0x784)||r(p+0xde3)||
           (r(p+0x798)&&!r(r(p+0xac)+0x3c)&&r(p+0x7cd)!=3))return 0;
        U node=r(p+0x8fd);
        if(m.get8(node+44)){m.set8(node+44,0);m.set8(p+0x901,1);}
        if(!m.get8(p+0x901)){
            if(r(node+40)){call(0x4943f0,r(node+40));w(r(p+0x8fd)+40,0);}
            node=r(p+0x8fd);erase(p,r(p+0x8f9),node,true);
        }else w(p+0x8fd,r(node));
        list=r(p+0x8f9);
        if(r(p+0x8fd)==r(list+4)){
            if(!m.get8(p+0x901)){
                if(r(list+8)){call(0x512340,p);call(0x52d0c0,r(p+0x8f9));}
                m.set8(p+0x901,0);w(p+0x902,0);return 0;
            }
            for(U it=r(r(list+4));it!=r(r(p+0x8f9)+4);){
                U next=r(it);
                if(r(it+8)==3)erase(p,r(p+0x8f9),it,false);
                it=next;
            }
            list=r(p+0x8f9);w(p+0x8fd,r(r(list+4)));
        }
        node=r(p+0x8fd);
        call(0x520ad0,p,{r(node+8),x(r(node+12)),y(r(node+16)),r(node+20),r(node+24)});
        node=r(p+0x8fd);
        if(r(node+8)==3)erase(p,r(p+0x8f9),node,true);
        return 1;
    }
public:
    explicit AliasTargetServices(M& memory):m(memory){}
    std::optional<U> tryRun(U entry,U self,std::initializer_list<U> args={}){
        if(entry!=0x51e890&&entry!=0x524130)return std::nullopt;
        if(args.size()!=(entry==0x51e890?1u:0u))throw std::invalid_argument("Alias target argument extent");
        return entry==0x51e890?actorEvent(self,*args.begin()):orderTick(self);
    }
    // Cursor-only serialization projections at526979..5269a5 and
    // 5292c6..5292f0. Save compares identity without reading cursor storage;
    // load reseeds from the owned list and follows positive index steps.
    U cursorIndex(U actor,U retained,U stream){
        U list=r(actor+0x8f9),head=r(list+4),index=0,result=retained;
        for(U node=r(head);node!=r(r(actor+0x8f9)+4);node=r(node),++index){
            call(0x52c9d0,node+8,{stream});
            if(node==r(actor+0x8fd))result=index;
        }
        return result;
    }
    void seedCursorAfterAppend(U actor){
        w(actor+0x902,r(actor+0x902)+1);
        w(actor+0x8fd,r(r(r(actor+0x8f9)+4)));
        call(0x512220,actor);
    }
    void restoreCursor(U actor,U index){
        U list=r(actor+0x8f9);w(actor+0x8fd,r(r(list+4)));
        if(signedValue(index)>0)do{w(actor+0x8fd,r(r(actor+0x8fd)));}while(--index);
    }
};
}
