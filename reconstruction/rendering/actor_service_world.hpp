#pragma once
#include "actor_services.hpp"
#include "actor_ownership.hpp"
#include <algorithm>
#include <array>
#include <cmath>
namespace mnm::reconstruction::actor_services {
template<class M> class WorldServices {
    M& m;
    U r(U p){return m.get32(p);} U w(U p,U v){m.set32(p,v);return v;}
    U call(U id,U self,std::initializer_list<U> args={}){return m.invoke(id,self,args);}
    U actor(U index){return index<r(0x6def5c)?r(0x6def58)+index*0xe4b:0;}
    U cell(U x,U y,U z){return actor_lifecycle::mapCell(m,x,y,z);}
    U wrap(U value,U size){
        if(!size||signed32(size)<0)throw std::invalid_argument("Original positive map extent required");
        if(signed32(value)<0){do{value+=size;}while(signed32(value)<0);}
        else while(value>=size)value-=size;
        return value;
    }
    U once(U value,U size){if(signed32(value)<0)value+=size;if(value>=size)value-=size;return value;}
    U difference(U current,U target,U size){U value=wrap(current-target,size);return value>size/2?value-size:value;}
    static U magnitude(U value){return signed32(value)<0?0-value:value;}
    void burn(U p,U source){
        if(!r(p+0x8c9)){
            if(!r(p+0x744)&&!r(p+0xddf))w(p+0xddf,0);
            w(p+0x744,r(0x6a6529+r(p+0xa8)*0x5c9));w(p+0x748,r(0x6bb743));w(p+0x74c,source);
        }
    }
    void terrain(U at,U below){
        U type=m.get16(at),catalog=r(0x65660c);
        if(r(catalog+type*0x164+0x94)==16){at=below;type=m.get16(at);}
        U successor=r(catalog+type*0x164+0xc8);
        if(successor!=U(-1)&&successor!=type)call(0x4eb820,at,{successor});
    }
    void value(U p){
        if(!r(p+0x92e))return;
        U definition=r(p+0xac),mode=r(p+0x124),sum=0,active=0,count=0;
        auto bonus=[&](U v){return mode==1||mode==2?v+divide(v*r(mode==1?0x5e1910:0x5e1914),100):v;};
        for(U i=0;i<28;++i){
            U v=r(definition+0x1a0+i*4);bool configured=r(0x6a5fd8+i*0x5c9)!=0;
            if(configured&&!r(p+0xf8))v=divide(v,3);
            v+=divide(r(p+0x8ed)*v,100);if(r(p+0x94))v*=2;v=bonus(v);if(r(p+0x7cd)==1)v+=r(0x6bfd88);
            sum+=v;if(configured){active+=v;++count;}
        }
        U health=r(p+0xe4);w(p+0x90a,(sum/28)*health);w(p+0x912,count?divide(active,count)*health:0);
        U type=r(p+0xa8);sum=active=count=0;
        for(U i=0;i<28;++i){
            bool configured=r(0x6a5fd8+i*0x5c9)!=0;U v=type==8||type==20?10:r(definition+0x280+i*4);
            if(type!=8&&type!=20&&configured&&!r(p+0xf8))v=divide(v,3);
            v=bonus(v);if(r(p+0x7cd)==2)v+=r(0x6c27c7);sum+=v;if(configured){active+=v;++count;}
        }
        w(p+0x90e,(sum/28)*health);w(p+0x916,count?divide(active,count)*health:0);w(p+0x91a,health);
        if(r(p+0x90))w(p+0x91a,health*3);
        if(r(p+0x8c))w(p+0x91a,r(p+0x91a)*2);
        auto maximum=[&](U a,U b){return signed32(a)>signed32(b)?a:b;};
        U total=r(p+0x90e)+r(p+0x91a)+maximum(r(p+0x90a),r(p+0x90e))+maximum(r(p+0x912),r(p+0x916));w(p+0x91e,total);
        // Exact float/double constants and x87 extended intermediates from
        // 5247ce..5248d5; the intermediate FSTs do not reload rounded values.
        long double multiplier=r(p+0x104)?0x1.3333340000000p+0L:1.0L;
        long double divisor=1.0L-static_cast<long double>(signed32(r(definition+0x5a9)*r(definition+0x5ad)))*0x1.a36e2eb1c432dp-15L;
        multiplier*=1.0L+static_cast<long double>(r(p+0x84))*0x1.0624dd2f1a9fcp-10L;
        multiplier*=1.0L+static_cast<long double>(r(p+0x88))*0x1.5d867c3ece2a5p-12L;
        multiplier*=1.0L+static_cast<long double>(signed32(r(definition+0x5c5)))*0x1.47ae147ae147bp-7L;
        multiplier*=1.0L+static_cast<long double>(signed32(r(definition+0x589)))*0x1.a36e2eb1c432dp-14L;
        multiplier=divisor*multiplier;
        if(type==8||type==20)multiplier*=0x1.19999a0000000p+0L;
        else if(type==10)multiplier*=0x1.051eb80000000p+0L;
        else if(type==12||type==21)multiplier*=0x1.0ccccc0000000p+0L;
        long double result=static_cast<long double>(signed32(total))*multiplier;
        U converted=std::isfinite(result)&&result>=-0x1p63L&&result<0x1p63L?U(std::int64_t(result)):0;
        w(p+0x92e,0);w(p+0x91e,converted);
    }
public:
    explicit WorldServices(M& memory):m(memory){}
    U run(U entry,U p,std::initializer_list<U> args={}){
        U a[8]{};unsigned n=0;for(U v:args){if(n==8)throw std::invalid_argument("world service argument extent");a[n++]=v;}
        switch(entry){
        case 0x508b00:actor_lifecycle::invalidate(m,p,a[0]);break;
        case 0x509230:actor_lifecycle::attach(m,p,a[0],a[1],a[2]);break;
        case 0x5093e0:actor_lifecycle::detach(m,p,a[0],a[1],a[2]);break;
        case 0x4943f0:
            for(;;){
                if(r(0x5fed7c)&&r(p)>124&&m.get8(0x6c5040)){
                    U index=r(p),type=r(p+40),file=call(0x59cae2,0,{0x6c5040,0x5dc850}),count=r(0x6c5140);
                    if(file){w(0x6c5140,count+1);call(0x59d50b,0,{file,0x5e00f8,count,0x5dc8d8+type*50,index,r(0x6c5144),0x5e0130});call(0x59c4c3,0,{file});}
                }
                U at=cell(r(p+8),r(p+12),r(p+16));w(p+0x194,at);
                if(r(p+4)){call(0x488330,p,{at});w(p+4,0);}
                U high=r(0x6898dc);
                if(r(p)+1==high){w(0x6898dc,0);while(high){--high;if(r(r(0x689498)+high*0x22e +4)){w(0x6898dc,high+1);break;}}}
                w(p+0x20e,0);w(p+0x212,255);w(p+0x216,U(-1));w(p+0x21a,U(-1));call(0x464c80,p+0x160);
                if(r(p+0x19c)){call(0x4de090,r(p+0x19c),{0,1});w(p+0x19c,0);}
                if((r(p+40)!=43&&r(p+40)!=70)||r(p+0x5c)!=2)break;
                U index=r(p+0x60);p=index<r(0x68949c)?r(0x689498)+index*0x22e:0;
            }break;
        case 0x508250:{
            U dx=a[0]-r(p+8),dy=a[1]-r(p+12),width=r(0x6c5494),height=r(0x6c5498);
            if(magnitude(dx)>width/2)dx+=signed32(dx)>0?0-width:width;
            if(magnitude(dy)>height/2)dy+=signed32(dy)>0?0-height:height;
            if(!dx&&!dy)break;
            U sx=signed32(dx)<0?U(-1):dx?1:0,sy=signed32(dy)<0?U(-1):dy?1:0;
            static constexpr U directions[]={7,0,1,6,0,2,5,4,3};U facing=directions[signed32(sx)+signed32(sy)*3+4];
            if(facing&1){U ax=magnitude(dx),ay=magnitude(dy),small=std::min(ax,ay),large=std::max(ax,ay);if(signed32(small*3)<signed32(large*2))facing=ax<ay?(signed32(dy)>0?4:0):(signed32(dx)>0?2:6);}
            w(p+0x608,facing);U camera=(facing-r(0x6e9850)*2)&7;w(p+0x60c,camera);if(m.get8(p+0x722))w(p+0x60c,(camera-4)&7);
            if(r(p+0x7cd)==3){U source=r(0x6898d0)+0x160;call(0x464ca0,p+0x7d5,{r(0x6894a0)+r(source+4)*0x114});call(0x464cb0,p+0x7d5,{r(source)+r(p+0x60c),0});}
            else if(r(p+0x7cd)==2&&r(p+0x7d1))call(0x494a50,r(p+0x7d1),{r(p+0x60c)});
            if(r(p+0x855))call(0x464e20,p+0x825,{r(p+0x60c)+8});
            break;
        }
        case 0x51a8f0:{
            if(!r(p+0xe4)||r(p+0x7c))break;
            auto apply=[&](U x,U y){U index=m.get16(cell(x,y,r(p+16))+6);if(index!=0xffff)m.fastcall(0x468470,p,index<r(0x6b124c)?r(0x6b1248)+index*0x7e:0,{});};
            apply(r(p+8),r(p+12));
            if(r(r(p+0xac)+8)==2)for(U i=0;i<3;++i){U x=once(r(p+8)+r(0x5e5594+i*8),r(0x6c5494)),y=once(r(p+12)+r(0x5e5598+i*8),r(0x6c5498));apply(x,y);}
            break;
        }
        case 0x51aa30:{
            if(!r(p+0xe4))break;
            U at=cell(r(p+8),r(p+12),r(p+16));
            if(m.get16(at+10)&16){U index=m.get16(at+2);while(index!=0xffff){U effect=index<r(0x68949c)?r(0x689498)+index*0x22e:0;if(r(effect+40)==14){burn(p,r(effect+0x212));break;}index=m.get16(effect+0x1a0);}}
            terrain(at,cell(r(p+8),r(p+12),r(p+16)-1));
            if(r(r(p+0xac)+8)==2)for(U i=0;i<3;++i){U x=once(r(p+8)+r(0x5e55ac+i*8),r(0x6c5494)),y=once(r(p+12)+r(0x5e55b0+i*8),r(0x6c5498));U neighbor=cell(x,y,r(p+16));if(m.get16(neighbor+10)&16)burn(p,U(-1));terrain(neighbor,cell(x,y,r(p+16)-1));}
            U definition=r(p+0xac),z=r(p+16),end=z+r(definition+12);
            for(;z<end;++z){U width=r(definition+8),y=wrap(r(p+12),r(0x6c5498));for(U row=0;row<width;++row,y=wrap(y+1,r(0x6c5498))){U x=wrap(r(p+8),r(0x6c5494));for(U column=0;column<width;++column,x=wrap(x+1,r(0x6c5494)))if(m.get8(cell(x,y,z)+9)&64){call(0x564e90,0x6f3a70,{2,p,U(-1),0,U(-1),U(-1)});return 0;}}}break;
        }
        case 0x520790:{
            U kind=r(p+0x5f0);if(!kind)kind=r(p+0x5ec);U type=2,x=0,y=0,z=0,target=0;
            switch(kind){
            case 1:case 7:case 9:case 10:{
                U dx=difference(r(p+8),wrap(r(p+0x96b),r(0x6c5494)),r(0x6c5494)),dy=difference(r(p+12),wrap(r(p+0x96f),r(0x6c5498)),r(0x6c5498));
                std::array<U,3> values{{magnitude(dx),magnitude(dy),magnitude(r(p+16)-r(p+0x973))}};std::sort(values.begin(),values.end(),[](U l,U r){return signed32(l)<signed32(r);});
                U offset=values[2]*2+values[1]+values[0]<21?0x96b:8;x=wrap(r(p+offset),r(0x6c5494));y=wrap(r(p+offset+4),r(0x6c5498));z=r(p+offset+8);break;
            }
            case 3:type=3;target=actor(r(p+0x64c));break;
            case 4:type=4;x=wrap(r(p+0x96b),r(0x6c5494));y=wrap(r(p+0x96f),r(0x6c5498));z=r(p+0x973);break;
            default:{U offset=kind==11?0xbc3:kind==13?0x6f8:0x96b;x=wrap(r(p+offset),r(0x6c5494));y=wrap(r(p+offset+4),r(0x6c5498));z=r(p+offset+8);break;}
            }
            w(a[0],type);w(a[0]+4,wrap(x,r(0x6c5494)));w(a[0]+8,wrap(y,r(0x6c5498)));w(a[0]+12,z);w(a[0]+16,target);return a[0];
        }
        case 0x5244f0:value(p);break;
        default:throw std::invalid_argument("Unrecovered actor world service");
        }
        return 0;
    }
};
}
