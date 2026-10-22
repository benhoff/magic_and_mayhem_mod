#pragma once
#include "actor_services.hpp"
namespace mnm::reconstruction::actor_services {
template<class M> class CollectionServices {
    M& m;
    U r(U p){return m.get32(p);} U w(U p,U v){m.set32(p,v);return v;}
    U call(U id,U self,std::initializer_list<U> args={}){return m.invoke(id,self,args);}
    static U words(U bytes){return (bytes>>2)|((bytes&0x80000000u)?0xc0000000u:0);}
    void constructCopy(U dst,U src){if(dst)w(dst,r(src));}
    void insert(U p,U position,U count,U value){
        U end=r(p+8);
        if(count<=words(r(p+12)-end)){
            U tail=words(end-position);
            if(tail<count){
                for(U src=position,dst=position+count*4;src!=end;src+=4,dst+=4)constructCopy(dst,src);
                U dst=r(p+8);for(U left=count-words(dst-position);left;--left,dst+=4)constructCopy(dst,value);
                for(U at=position,stop=r(p+8);at!=stop;at+=4)w(at,r(value));
                w(p+8,r(p+8)+count*4);
            }else if(count){
                for(U src=end-count*4,dst=end;src!=end;src+=4,dst+=4)constructCopy(dst,src);
                U dst=r(p+8),stop=dst-count*4;while(position!=stop){dst-=4;stop-=4;w(dst,r(stop));}
                for(U stop=position+count*4;position!=stop;position+=4)w(position,r(value));
                w(p+8,r(p+8)+count*4);
            }
            return;
        }
        U begin=r(p+4),size=begin?words(end-begin):0,growth=!begin||size<=count?count:size,total=size+growth;
        U allocation=call(0x597850,0,{(signed32(total)<0?0:total)*4}),dst=allocation;
        for(U src=r(p+4);src!=position;src+=4,dst+=4)constructCopy(dst,src);
        U afterPrefix=dst;for(U left=count;left;--left,dst+=4)constructCopy(dst,value);
        U stop=r(p+8);for(U src=position,out=afterPrefix+count*4;src!=stop;src+=4,out+=4)constructCopy(out,src);
        call(0x597870,0,{r(p+4)});w(p+12,allocation+total*4);
        U previous=r(p+4);if(!previous){w(p+4,allocation);w(p+8,allocation+count*4);}
        else{U oldSize=words(r(p+8)-previous);w(p+4,allocation);w(p+8,allocation+(oldSize+count)*4);}
    }
    void refreshActive(U manager){
        U record=r(manager);w(manager+32,0);
        for(U i=0;signed32(i)<signed32(r(manager+8));++i,record+=0x47c)if(r(record)&&r(record+8)!=2&&(r(record+0x27c)||(r(record+4)==2&&r(record+0x470)))){w(manager+32,1);break;}
    }
public:
    explicit CollectionServices(M& memory):m(memory){}
    U run(U entry,U p,std::initializer_list<U> args={},U second=0){
        U a[12]{};unsigned n=0;for(U v:args){if(n==12)throw std::invalid_argument("collection argument extent");a[n++]=v;}
        switch(entry){
        case 0x42a340:insert(p,a[0],a[1],a[2]);break;
        case 0x49cdb0:{
            U capacity=r(p+4),first=r(p)+125*0x22e;
            for(U index=125,at=first;index<capacity;++index,at+=0x22e)if(!r(at+4)&&!r(at+0x20e))return at;
            for(U index=125,at=first;index<r(p+4);++index,at+=0x22e)if(r(at+4)&&!r(at+0x20e)){
                U type=r(at+40);if(type==85||type==14||type==26){if(type==14)w(at+0x180,1);else if(type==26)call(0x494ab0,at,{27});else call(0x4943f0,at);}
            }
            return 0;
        }
        case 0x514e40:{
            U type=r(r(p+0xac)),health=r(p+0xe4);if(!type||type==24||type==25||type==26||!health)break;
            U experience=r(p+0x128)+a[0];w(p+0x128,experience);U mode=r(p+0x124),bonus=0,animation=0;
            if(!mode&&signed32(r(0x6a650d+r(p+0xa8)*0x5c9))<signed32(experience)){w(p+0x124,1);bonus=r(0x5e1910);}
            else{
                if(mode!=1||signed32(experience)<=signed32(r(0x6a6511+r(p+0xa8)*0x5c9))||r(0x6def78+r(p+0xa8)*4))break;
                w(p+0x124,2);w(p+0x15c,0);bonus=r(0x5e1914);animation=1;
            }
            U upgraded=health+(health*bonus)/100;w(p+0xe4,upgraded);U maximum=call(0x5075f0,p);if(maximum<upgraded)upgraded=call(0x5075f0,p);w(p+0xe4,upgraded);
            maximum=call(0x5075f0,p);call(0x58f7f0,p+0x6b4,{maximum});call(0x58f830,p+0x6b4,{r(p+0xe4)});call(0x58f890,p+0x6b4);
            U definition=r(p+0xac),power=r(definition+40);w(p+0x84,power+(power*bonus)/100);power=r(definition+20);w(p+0x88,power+(power*bonus)/100);
            if(animation)call(0x52ae60,0x6def58,{r(p+0xa8),1});
            call(0x464cb0,p+0x12c,{animation,0});w(p+0x92e,1);break;
        }
        case 0x564e90:{
            U event=a[0];if(event!=1&&event!=2&&event!=3&&event!=6&&event!=7&&event!=9)break;
            U record=r(p);
            for(U i=0;signed32(i)<signed32(r(p+8));++i,record+=0x47c){
                if(!r(record)||!call(0x565180,p,{record,event,a[1]}))continue;
                if(event==6||event==7){
                    if(r(record+0x138)!=a[2])continue;
                    bool empty=true;U x=record+0x144,y=0x5fc874;
                    for(;;++x,++y){U left=m.get8(x),right=m.get8(y);if(left!=right){empty=false;break;}if(!left)break;}
                    if(!empty){U owner=call(0x466900,0x643a80,{record+0x144});if(r(a[1]+0x174)!=divide(owner-0x643a80,0x154))continue;}
                    w(record+0x24,r(record+0x24)+1);
                }
                bool general=event==1||event==2||event==3;
                if(call(0x5654e0,p,{record,event,a[1],a[2],general?0:a[3],general?U(-1):a[4],general?U(-1):a[5]})){
                    call(0x566d80,p,{record,a[1]});if(!general)refreshActive(p);
                }
            }break;
        }
        default:throw std::invalid_argument("Unrecovered actor collection/event service");
        }
        (void)second;return 0;
    }
};
}
