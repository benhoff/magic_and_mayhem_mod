#pragma once
#include "actor_services.hpp"
#include <algorithm>
namespace mnm::reconstruction::actor_services {
// Concrete PE32 suppliers, over the same value-address storage as their callers.
// M owns imports, heap storage, and virtual interfaces. No host pointers escape.
template<class M> class Providers {
    M& m;
    U r(U p){return m.get32(p);} U w(U p,U v){m.set32(p,v);return v;}
    U call(U id,U p,std::initializer_list<U> a={}){return m.invoke(id,p,a);}
    bool equal(U a,U b){for(;;++a,++b){U c=m.get8(a);if(c!=m.get8(b))return false;if(!c)return true;}}
    U wrap(U v,U size){if(!size||signed32(size)<0)throw std::invalid_argument("positive provider extent");while(signed32(v)<0)v+=size;while(v>=size)v-=size;return v;}
    U scan(U p,bool initial){
        U result=0;
        for(;;){U at=r(p+24),tag=r(at);bool done=false;
            switch(tag){
            case 0:w(p+4,at);done=true;break;
            case 1:w(p+12,r(at+4));break;
            case 2:w(p+20,r(at+4));break;
            case 3:if(r(p+20)){w(p+20,r(p+20)-1);w(p+24,at+r(at+4)*44-44);}break;
            case 4:if(!r(p+32))w(p+24,at+r(at+4)*44-44);else w(p+32,0);break;
            case 5:result=r(at+4);done=!initial;break;
            case 6:w(p+8,0);w(p+4,0);result=1;done=true;break;
            default:break;
            }
            w(p+24,r(p+24)+44);if(done)return result;
        }
    }
    void refresh(U p){w(p+32,0);for(U i=0;signed32(i)<signed32(r(p+8));++i){U at=r(p)+i*0x47c;if(r(at)&&r(at+8)!=2&&(r(at+0x27c)||(r(at+4)==2&&r(at+0x470)))){w(p+32,1);break;}}}
public:
    explicit Providers(M& memory):m(memory){}
    U run(U entry,U p,std::initializer_list<U> args={},U second=0){
        U a[12]{};unsigned n=0;for(U v:args){if(n==12)throw std::invalid_argument("provider arguments");a[n++]=v;}
        switch(entry){
        case 0x464b20:return r(r(p)+20)-1;
        case 0x464b30:return r(p+8)+r(r(p+4)+a[0]*4)*44;
        case 0x464c50:for(U i=0;i<12;++i)w(p+i*4,0);break;
        case 0x464d70:{
            U definition=r(p+28),index=r(p);for(U off:{12u,16u,20u,4u,24u,8u,32u})w(p+off,0);
            U start=r(definition+8)+r(r(definition+4)+index*4)*44;w(p+8,1);w(p+24,start);
            if(!r(start))w(p+4,start);else call(0x465000,p);
            if(r(p+8)&&r(p+36)){
                w(p+40,0);w(p+36,0);U display,current,previous,count;
                do{display=r(p+24);current=r(p+4);U result=call(0x464ec0,p);previous=r(p+40);count=previous+1;w(p+40,count);if(!r(p+8)||result==2)break;}while(r(display)!=4||signed32(r(display+4))>=0);
                w(p+44,count);w(p+24,display);w(p+4,current);w(p+36,1);w(p+40,previous);w(p+8,1);
            }break;
        }
        case 0x465000:return scan(p,true);
        case 0x464ec0:{
            if(!r(p+8))return 0;
            if(r(p+36)){
                w(p+40,r(p+40)?r(p+40)-1:r(p+44));w(p+36,0);call(0x464d70,p);U result=0;
                for(U i=0;signed32(i)<signed32(r(p+40));++i)result=call(0x464ec0,p);
                w(p+36,1);return result;
            }
            if(r(p+12)>r(p+16)){w(p+16,r(p+16)+1);return 0;}
            U result=scan(p,false);w(p+16,0);return result;
        }
        case 0x4f7970:
            for(U i=0;i<4;++i)w(p+0x11+i*4,r(a[0]+i*4));
            w(p+0x51,r(p+0x19)-r(p+0x11));w(p+0x55,r(p+0x1d)-r(p+0x15));break;
        case 0x510480:return !r(p+0xe4)&&(r(p+0xf4)||r(p+0xf0));
        case 0x57dc20:w(0x658174,p);w(0x6a2dc8,second);w(0x6def50,a[0]);break;
        case 0x57dc60:w(0x6cbb6c,p);w(0x6e0008,second);w(0x6a49b8,a[0]);w(0x656618,a[1]);break;
        case 0x4a5a90:{U flags=m.get8(a[0]);if(a[1]==4)flags&=0xfc;else if(a[1]==1)flags&=0xf9;else if(a[1]==2)flags&=0xfa;m.set8(a[0],flags|a[1]);break;}
        case 0x545590:
            w(p+8,a[1]);w(p+4,a[0]);w(p+12,U(std::max(0,std::min(signed32(a[2]),signed32(r(0x5eb488)*2)))));
            w(p,0x5c73a8);w(p+4,wrap(r(p+4),r(0x5eb480)));w(p+8,wrap(r(p+8),r(0x5eb484)));break;
        case 0x545ae0:
            w(p+8,U(std::clamp(signed32(a[0]),0,255))/8);w(p+12,U(std::clamp(signed32(a[1]),0,255))/(r(0x6e1f88)?8:4));w(p+16,U(std::clamp(signed32(a[2]),0,255))/8);break;
        case 0x545e80:case 0x5456b0:w(p,0x5c73a8);break;
        case 0x545e60:case 0x545690:
            call(entry==0x545e60?0x545e80:0x5456b0,p);
            if(a[0]&1)call(0x597870,0,{p});
            return p;
        case 0x54aa10:w(p,0x5c73a8);if(a[0]&1)call(0x597870,0,{p});return p;
        case 0x54aa00:return 0x400;
        case 0x54aa80:return divide(divide(r(p+40),r(p+44))<<10,20);
        case 0x54e200:{U first=r(p),secondIndex=r(p+4);w(p,r(0x5eb940+first*4));w(p+4,r(0x5eb940+secondIndex*4));U result=r(p+8+first*4)^r(p+8+secondIndex*4);w(p+8+first*4,result);return result;}
        case 0x545e90:{
            call(0x545590,p,{a[0],a[1],a[2]});w(p,0x5c73c0);for(U off:{16u,20u,24u,48u})w(p+off,0);
            w(p+28,128);w(p+32,128);w(p+36,32);w(p+40,100);w(p+44,5);w(p+52,2);w(p+56,2);call(0x545ae0,p+52,{128,128,32});w(p,0x5c73f0);
            U x,y;auto positive=[&](){return call(0x54e200,0x6574c8)&0x7fffffff;};
            if(positive()&1){x=positive()%16;y=positive()%(16-x);}else{y=positive()%16;x=positive()%(16-y);}U z=16-x-y;
            if(positive()&1)x=0-x;
            if(positive()&1)y=0-y;
            if(positive()&1)z=0-z;
            w(p+16,x);w(p+20,y);w(p+24,z);return p;
        }
        case 0x5457e0:{
            U count=r(p+0x1c00),token=a[0];if(signed32(count)<0x700){w(p+count*4,token);w(p+0x1c00,r(p+0x1c00)+1);return 1;}
            U priority=call(r(r(token)+20),token),selected=U(-1);
            if(signed32(priority)>0x400&&signed32(r(p+0x1c04))<signed32(priority)){
                for(U i=0;signed32(i)<signed32(r(p+0x1c00));++i){U old=r(p+i*4),score=call(r(r(old)+20),old);if(signed32(score)<signed32(priority)){priority=score;selected=i;}}
                if(selected!=U(-1)){U old=r(p+selected*4);if(old)call(r(r(old)),old,{1});w(p+selected*4,token);if(signed32(r(p+0x1c04))<signed32(selected-1))w(p+0x1c04,selected-1);return 1;}
                w(p+0x1c04,priority);
            }return 0;
        }
        case 0x466900:for(U i=0;i<8;++i)if(equal(p+i*0x154+0x50,a[0]))return p+i*0x154;return 0;
        case 0x56ba50:case 0x56ba70:return signed32(a[0])<signed32(p);
        case 0x56ba60:case 0x56ba80:return signed32(a[0])>signed32(p);
        case 0x56b790:{
            U actor=a[1];
            for(U i=0;signed32(i)<signed32(r(0x6cbb56));++i){U region=r(0x6cbb52)+i*72;if(!equal(region+24,a[0]))continue;
                U z=r(actor+16),end=z+r(r(actor+0xac)+12);
                for(;z<end;++z){U y=wrap(r(actor+12),r(0x6c5498));
                    for(U row=0;row<r(r(actor+0xac)+8);++row,y=wrap(y+1,r(0x6c5498))){U x=wrap(r(actor+8),r(0x6c5494));
                        for(U col=0;col<r(r(actor+0xac)+8);++col,x=wrap(x+1,r(0x6c5494))){
                            auto inside=[&](U v,U low,U high){return signed32(high)<signed32(low)?!(signed32(v)<signed32(low)&&signed32(v)>signed32(high)):signed32(v)>=signed32(low)&&signed32(v)<=signed32(high);};
                            if(inside(x,r(region),r(region+12))&&inside(y,r(region+4),r(region+16))&&signed32(z)>=signed32(r(region+8))&&signed32(z)<=signed32(r(region+20)))return 1;
                        }
                    }
                }
            }return 0;
        }
        case 0x565180:{
            U at=a[0],event=a[1],actor=a[2];if(!r(at)||r(at+8))return 0;
            switch(event){
            case 1:if(r(at+4)==1)return r(0x6c4830)%20==0;return !r(at+0x1a8)&&!r(at+0x27c)&&(r(at+0x138)==U(-1)||(r(at+0x13c)!=6&&r(at+0x13c)!=5));
            case 3:if(r(at+4)==4&&!equal(at+0x88,0x5fc874)&&r(at+0xa8)==2&&!equal(at+0x2d0,0x5fc874))return 1;[[fallthrough]];
            case 2:if(!r(at+0x1a8)||!actor||(event==2?r(at+0x1f4)!=0:r(at+0x1f4)==0))return 0;return equal(at+0x1d4,0x5fc874)||call(0x56b790,0,{at+0x1d4,actor});
            case 4:if(r(at+4)==1)for(U i=0;signed32(i)<signed32(r(0x6f37fc));++i){U row=r(0x6f37f8)+i*64;if(r(row)&&equal(row+4,at+0x2cc))return r(row+40)==r(actor);}return 0;
            case 6:return r(at+0x138)!=U(-1)&&r(at+0x13c)==6;
            case 7:return r(at+0x138)!=U(-1)&&r(at+0x13c)==5;
            case 9:return r(at+0x27c)!=0;
            case 10:return r(at+4)==2;
            default:return 0;
            }
        }
        case 0x564d60:{
            U at=a[0];if(r(at+24)||r(at+48)){w(at+8,1);break;}U repeats=r(at+12);
            if(repeats!=U(-1)&&repeats){w(at+12,repeats-1);repeats-=1;}
            if(repeats==U(-1)||signed32(repeats)>0){w(at+8,0);w(at+36,0);for(U off:{16u,24u,48u})w(at+off,r(at+off+4));}
            else{w(at+8,2);if(r(at+0x27c))refresh(p);}break;
        }
        case 0x566d80:{
            U at=a[0],type=r(at+4);if(type==1){call(0x56b590,p,{at});break;}if(r(at+16)){w(at+8,1);break;}
            U target=0;switch(type){case 2:target=0x566e60;break;case 3:target=0x5685a0;break;case 4:target=0x5695c0;break;case 5:target=0x569c80;break;case 6:target=0x56a8d0;break;case 8:target=0x56b1e0;break;default:break;}
            if(target){if(type==5)call(target,p,{at,a[1]});else call(target,p,{at});}
            else if(type==0||type==9){if(type==9){for(U off:{0x2d0u,0x2ccu}){U i=r(at+off);if(signed32(i)>=0&&signed32(i)<60)m.set8(0x6f3808+i,off==0x2cc);}}call(0x564d60,p,{at});}
            break;
        }
        case 0x4d47f0:{
            if(!r(p+0x82)){
                U d=m.temporary(108);for(U i=0;i<27;++i)w(d+i*4,0);w(d,108);w(d+4,7);w(d+8,r(p+0x1d)-r(p+0x15)+4);w(d+12,r(p+0x19)-r(p+0x11)+8);w(d+104,0x40);
                call(0x58ad90,p+0x7e,{d,0x5e2e58,0x13d6});
                if(!r(p+0xb1)){U allocation=call(0x597850,0,{12});if(allocation){w(allocation+4,0x4d4140);w(allocation+8,p);w(allocation,0x5c6e80);}w(p+0xb1,allocation);w(p+0xa6,allocation);}
            }
            U parent=r(p+0x31),x=r(parent+32),y=r(parent+36),rect=m.temporary(16);
            w(rect,r(p+0x11)-x-4);w(rect+4,r(p+0x15)-y);w(rect+8,r(p+0x19)-x+4);w(rect+12,r(p+0x1d)-y+4);
            if(r(0x6e1ee4))call(0x58d5b0,parent,{2,p+0x7e,rect,0,0,1});else call(0x58c4a0,parent,{p+0x7e,rect,0,0});
            m.set8(p+0xb0,1);break;
        }
        case 0x4d72d0:{
            if(r(p+0x1555)==2)break;
            auto method=[&](U object,U slot){call(r(r(object)+slot),object);};method(p+0x151c,20);
            if(!r(p+0x1910)){
                w(p+0x1575,2);w(p+0x1579,3);w(p+0x157d,U(-1));method(p+0x151c,16);method(p+0x1596,16);
                for(U i=0;i<r(p+0x1518);++i){U row=p+i*0xc6;w(row+0x135,0);if(r(row+0x139)!=2)w(row+0x104,1);method(row+0xfc,16);}
                w(p+0x1910,1);if(r(0x68991c)&&r(0x6c4634)){m.set8(0x6c4659,0);call(0x4dcab0,0x6c4630);w(0x6c4634,0);call(0x4a5000,0x6c4638);}
            }else{
                w(p+0x1575,0);w(p+0x1579,1);w(p+0x157d,U(-1));method(p+0x151c,16);
                for(U i=0;i<r(p+0x1518);++i){U widget=p+0xfc+i*0xc6;w(widget+0x39,3);call(0x4a5000,widget);}w(p+0x1910,0);
            }break;
        }
        default:throw std::invalid_argument("unrecovered concrete actor provider");
        }return 0;
    }
};
}
