#pragma once
#include "actor_services.hpp"
#include <algorithm>
namespace mnm::reconstruction::actor_services {
template<class M> class PopupServices {
    M& m;
    U r(U p){return m.get32(p);} U w(U p,U v){m.set32(p,v);return v;}
    U call(U id,U self,std::initializer_list<U> args={}){return m.invoke(id,self,args);}
    void method(U object,U slot){call(r(r(object)+slot),object);}
    static bool blocked(U id,U artifact){return !artifact?(signed32(id)<27||id==52||id==54||id==86):((signed32(id)>27&&signed32(id)<50)||id==58||id==12);}
    void rect(U p,U left,U top,U right,U bottom,U y){
        U out=m.temporary(16);w(out,left);w(out+4,top);w(out+8,right);w(out+12,bottom);
        if(!r(0x6e1ee4))call(0x58c8a0,p+0x29,{0x6a4642,out,102,y});
        else call(0x58d5b0,p+0x29,{0,0x6a4642,out,102,y,1});
    }
    void draw(U sprite,U x,U y){
        if(r(sprite+28)==U(-1))call(r(0x6903dc),0,{sprite,x,y});
        else if((call(r(0x6c4e3c),0,{sprite,x,y,0,0})&0xffff)==1)call(0x57de00,0,{sprite,x,y,0,0});
    }
public:
    explicit PopupServices(M& memory):m(memory){}
    U run(U entry,U p,std::initializer_list<U> args={}){
        U a[8]{};unsigned n=0;for(U v:args){if(n==8)throw std::invalid_argument("popup argument extent");a[n++]=v;}
        switch(entry){
        case 0x4d3b40:{
            if(!r(p+0x1ef))break;
            U missing=80-divide(r(p+0xc7)*80,r(p+0xcb));
            if(missing)rect(p,81,0,161,missing,30);
            if(missing!=80)rect(p,0,missing,80,80,missing+30);
            w(p+0xdb,0);w(p+0xdf,128);call(0x4d2e50,p);if(r(r(p+0x1ef)+0x68))call(0x4d3c90,p);break;
        }
        case 0x4d3c90:{
            if(!r(0x6a4646))break;
            U pixels=call(0x58b660,0x6a4642),surface=r(0x6a4646);m.com(r(r(surface)+0x80),surface,{pixels});if(!pixels)break;
            U left=r(p+0x11)-r(0x6a4662),top=r(p+0x15)-r(0x6a4666),right=r(p+0x19)-r(0x6a4662)+1,bottom=r(p+0x1d)-r(0x6a4666)+1;
            m.fastcall(0x57dc20,pixels,r(0x6a465a)>>1,{r(0x6a4656)});m.fastcall(0x57dc60,top,left,{right,bottom});
            for(U i=0;i<4;++i){U count=r(r(0x6593e0)+12),index=i==0?0:i==1?(count!=0):count<i?0:i;U sprite=r(r(0x6593e8)+index*4)+r(0x6593ec);draw(sprite,left+(i&1?80:36),top+(i<2?40:98));}break;
        }
        case 0x4d5640:{
            w(p+0x1910,0);call(0x4d72d0,p);
            for(U i=0,cursor=p+0x15f3;i<3;++i,cursor+=0xc6){w(cursor+0x58,U(-1));for(U off:{U(-4),0u,4u,8u})w(cursor+off,U(-1));U widget=cursor-0x5d;method(widget,0x24);w(cursor-36,3);call(0x4a5000,widget);method(widget,0x14);}
            for(U i=0,cursor=p+0x159;i<26;++i,cursor+=0xc6){w(cursor+0x64,U(-1));m.set8(cursor+0x68,0);w(cursor+0x1d,0);for(U off:{U(-4),0u,4u,8u})w(cursor+off,U(-1));U widget=cursor-0x5d;method(widget,0x24);w(cursor-36,3);call(0x4a5000,widget);method(widget,0x14);}
            for(U off:{0x1908u,0x190cu})w(p+off,U(-1));
            for(U off:{0x1518u,0x1921u,0x183cu}){w(p+off,0);}
            break;
        }
        case 0x4d6620:{
            U descriptor=a[0];if(r(descriptor)>96)break;if(!r(descriptor+16)&&signed32(r(descriptor+4))>0)w(descriptor+16,1);
            U count=r(p+0x1518);
            for(U i=0;i<count;++i){U row=p+i*0xc6;if(r(row+0x1b5)==1&&r(row+0x1b1)==r(descriptor)){U value=(m.get8(row+0x1c1)+1)&255;m.set8(row+0x1c1,value);w(row+0x176,value<128?value:value-256);method(row+0xfc,0x10);return 0;}}
            if(count>25){
                if(!r(p+0x183c)){if(m.get8(0x6e2032))call(0x5c520c,0,{0,0x5e2f20,0x5e2ea0,0x42030});break;}
                U row=p+r(p+0x17ec)*0xc6,widget=row+0xfc,old=r(row+0x139);w(widget+0x39,0);if(old!=2)w(widget+8,1);method(widget,0x10);
                row=p+r(p+0x17ec)*0xc6;w(row+0x139,0);w(row+0x104,1);call(0x4d53d0,p,{r(p+0x17ec),r(row+0x1b1)});w(p+0x183c,r(p+0x183c)-1);call(0x59cb60,0,{p+0x17ec,p+0x17f4,r(p+0x183c)*8});
            }
            auto row=[&](){return p+r(p+0x1518)*0xc6;};
            w(row()+0x1b1,r(descriptor));w(row()+0x1b5,1);w(row()+0x1bd,a[1]);m.set8(row()+0x1c1,1);w(row()+0x176,1);w(row()+0x139,0);w(row()+0x104,1);
            w(row()+0x155,r(descriptor)+134);w(row()+0x159,U(-1));w(row()+0x15d,U(-1));w(row()+0x131,r(descriptor)+98);
            U definition=r(descriptor)<r(0x6b1254)?r(0x6b1250)+r(descriptor)*32:0,level=r(descriptor+16)==r(definition+12)?5:divide(r(descriptor+16)*5,r(definition+12));if(level>4)level=5;
            w(row()+0x161,level+128);w(row()+0x1b9,level);U widget=row()+0xfc,old=r(row()+0x139);w(widget+0x39,0);if(old!=2)w(widget+8,1);method(widget,0x10);
            if(!r(p+0x1910)){w(row()+0x135,3);call(0x4a5000,row()+0xfc);}U actor=r(p+0x18ec);
            if(r(actor+0x8f5)==r(actor+0x8f1)&&blocked(r(row()+0x1b1),r(row()+0x1b5))){w(row()+0x139,2);call(0x4a5000,row()+0xfc);}method(row()+0xfc,0x10);w(p+0x1518,r(p+0x1518)+1);break;
        }
        case 0x4d6c80:{
            U selected=r(p+0x1908);if(selected!=U(-1)){method(p+0xfc+selected*0xc6,0x24);U margin=r(0x6de6d5)?11:8;selected=r(p+0x1908);if(signed32(margin)<signed32(selected)){U row=p+(selected-margin)*0xc6;method(row+0xfc,0x10);method(row+0x36,0x10);}w(p+0x1908,U(-1));}
            selected=r(p+0x190c);if(selected!=U(-1)){method(p+0x1596+selected*0xc6,0x24);selected=r(p+0x190c);if(!selected){if(r(p+0x1921)>1)method(p+0x165c,0x10);}else if(selected==1){method(p+0x1596,0x10);if(r(p+0x1921)>2)method(p+0x1722,0x10);}else if(selected==2)method(p+0x165c,0x10);w(p+0x190c,U(-1));}break;
        }
        case 0x4d78b0:{
            U selected=r(p+0x1908);if(selected!=U(-1)){U row=p+selected*0xc6;if(blocked(r(row+0x1b1),r(row+0x1b5)))call(0x509bd0,r(p+0x18ec),{U(-1)});}
            for(U i=0;i<r(p+0x1518);++i){U row=p+i*0xc6;if(blocked(r(row+0x1b1),r(row+0x1b5))){w(row+0x139,2);call(0x4a5000,row+0xfc);method(row+0xfc,0x10);}}
            for(U i=0;i<std::min(r(p+0x17e8),r(p+0x1921));++i){U widget=p+0x1596+i*0xc6;if(blocked(r(widget+0xb5),r(widget+0xb9))){w(widget+0x3d,2);call(0x4a5000,widget);method(widget,0x10);}}break;
        }
        case 0x4d7a00:
            for(U i=0;i<r(p+0x1518);++i){U widget=p+0xfc+i*0xc6;w(widget+0x3d,0);if(r(p+0x1910))w(widget+8,1);method(widget,0x10);}
            for(U i=0;i<std::min(r(p+0x17e8),r(p+0x1921));++i){U widget=p+0x1596+i*0xc6;w(widget+0x3d,0);w(widget+8,1);method(widget,0x10);}break;
        case 0x4d7a90:{
            if(!r(p+0x18ec))break;
            if(!r(0x6de6d9)){call(0x58bc10,p+0x1840,{31});call(0x58bc10,p+0x1872,{31});if(r(r(p+0x18ec)+0xe4)){method(p+0x151c,0x1c);method(p+0x151c,0x10);}}
            else{
                call(0x58bc10,p+0x1840,{0});call(0x58bc10,p+0x1872,{0});method(p+0x151c,0x1c);if(!r(p+0x1910))call(0x4d72d0,p);
                U buffer=m.temporary(256);
                for(U i=0;i<2;++i){U width=r(0x6de6d5)?800:640,height=r(0x6de6d5)?600:480;call(0x59c1fd,0,{buffer,i?0x5e2f38u:0x5e2f54u,width,height});if(!call(0x58d1a0,p+(i?0x1872:0x1840),{buffer}))call(0x58bc10,p+(i?0x1872:0x1840),{0});}
                method(p+0x151c,0x18);method(p+0x151c,0x10);
            }
            if(r(r(p+0x18ec)+0xe4)){
                for(U i=0;i<26;++i)call(0x4d47f0,p+0xfc+i*0xc6);
                for(U i=0;i<26;++i)method(p+0xfc+i*0xc6,0x10);
                for(U i=0;i<std::min(r(p+0x17e8),r(p+0x1921));++i)call(0x4d47f0,p+0x1596+i*0xc6);
                for(U i=0;i<std::min(r(p+0x17e8),r(p+0x1921));++i)method(p+0x1596+i*0xc6,0x10);
                call(0x4d3b40,0x6c4e40);
            }break;
        }
        case 0x4d9f30:{
            if(!r(p+0x16d))break;
            call(0x58d1a0,p+0x169,{0x5e3030});U record=m.temporary(20);m.set8(record,0);w(record+4,24);w(record+8,0);w(record+12,r(p+0x179)-24);w(record+16,r(p+0x17d));
            U pixels=call(0x58b660,p+0x169);if(!pixels)break;
            m.fastcall(0x57dc20,pixels,r(p+0x181)>>1,{r(p+0x17d)});m.fastcall(0x57dc60,r(record+8),r(record+4),{r(record+12),r(record+16)});
            for(U flag:{8u,4u,16u})call(0x4a5a90,0x656680,{record,flag});
            call(0x4a5ce0,0x656680,{a[0],record+4,0,0,0,m.get8(record),0});U surface=r(p+0x16d);m.com(r(r(surface)+0x80),surface,{pixels});
            w(p+0x165,50);w(p+0x1b3,24);w(p+16,1);call(0x56f000,0x6b0198,{0x328,0,0,0,0,U(-1),U(-1)});break;
        }
        default:throw std::invalid_argument("Unrecovered actor popup service");
        }
        return 0;
    }
};
}
