#pragma once
#include "actor_services.hpp"
namespace mnm::reconstruction::actor_services {
// Further engine service bodies. Platform heap operations and widget virtual
// methods are explicit backend calls with callback-visible ordering.
template<class M> class Dependencies {
    M& m;
    U r(U p){return m.get32(p);} U w(U p,U v){m.set32(p,v);return v;}
    U call(U id,U self,std::initializer_list<U> args={}){return m.invoke(id,self,args);}
public:
    explicit Dependencies(M& memory):m(memory){}
    U run(U entry,U p,std::initializer_list<U> args={}){
        U a[12]{};unsigned n=0;for(U v:args){if(n==12)throw std::invalid_argument("dependency argument extent");a[n++]=v;}
        switch(entry){
        case 0x465130:{
            U initial=r(p+24);bool searching=true,seen=false;
            do{U tag=r(r(p+24));if(!tag){if(seen){searching=false;w(p+4,r(p+24));}}else if(tag==4)seen=true;else if(tag==6){w(p+24,initial);searching=false;}w(p+24,r(p+24)+44);}while(searching);break;
        }
        case 0x4667c0:
            w(p+0xaa0,U(-1));
            if(signed32(a[0])>=0&&signed32(a[0])<8){for(U i=0;i<8;++i){U owner=0x643a80+i*0x154;if(r(p+i*0x154)&&r(p+i*0x154+4)){if(i==a[0]){w(p+0xaa0,divide(owner-p,0x154));w(owner+8,1);}else w(owner+8,0);}}return r(p+0xaa0)!=U(-1);}return 0;
        case 0x485b10:
            if(m.get8(p)){U file=call(0x59cae2,0,{p,0x5dc850});if(file){U count=r(p+0x104);w(p+0x104,count+1);call(0x59d50b,0,{file,0x5dc828,count,a[0],a[1],r(p+0x100)});call(0x59c4c3,0,{file});}}break;
        case 0x4a5000:
            if(r(p+4)!=U(-1)){
                U table=r(p),context=0x641ab8+r(0x641fbc)*40;
                call(r(table),p,{0x40,r(context),r(context+4),r(context+8),r(context+12),r(context+16),r(context+20),r(context+24),r(context+28),r(context+32),r(context+36)});
            }
            if(r(0x5e04cc)==r(p+4))w(0x5e04cc,U(-1));
            w(p+8,0);break;
        case 0x4cb9b0:m.set8(p+0x79,0);call(0x4a5000,p);break;
        case 0x4d39b0:w(p+0x25,0);call(0x4a5000,p);break;
        case 0x4d3b20:if(r(p+0x1ef)){w(p+0xd7,a[0]);call(0x4d2e50,p);}break;
        case 0x4d6c60:if(r(p+4)){w(p+4,0);call(0x4d6e90,p);}break;
        case 0x4dea00:if(r(p+8)!=a[0]){w(p+8,a[0]);call(0x4de6c0,p);}break;
        case 0x50e3d0:w(p+0xe0,a[0]);call(0x464cb0,p+0xb0,{r(p+0x60c)+a[0],a[1]});break;
        case 0x513f50:{U source=r(0x6898d0)+0x240;call(0x464ca0,p+0x7d5,{r(0x6894a0)+r(source+4)*0x114});call(0x464cb0,p+0x7d5,{r(p+0x60c)+r(source),0});break;}
        case 0x5338c0:for(U pixels=r(p+4)>>1,at=r(p);pixels;--pixels,at+=2)m.set16(at,a[0]);break;
        case 0x54dce0:{
            U count=r(p+0xfc);for(U i=1;signed32(i)<signed32(count);++i)if(r(p+0x84+i*4)==a[0]){call(0x59cb60,0,{p+0x84+i*4,p+0x88+i*4,(count-i)*4});w(p+0xfc,r(p+0xfc)-1);break;}w(p+0x108,r(p+0x80+r(p+0xfc)*4));break;
        }
        case 0x58a680:
            if(r(p)&&a[0]<r(r(p)+12)-1)return r(r(p+4)+a[0]*4)+r(p+8);
            return p+12;
        case 0x58d5b0:{
            U row=0x6f6b50,index=0;while(r(row)){if(signed32(index)>=128)return 0;row+=44;++index;}if(signed32(index)>=128)break;
            w(row+8,p);w(row+12,a[1]);w(row+4,a[5]);w(row+16,a[0]);
            if(a[2])for(U i=0;i<4;++i)w(row+20+i*4,r(a[2]+i*4));
            w(row+36,a[3]);w(row+40,a[4]);w(row,1);w(0x6f98d4,r(0x6f98d4)+1);break;
        }
        case 0x5977d0:{
            U size=a[0]?a[0]:1,flags=a[1],heap=a[2];if(m.get8(0x5f20e8)&8)flags|=1;
            if(!r(0x5f2094))call(0x597d40,0);
            for(;;){if(!heap)heap=r(0x5f2094);if(heap){U result=call(0x598a10,0,{heap,size,flags});if(result)return result;}
                U retry=call(0x597df0,0,{size,0}),handler=r(0x5f1ec8);if(retry)continue;if(!handler)return 0;call(handler,0);
            }
        }
        case 0x597880:{
            if(!r(0x5f2094)&&!call(0x597d40,0))return 0;
            U flags=(m.get8(0x5f20e8)&8)?1:0,size=a[0]?(a[0]+3)&~3u:4;
            for(;;){U result=call(0x598a10,0,{r(0x5f2094),size,flags});if(result)return result;if(!call(0x597df0,0,{size,1}))return 0;}
        }
        case 0x598f70:{
            U header=a[0]&0xffff0000;if((m.get16(header+14)&0xfff8)!=0xcad0){m.fastcall(0x597b40,a[0],14,{});return 0;}call(r(header+8),a[0]);return 1;
        }
        default:throw std::invalid_argument("Unrecovered actor dependency service");
        }
        return 0;
    }
};
}
