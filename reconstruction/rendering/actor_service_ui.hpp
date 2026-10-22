#pragma once
#include "actor_services.hpp"
#include <algorithm>
namespace mnm::reconstruction::actor_services {
template<class M> class UiServices {
    M& m;
    U r(U p){return m.get32(p);} U w(U p,U v){m.set32(p,v);return v;}
    U call(U id,U self,std::initializer_list<U> args={}){return m.invoke(id,self,args);}
    void rectangle(U target,U surface,U left,U top,U right,U bottom,U x,U y){
        U rect=m.temporary(16);w(rect,left);w(rect+4,top);w(rect+8,right);w(rect+12,bottom);
        if(!r(0x6e1ee4))call(0x58c8a0,target,{surface,rect,x,y});
        else call(0x58d5b0,target,{0,surface,rect,x,y,1});
    }
    void copyString(U dst,U src){U i=0;do{m.set8(dst+i,m.get8(src+i));}while(m.get8(src+i++));}
    static bool disabled(U id,U artifact){return !artifact?(signed32(id)<27||id==52||id==54||id==86):((signed32(id)>27&&signed32(id)<50)||id==58||id==12);}
public:
    explicit UiServices(M& memory):m(memory){}
    U run(U entry,U p,std::initializer_list<U> args={}){
        U a[4]{};unsigned n=0;for(U v:args){if(n==4)throw std::invalid_argument("UI service argument extent");a[n++]=v;}
        switch(entry){
        case 0x414890:{
            U definition=r(p+0xac),description=m.temporary(22);
            w(description,r(definition+12));w(description+4,r(definition+8));w(description+8,r(p+0x104));w(description+12,r(definition+0x44));
            m.set8(description+16,0);w(description+17,r(p+0xa8));m.set8(description+21,m.get8(p+0x723)||r(p+0x5ec)==14);
            return call(0x4f4330,0x6c5490,{a[0],a[1],a[2],description});
        }
        case 0x4637c0:{
            U actor=r(p+0x89);if(!actor)break;U health=r(actor+0xe4),missing=48-divide(health*48,call(0x5075f0,actor));
            if(signed32(health)<signed32(r(p+0x8e))){if(missing)rectangle(p+0x21,p+0x53,49,0,96,missing,0,0);}
            else if(missing!=48)rectangle(p+0x21,p+0x53,0,missing,48,48,0,missing);
            w(p+0x8e,health);if(!health)w(p+0x89,0);break;
        }
        case 0x4d3a30:{
            if(!r(p+0x1ef))break;
            U missing=80-divide(a[0]*80,r(p+0xcb));
            if(signed32(a[0])<signed32(r(p+0xc7))){if(missing)rectangle(p+0x29,0x6a4642,81,0,161,missing,102,30);}
            else if(missing!=80)rectangle(p+0x29,0x6a4642,0,missing,80,80,102,missing+30);
            w(p+0xc7,a[0]);w(0x5e16fc,2);if(r(r(p+0x1ef)+0x68))call(0x4d3c90,p);break;
        }
        case 0x49d640:{
            U secondary=call(0x49cdb0,0x689498);if(!secondary)break;U description=m.temporary(252);
            for(U i=0;i<252;++i)m.set8(description+i,0);
            U actor=a[0],offset=r(r(actor+0xac)+0x198);
            w(description,r(actor+20)+offset);w(description+4,r(actor+24)+offset);w(description+8,r(actor+28));
            w(description+12,r(description));w(description+16,r(description+4));w(description+20,r(description+8));
            w(description+24,r(actor));w(description+28,r(actor));w(description+32,U(-1));w(description+36,U(-1));w(description+40,1);w(description+44,U(-1));w(description+48,1);
            call(0x493f20,secondary,{22,description});break;
        }
        case 0x4d4970:{
            call(0x4d5640,p);if(!r(p+0x18ec))break;U count=r(r(p+0x18ec)+0x56c);
            if(count>=26){if(m.get8(0x6e2032))call(0x5c520c,0,{0,0x5e2e78,0x5e2ea0,0x42030});break;}
            w(p+0x1518,0);w(p+0x1921,count);
            for(U i=0;i<count;++i){
                U row=p+r(p+0x1518)*0xc6,id=r(r(p+0x18ec)+0x570+i*4);w(row+0x1b1,id);w(row+0x1b5,0);w(row+0x1b9,U(-1));
                row=p+r(p+0x1518)*0xc6;w(row+0x155,r(row+0x1b1)+2);w(row+0x159,U(-1));w(row+0x15d,U(-1));
                row=p+r(p+0x1518)*0xc6;w(row+0x131,r(row+0x1b1)+5);U actor=r(p+0x18ec);
                if(r(actor+0x8f5)==r(actor+0x8f1)){row=p+r(p+0x1518)*0xc6;if(disabled(r(row+0x1b1),r(row+0x1b5))){w(row+0x139,2);call(0x4a5000,row+0xfc);}}
                w(p+0x1518,r(p+0x1518)+1);
            }
            for(U i=0,cursor=p+0x15f3;i<std::min(count,r(p+0x17e8));++i,cursor+=0xc6){
                U id=r(cursor-0x1442),widget=cursor-0x5d;w(cursor+0x58,id);w(cursor,U(-1));w(cursor-4,id+2);w(cursor+4,U(-1));w(cursor+8,0x7f);w(cursor-40,id+5);w(cursor+0x64,i);w(cursor-36,0);
                if(r(cursor-32)!=2)w(cursor-0x55,1);
                call(r(r(widget)+16),widget);
                U actor=r(p+0x18ec);if(r(actor+0x8f5)==r(actor+0x8f1)&&disabled(r(cursor+0x58),r(cursor+0x5c))){w(cursor-32,2);call(0x4a5000,widget);}
            }
            for(U i=0;i<r(r(p+0x18ec)+0x178);++i){U row=r(p+0x18ec)+0x17c+i*24;if(r(row+20))call(0x4d6620,p,{row,i});}
            break;
        }
        case 0x4d9ac0:{
            U index=0;while(index<r(p+0x594)&&r(p+8+index*0x8a)!=a[0])++index;if(index>=r(p+0x594))break;
            U row=p+index*0x8a;w(row+8,U(-1));w(row+0x86,0);w(row+0x8e,3);call(r(r(row+12)+20),row+12);w(row+0x45,3);call(0x4a5000,row+12);
            for(U i=0;i<r(p+0x59c);++i)if(r(p+0x56c+i*4)==index){call(0x59cb60,0,{p+0x56c+i*4,p+0x570+i*4,(r(p+0x59c)-i-1)*4});w(p+0x59c,r(p+0x59c)-1);}
            break;
        }
        case 0x4da690:
            if(signed32(r(p+0x161))<10&&!r(0x6e1f0c)){
                w(p+0x139+r(p+0x161)*4,a[0]);w(p+0x161,r(p+0x161)+1);
                if(signed32(r(p+0x165))<1){U text=call(0x58a680,p+0x2d,{r(p+0x139)});call(0x4d9f30,p,{text});call(0x59cb60,0,{p+0x139,p+0x13d,r(p+0x161)*4-4});w(p+0x161,r(p+0x161)-1);}
            }break;
        case 0x4da720:{U text=call(0x58a680,p+0x2d,{a[1]}),buffer=m.temporary(256);call(0x59c1fd,0,{buffer,0x5e0fc8,a[0],text});call(0x4d9f30,p,{buffer});break;}
        case 0x5055a0:{
            U type=a[0],name=a[1],descriptor=0x6dbf00+type*0x114,resource=0x6de0e8+type*36,result=1;
            if(!type||type==24||type==25||type==26){
                call(0x464ad0,descriptor);
                if(r(resource)){call(0x5979b0,0,{r(resource)});for(U off:{0u,4u,8u,12u})w(resource+off,0);}
                for(U i=0;i<4;++i)if(r(resource+16+i*4)){call(0x57d0d0,0,{r(resource+16+i*4)});w(resource+16+i*4,0);}
                if(r(resource+32)){call(0x5979b0,0,{r(resource+32)});w(resource+32,0);}
            }
            if(!r(descriptor)){result=call(0x4644d0,descriptor,{name,0});if(!result)return 0;if(r(0x6e9808))result=call(0x5050d0,type);if(!result)return 0;}
            if(!r(resource)){
                U path=m.temporary(256),part=m.temporary(4);
                if(!call(0x5c50c0,0,{name,256,path,part}))return 0;
                m.set8(r(part),0);U end=path;while(m.get8(end))++end;copyString(end,r(descriptor)+24);
                result=call(0x57d310,resource,{path,16});if(!result)return 0;
                for(U bank:{1u,2u,6u,4u,5u,7u})call(0x57d690,resource,{bank,16});
                call(0x505220,p,{type});
            }
            return result?1:0;
        }
        case 0x56c430:{
            U count=r(0x6f37fc),record=r(0x6f37f8),index=0;
            if(signed32(count)<=0)break;
            while(index<count&&(!r(record)||r(record+40)!=a[0]||a[0]==U(-1))){++index;record+=64;}
            if(index>=count||!record)break;
            U actor=a[0]<r(0x6def5c)?r(0x6def58)+a[0]*0xe4b:0;
            w(record+44,r(actor+8));w(record+48,r(actor+12));w(record+52,r(actor+16));w(record+40,U(-1));
            if(r(actor+0xa8)==27){w(record+36,3);break;}
            if(r(record+36)==1){w(record+36,2);if(r(record+60)){
                U buffer=m.temporary(256);m.set16(buffer,m.get16(0x5fc874));for(U i=2;i<256;++i)m.set8(buffer+i,0);m.set8(0x6dbc27,1);
                U text=call(0x474b40,1);call(0x59c1fd,0,{buffer,text,record+4});call(0x474b10,0x6db967,{buffer});
            }}break;
        }
        default:throw std::invalid_argument("Unrecovered actor UI/resource service");
        }
        return 0;
    }
};
}
