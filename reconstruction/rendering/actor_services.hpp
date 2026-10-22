#pragma once
#include <cstdint>
#include <initializer_list>
#include <stdexcept>

namespace mnm::reconstruction::actor_services {
using U=std::uint32_t;
inline std::int32_t signed32(U v) { return v<0x80000000u?std::int32_t(v):-1-std::int32_t(~v); }
inline U divide(U a,U b) {
    const auto left=signed32(a),right=signed32(b);
    if(!right || (left==INT32_MIN && right==-1))throw std::domain_error("Original signed division fault");
    return U(left/right);
}
// Value-address model. M owns all storage and external calls. It must preserve
// callback-visible writes and rereads; addresses are PE32 values, never host
// object pointers. The dispatcher covers only the entries listed below.
template<class M> class Services {
    M& m;
    U r(U p){return m.get32(p);} U w(U p,U v){m.set32(p,v);return v;}
    U call(U id,U self,std::initializer_list<U> args={}){return m.invoke(id,self,args);}
    void resetAnimation(U p){for(U off:{12u,16u,20u,4u,24u,8u,32u})w(p+off,0);}
    void clearMarkers(U p){
        U list=r(p+0x8f9);
        if(!r(list+8))return;
        U head=r(list+4),node=r(head);
        while(node!=head){
            if(r(node+8)==2 && r(node+40)){call(0x4943f0,r(node+40));w(node+40,0);}
            node=r(node);head=r(r(p+0x8f9)+4);
        }
    }
    U lookup(U list,U voice){
        const U head=r(list+4);U node=head;
        do {if(r(node+16)==voice)return node;node=r(node);}while(node!=head);
        return 0;
    }
    void retireSchedule(U list,U node){
        U head=r(list+4),tail=r(head+4);
        if(node==head)w(list+4,r(head));
        else if(node!=tail){
            w(r(node+4),r(node));w(r(node)+4,r(node+4));
            w(node+4,tail);w(node,head);w(tail,node);w(head+4,node);
        }
        if(r(node+20))w(r(node+20),0);
        w(node+8,0);w(node+12,U(-5000));w(node+16,0);w(node+20,0);
        w(node+28,U(-1));w(node+24,U(-1));
    }
    void clearVoice(U voice){const U manager=r(0x6f437c),list=manager+0x25c;retireSchedule(list,lookup(list,voice));}
    U com(U voice,U offset,std::initializer_list<U> rest={}){
        const U buffer=r(voice);return m.com(r(r(buffer)+offset),buffer,rest);
    }
    U retireVoice(U p,U children,U clear){
        U manager=r(0x6f437c);
        if(!r(manager+0x1c)||!r(manager+0x18))return 0;
        if(r(p)){
            w(0x6fa794,0);
            if(com(p,0x24,{0x6fa794}) || (r(0x6fa794)&1)){
                w(0x6fa798,com(p,0x48));
                if(clear)clearVoice(p);
                if(r(0x6fa798))return r(0x6fa798);
                w(0x6fa798,com(p,0x34,{0}));
                if(r(0x6fa798))return r(0x6fa798);
            }
        }
        if(children){
            w(0x6fa78c,r(p+32));
            while(r(0x6fa78c)){
                w(0x6fa798,retireVoice(r(0x6fa78c),0,1));
                if(clear)clearVoice(r(0x6fa78c));
                if(r(0x6fa798))return r(0x6fa798);
                w(0x6fa798,0);w(0x6fa78c,r(r(0x6fa78c)+32));
            }
        }
        return 0;
    }
    U playing(U p,U children){
        if(!r(p))return 0;
        w(0x6fa794,0);
        if(com(p,0x24,{0x6fa794}) || (r(0x6fa794)&1))return 1;
        if(children){
            w(0x6fa790,r(p+32));
            while(r(0x6fa790)){
                if(playing(r(0x6fa790),0))return 1;
                w(0x6fa790,r(r(0x6fa790)+32));
            }
        }
        return 0;
    }
public:
    explicit Services(M& memory):m(memory){}
    U run(U entry,U p,std::initializer_list<U> args={}){
        U a[5]{};unsigned n=0;for(U v:args){if(n==5)throw std::invalid_argument("service argument extent");a[n++]=v;}
        switch(entry){
        case 0x407b90:if(r(p+0x89)==1)w(p+0x89,0);else if(r(p+0x89)==3)w(p+0x89,2);break;
        case 0x411e50:{U type=r(p);return type==0||type==24||type==25||type==26;}
        case 0x464030:
            for(U i=0;signed32(i)<signed32(r(p+0x1c5));++i){U row=p+i*0x97;if(r(row+0x89)==a[0]){m.set8(row+0x92,0);call(0x4a5000,row);}}break;
        case 0x464ad0:
            for(U off:{0u,4u,8u})if(r(p+off))call(0x5979b0,0,{r(p+off)});
            w(p,0);w(p+4,0);w(p+8,0);break;
        case 0x464c80:resetAnimation(p);break;
        case 0x464ca0:w(p+0x1c,a[0]);break;
        case 0x464e20:{
            U descriptor=r(p+0x1c),base=r(descriptor+8),table=r(descriptor+4);
            U old=r(table+r(p)*4)*44;
            U current=divide(r(p+4)-base-old,44);
            U display=divide(r(p+24)-base-old,44);
            w(p,a[0]);U start=base+r(table+r(p)*4)*44;
            w(p+24,start+display*44);w(p+4,start+current*44);break;
        }
        case 0x465490:
            w(p+12,a[0]);if(r(p+4)&&r(p+8)&&a[0]==U(-1))call(0x4667c0,r(0x5fbf28),{U(-1)});break;
        case 0x466870:{
            if(a[0]==U(-1))return 0;
            U owner=p+a[0]*0x154;
            if(!r(owner))return 0;
            U index=r(owner+12);
            return index<r(0x6def5c)?r(0x6def58)+index*0xe4b:0;
        }
        case 0x474b10:{U i=0;do {m.set8(p+0x157+i,m.get8(a[0]+i));}while(m.get8(a[0]+i++));break;}
        case 0x474b40:return call(0x58a680,0x5fcc80,{p});
        case 0x49bcf0:return lookup(p,a[0]);
        case 0x49c530:return playing(p,a[0]);
        case 0x49e170:{
            U head=r(p+0x44c),node=r(head);
            while(node!=head){U next=r(node);if(r(r(node+8)+0x212)==a[0])call(0x490260,r(node+8));node=next;head=r(p+0x44c);}break;
        }
        case 0x4d3ea0:if(r(0x6a4646))call(0x4d3b40,p);break;
        case 0x4de090:return retireVoice(p,a[0],a[1]);
        case 0x4e1260:
            w(p+0x20,a[0]);
            for(U i=0;i<16;++i){static constexpr U ordinary[]={0x1800,0x3c00,0x7e00,0xff00,0xff00,0x7e00,0x3c00,0x1800};m.set16(p+i*2,a[1]?(i<2||i>13?0:0x3ffc):(i<8?ordinary[i]:0));}break;
        case 0x506080:w(0x6e9848,0);w(0x6e984c,0);w(0x6e9850,0);break;
        case 0x5075f0:{
            U definition=r(p+0xac),type=r(definition),base=r(definition+24);
            if(type==0||type==24||type==25||type==26){
                U owner=r(p+0x174);
                if(owner!=U(-1)&&0x643a80+owner*0x154){U wizard=r(0x643a90+owner*0x154);if(wizard!=U(-1)&&0x65b250+wizard*0x93a)return r(0x65b290+wizard*0x93a);}
                if(r(p+0x8c1))return r(p+0x8c1);
            }else if(r(p+0x124)==1||r(p+0x124)==2)return base+(base*r(r(p+0x124)==1?0x5e1910:0x5e1914))/100;
            return base;
        }
        case 0x50eb60:{U value=r(p+0x668)+(a[0]?1:U(-1));if(signed32(value)<(a[0]?1:0))value=a[0]?1:0;w(p+0x668,value);break;}
        case 0x5121c0:if(r(p+0x6e8)){call(0x4943f0,r(p+0x6e8));w(p+0x6e8,0);}clearMarkers(p);break;
        case 0x512340:clearMarkers(p);break;
        case 0x5137d0:
            for(U i=0;i<42;++i){U row=p+0x17c+i*24;if(r(row+20)&&r(row)>96)m.fastcall(0x469000,p,i,{r(row),U(-1)});}
            for(U i=0;i<42;++i){U row=p+0x17c+i*24;if(r(row+20)&&!((r(0x689920)==1||r(0x689920)==2)&&r(row)==19))m.fastcall(0x469000,p,i,{r(row),U(-1)});}break;
        case 0x51a880:{
            U owner=r(p+0x174),record=owner==U(-1)?0:0x643a80+owner*0x154;
            for(U i=0;i<8;++i){w(p+0x24+i*4,i==r(p+0x174)||(m.get8(record+0x18+i)&&m.get8(record+0x20+i))?30:0);w(p+0x48+i*4,0);}break;
        }
        case 0x52ae60:{
            U actor=r(p),best=0,chosen=0;w(p+(a[0]+8)*4,a[1]);
            if(!a[1])for(U count=r(p+0x228);count;--count,actor+=0xe4b)if(r(actor+4)&&r(actor+0xa8)==a[0]&&r(actor+0xe4)&&signed32(best)<signed32(r(actor+0x128))){best=r(actor+0x128);chosen=actor;}
            if(chosen)call(0x514e40,chosen,{0});
            break;
        }
        case 0x52d0c0:
            if(r(p+8)){U head=r(p+4),last=r(r(head+4)),node=r(head);while(node!=last){U next=r(node);w(r(node+4),r(node));w(r(node)+4,r(node+4));call(0x597870,0,{node});w(p+8,r(p+8)-1);node=next;}}break;
        case 0x54dad0:{
            U count=r(p+0x80);if(count>=30)break;
            if((a[0]==1||a[0]==2)&&signed32(count-1)>0)for(U i=count-1;i;--i)if(r(r(0x6f2a94)+r(p+8+i*4)*0x2dc+0x2d0))return 0;
            w(p+8+count*4,a[0]);U id=r(p+8+r(p+0x80)*4);w(p+0x80,r(p+0x80)+1);w(p+4,0);w(p,0);
            U catalog=r(0x6f2a94)+id*0x2dc;w(p+0x104,catalog);U sound=r(catalog+0x2d4);
            if(sound&&r(p+0xfc)<30){w(p+0x84+r(p+0xfc)*4,sound);w(p+0x108,sound);w(p+0xfc,r(p+0xfc)+1);}break;
        }
        case 0x54dba0:{
            if(r(r(p+0x104)+0x2d4)){
                U count=r(p+0xfc);for(U i=1;signed32(i)<signed32(count);++i)if(r(p+0x84+i*4)==r(r(p+0x104)+0x2d4)){call(0x59cb60,0,{p+0x84+i*4,p+0x88+i*4,(count-i)*4});w(p+0xfc,r(p+0xfc)-1);break;}
                w(p+0x108,r(p+0x80+r(p+0xfc)*4));
            }
            U count=r(p+0x80);for(U i=1;signed32(i)<signed32(count);++i)if(r(p+8+i*4)==a[0]){call(0x59cb60,0,{p+8+i*4,p+12+i*4,(count-i)*4});w(p+0x80,r(p+0x80)-1);break;}
            U id=r(p+4+r(p+0x80)*4);w(p+4,0);w(p,0);w(p+0x104,r(0x6f2a94)+id*0x2dc);break;
        }
        case 0x54dcb0:if(r(p+0xfc)<30){w(p+0x84+r(p+0xfc)*4,a[0]);w(p+0x108,a[0]);w(p+0xfc,r(p+0xfc)+1);}break;
        case 0x54ddf0:w(p+0x80,0);w(p+0xfc,0);w(p+0x104,0);w(p+0x108,0);break;
        case 0x556370:break;
        case 0x571d20:retireSchedule(p,a[0]);break;
        case 0x57d0d0:for(U node=a[0];node;){U next=r(node);call(0x5979b0,0,{node});node=next;}break;
        case 0x58f730:w(p,5);w(p+4,40);w(p+8,0);w(p+12,0);w(p+16,0);w(p+20,100);break;
        case 0x58f750:{
            w(p,a[0]);w(p+4,a[1]);if(r(p+28)){call(0x5979b0,0,{r(p+28)});w(p+28,0);w(p+32,0);}
            w(p+36,a[0]);w(p+40,a[1]);U size=a[0]*a[1]*2;w(p+44,1);
            if(r(p+28)){call(0x5979b0,0,{r(p+28)});w(p+28,0);w(p+32,0);}
            U buffer=call(0x597880,0,{size});w(p+28,buffer);if(buffer)w(p+32,size);call(0x5338c0,p+28,{31});break;
        }
        case 0x58f7f0:case 0x58f830:{
            U value=entry==0x58f7f0?r(p+16):a[0],maximum=entry==0x58f7f0?a[0]:r(p+20);
            if(entry==0x58f7f0)w(p+20,maximum);else w(p+16,value);
            if(signed32(maximum)<signed32(value))value=maximum;
            w(p+16,value);w(p+24,value?divide((r(p+4)-2)*value,maximum):0);break;
        }
        case 0x58f890:{
            U filled=r(p+24),step=filled?255/(filled>254?255:filled)+1:0,value=255,width=r(p),out=r(p+28);
            auto zeroRow=[&](){for(U x=0;x<width;++x)m.set16(out+x*2,0);out+=width*2;};
            zeroRow();U rows=r(p+4)-2;while(filled<rows){zeroRow();--rows;}
            if(signed32(rows)>0)do{
                m.set16(out,0);out+=2;
                for(U x=width-2;x;--x){U c=(value&0xffff)>255?255:value;U pixel=r(0x6e1f88)?((0x1f00|(c&0xf8))<<2):((0xff00|(c&0xfc))<<3);m.set16(out,pixel);out+=2;}
                m.set16(out,0);out+=2;value=signed32(value-step)<1?0:value-step;
            }while(--rows);zeroRow();break;
        }
        case 0x597850:return call(0x5977d0,0,{a[0],0,0});
        case 0x597870:case 0x5979b0:if(a[0])call(0x598f70,0,{a[0]});break;
        case 0x59c441:{
            U constructed=0,address=a[0];
            try{for(;signed32(constructed)<signed32(a[2]);++constructed,address+=a[1])call(a[3],address);}
            catch(...){while(constructed){--constructed;address-=a[1];call(a[4],address);}throw;}
            break;
        }
        default:throw std::invalid_argument("Unrecovered actor downstream service");
        }
        return 0;
    }
};
}
