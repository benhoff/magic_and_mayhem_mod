#pragma once
#include "actor_services.hpp"
#include "actor_ownership.hpp"
#include <cstring>
namespace mnm::reconstruction::actor_services {
// Build-specific item, spell and explosion orchestration. Concrete effect,
// raster, contribution and particle implementations are backend contracts.
template<class M> class EffectServices {
    M& m;
    U r(U p){return m.get32(p);} U w(U p,U v){m.set32(p,v);return v;}
    U call(U id,U self,std::initializer_list<U> args={}){return m.invoke(id,self,args);}
    U fast(U id,U self,U second,std::initializer_list<U> args={}){return m.fastcall(id,self,second,args);}
    static U bits(float f){U v;std::memcpy(&v,&f,4);return v;}
    U cell(U x,U y,U z){return actor_lifecycle::mapCell(m,x,y,z);}
    U wrap(U v,U size){if(!size||signed32(size)<0)throw std::invalid_argument("positive original map extent");while(signed32(v)<0)v+=size;while(v>=size)v-=size;return v;}
    U description(){U d=m.temporary(252);for(U i=0;i<63;++i)w(d+i*4,0);for(U i=0;i<10;++i)w(d+i*4,U(-1));w(d+40,1);w(d+44,U(-1));w(d+48,7);return d;}
    void contribute(U actor,U amount){for(U i=0;i<2;++i){U at=call(0x522770,actor,{i});if(at)w(at+0x2f,r(at+0x2f)+amount);}}
    U healingContribution(U actor,U item){U removed=r(actor+0xe4)-r(item+0x6a)*4;if(signed32(removed)<=0)removed=0;U restored=removed+r(item+0x6a)*4,max=call(0x5075f0,actor);if(signed32(restored)>=signed32(max))restored=call(0x5075f0,actor);return fast(0x416d80,r(actor+0xa8),restored,{restored-removed,0,0,0});}
public:
    explicit EffectServices(M& memory):m(memory){}
    U run(U entry,U p,std::initializer_list<U> args={},U second=0){
        U a[12]{};unsigned n=0;for(U v:args){if(n==12)throw std::invalid_argument("effect argument extent");a[n++]=v;}
        switch(entry){
        case 0x469000:{
            U spell=call(0x509790,p,{second});if(!r(spell+20)||r(spell)!=a[0])return 0;
            U x=r(p+8),y=r(p+12),z=r(p+16),item=m.get16(cell(x,y,z)+6);bool special=false;
            if(item!=0xffff){U at=item<r(0x6b124c)?r(0x6b1248)+item*0x7e:0;special=r(at+0x2a)==50&&r(at+0x6a)==U(-1);}
            if(!special){U coords=m.temporary(12);w(coords,z);w(coords+4,y);w(coords+8,x);if(!fast(0x4694e0,coords+8,coords+4,{coords}))return 0;x=r(coords+8);y=r(coords+4);z=r(coords);}
            fast(0x469170,r(spell),x,{y,z,r(spell+4),r(spell+8),r(spell+12),r(spell+16),special?1u:0u,special?0u:p});
            call(0x509aa0,p,{second});if(a[1]!=U(-1)&&(special||r(p+0x174)==r(0x644520)))call(0x4d53d0,0x6a2dd0,{a[1],a[0]});return 1;
        }
        case 0x468470:{
            U item=second;
            for(;;){
                U type=r(item+0x2a);if(signed32(type)>122||!r(item+4))return 0;
                if(!r(0x68991c)&&!call(0x506220,p)&&signed32(type)>6&&type!=19&&type!=61)return 0;
                if(type==61){
                    if(!r(p+0x7c)){call(0x51e7d0,p,{1});float ratio=static_cast<float>(static_cast<long double>(signed32(r(0x5e1908)))/static_cast<long double>(signed32(r(0x5e1904))));contribute(p,fast(0x416d80,r(p+0xa8),0,{0,0,0,bits(ratio)}));}
                    U next=call(0x5437f0,item)&0xffff;if(next==0xffff)return 0;item=next<r(0x6b124c)?r(0x6b1248)+next*0x7e:0;continue;
                }
                bool canonical=call(0x411e50,r(p+0xac))&&!r(p+0x7c),healing=type<=4||type==6;
                if(canonical){
                    if(healing){contribute(p,healingContribution(p,item));if(call(0x5097b0,p,{type,r(item+0x6a),r(item+0x6e),r(item+0x72),r(item+0x76),1})){fast(0x4680f0,p,item);call(0x543c60,item);}}
                    else if(type==19){
                        if(call(0x5097b0,p,{type,r(item+0x6a),r(item+0x6e),r(item+0x72),r(item+0x76),1})){
                            call(0x571310,0x658178,{0x401,r(p+8),r(p+12),r(p+16),0,0,0});call(0x543ee0,item,{37});w(0x6c5c88,r(0x6c5c88)-1);
                            float delta=static_cast<float>(signed32(r(item+0x6a)));U resource=call(0x51e500,p);U units=U(signed32(resource)/256);
                            contribute(p,fast(0x416d80,r(p+0xa8),0,{0,bits(static_cast<float>(static_cast<long double>(signed32(units))+delta)),bits(delta),0}));
                            for(U i=0;i<8;++i)if(r(p+36+i*4))call(0x507fa0,p,{i,r(item+0x6a)<<8});
                        }
                    }else if(type==50)fast(0x4681f0,p,item);
                    else if(type>=53&&type<=56){call(0x4db080,0x6c5268,{r(item+0x6a)-1});call(0x543c60,item);}
                    else if(call(0x5097b0,p,{type,r(item+0x6a),r(item+0x6e),r(item+0x72),r(item+0x76),type>=97&&type<=122?0u:1u})){fast(0x4680f0,p,item);call(0x543c60,item);}
                }else{
                    U definition=type<r(0x6b1254)?r(0x6b1250)+type*32:0;if(r(definition+20)!=2)return 0;
                    if(healing){
                        U amount=0;bool defined=false;
                        if((m.get8(0x6a6500+r(p+0xa8)*0x5c9)&2)&&r(p+0xe4)<call(0x5075f0,p)&&!r(p+0x7c)){
                            U old=r(p+0xe4),added=r(item+0x6a)*4,max=call(0x5075f0,p),health=old+added;if(health>=max)health=call(0x5075f0,p);
                            amount=fast(0x416d80,r(p+0xa8),health,{health-r(p+0xe4),0,0,0});defined=true;
                            U payload=r(item+0x6a),at=call(0x49cdb0,0x689498),d=description();
                            if(at){w(d,r(p+8)*32+16);w(d+4,r(p+12)*32+16);w(d+8,(r(p+16)+1)*16);w(d+12,r(d));w(d+16,r(d+4));w(d+24,r(p));w(d+28,r(p));w(d+32,type);w(d+36,payload);w(d+48,4);call(0x493f20,at,{38,d});}call(0x543c60,item);
                        }else if(m.get8(0x6a6500+r(p+0xa8)*0x5c9)&4){U owner=call(0x466870,0x643a80,{r(p+0x174)});if(owner){fast(0x467fe0,p,item);U removed=r(owner+0xe4)-r(item+0x6a)*4;if(signed32(removed)<=0)removed=0;U health=removed+r(item+0x6a)*4,max=call(0x5075f0,owner);if(signed32(health)>=signed32(max))health=call(0x5075f0,owner);amount=fast(0x416d80,0,health,{health-removed,0,0,0});defined=true;}}
                        // The baseline reads an uninitialized stack word when neither
                        // branch assigns the contribution. It is not a recovered
                        // deterministic value or an intentional native policy.
                        if(!defined)throw std::domain_error("indeterminate original item contribution");
                        contribute(p,amount);
                    }else if(type==50){if((m.get8(0x6a6500+r(p+0xa8)*0x5c9)&8)&&!r(p+0x7c))fast(0x4681f0,p,item);}
                    else if(!r(p+0x7c)&&(m.get8(0x6a6500+r(p+0xa8)*0x5c9)&4))fast(0x467fe0,p,item);
                }
                call(0x564e90,0x6f3a70,{6,p,type,0,U(-1),U(-1)});break;
            }break;
        }
        case 0x490260:{
            U type=r(p+0x4c),start=0,height=7;switch(type){case 65:start=0x167;break;case 69:start=0x153;height=6;break;case 70:start=0x159;break;case 91:start=0x14d;height=6;break;case 92:start=0x160;break;default:return 0;}
            U d=description();w(d,r(p+20));w(d+4,r(p+24));w(d+12,r(d));w(d+16,r(d+4));U at=r(p+0x194);
            for(U left=height-1;left;--left){U terrain=m.get16(at);if(terrain>=start&&terrain<start+height){
                U linear=divide(at-r(0x6c54dc),12),z=divide(linear,r(0x6c54a0)),rem=linear-r(0x6cb8c2+z*4),y=wrap(divide(rem,r(0x6c5494)),r(0x6c5498)),x=wrap(rem-r(0x6cb942+y*4),r(0x6c5494));
                U pos=m.temporary(12);call(0x4f49e0,0x6c5490,{x,y,z,pos});call(0x4eb710,at,{0});
                if(r(0x6e1f3c)){call(0x4f49e0,0x6c5490,{x,y,z,pos});U node=call(0x540430,0x6eaf08,{r(pos),r(pos+4),r(pos+8),r(pos),r(pos+4),r(pos+8)});if(node!=U(-1))call(0x540140,0x6eaf08,{x,y,z,node});}
                U slot=call(0x49cdb0,0x689498);if(slot){w(d+8,z*16);w(d+20,z*16);call(0x493f20,slot,{13,d});}at-=r(0x6c54a0)*12;
            }}
            U cx=r(p+20)+16,cy=r(p+24)+16,cz=r(p+28);
            for(U i=0;i<200;++i){U allocation=call(0x597850,0,{72});if(!allocation)continue;U rz=call(0x54e200,0x6574c8)&0x7fffffff,ry=call(0x54e200,0x6574c8)&0x7fffffff,rx=call(0x54e200,0x6574c8)&0x7fffffff;
                U particle=call(0x545e90,allocation,{cx+rx%5-2,cy+ry%5-2,cz+rz%5-2});if(particle&&!call(0x5457e0,0x6cbb8a,{particle}))call(r(r(particle)),particle,{1});
            }
            call(0x571310,0x658178,{0x402,r(p+8),r(p+12),r(p+16),0,0,0});U head=r(0x6898e4);for(U node=r(head);node!=head;node=r(node))if(r(node+8)==p){w(r(node+4),r(node));w(r(node)+4,r(node+4));call(0x597870,0,{node});w(0x6898e8,r(0x6898e8)-1);break;}call(0x4943f0,p);break;
        }
        default:throw std::invalid_argument("unrecovered actor effect service");
        }return 0;
    }
};
}
